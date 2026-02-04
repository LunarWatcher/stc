//              Copyright Catch2 Authors
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE.txt or copy at
//        https://www.boost.org/LICENSE_1_0.txt)

// SPDX-License-Identifier: BSL-1.0
#ifndef CATCH_REPORTER_CONSOLE_HPP_INCLUDED
#define CATCH_REPORTER_CONSOLE_HPP_INCLUDED

#include <catch2/reporters/catch_reporter_streaming_base.hpp>
#include <catch2/internal/catch_unique_ptr.hpp>

namespace Catch::Vendored {
    // Fwd decls
    class ModTablePrinter;

    class ModConsoleReporter : public StreamingReporterBase {
        Detail::unique_ptr<ModTablePrinter> m_tablePrinter;

    public:
        ModConsoleReporter(ReporterConfig&& config);
        ~ModConsoleReporter() override;

        virtual void noMatchingTestCases( StringRef unmatchedSpec ) override;
        virtual void reportInvalidTestSpec( StringRef arg ) override;

        virtual void assertionEnded(AssertionStats const& _assertionStats) override;

        virtual void sectionStarting(SectionInfo const& _sectionInfo) override;
        virtual void sectionEnded(SectionStats const& _sectionStats) override;

        virtual void benchmarkPreparing( StringRef name ) override;
        virtual void benchmarkStarting(BenchmarkInfo const& info) override;
        virtual void benchmarkEnded(BenchmarkStats<> const& stats) override;
        virtual void benchmarkFailed( StringRef error ) override;

        virtual void testCaseEnded(TestCaseStats const& _testCaseStats) override;
        virtual void testRunEnded(TestRunStats const& _testRunStats) override;
        virtual void testRunStarting(TestRunInfo const& _testRunInfo) override;

    private:
        void lazyPrint();

        void lazyPrintWithoutClosingBenchmarkTable();
        void lazyPrintRunInfo();
        void printTestCaseAndSectionHeader();

        void printClosedHeader(std::string const& _name);
        void printOpenHeader(std::string const& _name);

        // if string has a : in first line will set indent to follow it on
        // subsequent lines
        void printHeaderString(std::string const& _string, std::size_t indent = 0);

        void printTotalsDivider(Totals const& totals);

        bool m_headerPrinted = false;
        bool m_testRunInfoPrinted = false;
    };

} // end namespace Catch

#endif // CATCH_REPORTER_CONSOLE_HPP_INCLUDED
