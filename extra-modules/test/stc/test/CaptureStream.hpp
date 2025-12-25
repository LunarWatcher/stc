#pragma once

#include <iostream>
#include <sstream>

namespace stc::testutil {

/**
 * Utility class for redirecting streams. This is primarily meant for redirecting std::cout, std::cerr, and std::cin,
 * but should work with any standard stream.
 *
 * This is very useful if you want to test code that takes standard input that you want to simulate entering, or if you
 * want to test the stdout produced in some test.
 *
 * This function is in the extra module stc::testutil, and should never be used outside tests. It may still work there,
 * but it's not designed to be robust enough to handle real-world use.
 */
template <class T, typename = std::enable_if_t<std::is_base_of_v<std::ios_base, T>>>
class CaptureStream {
private:
    T& stream;
    // We need to store the buffer to restore it afterwards
    decltype(stream.rdbuf()) buffer;

public:
    std::stringstream content;

    [[nodiscard("Discarding the field means all the streams are immediately uncaptured")]]
    CaptureStream(T& stream) : stream(stream) {
        this->buffer = stream.rdbuf();
        this->stream.rdbuf(content.rdbuf());
    }

    ~CaptureStream() {
        restore();
    }

    void restore() {
        if (buffer != nullptr) {
            stream.rdbuf(buffer);
            buffer = nullptr;
        }
    }

    /**
     * Clears the content buffer. Useful if you ever run into EOL, and then want to add more content
     * afterwards.
     */
    void reset() {
        content = {};
        stream.rdbuf(content.rdbuf());
    }
};

/**
 * Utility struct that captures and wraps a CaptureStream for all three main streams (stdout, stderr, stdin). Can be
 * used as a shortcut if you want to capture all three streams.
 */
struct CaptureStandardStreams {
    CaptureStream<decltype(std::cout)> cout{std::cout};
    CaptureStream<decltype(std::cerr)> cerr{std::cerr};
    CaptureStream<decltype(std::cin)> cin{std::cin};
};

}
