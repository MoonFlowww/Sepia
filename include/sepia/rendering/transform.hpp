#pragma once

#include "../core/types.hpp"
#include <cmath>

namespace sepia{ namespace rendering {

// -----------
// CoordTransform | maps data-space to pixel-space and back.
// Hot path: called per-point. Kept minimal & inlineable.
// Supports linear and logarithmic scales on each axis.
// -----------
class CoordTransform {
public:
  CoordTransform() = default;

  void set(const BBox& data_box, const Rect& pixel_rect,
           ScaleType x_scale = ScaleType::Linear,
           ScaleType y_scale = ScaleType::Linear) {
    data_ = data_box;
    rect_ = pixel_rect;
    x_log_ = (x_scale == ScaleType::Log);
    y_log_ = (y_scale == ScaleType::Log);

    f64 x_lo = x_log_ ? safe_log10(data_box.x_min) : data_box.x_min;
    f64 x_hi = x_log_ ? safe_log10(data_box.x_max) : data_box.x_max;
    f64 y_lo = y_log_ ? safe_log10(data_box.y_min) : data_box.y_min;
    f64 y_hi = y_log_ ? safe_log10(data_box.y_max) : data_box.y_max;

    f64 dw = x_hi - x_lo;
    f64 dh = y_hi - y_lo;

    sx_ = (dw > 0) ? pixel_rect.w / dw : 1.0;
    sy_ = (dh > 0) ? pixel_rect.h / dh : 1.0;
    log_x_lo_ = x_lo;
    log_y_lo_ = y_lo;
  }

  // Data -> pixel
  inline f64 to_px_x(f64 x) const {
    f64 v = x_log_ ? safe_log10(x) : x;
    return rect_.x + (v - log_x_lo_) * sx_;
  }
  inline f64 to_px_y(f64 y) const {
    f64 v = y_log_ ? safe_log10(y) : y;
    return rect_.y + rect_.h - (v - log_y_lo_) * sy_;
  }

  // Pixel -> data
  inline f64 to_data_x(f64 px) const {
    f64 v = log_x_lo_ + (px - rect_.x) / sx_;
    return x_log_ ? std::pow(10.0, v) : v;
  }
  inline f64 to_data_y(f64 py) const {
    f64 v = log_y_lo_ + (rect_.y + rect_.h - py) / sy_;
    return y_log_ ? std::pow(10.0, v) : v;
  }

  const BBox& data_box() const { return data_; }
  const Rect& pixel_rect() const { return rect_; }

private:
  static inline f64 safe_log10(f64 v) {
    return std::log10(v > 1e-300 ? v : 1e-300);
  }

  BBox data_;
  Rect rect_;
  f64 sx_ = 1.0, sy_ = 1.0;
  f64 log_x_lo_ = 0.0, log_y_lo_ = 0.0;
  bool x_log_ = false, y_log_ = false;
};

}} // NS sepia::rendering
