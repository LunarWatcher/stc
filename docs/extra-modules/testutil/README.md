# `stc::testutil`

CMake target: `stc::testutil`

Namespace: `stc::testutil`

This module contains utilities meant specifically for testing. 

## Files

### CaptureStream.hpp

Used for capturing streams, but primarily `std::cout`, `std::cerr`, and `std::cin`. Very useful for unit testing (input-wise) simple command line applications.

### ExplicitStreamTestReporter.hpp

Custom Catch2 reporter that handles verbosity better. See also ExplicitStreamTestReporter.md
