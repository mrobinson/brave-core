/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ephemeral_storage/ephemeral_storage_tab_helper.h"

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
  return net::registry_controlled_domains::GetDomainAndRegistry(
      url, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

}  // namespace
namespace ephemeral_storage {

EphemeralStorageTabHelper::~EphemeralStorageTabHelper() {}

EphemeralStorageTabHelper::EphemeralStorageTabHelper(WebContents* web_contents)
    : WebContentsObserver(web_contents) {}

void EphemeralStorageTabHelper::ReadyToCommitNavigation(
    NavigationHandle* navigation_handle) {
  if (!navigation_handle->IsInMainFrame())
    return;
  if (navigation_handle->IsSameDocument())
    return;

  auto* browser_context = web_contents()->GetBrowserContext();
  auto site_instance = content::SiteInstance::CreateForURL(
      browser_context, navigation_handle->GetURL());
  auto* partition =
      BrowserContext::GetStoragePartition(browser_context, site_instance.get());

  std::string domain = URLToStorageDomain(navigation_handle->GetURL());
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

  ClearEphemeralStorageIfNecessary(domain);
}

void EphemeralStorageTabHelper::WebContentsDestroyed() {
  ClearEphemeralStorageIfNecessary(base::nullopt);
}

bool EphemeralStorageTabHelper::IsAnotherTabOpenWithStorageDomain(
    const std::string& storage_domain) {
  for (Browser* browser : *BrowserList::GetInstance()) {
    if (browser->profile() != web_contents()->GetBrowserContext())
      continue;

    TabStripModel* tab_strip = browser->tab_strip_model();
    for (int i = 0; i < tab_strip->count(); ++i) {
      WebContents* contents = tab_strip->GetWebContentsAt(i);
      const GURL& url = contents->GetLastCommittedURL();
      if (contents != web_contents() &&
          URLToStorageDomain(url) == storage_domain) {
        return true;
      }
    }
  }

  return false;
}

void EphemeralStorageTabHelper::ClearEphemeralStorageIfNecessary(
    base::Optional<std::string> new_domain) {
  if (!base::FeatureList::IsEnabled(blink::features::kBraveEphemeralStorage)) {
    return;
  }

  const GURL& url = web_contents()->GetLastCommittedURL();
  std::string domain = URLToStorageDomain(url);
  if (!IsAnotherTabOpenWithStorageDomain(domain)) {
    local_storage_namespace_map().erase(domain + "/ephemeral-local-storage");
  }

  if (!new_domain.has_value() || *new_domain != domain) {
    session_storage_namespace_map().erase(
        content::GetSessionStorageNamespaceId(web_contents()) +
        "/ephemeral-session-storage");
  }
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(EphemeralStorageTabHelper)

}  // namespace ephemeral_storage
