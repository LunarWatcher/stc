# Environment

## `std::string stc::getEnv(std::string name, std::string fail = "")`

Returns an environment variable called `name` if one exists, or `fail` otherwise (defaults to an empty string). On Windows and systems that support GNU's `secure_getenv`, a secure equivalent of `getenv` is used instead of the regular `getenv()`. See: https://stackoverflow.com/a/48572723/6296561

## `fs::path stc::expandUserPath(std::string inputPath)`

Converts path in the form of `~optionalUsername/blah` and converts it to a home path on an applicable OS. Note that `~optionalUsername` is invalid syntax on Windows, and it will silently fail with a runtime `std::cerr`. `~/blah` expansion works on Windows, Linux, and MacOS.

## `fs::path stc::getHome()`

Returns the home directory for the current user. This is a potentially light-weight equivalent of `expandUserPath` if you're planning on using the home directory for several things, and you'd like to cache it.

## `std::string stc::syscommand(std::string command, int* codeOutput = nullptr)`

Executes a system command, and returns the output.

Note that this is a blocking function; DO NOT use if you need to stream the output of a given command.

To get the exit code of the command as well, pass a pointer to an int in the second parameter.

## `std::string stc::syscommand(std::vector<const char*> command, int* codeOutput = nullptr)` [UNIX only]

This function is not yet available on Windows due to [lacking support for vector- or vararg-based function creation](https://stackoverflow.com/a/75554181). This may be revisited if I find a safer approach.

Similar to the string version of the command, but uses execv. This allows a vectorised form of the exec functions to be used, which makes it easier to avoid the cursed shell escape functions required to make `std::syscommand(std::string, int*)` safe. 

However, unlike the string form of this function, a shell is not invoked at all. Finding the path to executables is left to the invoker.

## `std::string stc::getHostname()`

Minimal wrapper around `gethostname` with conversion from char arrays to C++ strings.

This is primarily a convenience function. It doesn't need to do any OS compatibility checks, because `gethostname()` has a compatible function signature between Linux and Windows.
