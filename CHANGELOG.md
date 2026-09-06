# Changelog

This changelog is only maintained as of v2026-09-06. Versions prior to this were not tracked.

## v2026-09-06.1

* `stc`: Fixed `Process.hpp` not handling that `execve` returns on error, resulting in weird behaviour on execution failures

## v2026-09-06

* `stc`: Removed `minilog`. Switch to https://codeberg.org/LunarWatcher/minilog
* `stc::testutil`: Added `AugmentedXMLReporter`
* `stc::testutil`: Made the reporters link properly without requiring them to manually be included in a source file

