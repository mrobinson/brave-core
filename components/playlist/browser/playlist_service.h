/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_SERVICE_H_
#define BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_SERVICE_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/containers/queue.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/values.h"
#include "brave/components/playlist/browser/playlist_media_file_controller.h"
#include "brave/components/playlist/browser/playlist_types.h"
#include "components/keyed_service/core/keyed_service.h"

namespace base {
class FilePath;
class SequencedTaskRunner;
}  // namespace base

namespace content {
class BrowserContext;
}  // namespace content

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
}  // namespace network

class PrefService;

namespace playlist {

class PlaylistServiceObserver;
class PlaylistDBController;
class PlaylistPlayer;

class PlaylistService : public KeyedService,
                        public PlaylistMediaFileController::Client {
 public:
  PlaylistService(content::BrowserContext* context,
                  scoped_refptr<base::SequencedTaskRunner> task_runner);

  ~PlaylistService() override;

  void SetPlayer(std::unique_ptr<PlaylistPlayer> player);

  // False when |params| are not sufficient for new playlist.
  // brave_playlist.json explains in detail about below apis.
  void CreatePlaylistItem(const CreatePlaylistParams& params);
  base::Value GetAllPlaylistItems();
  base::Value GetPlaylistItem(const std::string& id);
  void RecoverPlaylistItem(const std::string& id);
  void DeletePlaylistItem(const std::string& id);
  void DeleteAllPlaylistItems();
  // TODO(simonhong): Remove this. Client should handle play request.
  void PlayItem(const std::string& id);

  void AddObserver(PlaylistServiceObserver* observer);
  void RemoveObserver(PlaylistServiceObserver* observer);

  bool GetThumbnailPath(const std::string& id, base::FilePath* thumbnail_path);

 private:
  using SimpleURLLoaderList =
      std::list<std::unique_ptr<network::SimpleURLLoader>>;

  // PlaylistMediaFileController::Client overrides:
  void OnMediaFileReady(base::Value&& playlist_value, bool partial) override;
  void OnMediaFileGenerationFailed(base::Value&& playlist_value) override;

  void OnPlaylistItemDirCreated(const std::string& id, bool directory_ready);

  void DownloadThumbnail(const std::string& id);
  void OnThumbnailDownloaded(const std::string& id,
                             SimpleURLLoaderList::iterator it,
                             base::FilePath path);
  void AddPlaylistToMediaFileGenerationQueue(const std::string& id);
  void GenerateMediaFiles();

  void OnDeletePlaylist(const std::string& playlist_id, bool success);

  base::SequencedTaskRunner* io_task_runner();

  // Delete orphaned playlist item directories that are not included in db.
  void CleanUp();
  void OnGetOrphanedPaths(const std::vector<base::FilePath> paths);

  void NotifyPlaylistChanged(const PlaylistChangeParams& params);

  base::FilePath GetPlaylistItemDirPath(const std::string& id) const;

  void UpdatePlaylistValue(const std::string& id, base::Value&& value);
  void RemovePlaylist(const std::string& id);

  // Playlist creation can be ready to play two steps.
  // Step 1. When creation is requested, requested info is put to db and
  //         notification is delivered to user with basic infos like playlist
  //         name and titles if provided. playlist is visible to user but it
  //         doesn't have thumbnail.
  // Step 1. Request thumbnail.
  //         Request video files and audio files and combined as a single
  //         video and audio file.
  //         Whenever thumbnail is fetched or media files are ready,
  //         it is notified.

  content::BrowserContext* context_;
  const base::FilePath base_dir_;
  base::queue<base::Value> pending_media_file_creation_jobs_;
  base::ObserverList<PlaylistServiceObserver> observers_;

  std::unique_ptr<PlaylistMediaFileController> video_media_file_controller_;
  std::unique_ptr<PlaylistMediaFileController> audio_media_file_controller_;

  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;

  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  SimpleURLLoaderList url_loaders_;

  std::unique_ptr<PlaylistPlayer> player_;
  PrefService* prefs_;

  base::WeakPtrFactory<PlaylistService> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PlaylistService);
};

}  // namespace playlist

#endif  // BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_SERVICE_H_
