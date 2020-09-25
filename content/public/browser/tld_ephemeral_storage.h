/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CONTENT_PUBLIC_BROWSER_TLD_EPHEMERAL_STORAGE_H_
#define BRAVE_CONTENT_PUBLIC_BROWSER_TLD_EPHEMERAL_STORAGE_H_

#include <string>
#include <utility>
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/common/content_export.h"

namespace content {

class BrowserContext;
class SessionStorageNamespace;
class StoragePartition;
class StoragePartition;
class WebContents;

CONTENT_EXPORT scoped_refptr<content::SessionStorageNamespace>
CreateSessionStorageNamespace(content::StoragePartition* partition,
                              const std::string& namespace_id);

CONTENT_EXPORT std::string GetSessionStorageNamespaceId(WebContents*);

// TLD storage is keyed by the BrowserContext (profile) and the TLD-specific
// security domain.
using TLDEphemeralStorageKey = std::pair<content::BrowserContext*, std::string>;

// This class is responsible for managing the lifetime of ephemeral storage
// cookies. Each instance is shared by each top-level frame with the same
// TLDEphemeralStorageKey. When the last top-level frame holding a reference
// is destroyed or navigates to a new storage domain, storage will be
// cleared.
//
// TODO(mrobinson): Have this class also manage ephemeral local storage and
// take care of handing out new instances of session storage.
class CONTENT_EXPORT TLDEphemeralStorage
    : public base::RefCounted<TLDEphemeralStorage> {
 public:
  TLDEphemeralStorage(TLDEphemeralStorageKey key,
                      StoragePartition* storage_partition);
  static TLDEphemeralStorage* Get(BrowserContext*, std::string storage_domain);
  static scoped_refptr<TLDEphemeralStorage> GetOrCreate(
      BrowserContext* browser_context,
      StoragePartition* storage_partition,
      std::string storage_domain);

 private:
  friend class RefCounted<TLDEphemeralStorage>;
  virtual ~TLDEphemeralStorage();

  TLDEphemeralStorageKey key_;
  StoragePartition* storage_partition_;

  base::WeakPtrFactory<TLDEphemeralStorage> weak_factory_{this};
};

}  // namespace content

#endif  // BRAVE_CONTENT_PUBLIC_BROWSER_TLD_EPHEMERAL_STORAGE_H_
