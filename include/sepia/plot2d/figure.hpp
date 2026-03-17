#pragma once

#include "../core/types.hpp"
#include "../params/params.hpp"
#include "../data/series.hpp"
#include "../plot2d/plot_entry.hpp"
#include "../plot2d/plot_command.hpp"
#include "../rendering/canvas.hpp"
#include "../rendering/transform.hpp"
#include "../rendering/text.hpp"
#include "../rendering/tick_engine.hpp"
#include <vector>
#include <string>
#include <cmath>

namespace sepia{ namespace plot2d {

// -------------------------------------------
// Figure — the central user-facing object.
//
// Usage:
//   Sepia::plot2d::Figure figure(700.0, 450.0);
//   figure.set_title("My Plot");
//   figure.set_xlabel("X");
//   figure.set_ylabel("Y");
//   figure.plot(x, y, n).data({.color = Color::red(), .width = 2.0});
//   figure.grid({.show = true, .major_color = {200,200,200}});
//   figure.render();
//   figure.save_ppm("out.ppm");
// -----------------------------------------------
class Figure {
public:
  Figure(f64 width, f64 height)
    : canvas_(static_cast<u32>(width), static_cast<u32>(height))
    , fig_w_(width), fig_h_(height) {}

  // --- Title & labels ---
  void set_title(const std::string& t)  { title_ = t; }
  void set_xlabel(const std::string& l) { xlabel_ = l; }
  void set_ylabel(const std::string& l) { ylabel_ = l; }

  // --- Parameter cluster setters (figure-level) ---
  void grid(const params::GridStyle& g)     { grid_style_ = g; }
  void axis(const params::AxisStyle& a)     { axis_style_ = a; }
  void legend(const params::LegendStyle& l) { legend_style_ = l; }
  void layout(const params::LayoutStyle& l) { layout_style_ = l; }
  void text(const params::TextStyle& t)     { text_style_ = t; }
  void perf(const params::PerfParams& p)    { perf_ = p; }

  // --- Plot commands — return a fluent builder ---

  // Owning: copies data into aligned storage
  PlotCommand plot(const f64* x, const f64* y, usize n) {
    return PlotCommand(*this, data::Series(x, y, n));
  }

  // Owning: move-in pre-built series
  PlotCommand plot(data::Series&& s) {
    return PlotCommand(*this, std::move(s));
  }

  // Non-owning: zero-copy reference to user memory
  PlotCommand plot_ref(const f64* x, const f64* y, usize n, usize stride = 1) {
    return PlotCommand(*this, data::ExternalSeries(x, y, n, stride));
  }

  // --- Rendering ---
  void render() {
    canvas_.clear(layout_style_.background);
    compute_plot_area();
    compute_data_bounds();
    setup_transform();
    render_grid();
    render_axes();
    render_data();
    render_legend();
    render_title_and_labels();
  }

  const rendering::Canvas& canvas() const { return canvas_; }
  rendering::Canvas& canvas() { return canvas_; }

  bool save_ppm(const std::string& path) const;

  // Access to internals for custom renderers
  const std::vector<PlotEntry>& entries() const { return entries_; }

private:
  friend class PlotCommand;

  void add_entry(PlotEntry&& e) { entries_.push_back(std::move(e)); }

  // -- Layout ---
  void compute_plot_area() {
    plot_area_ = {
      layout_style_.margin_left,
      layout_style_.margin_top,
      fig_w_ - layout_style_.margin_left - layout_style_.margin_right,
      fig_h_ - layout_style_.margin_top  - layout_style_.margin_bottom
    };
  }

  void compute_data_bounds() {
    data_bounds_ = {};
    for (auto& e : entries_)
    data_bounds_.merge(e.bounds());

    // Apply manual overrides
    if (!std::isnan(axis_style_.x_min)) data_bounds_.x_min = axis_style_.x_min;
    if (!std::isnan(axis_style_.x_max)) data_bounds_.x_max = axis_style_.x_max;
    if (!std::isnan(axis_style_.y_min)) data_bounds_.y_min = axis_style_.y_min;
    if (!std::isnan(axis_style_.y_max)) data_bounds_.y_max = axis_style_.y_max;

    // Add padding (5% linear, or 0.15 decades for log scale)
    if (!data_bounds_.empty()) {
      if (axis_style_.x_scale == ScaleType::Log && data_bounds_.x_min > 0) {
        f64 log_lo = std::log10(data_bounds_.x_min);
        f64 log_hi = std::log10(data_bounds_.x_max);
        f64 pad = (log_hi - log_lo) * 0.05;
        if (pad == 0) pad = 0.15;
        data_bounds_.x_min = std::pow(10.0, log_lo - pad);
        data_bounds_.x_max = std::pow(10.0, log_hi + pad);
      } else {
        f64 xpad = data_bounds_.width() * 0.05;
        if (xpad == 0) xpad = 1.0;
        data_bounds_.x_min -= xpad;
        data_bounds_.x_max += xpad;
      }

      if (axis_style_.y_scale == ScaleType::Log && data_bounds_.y_min > 0) {
        f64 log_lo = std::log10(data_bounds_.y_min);
        f64 log_hi = std::log10(data_bounds_.y_max);
        f64 pad = (log_hi - log_lo) * 0.05;
        if (pad == 0) pad = 0.15;
        data_bounds_.y_min = std::pow(10.0, log_lo - pad);
        data_bounds_.y_max = std::pow(10.0, log_hi + pad);
      } else {
        f64 ypad = data_bounds_.height() * 0.05;
        if (ypad == 0) ypad = 1.0;
        data_bounds_.y_min -= ypad;
        data_bounds_.y_max += ypad;
      }
    }
  }

  void setup_transform() {
    transform_.set(data_bounds_, plot_area_, axis_style_.x_scale, axis_style_.y_scale);
  }

  // --- Render passes ---
  void render_grid() {
    if (!grid_style_.show) return;
    auto xticks = rendering::TickEngine::compute(
      data_bounds_.x_min,
      data_bounds_.x_max,
      8,
      grid_style_.show_minor,
      axis_style_.x_scale
    );
    auto yticks = rendering::TickEngine::compute(
      data_bounds_.y_min,
      data_bounds_.y_max,
      6,
      grid_style_.show_minor,
      axis_style_.y_scale
    );

    for (auto& t : xticks) {
      f64 px = transform_.to_px_x(t.value);
      Color c = t.is_minor ? grid_style_.minor_color : grid_style_.major_color;
      f64 w = t.is_minor ? grid_style_.minor_width : grid_style_.major_width;
      f64 a = t.is_minor ? grid_style_.minor_alpha : grid_style_.major_alpha;
      c = c.with_alpha(static_cast<u32>(a * 255));
      canvas_.draw_line(px, plot_area_.y, px, plot_area_.y + plot_area_.h, c, w);
    }
    for (auto& t : yticks) {
      f64 py = transform_.to_px_y(t.value);
      Color c = t.is_minor ? grid_style_.minor_color : grid_style_.major_color;
      f64 w = t.is_minor ? grid_style_.minor_width : grid_style_.major_width;
      f64 a = t.is_minor ? grid_style_.minor_alpha : grid_style_.major_alpha;
      c = c.with_alpha(static_cast<u32>(a * 255));
      canvas_.draw_line(plot_area_.x, py, plot_area_.x + plot_area_.w, py, c, w);
    }
  }

  void render_axes() {
    if (!axis_style_.show) return;
    Color c = axis_style_.color;
    f64 w = axis_style_.width;
    f64 x0 = plot_area_.x, y0 = plot_area_.y;
    f64 x1 = x0 + plot_area_.w, y1 = y0 + plot_area_.h;

    // Box
    canvas_.draw_line(x0, y0, x1, y0, c, w);
    canvas_.draw_line(x1, y0, x1, y1, c, w);
    canvas_.draw_line(x1, y1, x0, y1, c, w);
    canvas_.draw_line(x0, y1, x0, y0, c, w);

    // Tick marks + labels
    auto xticks = rendering::TickEngine::compute(data_bounds_.x_min, data_bounds_.x_max, 8, false, axis_style_.x_scale);
    for (auto& t : xticks) {
      f64 px = transform_.to_px_x(t.value);
      canvas_.draw_line(px, y1, px, y1 + axis_style_.tick_size, c, w);
      if (!t.label.empty()) {
        i32 tw = rendering::text_width(t.label, 1);
        rendering::draw_text(
          canvas_,
          t.label,
          static_cast<i32>(px) - tw / 2,
          static_cast<i32>(y1 + axis_style_.tick_size + 3),
          text_style_.color,
          1
        );
      }
    }

    auto yticks = rendering::TickEngine::compute(data_bounds_.y_min, data_bounds_.y_max, 6, false, axis_style_.y_scale);
    for (auto& t : yticks) {
      f64 py = transform_.to_px_y(t.value);
      canvas_.draw_line(x0 - axis_style_.tick_size, py, x0, py, c, w);
      if (!t.label.empty()) {
        i32 tw = rendering::text_width(t.label, 1);
        rendering::draw_text(
          canvas_,
          t.label,
          static_cast<i32>(x0 - axis_style_.tick_size - 3) - tw,
          static_cast<i32>(py) - 3,
          text_style_.color, 
          1
        );
      }
    }
  }

  void render_data() {
    for (auto& entry : entries_) {
      auto xv = entry.x_view();
      auto yv = entry.y_view();
      const auto& st = entry.style;

      // LOD decimation if needed
      data::DataView rx = xv, ry = yv;
      data::Series decimated;
      if (perf_.lod_enable && xv.count >= perf_.lod_target_points) { // 1bit if, cheaper than regrouping both
        if(true){ // keep both if away from each others (stresstest 1T points 4K-ms current, 30K-ms regrouped)
          decimated = data::LttbDecimator::decimate(xv, yv, perf_.lod_target_points);
          rx = decimated.x_view();
          ry = decimated.y_view();
        }
      }

      Color c = st.color;
      if (st.alpha < 1.0)
        c = c.with_alpha(static_cast<u32>(st.alpha * 255));

      // Lines
      if (st.line_style != LineStyle::None && rx.count > 1) {
        for (usize i = 1; i < rx.count; ++i) {
          f64 px0 = transform_.to_px_x(rx[i-1]);
          f64 py0 = transform_.to_px_y(ry[i-1]);
          f64 px1 = transform_.to_px_x(rx[i]);
          f64 py1 = transform_.to_px_y(ry[i]);
          canvas_.draw_line(px0, py0, px1, py1, c, st.width);
        }
      }

      // Fill under curve
      if (st.fill && rx.count > 1) {
        f64 base_py = transform_.to_px_y(data_bounds_.y_min);
        for (usize i = 0; i < rx.count; ++i) {
          f64 px = transform_.to_px_x(rx[i]);
          f64 py = transform_.to_px_y(ry[i]);
          i32 ix = static_cast<i32>(px);
          i32 iy_top = static_cast<i32>(py);
          i32 iy_bot = static_cast<i32>(base_py);
          for (i32 y = iy_top; y <= iy_bot; ++y)
            canvas_.set_pixel(ix, y, st.fill_color);
        }
      }

      // Markers
      if (st.marker != MarkerStyle::None) {
        for (usize i = 0; i < rx.count; ++i) {
          i32 px = static_cast<i32>(transform_.to_px_x(rx[i]));
          i32 py = static_cast<i32>(transform_.to_px_y(ry[i]));
          canvas_.draw_circle(px, py, st.marker_size, c);
        }
      }
    }
  }

  void render_legend() {
    if (!legend_style_.show) return;
    // Collect labeled entries
    std::vector<const PlotEntry*> labeled;
    for (auto& e : entries_)
    if (!e.style.label.empty()) labeled.push_back(&e);
    if (labeled.empty()) return;

    i32 scale = 1;
    i32 line_h = rendering::text_height(scale) + 4;
    i32 max_w = 0;
    for (auto* e : labeled) {
      i32 w = rendering::text_width(e->style.label, scale);
      if (w > max_w) max_w = w;
    }

    i32 box_w = max_w + 30 + static_cast<i32>(legend_style_.padding * 2);
    i32 box_h = static_cast<i32>(labeled.size()) * line_h
      + static_cast<i32>(legend_style_.padding * 2);

    // Position
    i32 bx = static_cast<i32>(plot_area_.x + plot_area_.w) - box_w - 8;
    i32 by = static_cast<i32>(plot_area_.y) + 8;

    canvas_.fill_rect(bx, by, box_w, box_h, legend_style_.bg_color);
    canvas_.draw_rect(bx, by, box_w, box_h, legend_style_.border);

    i32 ty = by + static_cast<i32>(legend_style_.padding);
    for (auto* e : labeled) {
      i32 lx = bx + static_cast<i32>(legend_style_.padding);
      // Color swatch line
      canvas_.draw_line(
        lx, 
        ty + line_h / 2,
        lx + 18,
        ty + line_h / 2,
        e->style.color,
        2.0
      );

      rendering::draw_text(canvas_, e->style.label, lx + 24, ty, text_style_.color, scale);
      ty += line_h;
    }
  }

  void render_title_and_labels() {
    i32 scale = 2;
    // Title (centered, top)
    if (!title_.empty()) {
      i32 tw = rendering::text_width(title_, scale);
      i32 tx = static_cast<i32>(fig_w_ / 2) - tw / 2;
      rendering::draw_text(canvas_, title_, tx, 8, text_style_.color, scale);
    }

    // X label (centered, bottom)
    scale = 1;
    if (!xlabel_.empty()) {
      i32 tw = rendering::text_width(xlabel_, scale);
      i32 tx = static_cast<i32>(plot_area_.x + plot_area_.w / 2) - tw / 2;
      i32 ty = static_cast<i32>(fig_h_) - 15;
      rendering::draw_text(canvas_, xlabel_, tx, ty, text_style_.color, scale);
    }

    // Y label 
    if (!ylabel_.empty()) {
      i32 lh = rendering::text_height_vertical(ylabel_, scale);
      i32 tx = 5;
      i32 ty = static_cast<i32>(plot_area_.y + plot_area_.h/2)-lh/2;
      rendering::draw_text_vertical(canvas_, ylabel_, tx, ty, text_style_.color, scale);
    }
  }

  // ---State ---
  rendering::Canvas canvas_;
  f64 fig_w_, fig_h_;
  Rect plot_area_;
  BBox data_bounds_;
  rendering::CoordTransform transform_;

  std::string title_, xlabel_, ylabel_;

  params::GridStyle grid_style_;
  params::AxisStyle axis_style_;
  params::LegendStyle legend_style_;
  params::LayoutStyle layout_style_;
  params::TextStyle text_style_;
  params::PerfParams perf_;

  std::vector<PlotEntry> entries_;
};

}} // NS sepia::plot2d
