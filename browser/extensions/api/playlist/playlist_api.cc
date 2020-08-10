/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/api/playlist/playlist_api.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/notreached.h"
#include "brave/browser/playlist/desktop_playlist_player.h"
#include "brave/browser/playlist/playlist_service_factory.h"
#include "brave/common/extensions/api/brave_playlist.h"
#include "brave/components/playlist/browser/playlist_service.h"
#include "brave/components/playlist/browser/playlist_types.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"

using playlist::CreatePlaylistParams;
using playlist::PlaylistService;
using playlist::PlaylistServiceFactory;

namespace CreatePlaylistItem =
    extensions::api::brave_playlist::CreatePlaylistItem;
namespace GetPlaylistItem =
    extensions::api::brave_playlist::GetPlaylistItem;
namespace DeletePlaylistItem =
    extensions::api::brave_playlist::DeletePlaylistItem;
namespace RecoverPlaylistItem =
    extensions::api::brave_playlist::RecoverPlaylistItem;
namespace PlayItem = extensions::api::brave_playlist::PlayItem;
namespace RequestDownload = extensions::api::brave_playlist::RequestDownload;
namespace OnDownloadRequested =
    extensions::api::brave_playlist::OnDownloadRequested;

namespace {

constexpr char kNotExistPlaylistError[] = "Playlist does not exist";
constexpr char kFeatureDisabled[] = "Playlist feature is disabled";

PlaylistService* GetPlaylistService(content::BrowserContext* context) {
  return PlaylistServiceFactory::GetInstance()->GetForProfile(
      Profile::FromBrowserContext(context));
}

CreatePlaylistParams GetCreatePlaylistParamsFromCreateParams(
    const CreatePlaylistItem::Params::CreateParams& params) {
  CreatePlaylistParams p;
  p.playlist_name = params.playlist_name;
  p.playlist_thumbnail_url = params.thumbnail_url;

  for (const auto& file : params.video_media_files)
    p.video_media_files.emplace_back(file.url, file.title);
  for (const auto& file : params.audio_media_files)
    p.audio_media_files.emplace_back(file.url, file.title);
  return p;
}

}  // namespace

namespace extensions {
namespace api {

ExtensionFunction::ResponseAction
BravePlaylistCreatePlaylistItemFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<CreatePlaylistItem::Params> params(
      CreatePlaylistItem::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->CreatePlaylistItem(
      GetCreatePlaylistParamsFromCreateParams(params->create_params));
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
BravePlaylistGetAllPlaylistItemsFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  base::Value playlist = service->GetAllPlaylistItems();
  return RespondNow(
      OneArgument(base::Value::ToUniquePtrValue(std::move(playlist))));
}

ExtensionFunction::ResponseAction BravePlaylistGetPlaylistItemFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<GetPlaylistItem::Params> params(
      GetPlaylistItem::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  base::Value playlist = service->GetPlaylistItem(params->id);
  DCHECK(playlist.is_dict());

  if (playlist.DictEmpty())
    return RespondNow(Error(kNotExistPlaylistError));

  return RespondNow(OneArgument(base::Value::ToUniquePtrValue(
                                    std::move(playlist))));
}

ExtensionFunction::ResponseAction
BravePlaylistRecoverPlaylistItemFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<RecoverPlaylistItem::Params> params(
      RecoverPlaylistItem::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->RecoverPlaylistItem(params->id);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
BravePlaylistDeletePlaylistItemFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<DeletePlaylistItem::Params> params(
      DeletePlaylistItem::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->DeletePlaylistItem(params->id);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
BravePlaylistDeleteAllPlaylistItemsFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  service->DeleteAllPlaylistItems();
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistRequestDownloadFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<RequestDownload::Params> params(
      RequestDownload::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  auto event = std::make_unique<extensions::Event>(
      extensions::events::BRAVE_PLAYLIST_ON_DOWNLOAD_REQUESTED,
      OnDownloadRequested::kEventName,
      OnDownloadRequested::Create(params->url),
      browser_context());

  extensions::EventRouter::Get(browser_context())->BroadcastEvent(
      std::move(event));

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistPlayItemFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<PlayItem::Params> params(PlayItem::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  playlist::DesktopPlaylistPlayer player(
      Profile::FromBrowserContext(browser_context()));
  player.Play(params->id);
  return RespondNow(NoArguments());
}

}  // namespace api
}  // namespace extensions
