# `stc::testutil`

CMake target: `stc::testutil`

Namespace: `stc::testutil`

This module contains utilities meant specifically for testing.

This module is NOT header-only! The test reporters require being built into a `.cpp` file to be registered.

## Files

### CaptureStream.hpp

Used for capturing streams, but primarily `std::cout`, `std::cerr`, and `std::cin`. Very useful for unit testing (input-wise) simple command line applications.

### ExplicitStreamTestReporter.hpp

Custom Catch2 reporter that handles verbosity better. See also ExplicitStreamTestReporter.md

### AugmentedXMLReporter.hpp

Custom Catch2 reporter that makes the XML format more easily machine-processed
