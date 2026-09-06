#pragma once

#include "catch2/reporters/catch_reporter_registrars.hpp"
#include <catch2/reporters/catch_reporter_streaming_base.hpp>

#include <catch2/internal/catch_xmlwriter.hpp>
#include <catch2/catch_timer.hpp>


namespace stc::testutil {

using namespace Catch;

/**
 * This class and its corresponding implementation is based on Catch2's built-in XmlReporter, which is licensed under
 * the Boost Software License, Version 1.0.
 *
 * Source: https://github.com/catchorg/Catch2/blob/devel/src/catch2/reporters/catch_reporter_xml.hpp
 */
class AugmentedXMLReporter : public StreamingReporterBase {
public:
    AugmentedXMLReporter(ReporterConfig&& config);
    virtual ~AugmentedXMLReporter() = default;

    static std::string getDescription();
    void writeSourceInfo(const SourceLineInfo& sourceInfo);

    void testRunStarting(const TestRunInfo& testInfo) override;
    void testRunEnded(const TestRunStats& testRunStats) override;

    void testCasePartialStarting(const TestCaseInfo&, uint64_t) override;
    void testCasePartialEnded(const TestCaseStats&, uint64_t) override;

    void testCaseStarting(const TestCaseInfo& testInfo) override;
    void testCaseEnded(const TestCaseStats& testCaseStats) override;

    void assertionEnded(const AssertionStats& assertionStats) override;

    void sectionStarting(const SectionInfo& sectionInfo) override;
    void sectionEnded(const SectionStats& sectionStats) override;

    void benchmarkPreparing(StringRef name) override;
    void benchmarkStarting(const BenchmarkInfo&) override;
    void benchmarkEnded(const BenchmarkStats<>&) override;
    void benchmarkFailed(StringRef error) override;

private:
    Timer timer;
    XmlWriter writer;
    int sectionDepth = 0;
};

CATCH_REGISTER_REPORTER("AugmentedXML", AugmentedXMLReporter);

}
