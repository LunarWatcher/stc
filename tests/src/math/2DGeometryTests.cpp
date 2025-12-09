#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stc/Math.hpp>
#include <util/Vec2.hpp>

TEST_CASE("Line-line intersection", "[2D geometry][Math]") {
    SECTION("Non-inclusive") {
        REQUIRE_FALSE(
            stc::math::g2d::lineIntersectsLineExclusive<int64_t>(
                Vec2 {0, 0},
                Vec2 {10, 0},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsLineExclusive<int64_t>(
                Vec2 {-1, 1},
                Vec2 {10, 1},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE_FALSE(
            stc::math::g2d::lineIntersectsLineExclusive<int64_t>(
                Vec2 {0, 1},
                Vec2 {0, 11},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE_FALSE(
            stc::math::g2d::lineIntersectsLineExclusive<int64_t>(
                Vec2 {0, 0},
                Vec2 {0, 10},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE_FALSE(
            stc::math::g2d::lineIntersectsLineExclusive<int64_t>(
                Vec2 { 8, 5 },
                Vec2 { 11, 8 },

                Vec2 { 10, 10 },
                Vec2 { 12, 6}
            )
        );
    }

    SECTION("Inclusive") {
        REQUIRE(
            stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
                Vec2 { 0, 0 },
                Vec2 {10, 0},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
                Vec2 {-1, 1},
                Vec2 {10, 1},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
                Vec2 {-1, 0},
                Vec2 {10, 0},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
                Vec2 {0, 1},
                Vec2 {0, 11},
                Vec2 {0, 0},
                Vec2 {0, 10}
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
                Vec2 {0, 1},
                Vec2 {0, 2},
                Vec2 {0, 0},
                Vec2 {0, 1}
            )
        );

        REQUIRE(
            stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
                Vec2 { 8, 5 },
                Vec2 { 11, 8 },

                Vec2 { 10, 10 },
                Vec2 { 12, 6}
            )
        );
    }
}

TEST_CASE("Line-Rectangle intersection", "[2D geometry][Math]") {
    SECTION("Exclusive") {
        REQUIRE(
            stc::math::g2d::lineIntersectsRectangleExclusive<int64_t>(
                Vec2 { 0, 0 },
                Vec2 { 100, 100 },

                Vec2 { 10, 10 },
                Vec2 { 20, 80 },
                Vec2 { 80, 20 },
                Vec2 { 80, 80 }
            )
        );
        REQUIRE_FALSE(
            stc::math::g2d::lineIntersectsRectangleExclusive<int64_t>(
                Vec2 { 0, 0 },
                Vec2 { 0, 10 },

                Vec2 { 0, 0 },
                Vec2 { 0, 10 },
                Vec2 { 10, 0 },
                Vec2 { 10, 10 }
            )
        );
    }
    SECTION("Inclusive") {
        REQUIRE(
            stc::math::g2d::lineIntersectsRectangleInclusive<int64_t>(
                Vec2 { 0, 0 },
                Vec2 { 100, 100 },

                Vec2 { 10, 10 },
                Vec2 { 20, 80 },
                Vec2 { 80, 20 },
                Vec2 { 80, 80 }
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsRectangleInclusive<int64_t>(
                Vec2 { 0, 0 },
                Vec2 { 0, 10 },

                Vec2 { 0, 0 },
                Vec2 { 0, 10 },
                Vec2 { 10, 0 },
                Vec2 { 10, 10 }
            )
        );
    }
}

TEST_CASE("Point in rectangle", "[2D geometry][Math]") {
    SECTION("Exclusive") {
        REQUIRE_FALSE(
            stc::math::g2d::lineIntersectsRectangleExclusive<int64_t>(
                Vec2 { 8, 5 },
                Vec2 { 11, 8 },

                Vec2 { 10, 10 },
                Vec2 { 12, 6},
                Vec2 { 14, 12 },
                Vec2 { 17, 8 }
            )
        );
        REQUIRE(
            stc::math::g2d::lineIntersectsRectangleInclusive<int64_t>(
                Vec2 { 8, 5 },
                Vec2 { 11, 8 },

                Vec2 { 10, 10 },
                Vec2 { 12, 6},
                Vec2 { 14, 12 },
                Vec2 { 17, 8 }
            )
        );
    }
    SECTION("Inclusive") {

    }
}
