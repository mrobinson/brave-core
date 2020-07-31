/* Copyright 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/metrics/desktop_session_duration/desktop_profile_session_durations_service.h"

#include "brave/components/sync/driver/brave_sync_session_durations_metrics_recorder.h"

#define SyncSessionDurationsMetricsRecorder \
  BraveSyncSessionDurationsMetricsRecorder
#include "../../../../../../chrome/browser/metrics/desktop_session_duration/desktop_profile_session_durations_service.cc"
#undef SyncSessionDurationsMetricsRecorder
