/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/playlists/playlists_service.h"

#include <string>
#include <utility>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/memory/scoped_refptr.h"
#include "base/task/post_task.h"
#include "base/task_runner_util.h"
#include "brave/components/playlists/browser/playlists_controller.h"
#include "content/public/browser/browser_context.h"

#if !defined(OS_ANDROID)
#include "brave/browser/playlists/desktop_playlists_player.h"
#include "chrome/browser/profiles/profile.h"
#endif

namespace brave_playlists {

PlaylistsService::PlaylistsService(content::BrowserContext* context) {
  // TODO(simonhong): Remove this after router/player are moved to chrome layer.
  // Then, Controller will become service.
  scoped_refptr<base::SequencedTaskRunner> io_task_runner =
      base::CreateSequencedTaskRunner(
          { base::ThreadPool(), base::MayBlock(),
            base::TaskPriority::BEST_EFFORT,
            base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN });
  controller_.reset(new PlaylistsController(context, io_task_runner));

#if !defined(OS_ANDROID)
  playlists_player_.reset(
      new DesktopPlaylistsPlayer(Profile::FromBrowserContext(context),
                                 io_task_runner));
  controller_->set_player(playlists_player_.get());
#endif
}

PlaylistsService::~PlaylistsService() {
  controller_->set_player(nullptr);
}

}  // namespace brave_playlists
