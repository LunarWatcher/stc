# stc

Header-only utilities for C++17\* and up. Name inspired by [stb](https://github.com/nothings/stb) (in part so I can have `stb`, `stc`, and `std` in some of my projects :) )

Note: a few of the headers are inter-dependent

<sub>\*: Possibly subject to change</sub>

| Library | Version | Category | Description | Dependencies (not including stdlib includes) |
| --- | --- | --- | --- | --- |
| `stc/Environment.hpp` | 1.0.1  | OS compatibility | Filesystem and other environmental utils for OS-specific operations | `FS.hpp`, `Optional.hpp` |
| `stc/FS.hpp` | 1.0.0 | stdlib compatibility | Wrapper to deal with `experimental/filesystem` and `<filesystem>` | |
| `stc/Optional.hpp` | 1.0.0 | stdlib compatibility | Wrapper around `std::optional` and `std::experimental::optional` | | 
| `stc/StringUtil.hpp` | 1.0.0 | Utility library | Adds a few string operations that C++ does not (but should) have built into strings | |
| `stc/FntParser.hpp` | 1.0.0 | Utility library | Adds support for parsing text-based .fnt files | `FS.hpp` |

# Usage

All you have to do is set up `src/` as an include directory. Everything is header-only, and at the time of writing, requires no other setup.
