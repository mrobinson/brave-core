/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ephemeral_storage/ephemeral_storage_tab_helper.h"

#include <map>
#include <set>

#include "base/feature_list.h"
#include "base/hash/md5.h"
#include "base/no_destructor.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/session_storage_namespace.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "third_party/blink/public/common/features.h"

using content::BrowserContext;
using content::NavigationHandle;
using content::SessionStorageNamespace;
using content::WebContents;

namespace ephemeral_storage {

namespace {

using PerTLDEphemeralStorageMap =
    std::map<PerTLDEphemeralStorageKey, base::WeakPtr<PerTLDEphemeralStorage>>;

PerTLDEphemeralStorageMap& active_per_tld_storage_areas() {
  static base::NoDestructor<PerTLDEphemeralStorageMap> active_storage_areas;
  return *active_storage_areas.get();
}

std::string URLToStorageDomain(const GURL& url) {
  std::string domain = net::registry_controlled_domains::GetDomainAndRegistry(
      url, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);

  // GetDomainAndRegistry might return an empty string if this host is an IP
  // address or a file URL.
  if (domain.empty())
    domain = url::Origin::Create(url.GetOrigin()).Serialize();

  return domain;
}

// Session storage ids are expected to be 36 character long GUID strings. Since
// we are constructing our own ids, we convert our string into a 32 character
// hash and then use that make up our own GUID-like string. Because of the way
// we are constructing the string we should never collide with a real GUID and
// we only need to worry about hash collisions, which are unlikely.
std::string StringToSessionStorageId(const std::string& string) {
  std::string hash = base::MD5String(string) + "____";
  DCHECK_EQ(hash.size(), 36u);
  return hash;
}

}  // namespace

PerTLDEphemeralStorage::PerTLDEphemeralStorage(
    PerTLDEphemeralStorageKey key,
    scoped_refptr<content::SessionStorageNamespace> local_storage_namespace)
    : key_(key), local_storage_namespace_(local_storage_namespace) {
  DCHECK(active_per_tld_storage_areas().find(key) ==
         active_per_tld_storage_areas().end());
  active_per_tld_storage_areas().emplace(key, weak_factory_.GetWeakPtr());
}

PerTLDEphemeralStorage::~PerTLDEphemeralStorage() {
  active_per_tld_storage_areas().erase(key_);
}

EphemeralStorageTabHelper::EphemeralStorageTabHelper(WebContents* web_contents)
    : WebContentsObserver(web_contents) {
  DCHECK(base::FeatureList::IsEnabled(blink::features::kBraveEphemeralStorage));
}

EphemeralStorageTabHelper::~EphemeralStorageTabHelper() {}

void EphemeralStorageTabHelper::ReadyToCommitNavigation(
    NavigationHandle* navigation_handle) {
  if (!navigation_handle->IsInMainFrame())
    return;
  if (navigation_handle->IsSameDocument())
    return;

  const GURL& new_url = navigation_handle->GetURL();
  std::string new_domain = URLToStorageDomain(new_url);
  std::string previous_domain =
      URLToStorageDomain(web_contents()->GetLastCommittedURL());
  if (new_domain == previous_domain)
    return;

  auto* browser_context = web_contents()->GetBrowserContext();
  auto site_instance =
      content::SiteInstance::CreateForURL(browser_context, new_url);
  auto* partition =
      BrowserContext::GetStoragePartition(browser_context, site_instance.get());

  // If another EphemeralStorageTabHelper is already using a storage area for
  // our TLD, we can use that.  Otherwise, we create a fresh storage area.
  PerTLDEphemeralStorageKey key = std::make_pair(browser_context, new_domain);
  auto it = active_per_tld_storage_areas().find(key);
  if (it != active_per_tld_storage_areas().end()) {
    DCHECK(!it->second.WasInvalidated());

    // The map stores a weak pointer and we are upgrading it to a scoped_refptr
    // here.
    per_tld_ephemeral_storage_ = it->second.get();
  } else {
    std::string local_partition_id = new_domain + "/ephemeral-local-storage";
    auto local_storage_namespace = content::CreateSessionStorageNamespace(
        partition, StringToSessionStorageId(local_partition_id));
    per_tld_ephemeral_storage_ = base::MakeRefCounted<PerTLDEphemeralStorage>(
        key, local_storage_namespace);
  }

  // Session storage is always per-tab and never per-TLD, so we always delete
  // and recreate the session storage when switching domains.
  //
  // We need to explicitly release the storage namespace before recreating a
  // new one in order to make sure that we remove the final reference and free
  // it.
  session_storage_namespace_.reset();

  std::string session_partition_id =
      content::GetSessionStorageNamespaceId(web_contents()) +
      "/ephemeral-session-storage";
  session_storage_namespace_ = content::CreateSessionStorageNamespace(
      partition, StringToSessionStorageId(session_partition_id));
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(EphemeralStorageTabHelper)

}  // namespace ephemeral_storage
