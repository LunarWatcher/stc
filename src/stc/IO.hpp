#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <mutex>

namespace stc {
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
