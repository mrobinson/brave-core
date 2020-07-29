/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_PLAYLIST_UI_H_
#define BRAVE_BROWSER_UI_WEBUI_PLAYLIST_UI_H_

#include <memory>
#include <string>

#include "brave/browser/ui/webui/basic_ui.h"

namespace playlist {

class PlaylistUI : public BasicUI {
 public:
  PlaylistUI(content::WebUI* web_ui, const std::string& host);
  ~PlaylistUI() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(PlaylistUI);
};

}  // namespace playlist

#endif  // BRAVE_BROWSER_UI_WEBUI_PLAYLIST_UI_H_
