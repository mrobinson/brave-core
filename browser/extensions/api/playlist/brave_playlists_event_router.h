/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_BRAVE_PLAYLISTS_EVENT_ROUTER_H_
#define BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_BRAVE_PLAYLISTS_EVENT_ROUTER_H_

#include <string>

#include "base/scoped_observer.h"
#include "brave/components/playlists/browser/playlists_service.h"
#include "brave/components/playlists/browser/playlists_service_observer.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/event_router.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace brave_playlists {

class BravePlaylistsEventRouter : public extensions::BrowserContextKeyedAPI,
                                  public extensions::EventRouter::Observer,
                                  public PlaylistsServiceObserver {
 public:
  BravePlaylistsEventRouter(const BravePlaylistsEventRouter&) = delete;
  BravePlaylistsEventRouter& operator=(
      const BravePlaylistsEventRouter&) = delete;

  // extensions::BrowserContextKeyedAPI overrides:
  static extensions::BrowserContextKeyedAPIFactory<BravePlaylistsEventRouter>*
      GetFactoryInstance();
  static const char* service_name() { return "PlaylistAPI"; }

  // extensions::EventRouter::Observer overrides:
  void OnListenerAdded(const extensions::EventListenerInfo& details) override;

  // PlaylistsServiceObserver overrides:
  void OnPlaylistsChanged(const PlaylistsChangeParams& params) override;

 private:
  friend class extensions::BrowserContextKeyedAPIFactory<
      BravePlaylistsEventRouter>;

  explicit BravePlaylistsEventRouter(content::BrowserContext* context);
  ~BravePlaylistsEventRouter() override;

  content::BrowserContext* context_;
  ScopedObserver<PlaylistsService, PlaylistsServiceObserver> observed_{this};
};

}  // namespace brave_playlists

#endif  // BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_BRAVE_PLAYLISTS_EVENT_ROUTER_H_
