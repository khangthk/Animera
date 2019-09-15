//
//  graphics convert.hpp
//  Animera
//
//  Created by Indi Kernick on 15/9/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef graphics_convert_hpp
#define graphics_convert_hpp

#include <QtCore/qrect.h>
#include <Graphics/geometry.hpp>

constexpr gfx::Point convert(const QPoint p) {
  return {p.x(), p.y()};
}

constexpr gfx::Size convert(const QSize s) {
  return {s.width(), s.height()};
}

constexpr gfx::Rect convert(const QRect r) {
  return {{r.x(), r.y()}, {r.width(), r.height()}};
}

constexpr QRect convert(const gfx::Rect r) {
  return {r.p.x, r.p.y, r.s.w, r.s.h};
}

#endif
