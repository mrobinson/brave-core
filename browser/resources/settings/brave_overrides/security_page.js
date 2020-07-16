// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import {RegisterPolymerTemplateModifications} from 'chrome://brave-resources/polymer_overriding.js'

RegisterPolymerTemplateModifications({
  'settings-security-page': (templateContent) => {
    const link = templateContent.getElementById('advanced-protection-program-link')
    if (!link) {
      console.error('[Brave Settings Overrides] Could not find advanced-protection-program-link id on security page.')
    } else {
      link.setAttribute('hidden', 'true')
    }
    const toggle = templateContent.getElementById('passwordsLeakToggle')
    if (!toggle) {
      console.error('[Brave Settings Overrides] Could not find passwordsLeakToggle id on security page.')
    } else {
      toggle.setAttribute('hidden', 'true')
    }
  }
})
