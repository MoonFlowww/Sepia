#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace sepia {

// ---------- scalar types ----------
using f32 = float;
using f64 = double;
using i32 = std::int32_t;
using i64 = std::int64_t;
using u8  = std::uint8_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;

// ---------- color ----------
struct Color {
  u32 r = 0, g = 0, b = 0, a = 255;

  constexpr Color() = default;
  constexpr Color(u32 r, u32 g, u32 b, u32 a = 255) : r(r), g(g), b(b), a(a) {}

  static constexpr Color black()   { return {0, 0, 0}; }
  static constexpr Color white()   { return {255, 255, 255}; }
  static constexpr Color red()     { return {228, 26, 28}; }
  static constexpr Color blue()    { return {55, 126, 184}; }
  static constexpr Color green()   { return {77, 175, 74}; }
  static constexpr Color orange()  { return {255, 127, 0}; }
  static constexpr Color purple()  { return {152, 78, 163}; }
  static constexpr Color gray()    { return {150, 150, 150}; }

  constexpr Color with_alpha(u32 alpha) const { return {r, g, b, alpha}; }
};

// ---------- bounding box ----------
struct BBox {
  f64 x_min =  std::numeric_limits<f64>::max();
  f64 x_max = -std::numeric_limits<f64>::max();
  f64 y_min =  std::numeric_limits<f64>::max();
  f64 y_max = -std::numeric_limits<f64>::max();

  constexpr bool empty() const { return x_min > x_max; }

  void expand(f64 x, f64 y) {
    if (x < x_min) x_min = x;
    if (x > x_max) x_max = x;
    if (y < y_min) y_min = y;
    if (y > y_max) y_max = y;
  }

  void merge(const BBox& o) {
    if (o.empty()) return;
    expand(o.x_min, o.y_min);
    expand(o.x_max, o.y_max);
  }

  f64 width()  const { return x_max - x_min; }
  f64 height() const { return y_max - y_min; }
};

// ---------- rectangle (pixel space) ----------
struct Rect {
  f64 x = 0, y = 0, w = 0, h = 0;

  constexpr Rect() = default;
  constexpr Rect(f64 x, f64 y, f64 w, f64 h) : x(x), y(y), w(w), h(h) {}
};

// ---------- enums ----------
enum class LineStyle : u32 {
  Solid,
  Dashed,
  Dotted,
  DashDot,
  None,
};

enum class MarkerStyle : u32 {
  None,
  Circle,
  Square,
  Triangle,
  Cross,
  Diamond,
};

enum class ScaleType : u32 {
  Linear,
  Log,
};

} // ns sepia
