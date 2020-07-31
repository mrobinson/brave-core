/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_SETTINGS_BRAVE_SOCIAL_BLOCKING_HANDLER_H_
#define BRAVE_BROWSER_UI_WEBUI_SETTINGS_BRAVE_SOCIAL_BLOCKING_HANDLER_H_

#include <string>

#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"

class Profile;

namespace content {
class WebUIDataSource;
}

class BraveSocialBlockingHandler : public settings::SettingsPageUIHandler {
 public:
  BraveSocialBlockingHandler();
  ~BraveSocialBlockingHandler() override;
  static void AddLoadTimeData(content::WebUIDataSource* data_source,
                              Profile* profile);

 private:
  void RegisterMessages() override;
  void OnJavascriptAllowed() override {}
  void OnJavascriptDisallowed() override {}

  void SetGoogleLoginEnabled(const base::ListValue* args);

  Profile* profile_ = nullptr;
  base::WeakPtrFactory<BraveSocialBlockingHandler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BraveSocialBlockingHandler);
};

#endif  // BRAVE_BROWSER_UI_WEBUI_SETTINGS_BRAVE_SOCIAL_BLOCKING_HANDLER_H_
