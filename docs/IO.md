# IO

Cross-platform wrappers for IO-related stuff

## PasswordIO (class)

Utility RAII wrapper around platform-specific functions for disabling input echoing.

Example use:
```cpp
std::string password;
{
    std::cout << "Enter password: ";
    stc::PasswordIO io;
    std::getline(std::cin, password);
    // Note: due to the destructor implementation, the lock is auto-released
    // when the lock goes out of scope.
}

if (password == "1234") {
    // ...
}
```

**Warning:** This class is not thread-safe. It's possible for one thread to acquire the lock, and a different thread to immediately release it afterwards. It's recommended to limit use of this function to the main thread (or a thread acting as the main thread for input, if applicable). If you really _need_ to use this on several threads, it must be combined with a mutex or similar.

### release()

re-enables input echo. Note that this doesn't have to be manually called, as the destructor handles it for you.
