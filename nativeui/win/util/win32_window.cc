// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/util/win32_window.h"

#include "nativeui/state.h"
#include "nativeui/win/screen.h"
#include "nativeui/win/util/class_registrar.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

// static
const DWORD Win32Window::kWindowDefaultChildStyle =
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
const DWORD Win32Window::kWindowDefaultStyle =
    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

Win32Window::Win32Window(base::StringPiece16 class_name, HWND parent,
                         DWORD window_style, DWORD window_ex_style)
    : class_style_(CS_DBLCLKS),
      window_style_(window_style),
      window_ex_style_(window_ex_style) {
  if (parent == HWND_DESKTOP) {
    // Only non-child windows can have HWND_DESKTOP (0) as their parent.
    CHECK_EQ(static_cast<int>(window_style & WS_CHILD), 0);
    parent = GetWindowToParentTo(false);
  } else if (parent == ::GetDesktopWindow()) {
    // Any type of window can have the "Desktop Window" as their parent.
    parent = GetWindowToParentTo(true);
  } else if (parent != HWND_MESSAGE) {
    CHECK(::IsWindow(parent));
  }

  HWND hwnd = CreateWindowEx(
      window_ex_style,
      class_name.empty() ? reinterpret_cast<wchar_t*>(GetWindowClassAtom())
                         : class_name.data(),
      NULL, window_style, -1, -1, 1, 1, parent, NULL, NULL, this);
  // First nccalcszie (during CreateWindow) for captioned windows is
  // deliberately ignored so force a second one here to get the right
  // non-client set up.
  if (hwnd && (window_style & WS_CAPTION)) {
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
  }

  // For custom window we the hwnd_ is assigned in WM_NCCREATE.
  if (class_name.empty()) {
    // The window procedure should have set the data for us.
    CheckWindowCreated(hwnd_);
    CHECK_EQ(this, GetWindowUserData(hwnd));
  } else {
    hwnd_ = hwnd;
    SetWindowUserData(hwnd, this);
  }

  scale_factor_ = GetScaleFactorForHWND(hwnd_);
}

Win32Window::~Win32Window() {
  if (!::IsWindow(hwnd_))  // handle already destroyed.
    return;

  if (::GetParent(hwnd_) == NULL)  // removing a child window.
    ::SetParent(hwnd_, NULL);

  SetWindowUserData(hwnd_, NULL);
  ::DestroyWindow(hwnd_);
}

HICON Win32Window::GetDefaultWindowIcon() const {
  return nullptr;
}

HICON Win32Window::GetSmallWindowIcon() const {
  return nullptr;
}

LRESULT Win32Window::OnWndProc(UINT message, WPARAM w_param, LPARAM l_param) {
  LRESULT result = 0;

  HWND hwnd = hwnd_;
  if (message == WM_NCDESTROY)
    hwnd_ = NULL;

  // Handle the message if it's in our message map; otherwise, let the system
  // handle it.
  if (!ProcessWindowMessage(hwnd, message, w_param, l_param, result))
    result = DefWindowProc(hwnd, message, w_param, l_param);

  return result;
}

BOOL Win32Window::ProcessWindowMessage(
    HWND, UINT, WPARAM, LPARAM, LRESULT&, DWORD dwMsgMapID) {
  return false;
}

// static
LRESULT CALLBACK Win32Window::WndProc(HWND hwnd,
                                     UINT message,
                                     WPARAM w_param,
                                     LPARAM l_param) {
  if (message == WM_NCCREATE) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(l_param);
    Win32Window* window = reinterpret_cast<Win32Window*>(cs->lpCreateParams);
    DCHECK(window);
    SetWindowUserData(hwnd, window);
    window->hwnd_ = hwnd;
    return TRUE;
  }

  Win32Window* window =
      reinterpret_cast<Win32Window*>(GetWindowUserData(hwnd));
  if (!window)
    return 0;

  return window->OnWndProc(message, w_param, l_param);
}

ATOM Win32Window::GetWindowClassAtom() {
  HICON icon = GetDefaultWindowIcon();
  HICON small_icon = GetSmallWindowIcon();
  ClassInfo info(CS_DBLCLKS, icon, small_icon);
  return State::GetCurrent()->GetClassRegistrar()->RetrieveClassAtom(info);
}

}  // namespace nu
