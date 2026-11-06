# Changelog

This changelog is only maintained as of v2026-09-06. Versions prior to this were not tracked.

The version format is an augmented date format: `vyyyy-mm-dd[.patchnumber]`. `.patchnumber` is only in use when there are multiple releases in one day.

## v2026-11-06.1

* `stc`: Fixed a bug where `uri` throws on empty and single-char URIs 

## v2026-11-06

* `stc`: Added `filewatcher/FileWatcher.hpp` (Linux only)

## v2026-11-02.1

* `stc`: Fixed `stc::uri` not supporting paths only

## v2026-11-02

* `stc`: Added `stc::uri` for parsing URIs with fairly little validation

## v2026-09-13

* `stc::Unix::Process`: Added method for exposing the PID

## v2026-09-06.2

* `stc::testutil`: Fixed Augmented XML reporter not including stdout per test run

## v2026-09-06.1

* `stc`: Fixed `Process.hpp` not handling that `execve` returns on error, resulting in weird behaviour on execution failures

## v2026-09-06

* `stc`: Removed `minilog`. Switch to https://codeberg.org/LunarWatcher/minilog
* `stc::testutil`: Added `AugmentedXMLReporter`
* `stc::testutil`: Made the reporters link properly without requiring them to manually be included in a source file

