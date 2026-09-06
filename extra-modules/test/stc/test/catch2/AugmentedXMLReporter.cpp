#include "AugmentedXMLReporter.hpp"
#include "catch2/catch_test_case_info.hpp"
#include "catch2/catch_test_spec.hpp"
#include "catch2/catch_version.hpp"
#include "catch2/interfaces/catch_interfaces_config.hpp"
#include "catch2/internal/catch_string_manip.hpp"
#include "catch2/reporters/catch_reporter_streaming_base.hpp"

namespace stc::testutil {

AugmentedXMLReporter::AugmentedXMLReporter(ReporterConfig&& conf)
    : StreamingReporterBase(std::move(conf)),
      writer(m_stream)
{
    m_preferences.shouldRedirectStdOut = true;
    m_preferences.shouldReportAllAssertions = true;
    m_preferences.shouldReportAllAssertionStarts = false;
}

std::string AugmentedXMLReporter::getDescription() {
    return "Reports test results as a run-oriented XML document.";
}

void AugmentedXMLReporter::writeSourceInfo(const SourceLineInfo& sourceInfo) {
    writer.writeAttribute("filename"_sr, sourceInfo.file)
        .writeAttribute("line"_sr, sourceInfo.line);
}

void AugmentedXMLReporter::testRunStarting(const TestRunInfo& testInfo) {
    StreamingReporterBase::testRunStarting(testInfo);
    writer.startElement("Catch2TestRun")
        .writeAttribute("name"_sr, m_config->name())
        .writeAttribute("rng-seed"_sr, m_config->rngSeed())
        .writeAttribute("xml-format-version"_sr, 3)
        .writeAttribute("catch2-version"_sr, libraryVersion())
        .writeAttribute("generator", "stc::testutil::AugmentedXMLReporter");
    if (m_config->testSpec().hasFilters()) {
        writer.writeAttribute("filters"_sr, m_config->testSpec());
    }
}

void AugmentedXMLReporter::testCaseStarting(const TestCaseInfo& testInfo) {
    StreamingReporterBase::testCaseStarting(testInfo);
    writer.startElement("TestCase")
        .writeAttribute("name"_sr, trim(StringRef(testInfo.name)))
        .writeAttribute("tags"_sr, testInfo.tagsAsString());

    if (!testInfo.className.empty()) {
        writer.writeAttribute("class-name", testInfo.className);
    }

    writeSourceInfo(testInfo.lineInfo);

    if (m_config->showDurations() == ShowDurations::Always) {
        timer.start();
    }
    writer.ensureTagClosed();
}

void AugmentedXMLReporter::sectionStarting(const SectionInfo& sectionInfo) {
    StreamingReporterBase::sectionStarting(sectionInfo);
    if (sectionDepth++ > 0) {
        writer.startElement("Section")
            .writeAttribute("name"_sr, trim(StringRef(sectionInfo.name)));
        writeSourceInfo(sectionInfo.lineInfo);
        writer.endElement();
    }
}

void AugmentedXMLReporter::sectionEnded(SectionStats const& sectionStats) {
    StreamingReporterBase::sectionEnded(sectionStats);
    if (--sectionDepth == 0) {
        writer
            .startElement("Duration")
            .writeAttribute("durationInSeconds", sectionStats.durationInSeconds)
            .endElement();
        
    }
}

void AugmentedXMLReporter::assertionEnded(const AssertionStats& assertionStats) {

    AssertionResult const& result = assertionStats.assertionResult;

    bool includeResults = m_config->includeSuccessfulResults() || !result.isOk();

    if (includeResults || result.getResultType() == ResultWas::Warning) {
        for(auto const& msg : assertionStats.infoMessages) {
            if(msg.type == ResultWas::Info && includeResults) {
                auto t = writer.scopedElement("Info");
                writeSourceInfo(msg.lineInfo);
                t.writeText(msg.message);
            } else if (msg.type == ResultWas::Warning) {
                auto t = writer.scopedElement("Warning");
                writeSourceInfo(msg.lineInfo);
                t.writeText(msg.message);
            }
        }
    }

    if (!includeResults && result.getResultType() != ResultWas::Warning &&
        result.getResultType() != ResultWas::ExplicitSkip) {
        return;
    }

    if (result.hasExpression()) {
        writer.startElement("Expression")
            .writeAttribute("success"_sr, result.succeeded())
            .writeAttribute("type"_sr, result.getTestMacroName());

        writeSourceInfo(result.getSourceInfo());

        writer.scopedElement("Original")
            .writeText(result.getExpression());
        writer.scopedElement("Expanded")
            .writeText(result.getExpandedExpression());
    }

    switch(result.getResultType()) {
    case ResultWas::ThrewException:
        writer.startElement("Exception");
        writeSourceInfo(result.getSourceInfo());
        writer.writeText(result.getMessage());
        writer.endElement();
        break;
    case ResultWas::FatalErrorCondition:
        writer.startElement("FatalErrorCondition");
        writeSourceInfo(result.getSourceInfo());
        writer.writeText(result.getMessage());
        writer.endElement();
        break;
    case ResultWas::Info:
        writer.scopedElement("Info")
            .writeText(result.getMessage());
        break;
    case ResultWas::ExplicitFailure:
        writer.startElement("Failure");
        writeSourceInfo(result.getSourceInfo());
        writer.writeText(result.getMessage());
        writer.endElement();
        break;
    case ResultWas::ExplicitSkip:
        writer.startElement("Skip");
        writeSourceInfo(result.getSourceInfo());
        writer.writeText(result.getMessage());
        writer.endElement();
        break;
    case ResultWas::Warning:
        [[fallthrough]];
    default:
        break;
    }

    if(result.hasExpression()) {
        writer.endElement();
    }
}

void AugmentedXMLReporter::testCasePartialStarting(const TestCaseInfo&, uint64_t runNumber) {
    writer.startElement("TestRun")
        .writeAttribute("run-number", std::to_string(runNumber));
}
void AugmentedXMLReporter::testCasePartialEnded(const TestCaseStats& stats, uint64_t) {
    {
        XmlWriter::ScopedElement e = writer.scopedElement("OverallResults");
        e.writeAttribute("successes"_sr, stats.totals.assertions.passed);
        e.writeAttribute("failures"_sr, stats.totals.assertions.failed);
        e.writeAttribute("expectedFailures"_sr, stats.totals.assertions.failedButOk);
        e.writeAttribute("skipped"_sr, stats.totals.assertions.skipped > 0);
        e.writeAttribute("success"_sr, stats.totals.assertions.allOk());
    }
    writer.endElement();
}

void AugmentedXMLReporter::testCaseEnded(TestCaseStats const& testCaseStats) {
    StreamingReporterBase::testCaseEnded(testCaseStats);
    XmlWriter::ScopedElement e = writer.scopedElement("OverallResult");
    e.writeAttribute("success"_sr, testCaseStats.totals.assertions.allOk());
    e.writeAttribute("skips"_sr, testCaseStats.totals.assertions.skipped);

    if (m_config->showDurations() == ShowDurations::Always) {
        e.writeAttribute("durationInSeconds"_sr, timer.getElapsedSeconds());
    }
    if (!testCaseStats.stdOut.empty()) {
        writer.scopedElement("StdOut").writeText(
            trim(StringRef(testCaseStats.stdOut)),
            XmlFormatting::Newline
        );
    }

    if (!testCaseStats.stdErr.empty()) {
        writer.scopedElement("StdErr").writeText(
            trim(StringRef(testCaseStats.stdErr)),
            XmlFormatting::Newline
        );
    }

    writer.endElement();
}

void AugmentedXMLReporter::testRunEnded(const TestRunStats& testRunStats) {
    StreamingReporterBase::testRunEnded(testRunStats);
    writer.scopedElement("OverallResults")
        .writeAttribute("successes"_sr, testRunStats.totals.assertions.passed)
        .writeAttribute("failures"_sr, testRunStats.totals.assertions.failed)
        .writeAttribute("expectedFailures"_sr, testRunStats.totals.assertions.failedButOk)
        .writeAttribute("skips"_sr, testRunStats.totals.assertions.skipped);
    writer.scopedElement("OverallResultsCases")
        .writeAttribute("successes"_sr, testRunStats.totals.testCases.passed)
        .writeAttribute("failures"_sr, testRunStats.totals.testCases.failed)
        .writeAttribute("expectedFailures"_sr, testRunStats.totals.testCases.failedButOk)
        .writeAttribute("skips"_sr, testRunStats.totals.testCases.skipped);
    writer.endElement();
}

void AugmentedXMLReporter::benchmarkPreparing(StringRef name) {
    writer.startElement("BenchmarkResults")
        .writeAttribute("name"_sr, name);
}

void AugmentedXMLReporter::benchmarkStarting(const BenchmarkInfo &info) {
    writer.writeAttribute("samples"_sr, info.samples)
        .writeAttribute("resamples"_sr, info.resamples)
        .writeAttribute("iterations"_sr, info.iterations)
        .writeAttribute("clockResolution"_sr, info.clockResolution)
        .writeAttribute("estimatedDuration"_sr, info.estimatedDuration)
        .writeComment("All values in nano seconds"_sr);
}

void AugmentedXMLReporter::benchmarkEnded(const BenchmarkStats<>& benchmarkStats) {
    writer.scopedElement("mean")
        .writeAttribute("value"_sr, benchmarkStats.mean.point.count())
        .writeAttribute("lowerBound"_sr, benchmarkStats.mean.lower_bound.count())
        .writeAttribute("upperBound"_sr, benchmarkStats.mean.upper_bound.count())
        .writeAttribute("ci"_sr, benchmarkStats.mean.confidence_interval);
    writer.scopedElement("standardDeviation")
        .writeAttribute("value"_sr, benchmarkStats.standardDeviation.point.count())
        .writeAttribute("lowerBound"_sr, benchmarkStats.standardDeviation.lower_bound.count())
        .writeAttribute("upperBound"_sr, benchmarkStats.standardDeviation.upper_bound.count())
        .writeAttribute("ci"_sr, benchmarkStats.standardDeviation.confidence_interval);
    writer.scopedElement("outliers")
        .writeAttribute("variance"_sr, benchmarkStats.outlierVariance)
        .writeAttribute("lowMild"_sr, benchmarkStats.outliers.low_mild)
        .writeAttribute("lowSevere"_sr, benchmarkStats.outliers.low_severe)
        .writeAttribute("highMild"_sr, benchmarkStats.outliers.high_mild)
        .writeAttribute("highSevere"_sr, benchmarkStats.outliers.high_severe);
    writer.endElement();
}

void AugmentedXMLReporter::benchmarkFailed(StringRef error) {
    writer.scopedElement("failed").
        writeAttribute("message"_sr, error);
    writer.endElement();
}

}
