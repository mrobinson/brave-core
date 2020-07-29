/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/api/playlist/playlist_event_router.h"

#include <memory>
#include <utility>

#include "base/lazy_instance.h"
#include "brave/browser/playlist/playlist_service_factory.h"
#include "brave/common/extensions/api/brave_playlist.h"
#include "brave/components/playlist/browser/playlist_service.h"
#include "chrome/browser/profiles/profile.h"

namespace playlist {

namespace {

PlaylistService* GetPlaylistService(content::BrowserContext* context) {
  return PlaylistServiceFactory::GetInstance()->GetForProfile(
      Profile::FromBrowserContext(context));
}

}  // namespace

static base::LazyInstance<extensions::BrowserContextKeyedAPIFactory<
    PlaylistEventRouter>>::DestructorAtExit
    g_playlist_event_router = LAZY_INSTANCE_INITIALIZER;

// static
extensions::BrowserContextKeyedAPIFactory<PlaylistEventRouter>*
PlaylistEventRouter::GetFactoryInstance() {
  return g_playlist_event_router.Pointer();
}

PlaylistEventRouter::PlaylistEventRouter(
    content::BrowserContext* context)
    : context_(context) {
  extensions::EventRouter::Get(context)->RegisterObserver(
      this, extensions::api::brave_playlist::OnPlaylistChanged::kEventName);
}

PlaylistEventRouter::~PlaylistEventRouter() = default;

void PlaylistEventRouter::OnListenerAdded(
    const extensions::EventListenerInfo& details) {
  DCHECK_EQ(details.event_name,
            extensions::api::brave_playlist::OnPlaylistChanged::kEventName);
  if (auto* service = GetPlaylistService(context_)) {
    observed_.Add(service);
    extensions::EventRouter::Get(context_)->UnregisterObserver(this);
  }
}

void PlaylistEventRouter::OnPlaylistItemStatusChanged(
    const PlaylistChangeParams& params) {
  auto event = std::make_unique<extensions::Event>(
      extensions::events::BRAVE_PLAYLIST_ON_PLAYLIST_CHANGED,
      extensions::api::brave_playlist::OnPlaylistChanged::kEventName,
      extensions::api::brave_playlist::OnPlaylistChanged::Create(
          PlaylistChangeParams::GetPlaylistChangeTypeAsString(
              params.change_type),
          params.playlist_id),
      context_);

  extensions::EventRouter::Get(context_)->BroadcastEvent(std::move(event));
}

}  // namespace playlist
