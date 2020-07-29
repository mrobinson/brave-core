/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/api/playlist/brave_playlists_api.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "brave/browser/playlists/playlists_service_factory.h"
#include "brave/common/extensions/api/brave_playlists.h"
#include "brave/components/playlists/browser/playlists_service.h"
#include "brave/components/playlists/browser/playlists_types.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"

using brave_playlists::CreatePlaylistParams;
using brave_playlists::PlaylistsService;
using brave_playlists::PlaylistsServiceFactory;

namespace CreatePlaylist = extensions::api::brave_playlists::CreatePlaylist;
namespace GetPlaylist = extensions::api::brave_playlists::GetPlaylist;
namespace DeletePlaylist = extensions::api::brave_playlists::DeletePlaylist;
namespace RequestDownload = extensions::api::brave_playlists::RequestDownload;
namespace OnDownloadRequested =
    extensions::api::brave_playlists::OnDownloadRequested;

namespace {

constexpr char kNotExistPlaylistError[] = "Playlist does not exist";
constexpr char kFeatureDisabled[] = "Playlist feature is disabled";

PlaylistsService* GetPlaylistsService(content::BrowserContext* context) {
  return PlaylistsServiceFactory::GetInstance()->GetForProfile(
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

ExtensionFunction::ResponseAction BravePlaylistsCreatePlaylistFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
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

ExtensionFunction::ResponseAction BravePlaylistsGetAllPlaylistsFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  base::Value playlists = service->GetAllPlaylists();
  return RespondNow(
      OneArgument(base::Value::ToUniquePtrValue(std::move(playlists))));
}

ExtensionFunction::ResponseAction BravePlaylistsGetPlaylistFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
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

ExtensionFunction::ResponseAction BravePlaylistsRecoverPlaylistFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<GetPlaylist::Params> params(
      GetPlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->RecoverPlaylist(params->id);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistsDeletePlaylistFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<DeletePlaylist::Params> params(
      DeletePlaylist::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  service->DeletePlaylist(params->id);
  return RespondNow(NoArguments());
}

// TODO(simonhong): This api doesn't need callback. Delete.
ExtensionFunction::ResponseAction
BravePlaylistsDeleteAllPlaylistsFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  service->DeleteAllPlaylists();
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistsRequestDownloadFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
  if (!service) {
    return RespondNow(Error(kFeatureDisabled));
  }

  std::unique_ptr<RequestDownload::Params> params(
      RequestDownload::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  auto event = std::make_unique<extensions::Event>(
      extensions::events::BRAVE_PLAYLISTS_ON_DOWNLOAD_REQUESTED,
      OnDownloadRequested::kEventName,
      OnDownloadRequested::Create(params->url),
      browser_context());

  extensions::EventRouter::Get(browser_context())->BroadcastEvent(
      std::move(event));

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction BravePlaylistsPlayFunction::Run() {
  auto* service = GetPlaylistsService(browser_context());
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
