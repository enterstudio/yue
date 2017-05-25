// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/state.h"

#include "base/lazy_instance.h"
#include "base/threading/thread_local.h"
#include "third_party/yoga/yoga/Yoga.h"

#if defined(OS_WIN)
#include "nativeui/gfx/win/native_theme.h"
#include "nativeui/win/util/class_registrar.h"
#include "nativeui/win/util/subwin_holder.h"
#endif

namespace nu {

namespace {

// A lazily created thread local storage for quick access to a thread's message
// loop, if one exists. This should be safe and free of static constructors.
base::LazyInstance<base::ThreadLocalPointer<State>>::Leaky lazy_tls_ptr =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

State::State() : yoga_config_(YGConfigNew()) {
  DCHECK_EQ(GetCurrent(), nullptr) << "should only have one state per thread";

  lazy_tls_ptr.Pointer()->Set(this);
  PlatformInit();
}

State::~State() {
  YGConfigFree(yoga_config_);

  PlatformDestroy();
  DCHECK_EQ(GetCurrent(), this);
  lazy_tls_ptr.Pointer()->Set(nullptr);
}

// static
State* State::GetCurrent() {
  return lazy_tls_ptr.Pointer()->Get();
}

}  // namespace nu
