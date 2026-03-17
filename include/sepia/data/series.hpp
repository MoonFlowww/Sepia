#pragma once

#include "../core/types.hpp"
#include "../core/memory.hpp"
#include <cmath>
#include <cstring>

namespace sepia { namespace data {

// ------------------
// DataView — non-owning, zero-copy view into contiguous doubles.
// Core abstraction: all renderers consume DataViews.
// ------------------
struct DataView {
  const f64* ptr = nullptr;
  usize      count = 0;
  usize      stride = 1;  // element stride (1 = dense, >1 = interleaved / SoA skip)

  f64 operator[](usize i) const { return ptr[i * stride]; }
  bool empty() const { return count == 0 || ptr == nullptr; }
};

//------------
// Series | owns X/Y columns; aligned for vectorized passes.
// -----------
class Series {
public:
  Series() = default;

  // Construct from raw pointers (copies data into aligned storage)
  Series(const f64* x, const f64* y, usize n) : x_(n), y_(n) {
    std::memcpy(x_.data(), x, n * sizeof(f64));
    std::memcpy(y_.data(), y, n * sizeof(f64));
    recompute_bounds();
  }

  // Move-construct from aligned buffers
  Series(AlignedBuffer<f64>&& x, AlignedBuffer<f64>&& y) : x_(std::move(x)), y_(std::move(y)) {
    recompute_bounds();
  }

  DataView x_view() const { return {x_.data(), x_.size(), 1}; }
  DataView y_view() const { return {y_.data(), y_.size(), 1}; }

  usize size() const { return x_.size(); }
  const BBox& bounds() const { return bounds_; }

  f64*       x_data()       { return x_.data(); }
  f64*       y_data()       { return y_.data(); }
  const f64* x_data() const { return x_.data(); }
  const f64* y_data() const { return y_.data(); }

  void recompute_bounds() {
    bounds_ = {};
    const usize n = x_.size();
    for (usize i = 0; i < n; ++i)
      bounds_.expand(x_[i], y_[i]);
  }

private:
  AlignedBuffer<f64> x_;
  AlignedBuffer<f64> y_;
  BBox bounds_;
};

// --------------- 
// ExternalSeries | non-owning reference to user memory
// Zero-copy path when user already has contiguous f64 arrays
// ---------------
class ExternalSeries {
public:
  ExternalSeries() = default;
  ExternalSeries(const f64* x, const f64* y, usize n, usize stride = 1)
  : x_{x, n, stride}, y_{y, n, stride} {
    recompute_bounds();
  }

  DataView x_view() const { return x_; }
  DataView y_view() const { return y_; }
  usize size() const { return x_.count; }
  const BBox& bounds() const { return bounds_; }

  void recompute_bounds() {
    bounds_ = {};
    for (usize i = 0; i < x_.count; ++i)
      bounds_.expand(x_[i], y_[i]);
  }

private:
  DataView x_, y_;
  BBox bounds_;
};

// ------------------------ 
// LOD (Level-Of-Detail) decimation for mega-datasets.
// Largest-Triangle-Three-Buckets (LTTB) downsampler.
// -------------------------
class LttbDecimator {
public:
  // Decimate a dense (stride=1) series into 'target' points
  // Returns owned Series with decimated data
  static Series decimate(const DataView& xv, const DataView& yv, usize target) {
    const usize n = xv.count;
    if (target >= n || target < 3) {
      AlignedBuffer<f64> ox(n), oy(n);
      for (usize i = 0; i < n; ++i) { ox[i] = xv[i]; oy[i] = yv[i]; }
      return Series(std::move(ox), std::move(oy));
    }

    AlignedBuffer<f64> ox(target), oy(target);

    // Always keep first point
    ox[0] = xv[0];
    oy[0] = yv[0];

    const f64 bucket_size = static_cast<f64>(n - 2) / static_cast<f64>(target - 2);
    usize a = 0; // index of previously selected point

    for (usize i = 1; i < target - 1; ++i) {
      // Calculate bucket range
      usize bucket_start = static_cast<usize>((i - 1) * bucket_size) + 1;
      usize bucket_end   = static_cast<usize>(i * bucket_size) + 1;
      if (bucket_end > n - 1) bucket_end = n - 1;

      // Average of next bucket (for triangle area)
      usize next_start = static_cast<usize>(i * bucket_size) + 1;
      usize next_end   = static_cast<usize>((i + 1) * bucket_size) + 1;
      if (next_end > n) next_end = n;

      f64 avg_x = 0, avg_y = 0;
      usize next_count = next_end - next_start;
      for (usize j = next_start; j < next_end; ++j) {
        avg_x += xv[j];
        avg_y += yv[j];
      }
      if (next_count > 0) { avg_x /= next_count; avg_y /= next_count; }

      // Find point in current bucket with max triangle area
      f64 max_area = -1.0;
      usize max_idx = bucket_start;
      f64 ax = xv[a], ay = yv[a];

      for (usize j = bucket_start; j < bucket_end; ++j) {
        f64 area = std::abs(
          (ax - avg_x) * (yv[j] - ay) -
          (ax - xv[j]) * (avg_y - ay)
        ) * 0.5;
        if (area > max_area) { max_area = area; max_idx = j; }
      }

      ox[i] = xv[max_idx];
      oy[i] = yv[max_idx];
      a = max_idx;
    }

    // Always keep last point
    ox[target - 1] = xv[n - 1];
    oy[target - 1] = yv[n - 1];

    return Series(std::move(ox), std::move(oy));
  }
};

}} // NS sepia::data
