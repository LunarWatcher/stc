#include <catch2/catch_session.hpp>
// This _is_ used, do not remove.
#include "stc/test/catch2/ExplicitStreamTestReporter.hpp"

#ifndef _WIN32
#include <unistd.h>
#endif

int main(int argc, const char* argv[]) {

    auto sess = Catch::Session();
#ifndef _WIN32
    // Seems to be required for the stc::Unix::Process tests to pass. Not sure why.
    // Observationally:
    // * It only occasionally fails with `make test`
    // * setsid after fork() makes it worse
    // * setsid after fork() with setsid here causes PTY tests to fail
    // * It never fails in gdb
    // Need to read up on this later to try to identify the cause. This behaviour makes no sense, and I have no way to
    // debug it since it can't be reproduced when run through gdb. 
    // 
    // For context, the failure is `SIGTERM - termination request signal`
    // This initially lined up with ~Process() calling stop(), which sends SIGTERM, but it's reproducable with sigkill
    // invoked.
    // It was initially only in non-redirected mode, but depending on the setsid() configuration (read: always called
    // after fork()), it's reproducable everywhere. 
    // It's not the signal propagating, because that should've resulted in a SIGKILL instead. I wonder if catch2
    // registers signal handlers, and that causes at least part of the weirdness.
    // I do observe that the tests still pass, so this in particular lines up with catch2 incorrectly intercepting the
    // SIGTERM, and reporting it as a test failure, but because it doesn't abort, a later call to SUCCEED() results in
    // that being overridden.
    setsid();
#endif

    return sess.run(argc, argv);
}
