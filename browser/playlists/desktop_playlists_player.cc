/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/playlists/desktop_playlists_player.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/task/post_task.h"
#include "base/task_runner_util.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/scoped_tabbed_browser_displayer.h"
#include "content/public/browser/page_navigator.h"
#include "net/base/filename_util.h"

namespace brave_playlists {

namespace {

const char kHTMLTemplate[] =
    "<video id='v' controls autoplay "
    "onplay='a=document.getElementById(\"a\");a.currentTime=this.currentTime;a."
    "play();' "
    "onpause='a=document.getElementById(\"a\");a.pause()'><source "
    "src='video_file.mp4' type='video/mp4' /></video> <video id='a' autoplay "
    "style='display:none'><source src='audio_file.m4a' type='audio/mp4' "
    "/></video>";

int DoGenerateHTMLFileOnIOThread(const base::FilePath& html_file_path) {
  if (base::PathExists(html_file_path))
    base::DeleteFile(html_file_path, false);

  base::File html_file(html_file_path,
                       base::File::FLAG_CREATE | base::File::FLAG_WRITE);
  if (!html_file.IsValid())
    return -1;

  html_file.WriteAtCurrentPos(kHTMLTemplate, 322 /*kHTMLTemplate.length()*/);
  return 0;
}

}  // namespace

DesktopPlaylistsPlayer::DesktopPlaylistsPlayer(
    Profile* profile,
    scoped_refptr<base::SequencedTaskRunner> task_runner)
    : profile_(profile),
      io_task_runner_(task_runner),
      weak_factory_(this) {}

DesktopPlaylistsPlayer::~DesktopPlaylistsPlayer() {
}

void DesktopPlaylistsPlayer::Play(const base::FilePath& playlist_path) {
  auto html_file_path = playlist_path.Append(FILE_PATH_LITERAL("index.html"));
  base::PostTaskAndReplyWithResult(
      io_task_runner_.get(), FROM_HERE,
      base::BindOnce(&DoGenerateHTMLFileOnIOThread, html_file_path),
      base::BindOnce(&DesktopPlaylistsPlayer::OnHTMLFileGenerated,
                     weak_factory_.GetWeakPtr(),
                     html_file_path));
}

void DesktopPlaylistsPlayer::OnHTMLFileGenerated(
    const base::FilePath& html_file_path,
    int error_code) {
  if (error_code) {
    LOG(ERROR) << "couldn't create HTML file for player";
    return;
  }

  GURL html_file_url = net::FilePathToFileURL(html_file_path);
  chrome::ScopedTabbedBrowserDisplayer browser_displayer(profile_);
  content::OpenURLParams open_url_params(
      html_file_url, content::Referrer(),
      WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui::PAGE_TRANSITION_AUTO_TOPLEVEL, false);
  browser_displayer.browser()->OpenURL(open_url_params);
}

}  // namespace brave_playlists
