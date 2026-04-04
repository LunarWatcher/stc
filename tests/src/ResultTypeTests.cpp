#include "stc/Result.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic Result functionality on primitive results") {
    using ResultType = stc::Result<int, float>;

    SECTION("Result access on OK") {
        ResultType t = ResultType::ok(69);

        // Unwrap semantics
        REQUIRE_NOTHROW(t.unwrap());
        REQUIRE_THROWS(t.error());
        REQUIRE(t.unwrap() == 69);
    }

    SECTION("Result error access on error") {
        ResultType t = ResultType::err(6);

        // Unwrap semantics
        REQUIRE_THROWS(t.unwrap());
        REQUIRE_NOTHROW(t.error());
        REQUIRE(t.error() == 6);
    }
}


TEST_CASE("Basic Result functionality on non-primitive results") {
    struct MResultValue {
        int val;
    };
    enum class MErrorType {
        BadGirlError
    };

    using ResultType = stc::Result<MResultValue, MErrorType>;
    
    SECTION("Result access on OK") {
        ResultType t = ResultType::ok({69});

        // Unwrap semantics
        REQUIRE_NOTHROW(t.unwrap());
        REQUIRE_THROWS(t.error());

        REQUIRE(t->val == 69);
    }

    SECTION("Result error access on error") {
        ResultType t = ResultType::err(MErrorType::BadGirlError);

        // Unwrap semantics
        REQUIRE_THROWS(t.unwrap());
        REQUIRE_NOTHROW(t.error());

        REQUIRE_THROWS(t->val);
        REQUIRE(t.error() == MErrorType::BadGirlError);
    }
}

