/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_MODULES_STORAGE_BRAVE_DOM_WINDOW_STORAGE_H_
#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_MODULES_STORAGE_BRAVE_DOM_WINDOW_STORAGE_H_

namespace blink {

class ExceptionState;
class LocalDOMWindow;
class StorageArea;

class BraveDOMWindowStorage final : public GarbageCollected<BraveDOMWindowStorage>,
                                    public Supplement<LocalDOMWindow> {
  USING_GARBAGE_COLLECTED_MIXIN(BraveDOMWindowStorage);

 public:
  static const char kSupplementName[];

  static BraveDOMWindowStorage& From(LocalDOMWindow&);
  static StorageArea* sessionStorage(LocalDOMWindow&, ExceptionState&);
  static StorageArea* localStorage(LocalDOMWindow&, ExceptionState&);

  StorageArea* sessionStorage(ExceptionState&) const;
  StorageArea* localStorage(ExceptionState&) const;

  explicit BraveDOMWindowStorage(LocalDOMWindow&);

  void Trace(Visitor*) const override;

 private:
  mutable Member<StorageArea> ephemeral_session_storage_;
};

}  // namespace blink

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_MODULES_STORAGE_BRAVE_DOM_WINDOW_STORAGE_H_
