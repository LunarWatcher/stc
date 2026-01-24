#include "stc/minilog.hpp"
#include "stc/test/CaptureStream.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

TEST_CASE("Minilog benchmark", "[benchmark]") {
    stc::testutil::CaptureStandardStreams c;
    // The std::format benchmarks are used to get a measure of how much overhead there is in the logger.
    // If std::format does 100it/s and the logger does 99it/s, the actual performance of the actual logging bit is
    // negligible. (The real results are not that obvious though, because C++ streams are used for the output)
    BENCHMARK("std::format (baseline, no args)") {
        return std::format("Hello");
    };
    BENCHMARK("std::format (baseline, several args)") {
        return std::format(
            "Hello {} {}, also {}",
            "output", 42,
            "trans rights are human rights"
        );
    };
    BENCHMARK("minilog::info (no format args)") {
        return minilog::info("Hello");
    };
    BENCHMARK("minilog::info (several format args)") {
        return minilog::info(
            "Hello {} {}, also {}",
            "output", 42,
            "trans rights are human rights"
        );
    };
}
