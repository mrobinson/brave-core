/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/playlists/playlists_service.h"

#include <string>
#include <utility>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/task/post_task.h"
#include "base/task_runner_util.h"
#include "brave/components/playlists/browser/playlists_controller.h"
#include "content/public/browser/browser_context.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "brave/browser/extensions/brave_playlists_event_router.h"
#endif

namespace brave_playlists {
namespace {

const base::FilePath::StringType kBaseDirName(FILE_PATH_LITERAL("playlists"));

}  // namespace

PlaylistsService::PlaylistsService(content::BrowserContext* context)
    : base_dir_(context->GetPath().Append(kBaseDirName)),
      controller_(new PlaylistsController(context)),
      weak_factory_(this) {

#if BUILDFLAG(ENABLE_EXTENSIONS)
  playlists_event_router_.reset(new BravePlaylistsEventRouter(context));
  controller_->AddObserver(playlists_event_router_.get());
#endif
}

PlaylistsService::~PlaylistsService() {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  controller_->RemoveObserver(playlists_event_router_.get());
#endif
}

bool PlaylistsService::Init() {
  return base::PostTaskAndReplyWithResult(
      file_task_runner(), FROM_HERE,
      base::BindOnce(&base::CreateDirectory, base_dir_),
      base::BindOnce(&PlaylistsService::OnBaseDirectoryReady,
                     weak_factory_.GetWeakPtr()));
}

void PlaylistsService::OnBaseDirectoryReady(bool ready) {
  // If we can't create directory in context dir, give up.
  if (!ready)
    return;

  controller_->Init(base_dir_, file_task_runner_);
}

base::SequencedTaskRunner* PlaylistsService::file_task_runner() {
  if (!file_task_runner_) {
    file_task_runner_ = base::CreateSequencedTaskRunner(
        {base::ThreadPool(), base::MayBlock(), base::TaskPriority::BEST_EFFORT,
         base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN});
  }
  return file_task_runner_.get();
}

}  // namespace brave_playlists
