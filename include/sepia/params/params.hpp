#pragma once

#include "../core/types.hpp"
#include <string>

namespace sepia { namespace params {

// ------------------------------------------------
// Parameter clusters — POD structs with sane defaults.
// Users pass these via aggregate init:
//   .data({.color = Color::red(), .width = 2.0})
//   .grid({.show = true, .color = Color::gray()})
// ------------------------------------------------
// Controls the visual representation of data
struct DataStyle {
  Color color = Color::blue();
  f64 width = 1.5; // line width in px
  f64 alpha = 1.0; // opacity [0,1]
  LineStyle line_style = LineStyle::Solid;

  // Markers
  MarkerStyle marker = MarkerStyle::None;
  f64 marker_size = 4.0;

  // Fill under curve
  bool fill = false;
  Color fill_color = Color::blue().with_alpha(50);

  // Label for legend
  std::string label;
};

// Controls the grid
struct GridStyle {
  bool show = true;
  Color major_color = {220, 220, 220};
  Color minor_color = {240, 240, 240};
  f64 major_width = 1.0;
  f64 minor_width = 0.5;
  f64 major_alpha = 0.8;
  f64 minor_alpha = 0.4;
  bool show_minor = false;
};

// Controls axis appearance
struct AxisStyle {
  bool show = true;
  Color color = Color::black();
  f64 width = 1.0;
  f64 tick_size = 5.0;
  Color tick_color = Color::black();

  ScaleType x_scale = ScaleType::Linear;
  f64 x_min = 0.0;
  f64 x_max = 0.0;

  ScaleType y_scale = ScaleType::Linear;
  f64 y_min = 0.0;
  f64 y_max = 0.0;
};

// Controls legend
struct LegendStyle {
  bool show = true;
  Color bg_color = Color::white().with_alpha(230);
  Color border = Color::gray();
  f64 padding = 8.0;
  std::string position = "top-right"; // top-left, top-right, bottom-left, bottom-right
};

// Controls title / labels text rendering
struct TextStyle {
  Color color = Color::black();
  f64 font_size = 12.0;
  std::string font_face = "sans";
};

// Controls the overall figure layout margins
struct LayoutStyle {
  f64 margin_top = 50.0;
  f64 margin_bottom = 60.0;
  f64 margin_left = 70.0;
  f64 margin_right = 20.0;
  Color background = Color::white();
};

// Controls LOD / performance
struct PerfParams {
  bool lod_enable = false;  
  usize lod_target_points = 2000;   // target points after decimation
};

}} // NS sepia::params
