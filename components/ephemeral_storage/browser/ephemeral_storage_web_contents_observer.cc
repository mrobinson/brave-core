
/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/ephemeral_storage/browser/ephemeral_storage_web_contents_observer.h"

#include "content/browser/storage_partition_impl_map.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using content::BrowserContext;
using content::NavigationHandle;
using content::RenderFrameHost;
using content::RenderViewHost;
using content::StoragePartition;
using content::StoragePartitionConfig;
using content::StoragePartitionImplMap;
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
  RenderFrameHost* render_frame_host =
      navigation_handle->GetRenderFrameHost();

  StoragePartitionConfig config = GetEphemeralStorageConfig(
      render_frame_host->GetRenderViewHost());
  auto* partition = BrowserContext::GetStoragePartition(
      render_frame_host->GetBrowserContext(), config, false /* can_create */);
  if (!partition)
    return;

  partition->ClearData(StoragePartition::REMOVE_DATA_MASK_ALL,
                       StoragePartition::QUOTA_MANAGED_STORAGE_MASK_ALL,
                       GURL(), base::Time(), base::Time::Max(), base::DoNothing());
}

void EphemeralStorageWebContentsObserver::WebContentsDestroyed() {
  WebContents* contents = web_contents();
  BrowserContext* browser_context = contents->GetBrowserContext();
  StoragePartitionImplMap* partition_map =
      browser_context->GetStoragePartitionImplMap();
  if (partition_map) {
      partition_map->DeleteInMemory(
          GetEphemeralStorageConfig(contents->GetRenderViewHost()));
  }
}

// This must be kept in sync with the version in RenderViewHostImpl. This
// is duplicated in order to reduce the patch size.
// static
StoragePartitionConfig EphemeralStorageWebContentsObserver::GetEphemeralStorageConfig(
    RenderViewHost* host) {
  std::string domain = std::string("ephemeral-") +
      base::NumberToString(host->GetRoutingID());
  std::string name;
  return StoragePartitionConfig::Create(domain, name, true /* in_memory */);
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(EphemeralStorageWebContentsObserver)

}  // namespace ephemeral_storage
