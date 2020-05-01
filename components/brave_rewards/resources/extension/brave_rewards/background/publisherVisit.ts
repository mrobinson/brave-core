/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

interface PublisherVisitData {
  url: string
  publisherKey: string
  publisherName: string
  mediaKey?: string
  favIconUrl?: string
}

interface PublisherVisitResponse {
  mediaType: string
  errorMessage?: string
  data: PublisherVisitData | null
}

const handlePublisherVisit = (tabId: number, mediaType: string, data: PublisherVisitData) => {
  console.info('Visited a publisher url:')
  console.info(`  tabId=${tabId}`)
  console.info(`  mediaType=${mediaType}`)
  console.info(`  url=${data.url}`)
  console.info(`  publisherKey=${data.publisherKey}`)
  console.info(`  publisherName=${data.publisherName}`)
  console.info(`  favIconUrl=${data.favIconUrl}`)

  if (!data.publisherKey) {
    console.error('Failed to handle publisher visit: missing publisher key')
    return
  }

  chrome.braveRewards.getPublisherInfo(data.publisherKey, (result: number, info?: RewardsExtension.Publisher) => {
    console.debug(`getPublisherInfo: result=${result}`)

    if (result === 0 && info) {
      chrome.braveRewards.getPublisherPanelInfo(
        tabId,
        mediaType,
        data.publisherKey)
      return
    }

    // Failed to find publisher info corresponding to this key, so save it now
    if (result === 9) {
      chrome.braveRewards.savePublisherVisit(
        tabId,
        mediaType,
        data.url,
        data.publisherKey,
        data.publisherName,
        data.favIconUrl || '')
      return
    }
  })
}

const handleMediaPublisherVisit = (tabId: number, mediaType: string, data: PublisherVisitData) => {
  console.info('Visited a media publisher url:')
  console.info(`  tabId=${tabId}`)
  console.info(`  mediaType=${mediaType}`)
  console.info(`  url=${data.url}`)
  console.info(`  publisherKey=${data.publisherKey}`)
  console.info(`  publisherName=${data.publisherName}`)
  console.info(`  mediaKey=${data.mediaKey}`)
  console.info(`  favIconUrl=${data.favIconUrl}`)

  const mediaKey = data.mediaKey
  if (!mediaKey) {
    console.error('Failed to handle publisher visit: missing media key')
    return
  }

  chrome.braveRewards.getMediaPublisherInfo(mediaKey, (result: number, info?: RewardsExtension.Publisher) => {
    console.debug(`getMediaPublisherInfo: result=${result}`)

    if (result === 0 && info) {
      chrome.braveRewards.getPublisherPanelInfo(
        tabId,
        mediaType,
        data.publisherKey)
      return
    }

    // Failed to find publisher info corresponding to this key, so save it now
    if (result === 9) {
      chrome.braveRewards.saveMediaPublisherVisit(
        tabId,
        mediaType,
        data.url,
        data.publisherKey,
        data.publisherName,
        mediaKey,
        data.favIconUrl || '')
      return
    }
  })
}

// chrome.webRequest.onCompleted.addListener(
//   // Listener
//   function (details) {
//     if (details) {
//       const url = new URL(details.url)
//       const searchParams = new URLSearchParams(url.search)
//
//       const mediaId = getMediaIdFromParts(searchParams)
//       const mediaKey = '' // TODO(erogul): buildMediaKey(mediaId)
//       const duration = getMediaDurationFromParts(searchParams)
//
//       chrome.braveRewards.getMediaPublisherInfo(mediaKey, (result: number, info?: RewardsExtension.Publisher) => {
//         console.debug(`getMediaPublisherInfo: result=${result}`)
//
//         if (result === 0 && info) {
//           console.info('Updating media duration:')
//           console.info(`  tabId=${details.tabId}`)
//           console.info(`  url=${details.url}`)
//           console.info(`  publisherKey=${info.publisher_key}`)
//           console.info(`  mediaId=${mediaId}`)
//           console.info(`  mediaKey=${mediaKey}`)
//           console.info(`  favIconUrl=${info.favicon_url}`)
//           console.info(`  publisherName=${info.name}`)
//           console.info(`  duration=${duration}`)
//
//           chrome.braveRewards.updateMediaDuration(
//             details.tabId,
//             'youtube',
//             details.url,
//             info.publisher_key || '',
//             info.name || '',
//             mediaId,
//             mediaKey,
//             info.favicon_url || '',
//             duration)
//           return
//         }
//
//         // No publisher info for this video, fetch it from oembed interface
//         if (result === 9) {
//           // fetchPublisherInfoFromOembed(details.tabId, details.url)
//           return
//         }
//       })
//     }
//   },
//   // Filters
//   {
//     types: [
//       'image',
//       'media',
//       'script',
//       'xmlhttprequest'
//     ],
//     urls: [
//       'https://www.youtube.com/api/stats/watchtime?*'
//     ]
//   })

chrome.runtime.onMessageExternal.addListener((msg, sender, sendResponse) => {
  if (!sender || !sender.tab || !msg) {
    return
  }

  const windowId = sender.tab.windowId
  if (!windowId) {
    return
  }

  const tabId = sender.tab.id
  if (!tabId) {
    return
  }

  const response = msg as PublisherVisitResponse

  if (!response.data) {
    console.error(`Failed to retrieve publisher visit data: ${response.errorMessage}`)
    return
  }

  if (response.data.mediaKey) {
    handleMediaPublisherVisit(tabId, response.mediaType, response.data)
  } else {
    handlePublisherVisit(tabId, response.mediaType, response.data)
  }
})
