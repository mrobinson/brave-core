/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/settings/brave_social_blocking_handler.h"

#include <string>

#include "base/bind.h"
#include "base/values.h"
#include "brave/common/pref_names.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

BraveSocialBlockingHandler::BraveSocialBlockingHandler()
    : weak_ptr_factory_(this) {}

BraveSocialBlockingHandler::~BraveSocialBlockingHandler() {}

void BraveSocialBlockingHandler::RegisterMessages() {
  profile_ = Profile::FromWebUI(web_ui());
  web_ui()->RegisterMessageCallback(
      "setGoogleLoginEnabled",
      base::BindRepeating(&BraveSocialBlockingHandler::SetGoogleLoginEnabled,
                          base::Unretained(this)));
}

void BraveSocialBlockingHandler::SetGoogleLoginEnabled(
    const base::ListValue* args) {
  CHECK_EQ(args->GetSize(), 1U);
  CHECK(profile_);
  bool enabled;
  args->GetBoolean(0, &enabled);

  profile_->GetPrefs()->SetBoolean(prefs::kSigninAllowedOnNextStartup, enabled);
}

// static
void BraveSocialBlockingHandler::AddLoadTimeData(
    content::WebUIDataSource* data_source,
    Profile* profile) {
  data_source->AddBoolean(
      "googleLoginEnabledAtStartup",
      profile->GetPrefs()->GetBoolean(prefs::kSigninAllowedOnNextStartup));
}
