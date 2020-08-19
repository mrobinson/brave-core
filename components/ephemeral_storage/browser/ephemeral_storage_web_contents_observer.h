/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_EPHEMERAL_STROAGE_BROWSER_EPHEMERAL_STORAGE_WEB_CONTENTS_OBSERVER_H_
#define BRAVE_COMPONENTS_EPHEMERAL_STROAGE_BROWSER_EPHEMERAL_STORAGE_WEB_CONTENTS_OBSERVER_H_

//#include <map>
//#include <set>
//#include <string>
//#include <vector>

//#include "base/macros.h"
//#include "base/synchronization/lock.h"
//#include "base/strings/string16.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/browser/storage_partition_config.h"

namespace content {
class WebContents;
}

namespace ephemeral_storage {

class EphemeralStorageWebContentsObserver : public content::WebContentsObserver,
    public content::WebContentsUserData<EphemeralStorageWebContentsObserver> {
 public:
  explicit EphemeralStorageWebContentsObserver(content::WebContents*);
  ~EphemeralStorageWebContentsObserver() override;

  static content::StoragePartitionConfig GetEphemeralStorageConfig(
      content::RenderViewHost* host);

 protected:
  void ReadyToCommitNavigation(
      content::NavigationHandle* navigation_handle) override;
  void WebContentsDestroyed() override;

 private:
  friend class content::WebContentsUserData<EphemeralStorageWebContentsObserver>;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
  DISALLOW_COPY_AND_ASSIGN(EphemeralStorageWebContentsObserver);
};

}  // namespace ephemeral_storage

#endif  // BRAVE_COMPONENTS_EPHEMERAL_STROAGE_BROWSER_EPHEMERAL_STORAGE_WEB_CONTENTS_OBSERVER_H_
