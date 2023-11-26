# FileLock

Utility wrapper for creating a lockfile for cross-process file-based resource locking. Note that this class is NOT intended to lock the file being edited, but rather a separate file. This is to create a synchronisation object on the filesystem that can be used more flexibly than just locking a file. 

For example, this allows for an [apt](https://en.wikipedia.org/wiki/APT_(software))-style locking system, or something else requiring exclusive write access to a large amount of files. It can also be used in cases where the main file itself is opened in a way that would be blocked by the lock (for example `std::iostream`, or a library that automatically opens the file). 

Should you try to use it to lock a file being edited anyway, note that:
* The file descriptor is not exported from the class
* The lock will prevent other stuff from opening it, including `std::iostream`
* When unlocked, the file is automatically deleted (as long as nothing else has opened the file already)

The last point means that, if used on a file of value, the file would be destroyed when it's unlocked. You have been warned.

The file does not have to exist in advance (it's automatically created as well), but it should be named in a way that ensures the lockfile name is guaranteed not to be used by anything else. 

## FileLock (class)

### Types
#### `FileLock::Errors` (enum class)

Values:
* `OPEN_ERROR = 42` - if thrown, the lockfile couldn't be opened.
* `LOCK_ERROR = 43` - if thrown, the lockfile opened, but couldn't be locked. 

### `constructor(fs::path lockPath, bool lockNonblocking = true)` 

The constructor attempts to automatically acquire the lock. If it fails, it throws an error from `FileLock::Errors`.

Note that the lockNonblocking argument only takes effect on Linux, as Windows has different locking semantics. 

### Functions

#### `void unlock()`

Unlocks the lock. Note that when released, the lock instance cannot reacquire the lock. Instead, a new instance has to be created.

#### `bool hasLock()`

Returns whether or not the lock is currently acquired.

#### `std::shared_ptr<FileLock> dynamicAcquireLock(fs::path&, std::function<bool()> control, unsigned int sleepSeconds = 1)` [Static]

Utility function for polling availability.

On Linux, this is not the most resource-efficient approach if you just want to wait for the lock to be acquired. Passing `false` for `lockNonblocking` in the constructor means `lock()` will wait for the different process, and the continue. 

However, on Windows, or in an environment where you'd like to, for example, refresh the UI or notify the user on the same thread as the lock, this function can be used.

As long as `control` returns true, the function will continue to try to acquire the lock. This means that the function can be used to wait for some amount of time before stopping, for example. It can also be used to immediately notify the user that the lock couldn't be acquired. The exact use for this function is implementation-dependent.

**Note:** the result has to be kept in a local variable. Otherwise, the lock will be released immediately. 
