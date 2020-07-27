/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/api/playlist/brave_playlists_event_router.h"

#include <memory>
#include <utility>

#include "brave/common/extensions/api/brave_playlists.h"
#include "extensions/browser/event_router.h"

namespace brave_playlists {

BravePlaylistsEventRouter::BravePlaylistsEventRouter(
    content::BrowserContext* context)
    : context_(context) {
}

BravePlaylistsEventRouter::~BravePlaylistsEventRouter() = default;

void BravePlaylistsEventRouter::OnPlaylistsChanged(
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
