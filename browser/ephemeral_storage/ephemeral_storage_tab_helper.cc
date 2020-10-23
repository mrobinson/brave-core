/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ephemeral_storage/ephemeral_storage_tab_helper.h"

#include <set>
#include <string>
#include <utility>

#include "base/feature_list.h"
#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/session_storage_namespace.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "third_party/blink/public/common/features.h"

#if defined(OS_ANDROID)
#include "chrome/browser/android/tab_android.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#endif

using content::BrowserContext;
using content::NavigationHandle;
using content::WebContents;

namespace {

content::SessionStorageNamespaceMap& session_storage_namespace_map() {
  static base::NoDestructor<content::SessionStorageNamespaceMap>
      session_storage_namespace_map;
  return *session_storage_namespace_map.get();
}

content::SessionStorageNamespaceMap& local_storage_namespace_map() {
  static base::NoDestructor<content::SessionStorageNamespaceMap>
      local_storage_namespace_map;
  return *local_storage_namespace_map.get();
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

}  // namespace
namespace ephemeral_storage {

EphemeralStorageTabHelper::~EphemeralStorageTabHelper() {}

EphemeralStorageTabHelper::EphemeralStorageTabHelper(WebContents* web_contents)
    : WebContentsObserver(web_contents) {}

void EphemeralStorageTabHelper::ReadyToCommitNavigation(
    NavigationHandle* navigation_handle) {
  if (!base::FeatureList::IsEnabled(blink::features::kBraveEphemeralStorage))
    return;
  if (!navigation_handle->IsInMainFrame())
    return;
  if (navigation_handle->IsSameDocument())
    return;

  std::string domain = URLToStorageDomain(navigation_handle->GetURL());
  std::string previous_domain =
      URLToStorageDomain(web_contents()->GetLastCommittedURL());
  if (domain == previous_domain)
    return;

  ClearEphemeralStorageIfNecessary();

  auto* browser_context = web_contents()->GetBrowserContext();
  auto site_instance = content::SiteInstance::CreateForURL(
      browser_context, navigation_handle->GetURL());
  auto* partition =
      BrowserContext::GetStoragePartition(browser_context, site_instance.get());

  std::string local_partition_id = domain + "/ephemeral-local-storage";
  content::SessionStorageNamespaceMap::const_iterator it =
      local_storage_namespace_map().find(local_partition_id);
  if (it == local_storage_namespace_map().end()) {
    auto local_storage_namespace =
        content::CreateSessionStorageNamespace(partition, local_partition_id);
    local_storage_namespace_map()[local_partition_id] =
        std::move(local_storage_namespace);
  }

  std::string session_partition_id =
      content::GetSessionStorageNamespaceId(web_contents()) +
      "/ephemeral-session-storage";
  it = session_storage_namespace_map().find(session_partition_id);
  if (it == session_storage_namespace_map().end()) {
    auto session_storage_namespace =
        content::CreateSessionStorageNamespace(partition, session_partition_id);
    session_storage_namespace_map()[session_partition_id] =
        std::move(session_storage_namespace);
  }
}

void EphemeralStorageTabHelper::WebContentsDestroyed() {
  ClearEphemeralStorageIfNecessary();
}

bool EphemeralStorageTabHelper::IsDifferentWebContentsWithMatchingStorageDomain(
    WebContents* other_web_contents,
    const std::string& storage_domain) {
  DCHECK(other_web_contents);
  if (web_contents() == other_web_contents)
    return false;

  std::string other_storage_domain =
      URLToStorageDomain(other_web_contents->GetLastCommittedURL());
  return storage_domain == other_storage_domain;
}

bool EphemeralStorageTabHelper::IsAnotherTabOpenWithStorageDomain(
    const std::string& storage_domain) {
#if defined(OS_ANDROID)
  for (auto it = TabModelList::begin(); it != TabModelList::end(); ++it) {
    TabModel* tab_model = *it;
    if (tab_model->GetProfile() == profile) {
      for (int i = 0; i < tab_model->GetTabCount(); i++) {
        TabAndroid* tab = tab_model->GetTabAt(i);
        WebContents* web_contents = tab->web_contents();
        if (IsDifferentWebContentsWithMatchingStorageDomain(web_contents,
                                                            storage_domain))
          return true;
      }
    }
  }
#else
  for (Browser* browser : *BrowserList::GetInstance()) {
    if (browser->profile() == web_contents()->GetBrowserContext()) {
      TabStripModel* tab_strip = browser->tab_strip_model();
      for (int i = 0; i < tab_strip->count(); ++i) {
        WebContents* web_contents = tab_strip->GetWebContentsAt(i);
        if (IsDifferentWebContentsWithMatchingStorageDomain(web_contents,
                                                            storage_domain))
          return true;
      }
    }
  }
#endif

  return false;
}

void EphemeralStorageTabHelper::ClearEphemeralStorageIfNecessary() {
  if (!base::FeatureList::IsEnabled(blink::features::kBraveEphemeralStorage)) {
    return;
  }

  std::string previous_domain =
      URLToStorageDomain(web_contents()->GetLastCommittedURL());
  if (!IsAnotherTabOpenWithStorageDomain(previous_domain)) {
    local_storage_namespace_map().erase(previous_domain +
                                        "/ephemeral-local-storage");
  }

  // This method should only be called if we are navigating to a new domain or
  // shutting down this tab helper. This means that it is always appropriate to
  // clear the ephemeral session storage.
  session_storage_namespace_map().erase(
      content::GetSessionStorageNamespaceId(web_contents()) +
      "/ephemeral-session-storage");
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(EphemeralStorageTabHelper)

}  // namespace ephemeral_storage
