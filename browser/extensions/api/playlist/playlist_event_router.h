/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_PLAYLIST_EVENT_ROUTER_H_
#define BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_PLAYLIST_EVENT_ROUTER_H_

#include <string>

#include "base/scoped_observer.h"
#include "brave/components/playlist/browser/playlist_service.h"
#include "brave/components/playlist/browser/playlist_service_observer.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/event_router.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace playlist {

class PlaylistEventRouter : public extensions::BrowserContextKeyedAPI,
                            public extensions::EventRouter::Observer,
                            public PlaylistServiceObserver {
 public:
  PlaylistEventRouter(const PlaylistEventRouter&) = delete;
  PlaylistEventRouter& operator=(const PlaylistEventRouter&) = delete;

  // extensions::BrowserContextKeyedAPI overrides:
  static extensions::BrowserContextKeyedAPIFactory<PlaylistEventRouter>*
      GetFactoryInstance();
  static const char* service_name() { return "PlaylistAPI"; }

  // extensions::EventRouter::Observer overrides:
  void OnListenerAdded(const extensions::EventListenerInfo& details) override;

  // PlaylistServiceObserver overrides:
  void OnPlaylistItemStatusChanged(
      const PlaylistChangeParams& params) override;

 private:
  friend class extensions::BrowserContextKeyedAPIFactory<PlaylistEventRouter>;

  explicit PlaylistEventRouter(content::BrowserContext* context);
  ~PlaylistEventRouter() override;

  content::BrowserContext* context_;
  ScopedObserver<PlaylistService, PlaylistServiceObserver> observed_{this};
};

}  // namespace playlist

#endif  // BRAVE_BROWSER_EXTENSIONS_API_PLAYLIST_PLAYLIST_EVENT_ROUTER_H_
