/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_SERVER_API_API_UTIL_H_
#define BRAVELEDGER_SERVER_API_API_UTIL_H_

#include <string>

namespace ledger {
namespace server {
namespace api {

std::string GetServerUrl(const std::string& path);

}  // namespace api
}  // namespace server
}  // namespace ledger

#endif  // BRAVELEDGER_SERVER_API_API_UTIL_H_
