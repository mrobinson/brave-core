/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlist/browser/playlist_types.h"

#include "base/logging.h"
#include "base/notreached.h"

namespace playlist {

std::string PlaylistChangeParams::GetPlaylistChangeTypeAsString(
    PlaylistChangeParams::ChangeType type) {
  switch (type) {
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_ADDED:
      return "added";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_DELETED:
      return "deleted";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_ALL_DELETED:
      return "all_deleted";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_ABORTED:
      return "aborted";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_THUMBNAIL_READY:
      return "thumbnail_ready";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_THUMBNAIL_FAILED:
      return "thumbnail_failed";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_PLAY_READY:
      return "play_ready";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_PLAY_READY_PARTIAL:
      return "play_ready_partial";
    case PlaylistChangeParams::ChangeType::CHANGE_TYPE_NONE:  // fall through
    default:
      NOTREACHED();
      return "unknown";
  }
}

PlaylistChangeParams::PlaylistChangeParams() {}

PlaylistChangeParams::PlaylistChangeParams(ChangeType type,
                                             const std::string& id)
    : change_type(type), playlist_id(id) {}
PlaylistChangeParams::~PlaylistChangeParams() {}

MediaFileInfo::MediaFileInfo(const std::string& url, const std::string& title)
    : media_file_url(url), media_file_title(title) {}
MediaFileInfo::~MediaFileInfo() {}

CreatePlaylistParams::CreatePlaylistParams() {}
CreatePlaylistParams::~CreatePlaylistParams() {}
CreatePlaylistParams::CreatePlaylistParams(const CreatePlaylistParams& rhs) {
  playlist_thumbnail_url = rhs.playlist_thumbnail_url;
  playlist_name = rhs.playlist_name;
  video_media_files = rhs.video_media_files;
  audio_media_files = rhs.audio_media_files;
}

PlaylistInfo::PlaylistInfo() {}
PlaylistInfo::~PlaylistInfo() {}
PlaylistInfo::PlaylistInfo(const PlaylistInfo& rhs) {
  id = rhs.id;
  playlist_name = rhs.playlist_name;
  thumbnail_path = rhs.thumbnail_path;
  video_media_file_path = rhs.video_media_file_path;
  audio_media_file_path = rhs.audio_media_file_path;
  create_params = rhs.create_params;
  partial_ready = rhs.partial_ready;
}

}  // namespace playlist
