/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_DATA_SOURCE_H_
#define BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_DATA_SOURCE_H_

#include <string>

#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/url_data_source.h"

class GURL;

namespace base {
class FilePath;
class SequencedTaskRunner;
}  // namespace base

namespace playlist {

class PlaylistService;

// A URL data source for chrome://playlist-image/<playlist-id>
// resources, for use in webui pages that want to display downloaded
// playlist thumbnail images
class PlaylistDataSource : public content::URLDataSource {
 public:
  explicit PlaylistDataSource(PlaylistService* service);

  ~PlaylistDataSource() override;

  // content::URLDataSource implementation.
  std::string GetSource() override;
  void StartDataRequest(const GURL& url,
                        const content::WebContents::Getter& wc_getter,
                        content::URLDataSource::GotDataCallback
                            got_data_callback) override;
  std::string GetMimeType(const std::string&) override;
  bool AllowCaching() override;
  bool ShouldReplaceExistingSource() override;

 private:
  PlaylistService* service_;

  void StartDataRequestAfterPathExists(
      const base::FilePath& thumbnail_path,
      content::URLDataSource::GotDataCallback got_data_callback,
      bool path_exists);

  scoped_refptr<base::SequencedTaskRunner> task_runner_;

  base::WeakPtrFactory<PlaylistDataSource> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(PlaylistDataSource);
};

}  // namespace playlist

#endif  // BRAVE_COMPONENTS_PLAYLIST_BROWSER_PLAYLIST_DATA_SOURCE_H_
