/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_BROWSER_PUBLISHER_INFO_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_BROWSER_PUBLISHER_INFO_H_

#include <stdint.h>
#include <string>

namespace brave_rewards {

struct PublisherInfo {
  PublisherInfo();
  ~PublisherInfo();
  PublisherInfo(const PublisherInfo& info);

  std::string id;
  uint64_t duration;
  double score;
  uint32_t visits;
  uint32_t percent;
  double weight;
  uint32_t excluded;
  int32_t category;
  uint64_t reconcile_stamp;
  uint32_t status;
  uint64_t status_updated_at;
  std::string name;
  std::string url;
  std::string provider;
  std::string favicon_url;
};

}  // namespace brave_rewards

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_BROWSER_PUBLISHER_INFO_H_
