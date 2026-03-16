#include "../../include/sepia/plot2d/figure.hpp"
#include "../../include/sepia/output/ppm_writer.hpp"

namespace sepia{ namespace plot2d {

PlotCommand::~PlotCommand() {
    if (!committed_) commit();
}

void PlotCommand::commit() {
    if (committed_) return;
    committed_ = true;
    fig_.add_entry(std::move(entry_));
}

// Figure save
bool Figure::save_ppm(const std::string& path) const {
    return output::write_ppm(canvas_, path);
}

}} // NS sepia::plot2d
