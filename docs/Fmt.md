# Fmt

This file exports an alias namespace (`fmt`) set to:

1. An alias for `std` if `<format>` exists.
2. The namespace from [fmtlib/fmt](//github.com/fmtlib/fmt)


Note that fmtlib/fmt itself is not provided. It's up to the programmer to ensure the existence of the library, possibly conditionally. There are many, many different ways to provide the library, as well as test for `<format>`'s existence. 


## Deprecation notice

This header has problems, and none of the solutions are good. The header works fine if you're working in isolation, but if you have any libraries that use fmt, the header will error out on the fmt inclusion due to a redeclaration of the fmt namespace.

Some of the alternatives that were considered:

1. Use fmt's include guards to not include anything if fmt/format is already included (i.e. the fmt namespace is already available), and use the include guard to prevent fmt/format from being included
    * Can break libraries
2. Renaming the namespace to stcfmt, and exporting it for both
    * Breaks existing code, still has double includes

However, the drawbacks outweigh the advantages, and due to the problems the header introduces, it's considered deprecated.
