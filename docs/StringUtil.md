# StringUtil

## `std::vector<std::string> stc::string::split(std::string input, std::string delimiter, long long limit = -1)`.

Splits a string by a substring, up to limit times (or infinite if limit &lt; 0), and returns a vector containing the results.

## `std::vector<std::string> stc::split(std::string input, char delimiter, long long limit = -1)`

Splits a string by a character, up to limit times (or infinite of limit &lt; 0), and returns a vector containing the results.

## `std::vector<int> byteArrayOf(std::string in)`

Returns an integer array containing the numeric representation of each byte in the input string

## `std::string getByteString(std::string in)`

Returns a string equivalent of `byteArrayOf()`

## `replaceAll(std::string& input, std::string find, std::string replaceWith, size_t count = 0)`

Replaces all occurences of `find` in a string with `replaceWith`, up to `count` times. If `count == 0`, there's no limits on the number of replacements.

Note that this is an in-place replacement. The input string will be modified.

## `removeDuplicateWhitespace(std::string in, std::string& output)`

Removes all duplicate whitespace in a char, and outputs it in
