/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLISTS_BROWSER_PLAYLISTS_CONTROLLER_H_
#define BRAVE_COMPONENTS_PLAYLISTS_BROWSER_PLAYLISTS_CONTROLLER_H_

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
#include "brave/components/playlists/browser/playlists_media_file_controller.h"
#include "brave/components/playlists/browser/playlists_types.h"

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

namespace brave_playlists {

class PlaylistsControllerObserver;
class PlaylistsDBController;
class PlaylistsPlayer;

class PlaylistsController : PlaylistsMediaFileController::Client {
 public:
  PlaylistsController(content::BrowserContext* context,
                      scoped_refptr<base::SequencedTaskRunner> task_runner);

  ~PlaylistsController() override;

  void set_player(PlaylistsPlayer* player) { player_ = player; }

  // False when |params| are not sufficient for new playlist.
  // brave_playlists.json explains in detail about below apis.
  void CreatePlaylist(const CreatePlaylistParams& params);
  base::Value GetAllPlaylists();
  base::Value GetPlaylist(const std::string& id);
  void RecoverPlaylist(const std::string& id);
  void DeletePlaylist(const std::string& id);
  void DeleteAllPlaylists();
  // TODO(simonhong): Remove this. API can handle.
  void RequestDownload(const std::string& url);
  // TODO(simonhong): Remove this. API can handle.
  void Play(const std::string& id);

  void AddObserver(PlaylistsControllerObserver* observer);
  void RemoveObserver(PlaylistsControllerObserver* observer);

  bool GetThumbnailPath(const std::string& id, base::FilePath* thumbnail_path);

 private:
  using SimpleURLLoaderList =
      std::list<std::unique_ptr<network::SimpleURLLoader>>;

  // PlaylistsMediaFileController::Client overrides:
  void OnMediaFileReady(base::Value&& playlist_value, bool partial) override;
  void OnMediaFileGenerationFailed(base::Value&& playlist_value) override;

  void OnPlaylistItemDirCreated(const std::string& id, bool directory_ready);

  void DownloadThumbnail(const std::string& id);
  void OnThumbnailDownloaded(const std::string& id,
                             SimpleURLLoaderList::iterator it,
                             base::FilePath path);
  void OnPutPlayReadyPlaylist(base::Value&& playlist_value,
                              bool partial,
                              bool result);
  void AddPlaylistToMediaFileGenerationQueue(const std::string& id);
  void GenerateMediaFiles();

  void OnDeletePlaylist(const std::string& playlist_id, bool success);

  base::SequencedTaskRunner* io_task_runner();

  // Delete orphaned playlist item directories that are not included in db.
  void CleanUp();
  void OnGetOrphanedPaths(const std::vector<base::FilePath> paths);

  void NotifyPlaylistChanged(const PlaylistsChangeParams& params);

  base::FilePath GetPlaylistItemDirPath(const std::string& id) const;

  void UpdatePlaylistValue(const std::string& id, base::Value&& value);
  void RemovePlaylist(const std::string& id);

  // Playlist creation can be ready to play via below three steps.
  // Step 0. When creation is requested, requested info is put to db and
  //         notification is delivered to user with basic infos like playlist
  //         name and titles if provided. playlist is visible to user but it
  //         doesn't have thumbnail.
  // Step 1. Getting basic infos for showing this playlist to users. Currently
  //         it is only thumbnail image for this playlist.
  //         When thumbnail is fetched, it goes to step 2 and notifying to
  //         user about this playlist has thumbnail. But, still not ready to
  //         play.
  // Step 2. Getting media files and combining them as a single media file.
  //         Then, user can get notification about this playlist is ready to
  //         play.

  content::BrowserContext* context_;
  const base::FilePath base_dir_;
  base::queue<base::Value> pending_media_file_creation_jobs_;
  base::ObserverList<PlaylistsControllerObserver> observers_;

  std::unique_ptr<PlaylistsMediaFileController> video_media_file_controller_;
  std::unique_ptr<PlaylistsMediaFileController> audio_media_file_controller_;

  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;

  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  SimpleURLLoaderList url_loaders_;

  PlaylistsPlayer* player_ = nullptr;
  PrefService* prefs_;

  base::WeakPtrFactory<PlaylistsController> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PlaylistsController);
};

}  // namespace brave_playlists

#endif  // BRAVE_COMPONENTS_PLAYLISTS_BROWSER_PLAYLISTS_CONTROLLER_H_
