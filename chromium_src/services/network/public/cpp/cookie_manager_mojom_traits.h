/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_SERVICES_NETWORK_PUBLIC_CPP_COOKIE_MANAGER_MOJOM_TRAITS_H_
#define BRAVE_CHROMIUM_SRC_SERVICES_NETWORK_PUBLIC_CPP_COOKIE_MANAGER_MOJOM_TRAITS_H_

#define BRAVE_STRUCTTRAITSCOOKIEOPTIONSEPHEMERALSTORAGEDOMAIN                \
  static std::string ephemeral_storage_domain(const net::CookieOptions& o) { \
    return o.ephemeral_storage_domain_;                                      \
  }

#include "../../../../../services/network/public/cpp/cookie_manager_mojom_traits.h"

#undef BRAVE_STRUCTTRAITSCOOKIEOPTIONSEPHEMERALSTORAGEDOMAIN

#endif  // BRAVE_CHROMIUM_SRC_SERVICES_NETWORK_PUBLIC_CPP_COOKIE_MANAGER_MOJOM_TRAITS_H_
