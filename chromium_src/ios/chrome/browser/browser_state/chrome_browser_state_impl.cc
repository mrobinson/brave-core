/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ios/chrome/browser/browser_state/chrome_browser_state_impl.h"

#include "build/build_config.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/threading/thread_restrictions.h"
#include "components/keyed_service/ios/browser_state_dependency_manager.h"

namespace {

// Returns a bool indicating whether the necessary directories were able to be
// created (or already existed).
bool EnsureBrowserStateDirectoriesCreated(const base::FilePath& path) {
  // Create the browser state directory synchronously otherwise we would need to
  // sequence every otherwise independent I/O operation inside the browser state
  // directory with this operation. base::CreateDirectory() should be a
  // lightweight I/O operation and avoiding the headache of sequencing all
  // otherwise unrelated I/O after this one justifies running it on the main
  // thread.
  base::ThreadRestrictions::ScopedAllowIO allow_io_to_create_directory;

  if (!base::PathExists(path) && !base::CreateDirectory(path))
    return false;

  return true;
}

}

ChromeBrowserStateImpl::ChromeBrowserStateImpl(
    scoped_refptr<base::SequencedTaskRunner> io_task_runner,
    const base::FilePath& path)
    : ChromeBrowserState(io_task_runner),
      state_path_(path) {
  bool directories_created = EnsureBrowserStateDirectoriesCreated(state_path_);
  DCHECK(directories_created);

  //TODO: BRANDON
  // Bring up the policy system before creating |prefs_|.

  // RegisterBrowserStatePrefs(pref_registry_.get());
  // BrowserStateDependencyManager::GetInstance()
  //     ->RegisterBrowserStatePrefsForServices(pref_registry_.get());
  // prefs_ = CreateBrowserStatePrefs(state_path_,
  //                                  GetIOTaskRunner().get(),
  //                                  //io_task_runner_.get(),
  //                                  pref_registry_,
  //                                  nullptr, //policy_connector_ ? policy_connector_->GetPolicyService() :
  //                                  nullptr); //GetApplicationContext()->GetBrowserPolicyConnector()

  // // Register on BrowserState.
  // user_prefs::UserPrefs::Set(this, prefs_.get());

  // // Migrate obsolete prefs.
  // // PrefService* local_state = GetApplicationContext()->GetLocalState();
  // // MigrateObsoleteLocalStatePrefs(local_state);
  // // MigrateObsoleteBrowserStatePrefs(prefs_.get());

  BrowserStateDependencyManager::GetInstance()->CreateBrowserStateServices(
      this);

  // Listen for bookmark model load, to bootstrap the sync service.
  // bookmarks::BookmarkModel* model =
  //     ios::BookmarkModelFactory::GetForBrowserState(this);
  // model->AddObserver(new BookmarkModelLoadedObserver(this));
}

ChromeBrowserStateImpl::~ChromeBrowserStateImpl() {}

ChromeBrowserState* ChromeBrowserStateImpl::GetOriginalChromeBrowserState() {
  return this;
}

bool ChromeBrowserStateImpl::HasOffTheRecordChromeBrowserState() const {
  return false;
}

ChromeBrowserState*
ChromeBrowserStateImpl::GetOffTheRecordChromeBrowserState() {
  return nullptr;
}

void ChromeBrowserStateImpl::DestroyOffTheRecordChromeBrowserState() {}

BrowserStatePolicyConnector* ChromeBrowserStateImpl::GetPolicyConnector() {
  return nullptr;
}

PrefService* ChromeBrowserStateImpl::GetPrefs() {
  return nullptr;
}

PrefService* ChromeBrowserStateImpl::GetOffTheRecordPrefs() {
  return nullptr;
}

ChromeBrowserStateIOData* ChromeBrowserStateImpl::GetIOData() {
  return nullptr;
}

void ChromeBrowserStateImpl::ClearNetworkingHistorySince(
    base::Time time,
    const base::Closure& completion) {}

PrefProxyConfigTracker* ChromeBrowserStateImpl::GetProxyConfigTracker() {
  return nullptr;
}

net::URLRequestContextGetter* ChromeBrowserStateImpl::CreateRequestContext(
      ProtocolHandlerMap* protocol_handlers) {
  return nullptr;
}


bool ChromeBrowserStateImpl::IsOffTheRecord() const {
  return false;
}

base::FilePath ChromeBrowserStateImpl::GetStatePath() const {
  return state_path_;
}
