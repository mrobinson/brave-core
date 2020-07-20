/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_playlists_ui.h"

#include "brave/components/playlists/resources/grit/playlists_generated_map.h"
#include "components/grit/brave_components_resources.h"

namespace brave_playlists {

BravePlaylistsUI::BravePlaylistsUI(content::WebUI* web_ui,
                                   const std::string& name)
    : BasicUI(web_ui,
              name,
              kPlaylistsGenerated,
              kPlaylistsGeneratedSize,
              IDR_BRAVE_PLAYLISTS_HTML) {
}

BravePlaylistsUI::~BravePlaylistsUI() {}

}  // namespace brave_playlists
