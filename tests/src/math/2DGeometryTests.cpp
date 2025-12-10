#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stc/Math.hpp>
#include <util/Vec2.hpp>

TEST_CASE("Line-line intersection", "[2D geometry][Math]") {
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
            Vec2 { 0, 0 },
            Vec2 {10, 0},
            Vec2 {0, 0},
            Vec2 {10, 0}
        )
    );
    REQUIRE(
        stc::math::g2d::lineIntersectsLineInclusive<int64_t>(
            Vec2 { 0, 0 },
            Vec2 {0, 10},
            Vec2 {0, 10},
            Vec2 {10, 10}
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

TEST_CASE("Line-Rectangle intersection", "[2D geometry][Math]") {
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

TEST_CASE("Sidedness", "[2D geometry][Math]") {
    REQUIRE(
        stc::math::g2d::isPointOnLeftOfEdge<int64_t>(
            Vec2 { 0, 3 },
            Vec2 { -3, 0 },
            Vec2 {3, 0}
        )
        == -stc::math::g2d::isPointOnLeftOfEdge<int64_t>(
            Vec2 { 0, 3 },
            Vec2 {3, 0},
            Vec2 { -3, 0 }
        )
    );
    REQUIRE(
        stc::math::g2d::isPointOnLeftOfEdge<int64_t>(
            Vec2 { 11, 1 },
            Vec2 { 2, 3 },
            Vec2 { 9, 3 }
        ) < 0
    );
}

TEST_CASE("Point in 4-point rectangle", "[2D geometry][Math]") {
    SECTION("Exclusive") {
        REQUIRE_FALSE(
            stc::math::g2d::rectangleContainsPointExclusive<int64_t>(
                Vec2 { 11, 1 },

                Vec2 { 9, 5 },
                Vec2 { 9, 3 },
                Vec2 { 2, 5 },
                Vec2 { 2, 3 }
            )
        );
    }
    SECTION("Inclusive") {
        REQUIRE_FALSE(
            stc::math::g2d::rectangleContainsPointInclusive<int64_t>(
                Vec2 { 11, 1 },

                Vec2 { 9, 5 },
                Vec2 { 2, 5 },
                Vec2 { 9, 3 },
                Vec2 { 2, 3 }
            )
        );

    }
}

TEST_CASE("Point in 2-point rectangle", "[2D geometry][Math]") {

    REQUIRE_FALSE(
        stc::math::g2d::rectangleContainsPointInclusive<int64_t>(
            Vec2 { 11, 1 },

            Vec2 { 9, 5 },
            Vec2 { 2, 3 }
        )
    );
    REQUIRE(
        stc::math::g2d::rectangleContainsPointInclusive<int64_t>(
            Vec2 { 9, 4 },

            Vec2 { 9, 5 },
            Vec2 { 2, 3 }
        )
    );
    REQUIRE(
        stc::math::g2d::rectangleContainsPointInclusive<int64_t>(
            Vec2 { 4, 4 },

            Vec2 { 9, 5 },
            Vec2 { 2, 3 }
        )
    );

    for (int64_t x = 2; x <= 9; ++x) {
        for (int64_t y = 3; y < 5; ++y) {
            INFO("x=" << x << ", y=" << y);
            REQUIRE(
                stc::math::g2d::rectangleContainsPointInclusive<int64_t>(
                    Vec2 { x, y },

                    Vec2 { 9, 5 },
                    Vec2 { 2, 3 }
                )
            );
        }
    }
}

TEST_CASE("Common expected line intersections on a 2-point rectangle") {
    std::vector<std::tuple<std::string, bool, std::tuple<Vec2, Vec2, Vec2, Vec2>>> cases = {
        {
            "Horizontal line intersection",
            true,
            { {0, 3}, {2, 3}, {1, 0}, {5, 4} }
        },
        {
            "Horizontal line intersection (opposite edge)",
            true,
            { {4, 3}, {6, 3}, {1, 0}, {5, 4} }
        },
        {
            "Horizontal line intersection (reverse)",
            true,
            { {2, 3}, {0, 3}, {1, 0}, {5, 4} }
        },
        {
            "Vertical line intersection",
            true,
            { {2, 3}, {2, 1}, {1, 0}, {5, 4} }
        },
        {
            "Vertical line intersection (opposite edge)",
            true,
            { {2, -3}, {2, 1}, {1, 0}, {5, 4} }
        },
        {
            "Vertical line intersection (reverse)",
            true,
            { {2, 1}, {2, 3}, {1, 0}, {5, 4} }
        },
        {
            "Horizontal line non-intersect",
            false,
            { {0, 3}, {-2, 3}, {1, 0}, {5, 4} }
        },
        {
            "Vertical line non-intersect",
            false,
            { {2, 30}, {2, 10}, {1, 0}, {5, 4} }
        },
    };

    for (auto& [name, expected, args] : cases) {
        SECTION(name + " (inclusive)") {
            REQUIRE(
                stc::math::g2d::lineIntersectsRectangleInclusive<int64_t>(
                    std::get<0>(args),
                    std::get<1>(args),
                    std::get<2>(args),
                    std::get<3>(args)
                )
                == expected
            );
        }
        SECTION(name + " (exclusive)") {
            REQUIRE(
                stc::math::g2d::lineIntersectsRectangleExclusive<int64_t>(
                    std::get<0>(args),
                    std::get<1>(args),
                    std::get<2>(args),
                    std::get<3>(args)
                )
                == expected
            );
        }
    }
}

TEST_CASE("Edge-case line intersections on 2-point rectangle") {
    std::vector<std::tuple<std::string, bool, bool, std::tuple<Vec2, Vec2, Vec2, Vec2>>> cases = {
        {
            "Horizontal line intersection",
            false,
            true,
            { {0, 3}, {1, 3}, {1, 0}, {5, 4} }
        },
        {
            "Horizontal line intersection matching edge",
            false,
            true,
            { {1, 4}, {5, 4}, {1, 0}, {5, 4} }
        },
        {
            "Vertical line intersection matching edge",
            false,
            true,
            { {1, 0}, {1, 4}, {1, 0}, {5, 4} }
        },
    };

    for (auto& [name, exclusive, inclusive, args] : cases) {
        SECTION(name + " (exclusive)") {
            REQUIRE(
                stc::math::g2d::lineIntersectsRectangleExclusive<int64_t>(
                    std::get<0>(args),
                    std::get<1>(args),
                    std::get<2>(args),
                    std::get<3>(args)
                )
                == exclusive
            );
        }
        SECTION(name + " (inclusive)") {
            REQUIRE(
                stc::math::g2d::lineIntersectsRectangleInclusive<int64_t>(
                    std::get<0>(args),
                    std::get<1>(args),
                    std::get<2>(args),
                    std::get<3>(args)
                )
                == inclusive
            );
        }
    }
}
