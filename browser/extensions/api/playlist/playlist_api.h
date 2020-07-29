/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_PLAYLIST_API_H_
#define BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_PLAYLIST_API_H_

#include "extensions/browser/extension_function.h"

#include "base/values.h"

namespace extensions {
namespace api {

class BravePlaylistCreatePlaylistFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.createPlaylist", UNKNOWN)

 protected:
  ~BravePlaylistCreatePlaylistFunction() override {}

  ResponseAction Run() override;
};

class BravePlaylistGetAllPlaylistFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.getAllPlaylist", UNKNOWN)

 protected:
  ~BravePlaylistGetAllPlaylistFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class BravePlaylistGetPlaylistFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.getPlaylist", UNKNOWN)

 protected:
  ~BravePlaylistGetPlaylistFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class BravePlaylistRecoverPlaylistFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.recoverPlaylist", UNKNOWN)

 protected:
  ~BravePlaylistRecoverPlaylistFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class BravePlaylistDeletePlaylistFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.deletePlaylist", UNKNOWN)

 protected:
  ~BravePlaylistDeletePlaylistFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class BravePlaylistDeleteAllPlaylistFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.deleteAllPlaylist", UNKNOWN)

 protected:
  ~BravePlaylistDeleteAllPlaylistFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class BravePlaylistRequestDownloadFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.requestDownload", UNKNOWN)

 protected:
  ~BravePlaylistRequestDownloadFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

class BravePlaylistPlayFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("bravePlaylist.play", UNKNOWN)

 protected:
  ~BravePlaylistPlayFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace api
}  // namespace extensions

#endif  // BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_PLAYLISTS_API_H_
