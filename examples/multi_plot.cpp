#include "../include/sepia/sepia.hpp"
#include <cmath>
#include <vector>

int main() {
  const size_t N = 300;
  std::vector<sepia::f64> x(N), y1(N), y2(N), y3(N);

  for (size_t i = 0; i < N; ++i) {
    x[i]  = static_cast<double>(i) * 0.04;
    y1[i] = std::sin(x[i]);
    y2[i] = std::cos(x[i]) * 0.8;
    y3[i] = std::sin(x[i] * 2.0) * 0.5;
  }

  Sepia::plot2d::Figure figure(700.0, 450.0);
  figure.set_title("Multi-Plot Parameters");
  figure.set_xlabel("Time");
  figure.set_ylabel("Signal");

  figure.plot(x.data(), y1.data(), N)
    .data({.color = sepia::Color::blue(), .width = 2.0, .label = "sin(x)"});

  figure.plot(x.data(), y2.data(), N)
    .data({.color = sepia::Color::red(), .width = 1.5, .alpha = 0.9, .label = "cos(x)"});

  figure.plot(x.data(), y3.data(), N)
    .data({.color = sepia::Color::green(), .width = 1.0,
      .line_style = sepia::LineStyle::Dashed, .label = "sin(2x)"});

  figure.grid({.show = true, .show_minor = true});
  figure.layout({.margin_left = 80.0, .background = sepia::Color::white()});

  figure.render();
  figure.save_ppm("multi_plot.ppm");

  return 0;
}
