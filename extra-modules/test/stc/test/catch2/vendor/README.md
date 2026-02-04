This folder contains files copied straight from catch2, and these files are therefore under the  Boost Software License, per the catch2 license:

* Full license: https://github.com/catchorg/Catch2/blob/de7e8630134f46e67a6b59269436f9cab94cd28e/LICENSE.txt
* Source 1: https://github.com/catchorg/Catch2/blob/de7e8630134f46e67a6b59269436f9cab94cd28e/src/catch2/reporters/catch_reporter_console.cpp
* Source 2: https://github.com/catchorg/Catch2/blob/de7e8630134f46e67a6b59269436f9cab94cd28e/src/catch2/reporters/catch_reporter_console.hpp

Changes made:

* The reporter is no longer final
* The reporter now has virtual member functions so functionality can be overridden.
* Namespace changed to `Catch::Vendored` so it doesn't interfere with the built-in, unmodified reporter.
