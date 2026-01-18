/** \file */
#pragma once

#include <chrono>
#include <format>
#include "Colour.hpp"

/**
 * \brief Module containing a small logger. 
 *
 * Does the bare minimum to be a fancy terminal logger. Based on <format> with some features from <stc/Colour.hpp> used.
 *
 */
namespace minilog {

enum Level {
    DEBUG = 50,
    INFO = 60,
    WARNING = 70,
    ERROR = 80,
    CRITICAL = 90
};

struct Config {
    Level level = Level::DEBUG;
};

inline Config& config() {
    // This should be fine:
    // * https://stackoverflow.com/a/185730
    // * https://stackoverflow.com/a/189162
    //
    // This is also consistent with getStreamConfigIdx in Colour.hpp. I vaguely remember running into this problem
    // because shouldPrintColour is static, while getStreamConfigIdx is inline.
    // I really should read up on what inline actually is, because I'm pretty sure I've misunderstood *something*
    static Config instance;

    return instance;
}

template <Level level>
consteval std::string_view levelToString() {
    if constexpr (level == Level::DEBUG) {
        return "debug";
    } else if constexpr (level == Level::INFO) {
        return "info";
    } else if constexpr (level == Level::WARNING) {
        return "warning";
    } else if constexpr (level == Level::ERROR) {
        return "error";
    } else if constexpr (level == Level::CRITICAL) {
        return "critical";
    }
}

inline std::ostream& operator<<(std::ostream& ss, Level level) {
    if (level == Level::DEBUG) {
        ss << stc::colour::fg<stc::colour::FourBitColour::BRIGHT_BLACK>;
    } else if (level == Level::INFO) {
        ss << stc::colour::fg<stc::colour::FourBitColour::BLUE>;
    } else if (level == Level::WARNING) {
        ss << stc::colour::fg<stc::colour::FourBitColour::BRIGHT_YELLOW>;
    } else if (level == Level::ERROR) {
        ss << stc::colour::fg<stc::colour::FourBitColour::BRIGHT_RED>;
    } else if (level == Level::CRITICAL) {
        ss << stc::colour::fg<stc::colour::FourBitColour::RED>;
    }
    return ss;
}

template <Level level, class... Args>
inline void log(const std::format_string<Args...>& fmt, Args&&... args) {
    if (config().level > level) {
        return;
    }

    // This is good enough:
    // https://stackoverflow.com/a/26909227
    //
    // This isn't going to win any performance awards by any metric, but it's good enough for now. 
    // Not sure if I'll ever revisit performance.
    std::cout 
        << level
        << std::format(
            "{:%T} | {:<8} | {}\n",
            std::chrono::floor<std::chrono::milliseconds>(
                std::chrono::system_clock::now()
            ),
            levelToString<level>(),
            std::format<Args...>(fmt, std::forward<Args>(args)...)
        )
        << stc::colour::reset;
}


template <class... Args>
inline void debug(const std::format_string<Args...>& format, Args&... args) {
    log<Level::DEBUG, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void info(const std::format_string<Args...>& format, Args&... args) {
    log<Level::INFO, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void warn(const std::format_string<Args...>& format, Args&... args) {
    log<Level::WARNING, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void error(const std::format_string<Args...>& format, Args&... args) {
    log<Level::ERROR, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void critical(const std::format_string<Args...>& format, Args&... args) {
    log<Level::CRITICAL, Args...>(format, std::forward<Args>(args)...);
}

}
