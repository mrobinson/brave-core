/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define BRAVE_STRUCTTRAITSCOOKIEOPTIONS_READ             \
  GURL top_frame_url_for_ephemeral_storage;              \
  if (!mojo_options.ReadTopFrameUrlForEphemeralStorage(  \
          &top_frame_url_for_ephemeral_storage))         \
    return false;                                        \
  cookie_options->top_frame_url_for_ephemeral_storage_ = \
      top_frame_url_for_ephemeral_storage;

#include "../../../../../services/network/public/cpp/cookie_manager_mojom_traits.cc"
