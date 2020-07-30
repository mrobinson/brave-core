/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import '../../third_party/youtubedown/youtubedown'

function createPlaylist (url: string) {
  const ytdItems: chrome.bravePlaylist.YTDMediaItem[] = window.youtubedown_urls(url)
  if (ytdItems.length === 0) {
    console.log(' ##### got nothing from youtubedown_urls #####')
    return
  }
  const videoMediaFiles: chrome.bravePlaylist.CreateParamsMediaItem[] = makeMediaItems(ytdItems[0])
  let audioMediaFiles: chrome.bravePlaylist.CreateParamsMediaItem[] = []

  if (ytdItems.length > 1) {
    // Video is only available as separate video and audio files.
    // First item is always the video. Second item is always the audio.
    audioMediaFiles = makeMediaItems(ytdItems[1])
  }

  const thumbnailUrl = audioMediaFiles && audioMediaFiles[0].thumb || videoMediaFiles && videoMediaFiles[0].thumb
  const playlistName = getCleanPlaylistName(audioMediaFiles[0].title) || getCleanPlaylistName(videoMediaFiles[0].title)

  chrome.bravePlaylist.createPlaylistItem({
    thumbnailUrl,
    playlistName,
    videoMediaFiles,
    audioMediaFiles
  })
}

function getCleanPlaylistName (fileName: string) {
  return fileName && fileName.replace(/\[.*$/,'')
}

function makeMediaItems (item: chrome.bravePlaylist.YTDMediaItem) {
  let mediaFiles: chrome.bravePlaylist.CreateParamsMediaItem[]
  if (typeof item.url === 'string') {
    // Video is all in one file, but we'll pretend it's an array of 1 segment
    mediaFiles = [{ url: item.url, title: item.file, thumb: item.thumb }]
  } else {
    // Video is split into segments, which we will concatenate later
    mediaFiles = item.url.map(segment => ({ url: segment, title: item.file, thumb: item.thumb }))
  }
  return mediaFiles
}

chrome.bravePlaylist.onDownloadRequested.addListener((url) => {
  createPlaylist(url)
})
