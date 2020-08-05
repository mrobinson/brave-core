/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.base;

import android.content.SharedPreferences;
import android.util.Log;

import org.chromium.base.ContextUtils;

public abstract class BraveCommandLine {
    // Duplicate constant to avoid pull dependancy into base
    private static final String PREF_QA_VLOG_REWARDS = "qa_vlog_rewards";
    public static char[] getInitialCommandLine() {
        char[] buffer = null;
        SharedPreferences sharedPreferences = ContextUtils.getAppSharedPreferences();
        if (sharedPreferences.getBoolean(PREF_QA_VLOG_REWARDS, false)) {
            String bufferS = "--enable-logging=stderr --vmodule=\"*/bat-native-ads/*\"=6,\"*/brave_ads/*\"=6,\"*/brave_user_model/*\"=6,\"*/bat_ads/*\"=6,\"*/bat-native-ledger/*\"=6,\"*/brave_rewards/*\"=6";
            buffer = bufferS.toCharArray();
        }
        return buffer;
    }
}
