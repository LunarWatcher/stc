/** \file */
#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace stc {

/**
 * Helper class for taking password input. 
 *
 * Example use:
 * ```cpp
 * std::string password;
 * // Create a scope for the password input. You don't have to create a scope; you can also just call .release(). This
 * // is just my personal preference, as anything inside the scope is clearly password input, while everything outside
 * // isn't. This eliminates bugs from taking extra input and accidentally putting it before .release()
 * { 
 *     // Sets the flags that hides password input
 *     stc::PasswordIO io;
 *     std::string password;
 *     // You can get the password from stdin as usual
 *     std::getline(std::cin, password);
 * } // Once `io` goes out of scope, input echoing is reenabled
 * if (password == "1234") { ... }
 * ```
 *
 * **Warning:** This class is not thread-safe. It's possible for one thread to acquire the lock, and a different thread
 * to immediately release it afterwards. It's recommended to limit use of this function to the main thread (or a thread
 * acting as the main thread for input, if applicable). If you really _need_ to use this on several threads, it must be
 * combined with a mutex or similar.
 */
class PasswordIO {
private:
    bool freed = false;
    void set(bool enable) {
#ifdef _WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
        DWORD mode;
        GetConsoleMode(hStdin, &mode);

        if (!enable) {
            mode &= ~ENABLE_ECHO_INPUT;
        } else {
            mode |= ENABLE_ECHO_INPUT;
        }

        SetConsoleMode(hStdin, mode);
#else
        struct termios tty;
        tcgetattr(STDIN_FILENO, &tty);

        if (!enable) {
            tty.c_lflag &= ~ECHO;
        } else {
            tty.c_lflag |= ECHO;
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif

    }
public:
    PasswordIO() {
        set(false);
    }
    ~PasswordIO() {
        release();
    }

    void release() {
        if (freed) return;
        freed = true;

        set(true);
    }
};

}
