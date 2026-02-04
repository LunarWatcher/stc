# `ExplicitStreamTestReporter`

`ExplicitStreamTestReporter` is a test reporter for Catch2 inspired by Pytest's `-v` output. Note that at this time, it's purely an extension on top of the console reporter. Due to limitations in catch2[^1], the console reporter had to be ripped out of catch2 to make this possible.

The majority of the output from it (read: benchmarks, test suite information, and failure messages) is therefore identical to the standard `console` reporter, with extra information added about the tests running. 

## Example output
```
Edge-case line intersections on 2-point rectangle                      STARTING
  Vertical line intersection matching edge (inclusive)                 STARTING
  Vertical line intersection matching edge (inclusive)                 PASSED
Edge-case line intersections on 2-point rectangle                      PASSED

All functions compile                                                  STARTING
All functions compile                                                  PASSED

Process should handle signals appropriately                            STARTING
  PTY mode                                                             STARTING
    SIGKILL                                                            STARTING
    SIGKILL                                                            PASSED
  PTY mode                                                             PASSED
Process should handle signals appropriately                            PASSED

# ...

Validate deletion semantics                                            STARTING
  It should be removed automatically when a new lock is acquired       STARTING
  It should be removed automatically when a new lock is acquired       PASSED
Validate deletion semantics                                            PASSED

Ensure locking works                                                   STARTING

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tests is a Catch2 v3.12.0 host application.
Run with -? for options

-------------------------------------------------------------------------------
Ensure locking works
-------------------------------------------------------------------------------
/home/olivia/programming/cpp/stc/tests/src/LockTests.cpp:5
...............................................................................

/home/olivia/programming/cpp/stc/tests/src/LockTests.cpp:26: FAILED:
explicitly with message:
  hi

Ensure locking works                                                   FAILED

Clearing buffers should work                                           STARTING
Clearing buffers should work                                           PASSED

===============================================================================
test cases:  53 |  52 passed | 1 failed
assertions: 592 | 591 passed | 1 failed
```

Also note that all the output types are coloured for easier reading, but this isn't trivial to embed in the docs.

The tests are grouped, so each space-separated segment is one `TEST_CASE`. The indented segments are `SECTION`s, and they can nest arbitrarily deep. Obviously, at a certain point, it'll result in overflow and shift the state to a new line, but if you have tests that long and/or deep, that's on you.

## Extended description

The main goal of this reporter is to make it much clearer precisely what tests are running. While the default `console` reporter tells you what's going on if something fails, it doesn't lend itself to showing you what _should_ have failed. 

For example, the specific event that triggered this implementation was me adding a test in another project, and then noticing the assertion count was suspiciously low still. Without adding debug logging or a temporary `FAIL()`, I have no way to actually confirm if the tests I just added actually ran. 

The explicit output of this reporter (hence the name) is meant to avoid problems like these, as well as make it easier to group `stdout` output. Due to the reporter format, any stdout output[^2] will be grouped between a `STARTING` and a `SKIPPED/FAILED/PASSED`:
```
Syscommand should handle return codes                                  STARTING
sh: 1: gjdjgsdjgkfdsjkglfdjkglsfdjklgøsjklgøsjklgødjklgøsfdjklgø: not found
Syscommand should handle return codes                                  PASSED

stc::Unix::Environment defining existing variables shouldn't cause problems STARTING
stc::Unix::Environment defining existing variables shouldn't cause problems PASSED

Verify that stdout can be captured by CaptureStream                    STARTING
CaptureStream test output after release (if you're seeing this, the test has passed)
Verify that stdout can be captured by CaptureStream                    PASSED
```

This means that if you're running your entire test suite, and added debug logging that prints quite a few times, you can now actually see which logging is associated with what. You could of course filter what tests you run if it's logging specifically affecting something  that's known to directly cause a failure, but this isn't as applicable if you're looking for anything of interest in any number of test cases. 

No other catch reporters lend themselves to human-readable output like this. The automake reporter comes close, but it also discards all other human-readable at-a-glance-style information, so it eliminates itself. Most of the other reporters are explicitly only for machine readability, or so unnecessarily verbose again that it's hard to see just what's running. 

The console reporter does respect `--success`, which prints on success as well, but  that drags the verbosity so far it becomes useless again:

```
-------------------------------------------------------------------------------
When enabled, Process should output its command
-------------------------------------------------------------------------------
/home/olivia/programming/cpp/stc/tests/src/unix/UnixCommandTests.cpp:346
...............................................................................

/home/olivia/programming/cpp/stc/tests/src/unix/UnixCommandTests.cpp:357: PASSED:
  REQUIRE( result == 0 )
with expansion:
  0 == 0
with message:
  


/home/olivia/programming/cpp/stc/tests/src/unix/UnixCommandTests.cpp:358: PASSED:
  REQUIRE( capt.cerr.content.str() == "" )
with expansion:
  "" == ""
with message:
  


/home/olivia/programming/cpp/stc/tests/src/unix/UnixCommandTests.cpp:360: PASSED:
  REQUIRE( capt.cout.content.str() == R"(Exec: "/usr/bin/env" "bash" "-c" "echo" "hi"
)" )
with expansion:
  "Exec: "/usr/bin/env" "bash" "-c" "echo" "hi"
  "
  ==
  "Exec: "/usr/bin/env" "bash" "-c" "echo" "hi"
  "
with messages:
  

  |Exec: "/usr/bin/env" "bash" "-c" "echo" "hi"
  |

```

(this is with `-s -v quiet`)

Right now, the explicit reporter only focuses on section logging, but I do plan to expand it at some point. Particularly, allowing different verbosity levels, and eventually shedding the vendored `ConsoleReporter`.

## Usage

At this time, the reporter is only provided as a header. As far as I can tell, Catch2 requires the reporter to be in the runtime translation unit to register properly. This means you need a custom main file. If you do not have one, this is all you need:

```cpp
#include <catch2/catch_session.hpp>
// My editor whines that this is unused, but this is wrong.
#include "stc/test/catch2/ExplicitStreamTestReporter.hpp"

int main(int argc, const char* argv[]) {
    auto sess = Catch::Session();
    return sess.run(argc, argv);
}
```

Don't forget to link your tests against `stc::testutil`!

If you also want to change the default reporter, you can do this with:
```cmake
set(CATCH_CONFIG_DEFAULT_REPORTER "explicit" CACHE STRING "" FORCE)
# or find_package or whatever
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.12.0
)
# ...
```

If you just want it available as an option, use `./your-test-binary --reporter explicit` to use the reporter.

[^1]: Read: they made the fucking thing `final`. Very pointless imo, makes it very difficult to iteratively build reporters without losing functionality. I do not want to reimplement benchmarks and error reporting, I just want to reimplement how the section progression is displayed. This will change at some point:tm:, but it is not a priority.
[^2]: Race conditions willing anyway
