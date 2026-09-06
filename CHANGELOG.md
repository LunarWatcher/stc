# Changelog

This changelog is only maintained as of v2026-09-06. Versions prior to this were not tracked.

The version format is an augmented date format: `vyyyy-mm-dd[.patchnumber]`. `.patchnumber` is only in use when there are multiple releases in one day.

## v2026-09-06.2

* `stc::testutil`: Fixed Augmented XML reporter not including stdout per test run

## v2026-09-06.1

* `stc`: Fixed `Process.hpp` not handling that `execve` returns on error, resulting in weird behaviour on execution failures

## v2026-09-06

* `stc`: Removed `minilog`. Switch to https://codeberg.org/LunarWatcher/minilog
* `stc::testutil`: Added `AugmentedXMLReporter`
* `stc::testutil`: Made the reporters link properly without requiring them to manually be included in a source file

