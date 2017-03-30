// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/color.h"

#import <Cocoa/Cocoa.h>

#include "nativeui/gfx/geometry/safe_integer_conversions.h"

namespace nu {

NSColor* Color::ToNSColor() const {
  return [NSColor colorWithCalibratedRed:r() / 255.0
                                   green:g() / 255.0
                                    blue:b() / 255.0
                                   alpha:a() / 255.0];
}

Color GetThemeColor(Color::Theme theme) {
  NSColor* color;
  if (theme == Color::Theme::Text)
    color = [NSColor textColor];
  else
    color = [NSColor blackColor];
  CGFloat red, green, blue, alpha;
  color = [color colorUsingColorSpace:[NSColorSpace deviceRGBColorSpace]];
  [color getRed:&red green:&green blue:&blue alpha:&alpha];
  return Color(ToRoundedInt(255. * alpha),
               ToRoundedInt(255. * red),
               ToRoundedInt(255. * green),
               ToRoundedInt(255. * blue));
}

}  // namespace nu
