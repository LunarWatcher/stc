# Fmt

This file exports a `fmt` namespace. This is one of two namespaces, prioritised by header availability:

1. An alias for `std` if `<format>` exists.
2. The namespace from [fmtlib/fmt](//github.com/fmtlib/fmt)

Note that fmtlib/fmt itself is not provided. It's up to the programmer to ensure the existence of the library, possibly conditionally. There are many, many different ways to provide the library, as well as test for `<format>`'s existence. 

For my personal use, however, I use spdlog's conan package, which sources in fmtlib/fmt. This means I "risk" having both redundantly, but it's fine because a dependency uses it
