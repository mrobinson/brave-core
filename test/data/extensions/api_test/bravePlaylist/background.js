/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

function testHasAccess() {
  chrome.test.runTests([
    function hasAccess() {
      if (chrome.bravePlaylist) {
        chrome.test.succeed();
      } else {
        chrome.test.fail();
      }
    }
  ]);
}

function testCreatePlaylistNoCrash() {
  chrome.test.runTests([
    function createPlaylistNoCrash() {
      // Calling createPlaylistItem() API should not crash, even if all
      // parameters are empty.
      chrome.bravePlaylist.createPlaylistItem({
        thumbnailUrl: '',
        playlistName: '',
        videoMediaFiles: [],
        audioMediaFiles: []
      });
      chrome.test.succeed();
    }
  ]);
}

function createPlaylistAndWait(thumbnail_url, video_url, audio_url, expected_change_types, callback) {
  in_progress = true;
  chrome.bravePlaylist.onPlaylistItemStatusChanged.addListener(function(changeType, id) {
    const found = expected_change_types.find(expectedType => expectedType === changeType)
    if (!found) {
      console.log('got changeType=' + changeType + ' but it is not included in expected types list');
      callback(false);
    }
    expected_change_types = expected_change_types.filter(expectedType => expectedType !== changeType);
    if (expected_change_types.length == 0) {
      // We got all the events in the right order, so we're done.
      callback(true);
    }
  });
  if (video_url) {
    video_media_files = [{url:video_url, title:'test', thumb:''}];
  } else {
    video_media_files = [];
  }
  if (audio_url) {
    audio_media_files = [{url:audio_url, title:'test', thumb:''}];
  } else {
    audio_media_files = [];
  }
  chrome.bravePlaylist.createPlaylistItem({
    thumbnailUrl: thumbnail_url,
    playlistName: 'test',
    videoMediaFiles: video_media_files,
    audioMediaFiles: audio_media_files
  });
}

function testCreatePlaylist(thumbnail_url, video_url, audio_url, expected_change_types) {
  chrome.test.runTests([
    function createPlaylist() {
      createPlaylistAndWait(thumbnail_url, video_url, audio_url, expected_change_types, function(success) {
        if (success) {
          chrome.test.succeed();
        } else {
          chrome.test.fail();
        }
      });
    }
  ]);
}
