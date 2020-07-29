/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlist/browser/playlist_service.h"

#include <algorithm>
#include <utility>

#include "base/bind.h"
#include "base/containers/flat_set.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/post_task.h"
#include "base/task_runner_util.h"
#include "base/token.h"
#include "brave/components/playlist/browser/playlist_constants.h"
#include "brave/components/playlist/browser/playlist_data_source.h"
#include "brave/components/playlist/browser/playlist_service_observer.h"
#include "brave/components/playlist/browser/playlist_player.h"
#include "brave/components/playlist/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace playlist {
namespace {

const base::FilePath::StringType kBaseDirName(FILE_PATH_LITERAL("playlist"));

constexpr unsigned int kRetriesCountOnNetworkChange = 1;

const base::FilePath::StringType kThumbnailFileName(
    FILE_PATH_LITERAL("thumbnail"));

PlaylistInfo CreatePlaylistInfo(const CreatePlaylistParams& params) {
  PlaylistInfo p;
  p.id = base::Token::CreateRandom().ToString();
  p.playlist_name = params.playlist_name;
  p.create_params = params;
  return p;
}

base::Value GetValueFromMediaFile(const MediaFileInfo& info) {
  base::Value media_file(base::Value::Type::DICTIONARY);
  media_file.SetStringKey(kPlaylistMediaFileUrlKey, info.media_file_url);
  media_file.SetStringKey(kPlaylistMediaFileTitleKey, info.media_file_title);
  return media_file;
}

base::Value GetValueFromMediaFiles(
    const std::vector<MediaFileInfo>& media_files) {
  base::Value media_files_value(base::Value::Type::LIST);
  for (const MediaFileInfo& info : media_files)
    media_files_value.Append(GetValueFromMediaFile(info));
  return media_files_value;
}

base::Value GetValueFromCreateParams(const CreatePlaylistParams& params) {
  base::Value create_params_value(base::Value::Type::DICTIONARY);
  create_params_value.SetStringKey(kPlaylistPlaylistThumbnailUrlKey,
                                   params.playlist_thumbnail_url);
  create_params_value.SetStringKey(kPlaylistPlaylistNameKey,
                                   params.playlist_name);
  create_params_value.SetKey(kPlaylistVideoMediaFilesKey,
                             GetValueFromMediaFiles(params.video_media_files));
  create_params_value.SetKey(kPlaylistAudioMediaFilesKey,
                             GetValueFromMediaFiles(params.audio_media_files));
  return create_params_value;
}

base::Value GetTitleValueFromCreateParams(const CreatePlaylistParams& params) {
  base::Value titles_value(base::Value::Type::LIST);
  for (const MediaFileInfo& info : params.video_media_files)
    titles_value.Append(base::Value(info.media_file_title));
  return titles_value;
}

base::Value GetValueFromPlaylistInfo(const PlaylistInfo& info) {
  base::Value playlist_value(base::Value::Type::DICTIONARY);
  playlist_value.SetStringKey(kPlaylistIDKey, info.id);
  playlist_value.SetStringKey(kPlaylistPlaylistNameKey, info.playlist_name);
  playlist_value.SetStringKey(kPlaylistThumbnailPathKey, info.thumbnail_path);
  playlist_value.SetStringKey(kPlaylistVideoMediaFilePathKey,
                              info.video_media_file_path);
  playlist_value.SetStringKey(kPlaylistAudioMediaFilePathKey,
                              info.audio_media_file_path);
  playlist_value.SetBoolKey(kPlaylistPartialReadyKey, info.partial_ready);
  playlist_value.SetKey(kPlaylistTitlesKey,
                        GetTitleValueFromCreateParams(info.create_params));
  playlist_value.SetKey(kPlaylistCreateParamsKey,
                        GetValueFromCreateParams(info.create_params));
  return playlist_value;
}

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTagForURLLoad() {
  return net::DefineNetworkTrafficAnnotation("playlist_controller", R"(
      semantics {
        sender: "Brave playlist controller"
        description:
          "Fetching thumbnail image for newly created playlist item"
        trigger:
          "User-initiated for creating new playlist item"
        data:
          "Thumbnail for playlist item"
        destination: WEBSITE
      }
      policy {
        cookies_allowed: NO
      })");
}

std::vector<base::FilePath> GetOrphanedPaths(
    const base::FilePath& base_dir,
    const base::flat_set<std::string>& ids) {
  std::vector<base::FilePath> orphaned_paths;
  base::FileEnumerator dirs(base_dir, false, base::FileEnumerator::DIRECTORIES);
  for (base::FilePath name = dirs.Next(); !name.empty(); name = dirs.Next()) {
    std::string base_name =
#if defined(OS_WIN)
        base::UTF16ToUTF8(name.BaseName().value());
#else
        name.BaseName().value();
#endif

    if (ids.find(base_name) == ids.end())
      orphaned_paths.push_back(name);
  }
  return orphaned_paths;
}

}  // namespace

PlaylistService::PlaylistService(
    content::BrowserContext* context,
    scoped_refptr<base::SequencedTaskRunner> task_runner)
    : context_(context),
      base_dir_(context->GetPath().Append(kBaseDirName)),
      io_task_runner_(task_runner),
      url_loader_factory_(
          content::BrowserContext::GetDefaultStoragePartition(context)
              ->GetURLLoaderFactoryForBrowserProcess()),
      prefs_(user_prefs::UserPrefs::Get(context)),
      weak_factory_(this) {
  content::URLDataSource::Add(
      context,
      std::make_unique<PlaylistDataSource>(this));

  // TODO(pilgrim) dynamically set file extensions based on format
  // (may require changes to youtubedown parser)
  video_media_file_controller_.reset(new PlaylistMediaFileController(
      this, context_, FILE_PATH_LITERAL("video_source_files"),
      FILE_PATH_LITERAL("video_file.mp4"), kPlaylistVideoMediaFilePathKey,
      kPlaylistCreateParamsVideoMediaFilesPathKey));
  audio_media_file_controller_.reset(new PlaylistMediaFileController(
      this, context_, FILE_PATH_LITERAL("audio_source_files"),
      FILE_PATH_LITERAL("audio_file.m4a"), kPlaylistAudioMediaFilePathKey,
      kPlaylistCreateParamsAudioMediaFilesPathKey));

  CleanUp();
}

PlaylistService::~PlaylistService() = default;

void PlaylistService::SetPlayer(std::unique_ptr<PlaylistPlayer> player) {
    player_ = std::move(player);
}

void PlaylistService::NotifyPlaylistChanged(
    const PlaylistChangeParams& params) {
  VLOG(2) << __func__ << ": params="
          << PlaylistChangeParams::GetPlaylistChangeTypeAsString(
                 params.change_type);

  for (PlaylistServiceObserver& obs : observers_)
    obs.OnPlaylistItemStatusChanged(params);
}

void PlaylistService::AddPlaylistToMediaFileGenerationQueue(
    const std::string& id) {
  const base::Value* playlist_info =
      prefs_->Get(kPlaylistItems)->FindDictKey(id);
  if (!playlist_info) {
    LOG(ERROR) << __func__ << ": Invalid playlist id for recover: " << id;
    return;
  }

  VLOG(2) << __func__;
  // Add to pending jobs
  pending_media_file_creation_jobs_.push(playlist_info->Clone());

  // If either media file controller is generating a playlist media file,
  // delay the next playlist generation. It will be triggered when the current
  // one is finished.
  if (!video_media_file_controller_->in_progress() &&
      !audio_media_file_controller_->in_progress()) {
    GenerateMediaFiles();
  }
}

void PlaylistService::GenerateMediaFiles() {
  DCHECK(!video_media_file_controller_->in_progress() &&
         !audio_media_file_controller_->in_progress());
  DCHECK(!pending_media_file_creation_jobs_.empty());

  base::Value video_value(std::move(pending_media_file_creation_jobs_.front()));
  base::Value audio_value = video_value.Clone();
  pending_media_file_creation_jobs_.pop();
  VLOG(2) << __func__ << ": "
          << *video_value.FindStringKey(kPlaylistPlaylistNameKey);

  video_media_file_controller_->GenerateSingleMediaFile(std::move(video_value),
                                                        base_dir_);
  audio_media_file_controller_->GenerateSingleMediaFile(std::move(audio_value),
                                                        base_dir_);
}

base::FilePath PlaylistService::GetPlaylistItemDirPath(
    const std::string& id) const {
  return base_dir_.Append(GetPlaylistIDDirName(id));
}

void PlaylistService::UpdatePlaylistValue(const std::string& id,
                                          base::Value&& value) {
  base::Value playlist_items = prefs_->Get(kPlaylistItems)->Clone();
  playlist_items.SetKey(id, std::move(value));
  prefs_->Set(kPlaylistItems, playlist_items);
}

void PlaylistService::RemovePlaylist(const std::string& id) {
  base::Value playlist_items = prefs_->Get(kPlaylistItems)->Clone();
  playlist_items.RemoveKey(id);
  prefs_->Set(kPlaylistItems, playlist_items);
}

// TODO(simonhong): Add basic validation for |params|.
void PlaylistService::CreatePlaylist(const CreatePlaylistParams& params) {
  VLOG(2) << __func__;
  const PlaylistInfo info = CreatePlaylistInfo(params);
  UpdatePlaylistValue(info.id, GetValueFromPlaylistInfo(info));

  NotifyPlaylistChanged(
      {PlaylistChangeParams::ChangeType::CHANGE_TYPE_ADDED, info.id});

  base::PostTaskAndReplyWithResult(
      io_task_runner(),
      FROM_HERE,
      base::BindOnce(&base::CreateDirectory, GetPlaylistItemDirPath(info.id)),
      base::BindOnce(&PlaylistService::OnPlaylistItemDirCreated,
                     weak_factory_.GetWeakPtr(),
                     info.id));
}

void PlaylistService::OnPlaylistItemDirCreated(const std::string& id,
                                               bool directory_ready) {
  VLOG(2) << __func__;
  if (!directory_ready) {
    NotifyPlaylistChanged(
        {PlaylistChangeParams::ChangeType::CHANGE_TYPE_ABORTED, id});
    return;
  }

  DownloadThumbnail(id);
  AddPlaylistToMediaFileGenerationQueue(id);
}

void PlaylistService::DownloadThumbnail(const std::string& id) {
  const base::Value* item_value =
      prefs_->Get(kPlaylistItems)->FindDictKey(id);
  DCHECK(item_value);
  const base::Value* create_params_value =
      item_value->FindDictKey(kPlaylistCreateParamsKey);
  DCHECK(create_params_value);
  const std::string* thumbnail_url =
      create_params_value->FindStringKey(kPlaylistPlaylistThumbnailUrlKey);
  if (!thumbnail_url || thumbnail_url->empty()) {
    VLOG(2) << __func__ << ": thumbnail url is not available.";
    return;
  }

  auto request = std::make_unique<network::ResourceRequest>();
  request->url = GURL(*thumbnail_url);
  request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  auto loader = network::SimpleURLLoader::Create(
      std::move(request),
      GetNetworkTrafficAnnotationTagForURLLoad());
  loader->SetRetryOptions(
      kRetriesCountOnNetworkChange,
      network::SimpleURLLoader::RetryMode::RETRY_ON_NETWORK_CHANGE);

  auto iter = url_loaders_.insert(url_loaders_.begin(), std::move(loader));
  const base::FilePath thumbnail_path =
      GetPlaylistItemDirPath(id).Append(kThumbnailFileName);
  iter->get()->DownloadToFile(
      url_loader_factory_.get(),
      base::BindOnce(&PlaylistService::OnThumbnailDownloaded,
                     base::Unretained(this),
                     id,
                     std::move(iter)),
      thumbnail_path);
}

void PlaylistService::OnThumbnailDownloaded(const std::string& id,
                                            SimpleURLLoaderList::iterator it,
                                            base::FilePath path) {
  // When delete all is requested during the thumbnail downloading, we should
  // just return. |url_loaders_| is cleared.
  if (url_loaders_.empty())
    return;

  url_loaders_.erase(it);

  if (path.empty()) {
    VLOG(2) << __func__ << ": thumbnail fetching failed for " << id;
    NotifyPlaylistChanged(
        {PlaylistChangeParams::ChangeType::CHANGE_TYPE_THUMBNAIL_FAILED, id});
    return;
  }

  const std::string thumbnail_path =
#if defined(OS_WIN)
      base::UTF16ToUTF8(path.value());
#else
      path.value();
#endif

  const base::Value* value = prefs_->Get(kPlaylistItems)->FindDictKey(id);
  DCHECK(value);
  if (value) {
    base::Value copied_value = value->Clone();
    copied_value.SetStringKey(kPlaylistThumbnailPathKey, thumbnail_path);
    UpdatePlaylistValue(id, std::move(copied_value));
    NotifyPlaylistChanged(
        {PlaylistChangeParams::ChangeType::CHANGE_TYPE_THUMBNAIL_READY, id});
  }
}

base::Value PlaylistService::GetAllPlaylist() {
  base::Value playlist(base::Value::Type::LIST);
  for (const auto& it: prefs_->Get(kPlaylistItems)->DictItems()) {
    base::Value item = it.second.Clone();
    item.RemoveKey(kPlaylistCreateParamsKey);
    playlist.Append(std::move(item));
  }

  return playlist;
}

base::Value PlaylistService::GetPlaylist(const std::string& id) {
  const base::Value* item_value_ptr =
      prefs_->Get(kPlaylistItems)->FindDictKey(id);
  if (item_value_ptr) {
    base::Value item = item_value_ptr->Clone();
    item.RemoveKey(kPlaylistCreateParamsKey);
    return item;
  }
  return {};
}

void PlaylistService::RecoverPlaylist(const std::string& id) {
  const base::Value* playlist_info =
      prefs_->Get(kPlaylistItems)->FindDictKey(id);
  if (!playlist_info) {
    LOG(ERROR) << __func__ << ": Invalid playlist id for recover: " << id;
    return;
  }

  VLOG(2) << __func__ << ": This is in recovering";

  const std::string* thumbnail_path_str =
      playlist_info->FindStringPath(kPlaylistThumbnailPathKey);
  const bool has_thumbnail =
      !!thumbnail_path_str && !thumbnail_path_str->empty();
  if (!has_thumbnail)
    DownloadThumbnail(id);

  base::Optional<bool> partial_ready =
      playlist_info->FindBoolPath(kPlaylistPartialReadyKey);
  const std::string* video_media_file_path =
      playlist_info->FindStringPath(kPlaylistVideoMediaFilePathKey);
  const std::string* audio_media_file_path =
      playlist_info->FindStringPath(kPlaylistAudioMediaFilePathKey);
  // Only try to regenerate if partial ready or there is no media file.
  if (!video_media_file_path || video_media_file_path->empty() ||
      !audio_media_file_path || audio_media_file_path->empty() ||
      *partial_ready) {
    VLOG(2) << __func__ << ": Regenerate media file";
    AddPlaylistToMediaFileGenerationQueue(id);
  }
}

void PlaylistService::DeletePlaylist(const std::string& id) {
  // Cancel if currently downloading item is id.
  if (video_media_file_controller_->current_playlist_id() == id) {
    video_media_file_controller_->RequestCancelCurrentPlaylistGeneration();
    audio_media_file_controller_->RequestCancelCurrentPlaylistGeneration();
  }

  RemovePlaylist(id);

  NotifyPlaylistChanged(
      {PlaylistChangeParams::ChangeType::CHANGE_TYPE_DELETED, id});

  // Delete assets from filesystem after updating db.
  video_media_file_controller_->DeletePlaylist(GetPlaylistItemDirPath(id));
  audio_media_file_controller_->DeletePlaylist(GetPlaylistItemDirPath(id));
}

void PlaylistService::DeleteAllPlaylist() {
  VLOG(2) << __func__;

  // Cancel currently generated playlist if needed and pending thumbnail
  // download jobs.
  video_media_file_controller_->RequestCancelCurrentPlaylistGeneration();
  audio_media_file_controller_->RequestCancelCurrentPlaylistGeneration();
  url_loaders_.clear();
  base::queue<base::Value> empty_queue;
  std::swap(pending_media_file_creation_jobs_, empty_queue);

  prefs_->ClearPref(kPlaylistItems);

  NotifyPlaylistChanged(
      {PlaylistChangeParams::ChangeType::CHANGE_TYPE_ALL_DELETED, ""});

  CleanUp();
}

void PlaylistService::Play(const std::string& id) {
  const base::Value* playlist_info =
      prefs_->Get(kPlaylistItems)->FindDictKey(id);
  if (!playlist_info) {
    LOG(ERROR) << __func__ << ": Invalid playlist id for play: " << id;
    return;
  }

  base::Optional<bool> partial_ready =
      playlist_info->FindBoolPath(kPlaylistPartialReadyKey);
  if (!partial_ready && partial_ready.value()) {
    LOG(ERROR) << __func__ << ": Playlist is not ready to play: " << id;
    return;
  }

  if (player_)
    player_->Play(GetPlaylistItemDirPath(id));
}

void PlaylistService::AddObserver(PlaylistServiceObserver* observer) {
  observers_.AddObserver(observer);
}

void PlaylistService::RemoveObserver(
    PlaylistServiceObserver* observer) {
  observers_.RemoveObserver(observer);
}

void PlaylistService::OnMediaFileReady(base::Value&& playlist_value,
                                       bool partial) {
  // TODO(simonhong): Revisit how |partial| handles.
  if (video_media_file_controller_->in_progress() ||
      audio_media_file_controller_->in_progress())
    partial = true;
  VLOG(2) << __func__ << ": "
          << *playlist_value.FindStringKey(kPlaylistPlaylistNameKey) << " "
          << partial;

  const std::string playlist_id =
      *playlist_value.FindStringKey(kPlaylistIDKey);
  UpdatePlaylistValue(playlist_id, std::move(playlist_value));

  NotifyPlaylistChanged(
      {partial
           ? PlaylistChangeParams::ChangeType::CHANGE_TYPE_PLAY_READY_PARTIAL
           : PlaylistChangeParams::ChangeType::CHANGE_TYPE_PLAY_READY,
       playlist_id});

  // TODO(simonhong): Revisit how |partial| handles.
  // If partial is true here, one of video or audio media files are not ready.
  if (partial)
    return;

  if (!pending_media_file_creation_jobs_.empty())
    GenerateMediaFiles();
}

void PlaylistService::OnMediaFileGenerationFailed(
    base::Value&& playlist_value) {
  VLOG(2) << __func__ << ": "
          << *playlist_value.FindStringKey(kPlaylistPlaylistNameKey);

  video_media_file_controller_->RequestCancelCurrentPlaylistGeneration();
  audio_media_file_controller_->RequestCancelCurrentPlaylistGeneration();
  const std::string playlist_id =
      *playlist_value.FindStringKey(kPlaylistIDKey);
  NotifyPlaylistChanged(
      {PlaylistChangeParams::ChangeType::CHANGE_TYPE_ABORTED, playlist_id});

  if (!pending_media_file_creation_jobs_.empty())
    GenerateMediaFiles();
}

void PlaylistService::OnGetOrphanedPaths(
    const std::vector<base::FilePath> orphaned_paths) {
  if (orphaned_paths.empty()) {
    VLOG(2) << __func__ << ": No orphaned playlist";
    return;
  }

  for (const auto& path : orphaned_paths) {
    VLOG(2) << __func__ << ": " << path << " is orphaned";
    video_media_file_controller_->DeletePlaylist(path);
    audio_media_file_controller_->DeletePlaylist(path);
  }
}

void PlaylistService::CleanUp() {
  base::Value playlist = GetAllPlaylist();

  base::flat_set<std::string> ids;
  for (const auto& item : playlist.GetList()) {
    const std::string* id = item.FindStringKey(kPlaylistIDKey);
    if (id)
      ids.insert(*id);
  }

  base::PostTaskAndReplyWithResult(
      io_task_runner(), FROM_HERE,
      base::BindOnce(&GetOrphanedPaths, base_dir_, ids),
      base::BindOnce(&PlaylistService::OnGetOrphanedPaths,
                     weak_factory_.GetWeakPtr()));
}

bool PlaylistService::GetThumbnailPath(const std::string& id,
                                       base::FilePath* thumbnail_path) {
  *thumbnail_path = GetPlaylistItemDirPath(id).Append(kThumbnailFileName);
  if (thumbnail_path->ReferencesParent()) {
    thumbnail_path->clear();
    return false;
  }
  return true;
}

base::SequencedTaskRunner* PlaylistService::io_task_runner() {
  return io_task_runner_.get();
}

}  // namespace playlist
