/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_PLAYLISTS_DESKTOP_PLAYLISTS_PLAYER_H_
#define BRAVE_BROWSER_PLAYLISTS_DESKTOP_PLAYLISTS_PLAYER_H_

#include "brave/components/playlists/browser/playlists_player.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"

class Profile;

namespace base {
class FilePath;
class SequencedTaskRunner;
}  // namespace base

namespace brave_playlists {

// Demo purpose player on desktop.
class DesktopPlaylistsPlayer : public PlaylistsPlayer {
 public:
  DesktopPlaylistsPlayer(Profile* profile,
                         scoped_refptr<base::SequencedTaskRunner> task_runner);
  ~DesktopPlaylistsPlayer() override;

  DesktopPlaylistsPlayer(const DesktopPlaylistsPlayer&) = delete;
  DesktopPlaylistsPlayer& operator=(const DesktopPlaylistsPlayer&) = delete;

  // PlaylistsPlayer overrides:
  void Play(const base::FilePath& playlist_path) override;

 private:
  void OnHTMLFileGenerated(const base::FilePath& html_file_path,
                           int error_code);

  Profile* profile_;
  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;

  base::WeakPtrFactory<DesktopPlaylistsPlayer> weak_factory_;
};

}  // namespace brave_playlists

#endif  // BRAVE_BROWSER_PLAYLISTS_DESKTOP_PLAYLISTS_PLAYER_H_
