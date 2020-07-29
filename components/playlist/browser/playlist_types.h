/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_TYPES_H_
#define BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_TYPES_H_

#include <string>
#include <vector>

namespace playlist {

// Partial means some source media files are not available.
// TODO(simonhong): Partial status could be deleted. When PlaylistItem can't get
// complete media file, it's status should be treated as failed status.
struct PlaylistChangeParams {
  enum class ChangeType {
    CHANGE_TYPE_NONE,
    CHANGE_TYPE_ADDED,               // New playlist added but not ready state
    CHANGE_TYPE_THUMBNAIL_READY,     // Thumbnail ready to use for playlist
    CHANGE_TYPE_THUMBNAIL_FAILED,    // Failed to fetch thumbnail
    CHANGE_TYPE_PLAY_READY,          // Playlist ready to play
    CHANGE_TYPE_PLAY_READY_PARTIAL,  // Playlist ready to play but partial
    CHANGE_TYPE_DELETED,             // A playlist deleted
    CHANGE_TYPE_ALL_DELETED,         // All playlist are deleted
    CHANGE_TYPE_ABORTED,             // Aborted during the creation process
    CHANGE_TYPE_RECOVER_FAILED,      // Aborted during the creation process
  };
  static std::string GetPlaylistChangeTypeAsString(
      PlaylistChangeParams::ChangeType type);

  PlaylistChangeParams();
  PlaylistChangeParams(ChangeType type, const std::string& id);
  ~PlaylistChangeParams();

  ChangeType change_type = ChangeType::CHANGE_TYPE_NONE;
  std::string playlist_id;
};

struct MediaFileInfo {
  MediaFileInfo(const std::string& url, const std::string& title);
  ~MediaFileInfo();

  std::string media_file_url;
  std::string media_file_title;
};

struct CreatePlaylistParams {
  CreatePlaylistParams();
  CreatePlaylistParams(const CreatePlaylistParams& rhs);
  ~CreatePlaylistParams();

  std::string playlist_thumbnail_url;
  std::string playlist_name;
  std::vector<MediaFileInfo> video_media_files;
  std::vector<MediaFileInfo> audio_media_files;
};

// TODO(simonhong): Rename to PlaylistItemInfo
struct PlaylistInfo {
  PlaylistInfo();
  PlaylistInfo(const PlaylistInfo& rhs);
  ~PlaylistInfo();

  std::string id;
  // TODO(simonhong): Delete this. |create_params| has it.
  std::string playlist_name;
  std::string thumbnail_path;
  std::string video_media_file_path;
  std::string audio_media_file_path;
  bool partial_ready{false};

  CreatePlaylistParams create_params;
};

}  // namespace playlist

#endif  // BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_TYPES_H_
