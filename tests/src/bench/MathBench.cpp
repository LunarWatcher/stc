#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <util/Vec2.hpp>
#include <stc/Math.hpp>

TEST_CASE("Math benchmarks", "[benchmark]") {

    BENCHMARK("rectangleContainsPointInclusive") {
        return stc::math::g2d::rectangleContainsPointInclusive<int64_t>( 
            Vec2 { 5, 5 },

            Vec2 { 0, 5 },
            Vec2 { 6, -1 },
            Vec2 { 6, 12 },
            Vec2 { 12, 5 }
        );
    };
}

TEST_CASE("rectangleContainsPoint benchmark correctness") {
    SECTION("Inclusive") {
        REQUIRE(
            stc::math::g2d::rectangleContainsPointInclusive<int64_t>( 
                Vec2 { 5, 5 },

                Vec2 { 0, 5 },
                Vec2 { 6, -1 },
                Vec2 { 6, 12 },
                Vec2 { 12, 5 }
            )
        );
    }
    SECTION("Exclusive") {
        REQUIRE(
            stc::math::g2d::rectangleContainsPointExclusive<int64_t>( 
                Vec2 { 5, 5 },

                Vec2 { 0, 5 },
                Vec2 { 6, -1 },
                Vec2 { 6, 12 },
                Vec2 { 12, 5 }
            )
        );
    }
}
