#pragma once

#include "catch2/interfaces/catch_interfaces_reporter.hpp"
#include "stc/Colour.hpp"
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>
#include "vendor/ConsoleReporter.hpp"

#include <iostream>

class ExplicitStreamTestReporter : public Catch::Vendored::ModConsoleReporter {
public:
    ExplicitStreamTestReporter(Catch::ReporterConfig&& config)
        : Catch::Vendored::ModConsoleReporter(std::move(config)) {}
    int64_t secLevel = 0;

    static std::string getDescription() {
        return "A test reporter designed to be much more useful for terminal users";
    }

    void sectionStarting(const Catch::SectionInfo& section) override {
        Catch::Vendored::ModConsoleReporter::sectionStarting(section);
        std::cout
            << std::format("{:<70}", getTabs() + section.name)
            << stc::colour::fg<stc::colour::FourBitColour::BRIGHT_YELLOW>
            << " STARTING"
            << stc::colour::reset
            << std::endl;
        ++secLevel;
    }
    void sectionEnded(const Catch::SectionStats& section) override {
        Catch::Vendored::ModConsoleReporter::sectionEnded(section);
        --secLevel;
        std::cout
            << std::format("{:<70}", getTabs() + section.sectionInfo.name)
            << " ";
        if (section.assertions.failed) {
            std::cout 
                << stc::colour::fg<stc::colour::FourBitColour::RED>
                << "FAILED"
                << stc::colour::reset;
        } else if (section.assertions.skipped) {
            std::cout 
                << stc::colour::fg<stc::colour::FourBitColour::MAGENTA>
                << "SKIPPED"
                << stc::colour::reset;
        } else {
            std::cout 
                << stc::colour::fg<stc::colour::FourBitColour::GREEN>
                << "PASSED"
                << stc::colour::reset;
        }

        std::cout << "\n";
        if (secLevel == 0) {
            std::cout << "\n";
        }
    }

    std::string getTabs() {
        std::stringstream out;

        for (int64_t i = 0; i < secLevel; ++i) {
            out << "  ";
        }

        return out.str();
    }

};

CATCH_REGISTER_REPORTER("explicit", ExplicitStreamTestReporter)
