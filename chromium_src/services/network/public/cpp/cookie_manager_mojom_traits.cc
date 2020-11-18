/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define BRAVE_STRUCTTRAITSCOOKIEOPTIONS_READ         \
  GURL top_frame_url;                                \
  if (!mojo_options.ReadTopFrameUrl(&top_frame_url)) \
    return false;                                    \
  cookie_options->top_frame_url_ = top_frame_url;

#include "../../../../../services/network/public/cpp/cookie_manager_mojom_traits.cc"
