#include "catch_amalgamated.hpp"
#include "sepia.hpp"

using namespace Sepia;
using namespace Sepia::rendering;

// Shared fixture: data [0,10]x[0,10], pixel rect x=100 y=100 w=200 h=200
static CoordTransform make_linear() {
    BBox box; box.expand(0, 0); box.expand(10, 10);
    detail::Rect rect(100, 100, 200, 200);
    CoordTransform t;
    t.set(box, rect, ScaleType::Linear, ScaleType::Linear);
    return t;
}

static CoordTransform make_log() {
    BBox box; box.expand(1, 1); box.expand(100, 100);
    detail::Rect rect(100, 100, 200, 200);
    CoordTransform t;
    t.set(box, rect, ScaleType::Log, ScaleType::Log);
    return t;
}

TEST_CASE("CoordTransform linear x forward") {
    auto t = make_linear();
    // sx = pw/dw = 200/10 = 20; to_px_x(5) = rect.x + (5-0)*20 = 200
    CHECK(t.to_px_x(5) == Catch::Approx(200.0));
}

TEST_CASE("CoordTransform linear y forward (y-axis inverted)") {
    auto t = make_linear();
    // sy = ph/dh = 200/10 = 20; to_px_y(5) = rect.y + ph - (5-0)*20 = 200
    CHECK(t.to_px_y(5) == Catch::Approx(200.0));
}

TEST_CASE("CoordTransform linear round-trip x") {
    auto t = make_linear();
    CHECK(t.to_data_x(t.to_px_x(3.7)) == Catch::Approx(3.7).epsilon(1e-12));
}

TEST_CASE("CoordTransform linear round-trip y") {
    auto t = make_linear();
    CHECK(t.to_data_y(t.to_px_y(6.2)) == Catch::Approx(6.2).epsilon(1e-12));
}

TEST_CASE("CoordTransform linear boundary min") {
    auto t = make_linear();
    // x=0 -> left edge of plot; y=10 (max data) -> top edge of plot
    CHECK(t.to_px_x(0)  == Catch::Approx(100.0));
    CHECK(t.to_px_y(10) == Catch::Approx(100.0));
}

TEST_CASE("CoordTransform linear boundary max") {
    auto t = make_linear();
    // x=10 -> right edge; y=0 (min data) -> bottom edge
    CHECK(t.to_px_x(10) == Catch::Approx(300.0));
    CHECK(t.to_px_y(0)  == Catch::Approx(300.0));
}

TEST_CASE("CoordTransform log x midpoint") {
    auto t = make_log();
    // log10(1)=0, log10(100)=2 -> range=2; sx=200/2=100
    // to_px_x(10) = 100 + (log10(10)-0)*100 = 200 (midpoint of [1,100] in log space)
    CHECK(t.to_px_x(10) == Catch::Approx(200.0).epsilon(1e-9));
}

TEST_CASE("CoordTransform log round-trip x") {
    auto t = make_log();
    // relative tolerance since values span [1,100]
    CHECK(t.to_data_x(t.to_px_x(50)) == Catch::Approx(50.0).epsilon(50.0 * 1e-9));
}
