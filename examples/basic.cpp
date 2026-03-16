#include "../include/sepia/sepia.hpp"
#include <cmath>
#include <vector>

int main() {
  const size_t N = 200;
  std::vector<sepia::f64> x(N), y(N);
  for (size_t i = 0; i < N; ++i) {
    x[i] = static_cast<double>(i) * 0.05;
    y[i] = std::sin(x[i]);
  }

  Sepia::plot2d::Figure figure(700.0, 450.0);
  figure.set_title("Sine Wave");
  figure.set_xlabel("Time");
  figure.set_ylabel("Amplitude");

  figure.plot(x.data(), y.data(), N)
    .data({.color = sepia::Color::blue(), .width = 2.0, .label = "sin(x)"});

  figure.grid({.show = true, .major_color = {210, 210, 210}});

  figure.render();
  figure.save_ppm("basic.ppm");

  return 0;
}
