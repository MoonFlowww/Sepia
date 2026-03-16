#pragma once

#include "../core/types.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <cstdio>

namespace sepia{ namespace rendering {

struct Tick {
  f64         value;
  std::string label;
  bool        is_minor = false;
};

class TickEngine {
public:
  static std::vector<Tick> compute(f64 lo, f64 hi, usize target_ticks = 6,
                                   bool include_minor = false,
                                   ScaleType scale = ScaleType::Linear) {
    if (scale == ScaleType::Log)
      return compute_log(lo, hi, include_minor);
    return compute_linear(lo, hi, target_ticks, include_minor);
  }

private:
  static std::vector<Tick> compute_linear(f64 lo, f64 hi, usize target_ticks, bool include_minor) {
    std::vector<Tick> ticks;
    if (hi <= lo) return ticks;

    f64 range = hi - lo;
    f64 rough_step = range / static_cast<f64>(target_ticks);
    f64 mag = std::pow(10.0, std::floor(std::log10(rough_step)));
    f64 norm = rough_step / mag;

    f64 nice;
    if (norm < 1.5)      nice = 1.0;
    else if (norm < 3.0) nice = 2.0;
    else if (norm < 7.0) nice = 5.0;
    else                 nice = 10.0;

    f64 step = nice * mag;
    f64 start = std::floor(lo / step) * step;

    for (f64 v = start; v <= hi + step * 0.001; v += step) {
      if (v < lo - step * 0.001) continue;
      ticks.push_back({v, format_value(v, step), false});

      if (include_minor) {
        f64 minor_step = step / 5.0;
        for (int m = 1; m < 5; ++m) {
          f64 mv = v + m * minor_step;
          if (mv > lo && mv < hi)
            ticks.push_back({mv, "", true});
        }
      }
    }
    return ticks;
  }

  // Log-scale ticks: major ticks at powers of 10, minor at 2..9 within each decade
  static std::vector<Tick> compute_log(f64 lo, f64 hi, bool include_minor) {
    std::vector<Tick> ticks;
    if (hi <= 0 || lo <= 0) return ticks;

    int exp_lo = static_cast<int>(std::floor(std::log10(lo)));
    int exp_hi = static_cast<int>(std::ceil(std::log10(hi)));

    for (int e = exp_lo; e <= exp_hi; ++e) {
      f64 val = std::pow(10.0, e);
      if (val >= lo && val <= hi)
        ticks.push_back({val, format_log_value(val), false});

      if (include_minor) {
        for (int m = 2; m <= 9; ++m) {
          f64 mv = m * val;
          if (mv > lo && mv < hi)
            ticks.push_back({mv, "", true});
        }
      }
    }
    return ticks;
  }

  static std::string format_value(f64 v, f64 step) {
    char buf[64];
    if (step >= 1.0 && std::abs(v) < 1e12) {
      if (std::abs(v - std::round(v)) < 1e-9)
        std::snprintf(buf, sizeof(buf), "%.0f", v);
      else
        std::snprintf(buf, sizeof(buf), "%.1f", v);
    } else if (step >= 0.01) {
      std::snprintf(buf, sizeof(buf), "%.2f", v);
    } else {
      std::snprintf(buf, sizeof(buf), "%.4g", v);
    }
    return buf;
  }

  static std::string format_log_value(f64 v) {
    char buf[64];
    if (v >= 1.0 && v < 1e7) {
      std::snprintf(buf, sizeof(buf), "%.0f", v);
    } else {
      std::snprintf(buf, sizeof(buf), "%.0e", v);
    }
    return buf;
  }
};

}} // NS sepia::rendering
