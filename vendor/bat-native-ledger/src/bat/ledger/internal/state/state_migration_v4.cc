/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <utility>

#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/state/state_keys.h"
#include "bat/ledger/internal/state/state_migration_v4.h"

namespace braveledger_state {

StateMigrationV4::StateMigrationV4(bat_ledger::LedgerImpl* ledger) :
    ledger_(ledger) {
}

StateMigrationV4::~StateMigrationV4() = default;

void StateMigrationV4::Migrate(ledger::ResultCallback callback) {
  const auto seed = ledger_->ledger_client()->GetStringState(
      ledger::kStateRecoverySeed);
  if (seed.empty()) {
    callback(ledger::Result::LEDGER_OK);
    return;
  }

  // Auto contribute
  auto enabled = ledger_->ledger_client()->GetBooleanState(
      ledger::kStateAutoContributeEnabled);
  ledger_->database()->SaveEventLog(
      ledger::kStateAutoContributeEnabled,
      std::to_string(enabled));

  // Seed
  if (seed.size() > 1) {
    const std::string event_string = std::to_string(seed[0]+seed[1]);
    ledger_->database()->SaveEventLog(ledger::kStateRecoverySeed, event_string);
  }

  // Payment id
  ledger_->database()->SaveEventLog(
      ledger::kStatePaymentId,
      ledger_->ledger_client()->GetStringState(ledger::kStatePaymentId));

  // Enabled
  enabled = ledger_->ledger_client()->GetBooleanState(ledger::kStateEnabled);
  ledger_->database()->SaveEventLog(
      ledger::kStateEnabled,
      std::to_string(enabled));

  // Next reconcile
  const auto reconcile_stamp = ledger_->ledger_client()->GetUint64State(
      ledger::kStateNextReconcileStamp);
  ledger_->database()->SaveEventLog(
      ledger::kStateNextReconcileStamp,
      std::to_string(reconcile_stamp));

  // Creation stamp
  const auto creation_stamp =
      ledger_->ledger_client()->GetUint64State(ledger::kStateCreationStamp);
  ledger_->database()->SaveEventLog(
      ledger::kStateCreationStamp,
      std::to_string(creation_stamp));

  callback(ledger::Result::LEDGER_OK);
}

}  // namespace braveledger_state
