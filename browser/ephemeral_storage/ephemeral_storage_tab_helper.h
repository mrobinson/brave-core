/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EPHEMERAL_STORAGE_EPHEMERAL_STORAGE_TAB_HELPER_H_
#define BRAVE_BROWSER_EPHEMERAL_STORAGE_EPHEMERAL_STORAGE_TAB_HELPER_H_

#include <string>
#include <utility>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/session_storage_namespace.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace content {
class WebContents;
class BrowserContext;
}  // namespace content

namespace ephemeral_storage {

// Per-TLD storage is keyed by the BrowserContext (Profile) and the TLD-specific
// security domain.
using PerTLDEphemeralStorageKey =
    std::pair<content::BrowserContext*, std::string>;

// This data structure tracks all per-TLD ephemeral storage. Currently the only
// per-TLD data is DOM localStorage, but in the future this will include other
// types of storage.  We use reference counting to determine when no more
// WebContents are using this storage area. When that happens, the data is
// cleared automatically, resulting in the storage persisting only as long as
// at least one top-level WebContents shares the same TLD. In order to create
// new references, PerTLDEphemeralStorage maintains a map of WeakPtrs that is
// kept up-to-date with active storage areas.
class PerTLDEphemeralStorage : public base::RefCounted<PerTLDEphemeralStorage> {
 public:
  PerTLDEphemeralStorage(
      PerTLDEphemeralStorageKey key,
      scoped_refptr<content::SessionStorageNamespace> local_storage_namespace);

 private:
  friend class RefCounted<PerTLDEphemeralStorage>;
  virtual ~PerTLDEphemeralStorage();

  PerTLDEphemeralStorageKey key_;
  scoped_refptr<content::SessionStorageNamespace> local_storage_namespace_;
  base::WeakPtrFactory<PerTLDEphemeralStorage> weak_factory_{this};
};

class EphemeralStorageTabHelper
    : public content::WebContentsObserver,
      public content::WebContentsUserData<EphemeralStorageTabHelper> {
 public:
  explicit EphemeralStorageTabHelper(content::WebContents* web_contents);
  ~EphemeralStorageTabHelper() override;

 protected:
  void ReadyToCommitNavigation(
      content::NavigationHandle* navigation_handle) override;

 private:
  friend class content::WebContentsUserData<EphemeralStorageTabHelper>;
  scoped_refptr<content::SessionStorageNamespace> session_storage_namespace_;
  scoped_refptr<PerTLDEphemeralStorage> per_tld_ephemeral_storage_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace ephemeral_storage

#endif  // BRAVE_BROWSER_EPHEMERAL_STORAGE_EPHEMERAL_STORAGE_TAB_HELPER_H_
