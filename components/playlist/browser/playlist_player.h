/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_PLAYER_H_
#define BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_PLAYER_H_

namespace base {
class FilePath;
}  // namespace base

namespace playlist {

class PlaylistPlayer {
 public:
  // |playlist_path| is the directory that holds each playlist item's data.
  virtual void Play(const base::FilePath& playlist_path) = 0;

  virtual ~PlaylistPlayer() {}
};

}  // namespace playlist

#endif  // BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_PLAYER_H_
