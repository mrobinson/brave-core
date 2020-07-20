/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_PLAYLISTS_PLAYLISTS_SERVICE_H_
#define BRAVE_BROWSER_PLAYLISTS_PLAYLISTS_SERVICE_H_

#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "brave/components/playlists/browser/playlists_controller.h"
#include "components/keyed_service/core/keyed_service.h"
#include "extensions/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "brave/browser/extensions/brave_playlists_event_router.h"
#endif

namespace base {
class SequencedTaskRunner;
}  // namespace base

class Profile;

namespace brave_playlists {
class PlaylistsController;

#if BUILDFLAG(ENABLE_EXTENSIONS)
class BravePlaylistsEventRouter;
#endif

class PlaylistsService : public KeyedService {
 public:
  explicit PlaylistsService(content::BrowserContext* context);
  ~PlaylistsService() override;

  bool Init();

  PlaylistsController* controller() const { return controller_.get(); }

 private:
  void OnBaseDirectoryReady(bool ready);

  base::SequencedTaskRunner* file_task_runner();

  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;
  base::FilePath base_dir_;
  std::unique_ptr<PlaylistsController> controller_;
#if BUILDFLAG(ENABLE_EXTENSIONS)
  std::unique_ptr<BravePlaylistsEventRouter> playlists_event_router_;
#endif

  base::WeakPtrFactory<PlaylistsService> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PlaylistsService);
};

}  // namespace brave_playlists

#endif  // BRAVE_BROWSER_PLAYLISTS_PLAYLISTS_SERVICE_H_
