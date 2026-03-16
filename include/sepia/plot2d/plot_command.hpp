#pragma once

#include "../core/types.hpp"
#include "../params/params.hpp"
#include "../data/series.hpp"
#include "../plot2d/plot_entry.hpp"

namespace sepia{ namespace plot2d {

//-------------------------------------- ---
// PlotCommand — fluent builder returned by figure.plot(x, y, n).
// Method chaining: figure.plot(x,y,n).data({...}).grid({...});
// Each sub-function attaches a parameter cluster.
// --------------------------------------------

class Figure;

class PlotCommand {
public:
  PlotCommand(Figure& fig, data::Series&& series)
  : fig_(fig), entry_{std::move(series), {}} {}

  PlotCommand(Figure& fig, data::ExternalSeries&& series)
  : fig_(fig), entry_{std::move(series), {}} {}

  // Parameter cluster: data visual style
  PlotCommand& data(const params::DataStyle& style) {
    entry_.style = style;
    return *this;
  }

  PlotCommand& color(Color c) { entry_.style.color = c; return *this; }
  PlotCommand& width(f64 w) { entry_.style.width = w; return *this; }
  PlotCommand& alpha(f64 a) { entry_.style.alpha = a; return *this; }
  PlotCommand& label(const std::string& l) { entry_.style.label = l; return *this; }
  PlotCommand& line(LineStyle s) { entry_.style.line_style = s; return *this; }
  PlotCommand& marker(MarkerStyle m, f64 size = 4.0) {
    entry_.style.marker = m;
    entry_.style.marker_size = size;
    return *this;
  }
  PlotCommand& fill(bool on = true, Color c = Color::blue().with_alpha(50)) {
    entry_.style.fill = on;
    entry_.style.fill_color = c;
    return *this;
  }

  ~PlotCommand();

private:
  Figure& fig_;
  PlotEntry entry_;
  bool committed_ = false;

  void commit();
  friend class Figure;
};

}} // NS sepia::plot2d
