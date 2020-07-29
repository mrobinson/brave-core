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
#include "brave/browser/playlist/playlist_service_factory.h"
#include "brave/common/extensions/api/brave_playlist.h"
#include "brave/components/playlist/browser/playlist_service.h"
#include "brave/components/playlist/browser/playlist_types.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"

using playlist::CreatePlaylistParams;
using playlist::PlaylistService;
using playlist::PlaylistServiceFactory;

namespace CreatePlaylist = extensions::api::brave_playlist::CreatePlaylist;
namespace GetPlaylist = extensions::api::brave_playlist::GetPlaylist;
namespace DeletePlaylist = extensions::api::brave_playlist::DeletePlaylist;
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
    const CreatePlaylist::Params::CreateParams& params) {
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

ExtensionFunction::ResponseAction BravePlaylistCreatePlaylistFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<CreatePlaylist::Params> params(
      CreatePlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->CreatePlaylist(
      GetCreatePlaylistParamsFromCreateParams(params->create_params));
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistGetAllPlaylistFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  base::Value playlist = service->GetAllPlaylist();
  return RespondNow(
      OneArgument(base::Value::ToUniquePtrValue(std::move(playlist))));
}

ExtensionFunction::ResponseAction BravePlaylistGetPlaylistFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<GetPlaylist::Params> params(
      GetPlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  base::Value playlist = service->GetPlaylist(params->id);
  DCHECK(playlist.is_dict());

  if (playlist.DictEmpty())
    return RespondNow(Error(kNotExistPlaylistError));

  return RespondNow(OneArgument(base::Value::ToUniquePtrValue(
                                    std::move(playlist))));
}

ExtensionFunction::ResponseAction BravePlaylistRecoverPlaylistFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<GetPlaylist::Params> params(
      GetPlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->RecoverPlaylist(params->id);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistDeletePlaylistFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<DeletePlaylist::Params> params(
      DeletePlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->DeletePlaylist(params->id);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction
BravePlaylistDeleteAllPlaylistFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  service->DeleteAllPlaylist();
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

ExtensionFunction::ResponseAction BravePlaylistPlayFunction::Run() {
  auto* service = GetPlaylistService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<GetPlaylist::Params> params(
      GetPlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  // TODO(simonhong): Use standalone player instead of asking to service.
  service->Play(params->id);
  return RespondNow(NoArguments());
}

}  // namespace api
}  // namespace extensions
