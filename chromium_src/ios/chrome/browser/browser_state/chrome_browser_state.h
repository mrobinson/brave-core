/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_IOS_CHROME_BROWSER_BROWSER_STATE_CHROME_BROWSER_STATE_H_
#define BRAVE_CHROMIUM_SRC_IOS_CHROME_BROWSER_BROWSER_STATE_CHROME_BROWSER_STATE_H_

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "ios/web/public/browser_state.h"

class PrefService;

namespace base {
class SequencedTaskRunner;
}

namespace sync_preferences {
class PrefServiceSyncable;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

extern const char kProductDirName[];
extern const char kIOSChromeInitialBrowserState[];

class ChromeBrowserState : public web::BrowserState {
 public:
  explicit ChromeBrowserState(const base::FilePath& name);
  ~ChromeBrowserState() override;

  static ChromeBrowserState* FromBrowserState(BrowserState* browser_state);

  net::URLRequestContextGetter* GetRequestContext() override;
  bool IsOffTheRecord() const override;
  base::FilePath GetStatePath() const override;

  virtual ChromeBrowserState* GetOriginalChromeBrowserState();
  PrefService* GetPrefs();
  scoped_refptr<base::SequencedTaskRunner> GetIOTaskRunner();

 private:
  // net::URLRequestContextGetter* CreateRequestContext();

  base::FilePath state_path_;
  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;
  scoped_refptr<user_prefs::PrefRegistrySyncable> pref_registry_;
  std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs_;
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;

  DISALLOW_COPY_AND_ASSIGN(ChromeBrowserState);
};

#endif /* BRAVE_CHROMIUM_SRC_IOS_CHROME_BROWSER_BROWSER_STATE_CHROME_BROWSER_STATE_H_ */
