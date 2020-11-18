/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "net/cookies/cookie_monster.h"

#include <iostream>
#include <memory>
#include "net/base/url_util.h"

#define CookieMonster SimpleCookieMonster
#include "../../../../net/cookies/cookie_monster.cc"
#undef CookieMonster

namespace net {

namespace {

CookieOptions OptionsWithoutEphemeralStorageURL(const CookieOptions& options) {
  CookieOptions new_options(options);
  new_options.top_frame_url_ = GURL();
  return new_options;
}

}  // namespace

CookieMonster::CookieMonster(scoped_refptr<PersistentCookieStore> store,
                             NetLog* net_log)
    : SimpleCookieMonster(store, net_log),
      net_log_(
          NetLogWithSource::Make(net_log, NetLogSourceType::COOKIE_STORE)) {}

CookieMonster::CookieMonster(scoped_refptr<PersistentCookieStore> store,
                             base::TimeDelta last_access_threshold,
                             NetLog* net_log)
    : SimpleCookieMonster(store, last_access_threshold, net_log),
      net_log_(
          NetLogWithSource::Make(net_log, NetLogSourceType::COOKIE_STORE)) {}

CookieMonster::~CookieMonster() {}

SimpleCookieMonster*
CookieMonster::GetOrCreateEphemeralCookieStoreForTopFrameURL(
    const GURL& top_frame_url) {
  std::string domain = URLToEphemeralStorageDomain(top_frame_url);
  auto it = ephemeral_cookie_stores_.find(domain);
  if (it != ephemeral_cookie_stores_.end())
    return it->second.get();

  return ephemeral_cookie_stores_
      .emplace(domain,
               new SimpleCookieMonster(nullptr /* store */, net_log_.net_log()))
      .first->second.get();
}

void CookieMonster::SetCanonicalCookieAsync(
    std::unique_ptr<CanonicalCookie> cookie,
    const GURL& source_url,
    const CookieOptions& options,
    SetCookiesCallback callback) {
  if (!options.top_frame_url_.is_empty()) {
    SimpleCookieMonster* ephemeral_monster =
        GetOrCreateEphemeralCookieStoreForTopFrameURL(options.top_frame_url_);
    ephemeral_monster->SetCanonicalCookieAsync(
        std::move(cookie), source_url,
        OptionsWithoutEphemeralStorageURL(options), std::move(callback));
    return;
  }
  SimpleCookieMonster::SetCanonicalCookieAsync(std::move(cookie), source_url,
                                               options, std::move(callback));
}

void CookieMonster::GetCookieListWithOptionsAsync(
    const GURL& url,
    const CookieOptions& options,
    GetCookieListCallback callback) {
  if (!options.top_frame_url_.is_empty()) {
    SimpleCookieMonster* ephemeral_monster =
        GetOrCreateEphemeralCookieStoreForTopFrameURL(options.top_frame_url_);
    ephemeral_monster->GetCookieListWithOptionsAsync(
        url, OptionsWithoutEphemeralStorageURL(options), std::move(callback));
    return;
  }
  SimpleCookieMonster::GetCookieListWithOptionsAsync(url, options,
                                                     std::move(callback));
}

void CookieMonster::DeleteCanonicalCookieAsync(const CanonicalCookie& cookie,
                                               DeleteCallback callback) {
  SimpleCookieMonster::DeleteCanonicalCookieAsync(cookie, std::move(callback));
}

void CookieMonster::DeleteAllCreatedInTimeRangeAsync(
    const CookieDeletionInfo::TimeRange& creation_range,
    DeleteCallback callback) {
  for (auto& it : ephemeral_cookie_stores_) {
    it.second->DeleteAllCreatedInTimeRangeAsync(creation_range,
                                                DeleteCallback());
  }
  SimpleCookieMonster::DeleteAllCreatedInTimeRangeAsync(creation_range,
                                                        std::move(callback));
}

void CookieMonster::DeleteAllMatchingInfoAsync(CookieDeletionInfo delete_info,
                                               DeleteCallback callback) {
  if (delete_info.ephemeral_storage_domain.has_value()) {
    ephemeral_cookie_stores_.erase(*delete_info.ephemeral_storage_domain);
    std::move(callback).Run(0);
    return;
  }

  for (auto& it : ephemeral_cookie_stores_) {
    it.second->DeleteAllMatchingInfoAsync(delete_info, DeleteCallback());
  }
  SimpleCookieMonster::DeleteAllMatchingInfoAsync(delete_info,
                                                  std::move(callback));
}

void CookieMonster::DeleteSessionCookiesAsync(DeleteCallback callback) {
  for (auto& it : ephemeral_cookie_stores_) {
    it.second->DeleteSessionCookiesAsync(DeleteCallback());
  }
  SimpleCookieMonster::DeleteSessionCookiesAsync(std::move(callback));
}

void CookieMonster::SetCookieableSchemes(
    const std::vector<std::string>& schemes,
    SetCookieableSchemesCallback callback) {
  for (auto& it : ephemeral_cookie_stores_) {
    it.second->SetCookieableSchemes(schemes, SetCookieableSchemesCallback());
  }
  SimpleCookieMonster::SetCookieableSchemes(schemes, std::move(callback));
}

}  // namespace net
