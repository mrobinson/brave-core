/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ephemeral_storage/browser/ephemeral_storage_web_contents_observer.h"

#include "content/browser/dom_storage/dom_storage_context_wrapper.h"
#include "content/browser/renderer_host/render_view_host_delegate.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/storage_partition_impl_map.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using content::BrowserContext;
using content::DOMStorageContextWrapper;
using content::NavigationHandle;
using content::SiteInstance;
using content::StoragePartition;
using content::WebContents;

namespace ephemeral_storage {

EphemeralStorageWebContentsObserver::~EphemeralStorageWebContentsObserver() {
}

EphemeralStorageWebContentsObserver::EphemeralStorageWebContentsObserver(
    WebContents* web_contents)
    : WebContentsObserver(web_contents) {
}

void EphemeralStorageWebContentsObserver::ReadyToCommitNavigation(
      NavigationHandle* navigation_handle) {
  if (!navigation_handle->IsInMainFrame())
    return;
  if (!navigation_handle->IsSameDocument())
    return;

  ClearEphemeralStorage();
}

void EphemeralStorageWebContentsObserver::WebContentsDestroyed() {
  ClearEphemeralStorage();
}

void EphemeralStorageWebContentsObserver::ClearEphemeralStorage() {
  WebContents* contents = web_contents();
  BrowserContext* browser_context = contents->GetBrowserContext();
  SiteInstance* site_instance = contents->GetSiteInstance();
  StoragePartition* storage_partition = BrowserContext::GetStoragePartition(
      browser_context, site_instance);
  DOMStorageContextWrapper* dom_storage_context =
      static_cast<DOMStorageContextWrapper*>(
          storage_partition->GetDOMStorageContext());

  storage::mojom::SessionStorageControl* session_storage_control =
      dom_storage_context->GetSessionStorageControl();
  if (!session_storage_control)
    return;

  std::string session_namespace_id =
      contents->GetRenderViewHost()->GetDelegate()->GetSessionStorageNamespace(
              site_instance)->id() + "ephemeral-session-storage";
  session_storage_control->DeleteNamespace(
       session_namespace_id, false /* should_persist */);

  std::string local_namespace_id =
      contents->GetRenderViewHost()->GetDelegate()->GetSessionStorageNamespace(
              site_instance)->id() + "ephemeral-local-storage";
  session_storage_control->DeleteNamespace(
        local_namespace_id, false /* should_persist */);
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(EphemeralStorageWebContentsObserver)

}  // namespace ephemeral_storage
