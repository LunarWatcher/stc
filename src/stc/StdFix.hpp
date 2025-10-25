/** \file */
#pragma once

#include <istream>
#include <string>

namespace stc {

namespace StdFix {

/**
 * Implementation of std::getline that handles \r, \n, and \r\n. 
 *
 * This lets you consume arbitrary files of unknown line endings without breaking hard. 
 *
 * \param is An input stream to get the line from 
 * \param[out] str The string to put the line in.
 */
inline std::istream& getline(std::istream& is, std::string& str) {
    char ch;
    str.clear();

    while (!is.eof()) {
        is.get(ch);
        // Streams are weird with the null character. While this may seem like a redundant check,
        // it avoids dupe characters.
        if (is.eof()) break;
        if (ch == '\r') {
            if (!is.eof() && is.peek() == '\n') {
                is.get(ch);
                // We don't care whether the call above resulted in an EOF; we're exiting anyway, so it's all good
            }
            break;
        } else if (ch == '\n') {
            break;
        }

        str += ch;
    }

    return is;
}

}

}
