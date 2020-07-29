/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/playlist_ui.h"

#include "brave/components/playlist/resources/grit/playlist_generated_map.h"
#include "components/grit/brave_components_resources.h"

namespace playlist {

PlaylistUI::PlaylistUI(content::WebUI* web_ui, const std::string& name)
    : BasicUI(web_ui,
              name,
              kPlaylistGenerated,
              kPlaylistGeneratedSize,
              IDR_PLAYLIST_HTML) {
}

PlaylistUI::~PlaylistUI() {}

}  // namespace playlist
