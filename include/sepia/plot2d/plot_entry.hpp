#pragma once

#include "../core/types.hpp"
#include "../params/params.hpp"
#include "../data/series.hpp"
#include <variant>

namespace sepia{ namespace plot2d {

// -------------------------------- ------
// PlotEntry — one .plot() call stored for deferred rendering.
// Holds either an owned Series or a non-owning ExternalSeries.
//------------------------------------------
using SeriesVariant = std::variant<data::Series, data::ExternalSeries>;

struct PlotEntry {
  SeriesVariant         series;
  params::DataStyle     style;

  data::DataView x_view() const {
    return std::visit([](auto& s) { return s.x_view(); }, series);
  }
  data::DataView y_view() const {
    return std::visit([](auto& s) { return s.y_view(); }, series);
  }
  usize size() const {
    return std::visit([](auto& s) { return s.size(); }, series);
  }
  BBox bounds() const {
    return std::visit([](auto& s) { return s.bounds(); }, series);
  }
};

}} // NS sepia::plot2d
