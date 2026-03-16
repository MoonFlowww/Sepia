#pragma once

#include "../core/types.hpp"
#include "../core/memory.hpp"
#include <algorithm>
#include <cmath>

namespace sepia { namespace rendering {

// --------------------
// Canvas — raw RGBA pixel buffer, Agg-compatible.
// All rendering ultimately writes to a Canvas.
// --------------------
class Canvas {
public:
  Canvas() = default;

  Canvas(u32 width, u32 height)
    : width_(width), height_(height)
    , pixels_(static_cast<usize>(width) * height * 4)
  {
    clear({255, 255, 255, 255});
  }

  void clear(Color c) {
    u32* px = reinterpret_cast<u32*>(pixels_.data());
    u32 packed = pack(c);
    usize total = static_cast<usize>(width_) * height_;
    std::fill(px, px + total, packed);
  }

  // Alpha-blended pixel write
  void set_pixel(i32 x, i32 y, Color c) {
    if (x < 0 || y < 0 || x >= static_cast<i32>(width_) || y >= static_cast<i32>(height_)) return;
    usize idx = (static_cast<usize>(y) * width_ + x) * 4;
    u8* dst = pixels_.data() + idx;
    if (c.a == 255) {
      dst[0] = static_cast<u8>(c.r);
      dst[1] = static_cast<u8>(c.g);
      dst[2] = static_cast<u8>(c.b);
      dst[3] = 255;
    } else {
      u32 sa = c.a;
      u32 da = 255 - sa;
      dst[0] = static_cast<u8>((c.r * sa + dst[0] * da) / 255);
      dst[1] = static_cast<u8>((c.g * sa + dst[1] * da) / 255);
      dst[2] = static_cast<u8>((c.b * sa + dst[2] * da) / 255);
      dst[3] = static_cast<u8>(sa + (dst[3] * da) / 255);
    }
  }

  // Bresenham thick line (anti-aliased version delegated to Agg backend)
  void draw_line(f64 x0, f64 y0, f64 x1, f64 y1, Color c, f64 width = 1.0) {
    // Xiaolin Wu-style anti-aliased line
    draw_line_aa(x0, y0, x1, y1, c, width);
  }

  void fill_rect(i32 x, i32 y, i32 w, i32 h, Color c) {
    i32 x0 = std::max(x, 0);
    i32 y0 = std::max(y, 0);
    i32 x1 = std::min(x + w, static_cast<i32>(width_));
    i32 y1 = std::min(y + h, static_cast<i32>(height_));
    for (i32 py = y0; py < y1; ++py)
      for (i32 px = x0; px < x1; ++px)
        set_pixel(px, py, c);
  }

  void draw_rect(i32 x, i32 y, i32 w, i32 h, Color c, f64 line_w = 1.0) {
    draw_line(x, y, x + w, y, c, line_w);
    draw_line(x + w, y, x + w, y + h, c, line_w);
    draw_line(x + w, y + h, x, y + h, c, line_w);
    draw_line(x, y + h, x, y, c, line_w);
  }

  void draw_circle(i32 cx, i32 cy, f64 radius, Color c) {
    i32 r = static_cast<i32>(radius);
    for (i32 dy = -r; dy <= r; ++dy) {
      for (i32 dx = -r; dx <= r; ++dx) {
        if (dx * dx + dy * dy <= r * r)
          set_pixel(cx + dx, cy + dy, c);
      }
    }
  }

  u8*       data()         { return pixels_.data(); }
  const u8* data()   const { return pixels_.data(); }
  u32       width()  const { return width_; }
  u32       height() const { return height_; }
  usize     stride() const { return static_cast<usize>(width_) * 4; }

private:
  static u32 pack(Color c) {
    return static_cast<u32>(c.r)
    | (static_cast<u32>(c.g) << 8)
    | (static_cast<u32>(c.b) << 16)
    | (static_cast<u32>(c.a) << 24);
  }

  // Xiaolin Wu anti-aliased line
  void draw_line_aa(f64 x0, f64 y0, f64 x1, f64 y1, Color c, f64 /*width*/) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }

    f64 dx = x1 - x0;
    f64 dy = y1 - y0;
    f64 gradient = (dx == 0.0) ? 1.0 : dy / dx;

    // First endpoint
    f64 xend = std::round(x0);
    f64 yend = y0 + gradient * (xend - x0);
    f64 xgap = rfpart(x0 + 0.5);
    i32 xpxl1 = static_cast<i32>(xend);
    i32 ypxl1 = static_cast<i32>(std::floor(yend));
    if (steep) {
      plot(ypxl1, xpxl1, c, rfpart(yend) * xgap);
      plot(ypxl1 + 1, xpxl1, c, fpart(yend) * xgap);
    } else {
      plot(xpxl1, ypxl1, c, rfpart(yend) * xgap);
      plot(xpxl1, ypxl1 + 1, c, fpart(yend) * xgap);
    }
    f64 intery = yend + gradient;

    // Second endpoint
    xend = std::round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5);
    i32 xpxl2 = static_cast<i32>(xend);
    i32 ypxl2 = static_cast<i32>(std::floor(yend));
    if (steep) {
      plot(ypxl2, xpxl2, c, rfpart(yend) * xgap);
      plot(ypxl2 + 1, xpxl2, c, fpart(yend) * xgap);
    } else {
      plot(xpxl2, ypxl2, c, rfpart(yend) * xgap);
      plot(xpxl2, ypxl2 + 1, c, fpart(yend) * xgap);
    }

    // Main loop
    for (i32 x = xpxl1 + 1; x < xpxl2; ++x) {
      i32 iy = static_cast<i32>(std::floor(intery));
      if (steep) {
        plot(iy, x, c, rfpart(intery));
        plot(iy + 1, x, c, fpart(intery));
      } else {
        plot(x, iy, c, rfpart(intery));
        plot(x, iy + 1, c, fpart(intery));
      }
      intery += gradient;
    }
  }

  static f64 fpart(f64 x) { return x - std::floor(x); }
  static f64 rfpart(f64 x) { return 1.0 - fpart(x); }

  void plot(i32 x, i32 y, Color c, f64 brightness) {
    u32 a = static_cast<u32>(c.a * brightness);
    set_pixel(x, y, {c.r, c.g, c.b, a});
  }

  u32 width_ = 0;
  u32 height_ = 0;
  AlignedBuffer<u8> pixels_;
};

}} // NS sepia::rendering
