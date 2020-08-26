/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../../../../../../third_party/blink/renderer/modules/storage/dom_window_storage.cc"
#include "third_party/blink/renderer/modules/storage/brave_dom_window_storage.h"

namespace blink {

class EphemeralStorageNamespaces
    : public GarbageCollected<EphemeralStorageNamespaces>,
      public Supplement<Page> {
  USING_GARBAGE_COLLECTED_MIXIN(StorageNamespace);

 public:
  EphemeralStorageNamespaces(
      StorageController* controller,
      const String& session_storage_id,
      const String& local_storage_id);
  virtual ~EphemeralStorageNamespaces() = default;

  static const char kSupplementName[];
  static EphemeralStorageNamespaces* From(Page* page);

  StorageNamespace* session_storage() { return session_storage_.Get(); }
  StorageNamespace* local_storage() { return local_storage_.Get(); }
  void Trace(Visitor* visitor) const override;

 private:
  Member<StorageNamespace> session_storage_;
  Member<StorageNamespace> local_storage_;
};

const char EphemeralStorageNamespaces::kSupplementName[] = "EphemeralSessionStorageNamespaces";

EphemeralStorageNamespaces::EphemeralStorageNamespaces(
    StorageController* controller,
    const String& session_storage_id,
    const String& local_storage_id)
    : session_storage_(MakeGarbageCollected<StorageNamespace>(controller, session_storage_id)),
      local_storage_(MakeGarbageCollected<StorageNamespace>(controller, local_storage_id)) { }

void EphemeralStorageNamespaces::Trace(Visitor* visitor) const {
  visitor->Trace(session_storage_);
  visitor->Trace(local_storage_);
  Supplement<Page>::Trace(visitor);
}

// static
EphemeralStorageNamespaces* EphemeralStorageNamespaces::From(Page* page) {
  if (!page)
    return nullptr;

  EphemeralStorageNamespaces* supplement =
      Supplement<Page>::From<EphemeralStorageNamespaces>(page);
  if (supplement)
      return supplement;

  // The ephemeral session storage namespace is constructed using the
  // normal session storage id. We get it here and transform it.
  StorageNamespace* session_storage_namespace = StorageNamespace::From(page);
  if (!session_storage_namespace)
    return nullptr;

  String session_storage_id =
      session_storage_namespace->namespace_id() +
      String("ephemeral-session-storage");
  String local_storage_id =
      session_storage_namespace->namespace_id() +
      String("ephemeral-local-storage");
  supplement = MakeGarbageCollected<EphemeralStorageNamespaces>(
       StorageController::GetInstance(), session_storage_id, local_storage_id);

  ProvideTo(*page, supplement);
  return supplement;
}

// static
const char BraveDOMWindowStorage::kSupplementName[] = "DOMWindowStorage";

// static
BraveDOMWindowStorage& BraveDOMWindowStorage::From(LocalDOMWindow& window) {
  BraveDOMWindowStorage* supplement =
      Supplement<LocalDOMWindow>::From<BraveDOMWindowStorage>(window);
  if (!supplement) {
    supplement = MakeGarbageCollected<BraveDOMWindowStorage>(window);
    ProvideTo(window, supplement);
  }
  return *supplement;
}

// static
StorageArea* BraveDOMWindowStorage::sessionStorage(LocalDOMWindow& window,
                                                   ExceptionState& exception_state) {
  return From(window).sessionStorage(exception_state);
}

// static
StorageArea* BraveDOMWindowStorage::localStorage(LocalDOMWindow& window,
                                                 ExceptionState& exception_state) {
  return From(window).localStorage(exception_state);
}

BraveDOMWindowStorage::BraveDOMWindowStorage(LocalDOMWindow& window)
    : Supplement<LocalDOMWindow>(window) {}

StorageArea* BraveDOMWindowStorage::sessionStorage(ExceptionState& exception_state) const {
  if (ephemeral_session_storage_)
    return ephemeral_session_storage_;

  LocalDOMWindow* window = GetSupplementable();
  auto* storage =
      DOMWindowStorage::From(*window).sessionStorage(exception_state);

  if (!window->IsCrossSiteSubframe() || !storage)
    return storage;


  Page* page = window->GetFrame()->GetDocument()->GetPage();
  EphemeralStorageNamespaces* namespaces = EphemeralStorageNamespaces::From(page);
  if (!namespaces)
    return nullptr;

  Document* document = window->GetFrame()->GetDocument();
  auto storage_area =
      namespaces->local_storage()->GetCachedArea(document->GetSecurityOrigin());
  ephemeral_session_storage_ =
      StorageArea::Create(document->GetFrame(), std::move(storage_area),
                          StorageArea::StorageType::kSessionStorage);
  return ephemeral_session_storage_;
}

StorageArea* BraveDOMWindowStorage::localStorage(ExceptionState& exception_state) const {
  LocalDOMWindow* window = GetSupplementable();
  auto* storage =
      DOMWindowStorage::From(*window).localStorage(exception_state);

  if (!window->IsCrossSiteSubframe() || !storage)
    return storage;

  Page* page = window->GetFrame()->GetDocument()->GetPage();
  EphemeralStorageNamespaces* namespaces = EphemeralStorageNamespaces::From(page);
  if (!namespaces)
    return nullptr;

  Document* document = window->GetFrame()->GetDocument();
  auto storage_area =
      namespaces->local_storage()->GetCachedArea(document->GetSecurityOrigin());
  ephemeral_session_storage_ =
      StorageArea::Create(document->GetFrame(), std::move(storage_area),
                          StorageArea::StorageType::kSessionStorage);
  return ephemeral_session_storage_;
}

void BraveDOMWindowStorage::Trace(Visitor* visitor) const {
  visitor->Trace(ephemeral_session_storage_);
  Supplement<LocalDOMWindow>::Trace(visitor);
}

}  // namespace blink
