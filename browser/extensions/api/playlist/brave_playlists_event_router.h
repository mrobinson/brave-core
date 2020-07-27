/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EXTENSIONS_BRAVE_PLAYLISTS_EVENT_ROUTER_H_
#define BRAVE_BROWSER_EXTENSIONS_BRAVE_PLAYLISTS_EVENT_ROUTER_H_

#include <string>

#include "brave/components/playlists/browser/playlists_controller_observer.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace brave_playlists {

class BravePlaylistsEventRouter : public PlaylistsControllerObserver {
 public:
  explicit BravePlaylistsEventRouter(content::BrowserContext* context);
  ~BravePlaylistsEventRouter() override;

  BravePlaylistsEventRouter(const BravePlaylistsEventRouter&) = delete;
  BravePlaylistsEventRouter& operator=(
      const BravePlaylistsEventRouter&) = delete;

  // PlaylistsControllerObserver overrides:
  void OnPlaylistsChanged(const PlaylistsChangeParams& params) override;

 private:
  content::BrowserContext* context_;
};

}  // namespace brave_playlists

#endif  // BRAVE_BROWSER_EXTENSIONS_BRAVE_PLAYLISTS_EVENT_ROUTER_H_
