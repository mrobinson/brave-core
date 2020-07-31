/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_SYNC_DRIVER_BRAVE_SYNC_SESSION_DURATIONS_METRICS_RECODER_H_  // NOLINT
#define BRAVE_COMPONENTS_SYNC_DRIVER_BRAVE_SYNC_SESSION_DURATIONS_METRICS_RECODER_H_  // NOLINT

#include "components/sync/driver/sync_service.h"
#include "components/sync/driver/sync_session_durations_metrics_recorder.h"

namespace syncer {

class BraveSyncSessionDurationsMetricsRecorder
    : public syncer::SyncServiceObserver,
      public signin::IdentityManager::Observer {
 public:
  BraveSyncSessionDurationsMetricsRecorder(
      SyncService* sync_service,
      signin::IdentityManager* identity_manager);
  ~BraveSyncSessionDurationsMetricsRecorder() override;

  void OnSessionStarted(base::TimeTicks session_start) {}
  void OnSessionEnded(base::TimeDelta session_length) {}

  void OnStateChanged(syncer::SyncService* sync) override {}
  void OnRefreshTokenUpdatedForAccount(
      const CoreAccountInfo& account_info) override {}
  void OnRefreshTokenRemovedForAccount(
      const CoreAccountId& account_id) override {}
  void OnRefreshTokensLoaded() override {}
  void OnErrorStateOfRefreshTokenUpdatedForAccount(
      const CoreAccountInfo& account_info,
      const GoogleServiceAuthError& error) override {}
  void OnAccountsInCookieUpdated(
      const signin::AccountsInCookieJarInfo& accounts_in_cookie_jar_info,
      const GoogleServiceAuthError& error) override {}
};

}  // namespace syncer

#endif  // BRAVE_COMPONENTS_SYNC_DRIVER_BRAVE_SYNC_SESSION_DURATIONS_METRICS_RECODER_H_  // NOLINT