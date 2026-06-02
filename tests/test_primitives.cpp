#include "catch_amalgamated.hpp"
#include "sepia.hpp"

using namespace Sepia;

// --- BBox ---

TEST_CASE("BBox default is empty") {
    BBox b;
    CHECK(b.empty());
}

TEST_CASE("BBox expand single point") {
    BBox b;
    b.expand(3.0, 7.0);
    CHECK_FALSE(b.empty());
    CHECK(b.x_min == Catch::Approx(3.0));
    CHECK(b.x_max == Catch::Approx(3.0));
    CHECK(b.y_min == Catch::Approx(7.0));
    CHECK(b.y_max == Catch::Approx(7.0));
}

TEST_CASE("BBox expand two points sets correct min/max") {
    BBox b;
    b.expand(5.0, 1.0);
    b.expand(2.0, 9.0);
    CHECK(b.x_min == Catch::Approx(2.0));
    CHECK(b.x_max == Catch::Approx(5.0));
    CHECK(b.y_min == Catch::Approx(1.0));
    CHECK(b.y_max == Catch::Approx(9.0));
    CHECK(b.width()  == Catch::Approx(3.0));
    CHECK(b.height() == Catch::Approx(8.0));
}

TEST_CASE("BBox merge two non-overlapping boxes") {
    BBox a, b;
    a.expand(0, 0); a.expand(5, 5);
    b.expand(3, 3); b.expand(10, 10);
    a.merge(b);
    CHECK(a.x_min == Catch::Approx(0.0));
    CHECK(a.x_max == Catch::Approx(10.0));
    CHECK(a.y_min == Catch::Approx(0.0));
    CHECK(a.y_max == Catch::Approx(10.0));
}

TEST_CASE("BBox merge with empty is a no-op") {
    BBox a, empty;
    a.expand(1, 2); a.expand(3, 4);
    a.merge(empty);
    CHECK(a.x_min == Catch::Approx(1.0));
    CHECK(a.x_max == Catch::Approx(3.0));
}

// --- Color ---

TEST_CASE("Color factory methods return correct RGB values") {
    CHECK(Color::black().r == 0);
    CHECK(Color::black().g == 0);
    CHECK(Color::black().b == 0);

    CHECK(Color::white().r == 255);
    CHECK(Color::white().g == 255);
    CHECK(Color::white().b == 255);

    CHECK(Color::red().r   == 228);
    CHECK(Color::red().g   == 26);
    CHECK(Color::red().b   == 28);

    CHECK(Color::blue().r  == 55);
    CHECK(Color::blue().g  == 126);
    CHECK(Color::blue().b  == 184);

    CHECK(Color::green().r == 77);
    CHECK(Color::green().g == 175);
    CHECK(Color::green().b == 74);
}

TEST_CASE("Color default alpha is 255") {
    CHECK(Color::red().a   == 255);
    CHECK(Color::black().a == 255);
    CHECK(Color(10, 20, 30).a == 255);
}

TEST_CASE("Color with_alpha changes alpha and preserves RGB") {
    Color c = Color::red().with_alpha(128);
    CHECK(c.r == Color::red().r);
    CHECK(c.g == Color::red().g);
    CHECK(c.b == Color::red().b);
    CHECK(c.a == 128u);
}

TEST_CASE("Color with_alpha(0) produces fully transparent") {
    Color c = Color::white().with_alpha(0);
    CHECK(c.a == 0u);
    CHECK(c.r == 255u);
}
