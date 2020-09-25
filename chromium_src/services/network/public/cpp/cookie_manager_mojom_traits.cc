/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define BRAVE_STRUCTTRAITSCOOKIEOPTIONS_READ                               \
  std::string ephemeral_storage_domain;                                    \
  if (!mojo_options.ReadEphemeralStorageDomain(&ephemeral_storage_domain)) \
    return false;                                                          \
  cookie_options->ephemeral_storage_domain_ = ephemeral_storage_domain;

#include "../../../../../services/network/public/cpp/cookie_manager_mojom_traits.cc"
