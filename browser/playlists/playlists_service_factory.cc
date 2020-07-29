/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/playlists/playlists_service_factory.h"

#include <memory>

#include "base/task/post_task.h"
#include "brave/components/playlists/browser/features.h"
#include "brave/components/playlists/browser/playlists_data_source.h"
#include "brave/components/playlists/browser/playlists_service.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/url_data_source.h"

#if !defined(OS_ANDROID)
#include "brave/browser/playlists/desktop_playlists_player.h"
#include "chrome/browser/profiles/profile.h"
#endif

namespace brave_playlists {

// static
PlaylistsServiceFactory* PlaylistsServiceFactory::GetInstance() {
  return base::Singleton<PlaylistsServiceFactory>::get();
}

// static
PlaylistsService* PlaylistsServiceFactory::GetForProfile(Profile* profile) {
  if (base::FeatureList::IsEnabled(brave_playlists::features::kBravePlaylists))
    return static_cast<PlaylistsService*>(
        GetInstance()->GetServiceForBrowserContext(profile, true));

  return nullptr;
}

PlaylistsServiceFactory::PlaylistsServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "PlaylistsService",
          BrowserContextDependencyManager::GetInstance()) {}

PlaylistsServiceFactory::~PlaylistsServiceFactory() {}

KeyedService* PlaylistsServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  scoped_refptr<base::SequencedTaskRunner> io_task_runner =
      base::CreateSequencedTaskRunner(
          { base::ThreadPool(), base::MayBlock(),
            base::TaskPriority::BEST_EFFORT,
            base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN });
  auto* service = new PlaylistsService(context, io_task_runner);
  content::URLDataSource::Add(
      context,
      std::make_unique<BravePlaylistsSource>(service));

#if !defined(OS_ANDROID)
  std::unique_ptr<PlaylistsPlayer> playlists_player(
      new DesktopPlaylistsPlayer(Profile::FromBrowserContext(context),
                                 io_task_runner));
  service->SetPlayer(std::move(playlists_player));
#endif

  return service;
}

content::BrowserContext* PlaylistsServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextRedirectedInIncognito(context);
}

}  // namespace brave_playlists
