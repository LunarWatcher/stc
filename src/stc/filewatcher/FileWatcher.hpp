#pragma once

#include "FileWatcherCommon.hpp"

#ifndef __WIN32__
#include "LinuxFileWatcher.hpp"
#else
#error "Windows is not yet supported"
#endif

namespace stc {

/**
 * Contains the real type declaration for the FileWatcher. The signature is guaranteed to be identical on all supported
 * platforms, but the raw underlying type is not.
 * The FileWatcher type is aliased to the platform-specific FileWatcher.
 *
 * ## Description
 *
 * The FileWatcher is a class that can be used to watch for changes in files and folders. It emits standardised events
 * for the processed events, so all code written for it would be cross-platform if it had a windows implementation.
 * The FileWatcher offers some config options for ignoring folders, but ignoring files has to be done at an application
 * level if applicable. The third argument to the constructor can be used to define a callback for whether or not a
 * folder should be watched. This avoids needing to handle as many file events.
 *
 * ## Implementation caveats
 *
 * Unless otherwise noted in this section in the future, the FileWatcher does not attempt to guarantee perfect
 * watching. There are may cases where the output of the FileWatcher may start to drift from the system. This class
 * should NOT be used if a 100% accurate record of all changes to the filesystem has to be maintained. This class is
 * optimized for use-cases where knowing that _some_ change has happened is more useful than knowing every single
 * recorded event that's part of the change.
 *
 * Events may be missed due to notably the Linux implementation (using inotify) not supporting recursive watching. This
 * means that rapid succession folder creation and deletion can miss individual events. If you need more granularity
 * than what inotify offers, this is not the library you are looking for.
 */
using FileWatcher =
#ifndef __WIN32__
    linux_impl::LinuxFileWatcher;
#endif

}

