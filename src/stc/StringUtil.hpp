/** \file 
 *
 * Whole bunch of stringutils. 
 */
#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

namespace stc::string {

/**
 * Splits a string by a character.
 * 
 * This is partly covered by C++20 ranges, but C++20 ranges does not offer limiting the number of splits, which is very
 * useful in certain parsing tasks. If you have no need for the limit argument, you probably want ranges instead.
 *
 * \see https://en.cppreference.com/w/cpp/ranges/split_view.html
 */
inline std::vector<std::string> split(const std::string& input, const char delimiter, int64_t limit = -1) {
    if (delimiter == 0) {
        std::vector<std::string> out;
        std::transform(input.begin(), input.end(), std::back_inserter(out), [](const char& chr) {
            return std::string(1, chr);
        });
        return out;
    } else if (limit == 0) {
        return {input};
    } else if (input.empty()) {
        return {};
    }

    std::vector<std::string> out;
    std::stringstream stream(input);
    std::string line;
    int64_t count = 0;
    while (getline(stream, line, delimiter)) {
        out.push_back(line);
        count++;
        if (count == limit) {
            break;
        }
    }
    std::stringstream remainder;
    remainder << stream.rdbuf();
    std::string res = remainder.str();
    if (res != "") {
        out.push_back(res);
    } else if (input.at(input.size() - 1) == delimiter) {
        // Edge-case; delimiter last
        out.push_back("");
    }

    return out;
}

/**
 * Splits a string by a substring. Calls the method splitting by character if delimiter.size() <= 1.
 *
 * This is partly covered by C++20 ranges, but C++20 ranges does not offer limiting the number of splits, which is very
 * useful in certain parsing tasks. If you have no need for the limit argument, you probably want ranges instead.
 *
 * \see https://en.cppreference.com/w/cpp/ranges/split_view.html
 */
inline std::vector<std::string> split(const std::string& input, const std::string& delimiter, int64_t limit = -1) {
    if (delimiter.size() == 1) {
        return split(input, delimiter[0], limit);
    } else if (delimiter.size() == 0) {
        return split(input, 0, limit);
    } else if (limit == 0) {
        return {input};
    } else if (input.empty()) {
        return {};
    }

    std::vector<std::string> out;
    size_t pos = 0;
    size_t index = 0;
    std::string token;
    int64_t count = 0;
    while ((pos = input.find(delimiter, index)) != std::string::npos) {
        // pos - index, because this shit operates on length rather than indices.
        token = input.substr(index, pos - index);
        index = pos + delimiter.size();
        out.push_back(token);
        count++;
        if (count == limit) {
            break;
        }
    }
    if (index < input.size()) {
        out.push_back(input.substr(index));
    } else if (index == input.size()) {
        // A delimiter was the last token
        out.push_back("");
    }
    return out;

}

inline std::vector<int> byteArrayOf(const std::string& input) {
    std::vector<int> out;
    for (auto& chr : input) {
        out.push_back(chr);
    }
    return out;
}

/**
 * Turns a string into a string of bytes. Useful for debugging the internal representation of a string 
 */
inline std::string getByteString(const std::string& input) {
    std::string output;
    for (const char c : input) {
        output += std::to_string(c) + " ";
    }
    return output;
}

inline void replaceAll(std::string& input, const std::string& find, const std::string& replaceWith, size_t limit = 0) {
    if (find.empty()) {
        return;
    }

    size_t pos = 0;
    size_t count = 0;
    while ((pos = input.find(find, pos)) != std::string::npos) {
        input.replace(pos, find.length(), replaceWith);
        pos += replaceWith.length();

        if (++count == limit) {
            break;
        }
    }
}

/**
 * Strips sequential whitespace, but leaving one intact. This means at most one whitespace is preserved.
 */
inline void removeDuplicateWhitespace(const std::string& input, std::string& output) {
    std::unique_copy(
        input.begin(), input.end(), std::back_inserter(output),
        [](char a, char b) {
            return std::isspace(a) != 0 && std::isspace(b) != 0;
        });
}

}
