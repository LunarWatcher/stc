# Environment

## `std::string stc::getEnv(std::string name, std::string fail = "")`

Returns an environment variable called `name` if one exists, or `fail` otherwise (defaults to an empty string). On Windows and systems that support GNU's `secure_getenv`, a secure equivalent of `getenv` is used instead of the regular `getenv()`. See: https://stackoverflow.com/a/48572723/6296561

## `std::string stc::expandUserPath(std::string inputPath)`

Converts path in the form of `~optionalUsername/blah` and converts it to a home path on an applicable OS. Note that `~optionalUsername` is invalid syntax on Windows, and it will silently fail with a runtime `std::cerr`. `~/blah` expansion works on Windows, Linux, and MacOS.

## `std::string getHome()`

Returns the home directory for the current user. This is a potentially light-weight equivalent of `expandUserPath` if you're planning on using the home directory for several things, and you'd like to cache it.

