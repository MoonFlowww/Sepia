#pragma once

#include "../rendering/canvas.hpp"
#include <fstream>
#include <string>

namespace sepia{ namespace output {

inline bool write_ppm(const rendering::Canvas& canvas, const std::string& path) {
  std::ofstream f(path, std::ios::binary);
  if (!f.is_open()) return false;

  f << "P6\n" << canvas.width() << " " << canvas.height() << "\n255\n";

  const u8* data = canvas.data();
  usize w = canvas.width();
  usize h = canvas.height();

  // Canvas is RGBA, PPM needs RGB
  for (usize y = 0; y < h; ++y) {
    for (usize x = 0; x < w; ++x) {
      usize idx = (y * w + x) * 4;
      f.put(static_cast<char>(data[idx + 0]));
      f.put(static_cast<char>(data[idx + 1]));
      f.put(static_cast<char>(data[idx + 2]));
    }
  }

  return f.good();
}

}} // NS sepia::output
