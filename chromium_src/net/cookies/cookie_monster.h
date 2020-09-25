/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_NET_COOKIES_COOKIE_MONSTER_H_
#define BRAVE_CHROMIUM_SRC_NET_COOKIES_COOKIE_MONSTER_H_

#define BRAVE_COOKIE_MONSTER_H                             \
  std::map<std::string, std::unique_ptr<CookieMonster>>    \
      ephemeral_cookie_stores_;                            \
  CookieMonster* GetOrCreateEphemeralCookieStoreForDomain( \
      const std::string& domain);

#include "../../../../../net/cookies/cookie_monster.h"

#undef BRAVE_COOKIE_MONSTER_H

#endif  // BRAVE_CHROMIUM_SRC_NET_COOKIES_COOKIE_MONSTER_H_
