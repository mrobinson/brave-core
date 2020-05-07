//
//  BraveBrowserState.cpp
//  Sources
//
//  Created by brandon on 2020-05-06.
//

#include "BraveBrowserState.h"

#include <memory>
#include <utility>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/sequenced_task_runner.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "components/variations/net/variations_http_headers.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {
// All ChromeBrowserState will store a dummy base::SupportsUserData::Data
// object with this key. It can be used to check that a web::BrowserState
// is effectively a ChromeBrowserState when converting.
const char kBrowserStateIsChromeBrowserState[] = "IsChromeBrowserState";
}

BraveBrowserState::BraveBrowserState(
    scoped_refptr<base::SequencedTaskRunner> io_task_runner)
    : io_task_runner_(std::move(io_task_runner)) {
  DCHECK(io_task_runner_);
  SetUserData(kBrowserStateIsChromeBrowserState,
              std::make_unique<base::SupportsUserData::Data>());
}

BraveBrowserState::~BraveBrowserState() {}

// static
BraveBrowserState* BraveBrowserState::FromBrowserState(
    web::BrowserState* browser_state) {
  if (!browser_state)
    return nullptr;

  DCHECK(browser_state->GetUserData(kBrowserStateIsChromeBrowserState));
  return static_cast<ChromeBrowserState*>(browser_state);
}

// static
BraveBrowserState* BraveBrowserState::FromWebUIIOS(web::WebUIIOS* web_ui) {
  return FromBrowserState(web_ui->GetWebState()->GetBrowserState());
}

std::string BraveBrowserState::GetDebugName() {
  std::string name =
      GetOriginalChromeBrowserState()->GetStatePath().BaseName().MaybeAsASCII();
  if (name.empty()) {
    name = "UnknownBrowserState";
  }
  return name;
}

scoped_refptr<base::SequencedTaskRunner> BraveBrowserState::GetIOTaskRunner() {
  return io_task_runner_;
}

sync_preferences::PrefServiceSyncable* BraveBrowserState::GetSyncablePrefs() {
  return static_cast<sync_preferences::PrefServiceSyncable*>(GetPrefs());
}

net::URLRequestContextGetter* BraveBrowserState::GetRequestContext() {
  DCHECK_CURRENTLY_ON(web::WebThread::UI);
  if (!request_context_getter_) {
    ProtocolHandlerMap protocol_handlers;
    protocol_handlers[kChromeUIScheme] =
        web::URLDataManagerIOSBackend::CreateProtocolHandler(this);
    request_context_getter_ =
        base::WrapRefCounted(CreateRequestContext(&protocol_handlers));
  }
  return request_context_getter_.get();
}

void BraveBrowserState::UpdateCorsExemptHeader(
    network::mojom::NetworkContextParams* params) {
  variations::UpdateCorsExemptHeaderForVariations(params);
}
