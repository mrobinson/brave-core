/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_SERVER_API_API_SERVER_H_
#define BRAVELEDGER_SERVER_API_API_SERVER_H_

#include <memory>

#include "bat/ledger/ledger.h"
#include "bat/ledger/internal/server/api/endpoints/get_parameters.h"

namespace bat_ledger {
class LedgerImpl;
}

namespace ledger {
namespace server {
namespace api {

class Server {
 public:
  explicit Server(bat_ledger::LedgerImpl* ledger);
  ~Server();

  GetParameters* get_parameters() const;

 private:
  bat_ledger::LedgerImpl* ledger_;  // NOT OWNED
  std::unique_ptr<GetParameters> get_parameters_;
};

}  // namespace api
}  // namespace server
}  // namespace ledger

#endif  // BRAVELEDGER_SERVER_API_API_SERVER_H_
