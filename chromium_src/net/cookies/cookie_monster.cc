/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "net/cookies/cookie_monster.h"

#include <iostream>
#include <memory>

namespace net {

CookieMonster* CookieMonster::GetOrCreateEphemeralCookieStoreForDomain(
    const std::string& domain) {
  auto it = ephemeral_cookie_stores_.find(domain);
  if (it != ephemeral_cookie_stores_.end())
    return it->second.get();

  return ephemeral_cookie_stores_
      .emplace(domain,
               new CookieMonster(nullptr /* store */, net_log_.net_log()))
      .first->second.get();
}

}  // namespace net

#define BRAVE_SETCANONICALCOOKIE                                             \
  if (!options.ephemeral_storage_domain_.empty()) {                          \
    CookieMonster* ephemeral_monster =                                       \
        GetOrCreateEphemeralCookieStoreForDomain(                            \
            options.ephemeral_storage_domain_);                              \
    CookieOptions new_options(options);                                      \
    new_options.ephemeral_storage_domain_ = std::string();                   \
    ephemeral_monster->SetCanonicalCookie(std::move(cc), source_url,         \
                                          new_options, std::move(callback)); \
    return;                                                                  \
  }

#define BRAVE_GETCOOKIELISTWITHOPTIONS                                \
  if (!options.ephemeral_storage_domain_.empty()) {                   \
    CookieMonster* ephemeral_monster =                                \
        GetOrCreateEphemeralCookieStoreForDomain(                     \
            options.ephemeral_storage_domain_);                       \
    CookieOptions new_options(options);                               \
    new_options.ephemeral_storage_domain_ = std::string();            \
    ephemeral_monster->GetCookieListWithOptions(url, new_options,     \
                                                std::move(callback)); \
    return;                                                           \
  }

#define BRAVE_DELETEALLCREATEDINTIMERANGE                                     \
  for (auto& it : ephemeral_cookie_stores_) {                                 \
    it.second->DeleteAllCreatedInTimeRange(creation_range, DeleteCallback()); \
  }

#define BRAVE_DELETESESSIONCOOKIES                     \
  for (auto& it : ephemeral_cookie_stores_) {          \
    it.second->DeleteSessionCookies(DeleteCallback()); \
  }

// If the special ephemeral_storage_domain property is set on the deletion
// filter, we want to just delete all the cookies in that ephemeral storage
// domain and ignore the rest of the filter. This avoids having to add another
// interface to the CookieStore.
#define BRAVE_DELETEALLMATCHINGINFO                                        \
  if (delete_info.ephemeral_storage_domain.has_value()) {                  \
    ephemeral_cookie_stores_.erase(*delete_info.ephemeral_storage_domain); \
    std::move(callback).Run(0);                                            \
    return;                                                                \
  }                                                                        \
  for (auto& it : ephemeral_cookie_stores_) {                              \
    it.second->DeleteAllMatchingInfo(delete_info, DeleteCallback());       \
  }

#define BRAVE_DELETECANONICALCOOKIE                             \
  for (auto& it : ephemeral_cookie_stores_) {                   \
    it.second->DeleteCanonicalCookie(cookie, DeleteCallback()); \
  }

#include "../../../../net/cookies/cookie_monster.cc"

#undef BRAVE_DELETECANONICALCOOKIE
#undef BRAVE_DELETEALLMATCHINGINFO
#undef BRAVE_DELETEALLCREATEDINTIMERANGE
#undef BRAVE_GETCOOKIELISTWITHOPTIONS
#undef BRAVE_GETCOOKIELISTWITHOPTIONS
#undef BRAVE_SETCANONICALCOOKIE
