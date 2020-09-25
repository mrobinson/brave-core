/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "services/network/restricted_cookie_manager.h"

#include "net/base/url_util.h"
#include "net/base/features.h"

namespace {

void AddEphemeralStorageDomainIfNecessary(const GURL& url,
                                          const url::Origin& top_frame_origin,
                                          net::CookieOptions* options) {
  if (!base::FeatureList::IsEnabled(net::features::kBraveEphemeralStorage))
    return;
  if (url::Origin::Create(url) == top_frame_origin)
    return;
  options->ephemeral_storage_domain_ =
      net::URLToEphemeralStorageDomain(top_frame_origin.GetURL());
}

}  // namespace

#define BRAVE_GETALLFORURL \
  AddEphemeralStorageDomainIfNecessary(url, top_frame_origin, &net_options);

#define BRAVE_SETCANONICALCOOKIE \
  AddEphemeralStorageDomainIfNecessary(url, top_frame_origin, &options);

#include "../../../../../services/network/restricted_cookie_manager.cc"
