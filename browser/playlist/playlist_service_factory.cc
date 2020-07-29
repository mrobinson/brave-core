/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/playlist/playlist_service_factory.h"

#include <memory>

#include "base/feature_list.h"
#include "base/task/post_task.h"
#include "brave/components/playlist/browser/features.h"
#include "brave/components/playlist/browser/playlist_service.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

#if !defined(OS_ANDROID)
#include "brave/browser/playlist/desktop_playlist_player.h"
#include "chrome/browser/profiles/profile.h"
#endif

namespace playlist {

// static
PlaylistServiceFactory* PlaylistServiceFactory::GetInstance() {
  return base::Singleton<PlaylistServiceFactory>::get();
}

// static
PlaylistService* PlaylistServiceFactory::GetForProfile(Profile* profile) {
  if (base::FeatureList::IsEnabled(playlist::features::kPlaylist))
    return static_cast<PlaylistService*>(
        GetInstance()->GetServiceForBrowserContext(profile, true));

  return nullptr;
}

PlaylistServiceFactory::PlaylistServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "PlaylistService",
          BrowserContextDependencyManager::GetInstance()) {}

PlaylistServiceFactory::~PlaylistServiceFactory() {}

KeyedService* PlaylistServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  scoped_refptr<base::SequencedTaskRunner> io_task_runner =
      base::CreateSequencedTaskRunner(
          { base::ThreadPool(), base::MayBlock(),
            base::TaskPriority::BEST_EFFORT,
            base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN });
  auto* service = new PlaylistService(context, io_task_runner);

#if !defined(OS_ANDROID)
  std::unique_ptr<PlaylistPlayer> playlist_player(
      new DesktopPlaylistPlayer(Profile::FromBrowserContext(context),
                                 io_task_runner));
  service->SetPlayer(std::move(playlist_player));
#endif

  return service;
}

content::BrowserContext* PlaylistServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextRedirectedInIncognito(context);
}

}  // namespace playlist
