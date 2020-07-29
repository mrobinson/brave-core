/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/api/playlist/brave_playlists_event_router.h"

#include <memory>
#include <utility>

#include "base/lazy_instance.h"
#include "brave/browser/playlists/playlists_service_factory.h"
#include "brave/common/extensions/api/brave_playlists.h"
#include "brave/components/playlists/browser/playlists_service.h"
#include "chrome/browser/profiles/profile.h"

namespace brave_playlists {

namespace {

PlaylistsService* GetPlaylistsService(content::BrowserContext* context) {
  return PlaylistsServiceFactory::GetInstance()->GetForProfile(
      Profile::FromBrowserContext(context));
}

}  // namespace

static base::LazyInstance<extensions::BrowserContextKeyedAPIFactory<
    BravePlaylistsEventRouter>>::DestructorAtExit
    g_playlist_event_router = LAZY_INSTANCE_INITIALIZER;

// static
extensions::BrowserContextKeyedAPIFactory<BravePlaylistsEventRouter>*
BravePlaylistsEventRouter::GetFactoryInstance() {
  return g_playlist_event_router.Pointer();
}

BravePlaylistsEventRouter::BravePlaylistsEventRouter(
    content::BrowserContext* context)
    : context_(context) {
  extensions::EventRouter::Get(context)->RegisterObserver(
      this, extensions::api::brave_playlists::OnPlaylistsChanged::kEventName);
}

BravePlaylistsEventRouter::~BravePlaylistsEventRouter() = default;

void BravePlaylistsEventRouter::OnListenerAdded(
    const extensions::EventListenerInfo& details) {
  DCHECK_EQ(details.event_name,
            extensions::api::brave_playlists::OnPlaylistsChanged::kEventName);
  if (auto* service = GetPlaylistsService(context_)) {
    observed_.Add(service);
    extensions::EventRouter::Get(context_)->UnregisterObserver(this);
  }
}

void BravePlaylistsEventRouter::OnPlaylistItemStatusChanged(
    const PlaylistsChangeParams& params) {
  auto event = std::make_unique<extensions::Event>(
      extensions::events::BRAVE_PLAYLISTS_ON_PLAYLISTS_CHANGED,
      extensions::api::brave_playlists::OnPlaylistsChanged::kEventName,
      extensions::api::brave_playlists::OnPlaylistsChanged::Create(
          PlaylistsChangeParams::GetPlaylistsChangeTypeAsString(
              params.change_type),
          params.playlist_id),
      context_);

  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

}  // namespace brave_playlists
