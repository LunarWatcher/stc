/** \file */
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <cstdlib>
#include <regex>
#include <array>
#include <cstdio>
#include <iostream>

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>
#else
#define NOMINMAX
#include <Windows.h>
#include <io.h>

#define popen _popen
#define pclose _pclose
#define WEXITSTATUS
#endif

namespace stc {

inline std::string getEnv(const char* name, const std::string& fail = "") {
#if defined(_WIN32) || defined(_WIN64)
    char* value = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&value, &len, name);
    if (err != 0 || value == nullptr)
        return fail;

    return std::string(value);
#elif defined _GNU_SOURCE
    // _GNU_SOURCE is predefined with libstdc++ (GCC's stdlib),
    // and when it's defined, it grants access to secure_getenv.
    // Dunno if this works outside linux, hence why there's no
    // forced enabling of _GNU_SOURCE
    char* output = secure_getenv(name);
    if (!output)
        return fail;
    return std::string(output);
#else
#warning "Failed to find a secure getenv() for your OS - please open an issue at https://github.com/LunarWatcher/stc if you know of an equivalent."
    char* v = std::getenv(name);
    if (v == nullptr) return fail;
    // The string may need to be copied here instead, but iDunno
    // Most operating systems shouldn't be using this anyway, so
    // it shouldn't be a problem.
    return std::string(v);
#endif
}

/**
 * Expands a user path (AKA a path starting with ~), independently of the OS.
 * This requires several OS-specific calls (specifically between Windows and UNIX, from what I can tell.
 * There's not much of a difference in this area between for an instance Linux and Mac. Special
 * implementation requirements will be taken when we get to a point where it's needed)
 */
inline std::filesystem::path expandUserPath(const std::string& inputPath) {
    // Convert all backslashes to forward slashes for processing (fuck you Windows)
    std::string rawPath = std::regex_replace(inputPath, std::regex("\\\\"), "/");

    // In order to universally support paths without doing a hacky if-check elsewhere,
    // this just returns the path itself if it isn't a home path.
    // Other paths should work themselves out
    if (rawPath.at(0) != '~')
        return rawPath;

    std::optional<std::string> username;
    std::string remainingPath;

    // The next part of the code is used to parse off the username,
    // and grab the rest of the path. This is necessary to reconstruct
    // the path afterwards.
    std::string mod = rawPath.substr(1);
    std::vector<std::string> pathSplit; // = StrUtil::splitString(mod, "/", 1);

    // splitString but smaller so I don't have to import everything
    size_t pos = 0;
    std::string token, cache;
    pos = mod.find('/');
    token = mod.substr(0, pos);
    cache = mod;
    cache.erase(0, pos + 1);
    pathSplit.push_back(token);
    pathSplit.push_back(cache);

    //  Parse off the username
    if (rawPath.length() >= 2 && rawPath.at(1) != '/') {
        // ~username
        // ~username/path
        username = pathSplit.at(0);
        remainingPath = pathSplit.at(1);
    } else if (rawPath.find('/') != std::string::npos) {
        // The path contains a slash.

        remainingPath = pathSplit.at(1);
    }

    // We've by now found a username or not. This means we can actually expand
    // the path.

    std::string homePath = "";
#if defined(_WIN32) || defined(_WIN64)

    if (!username.has_value()) {
        auto userProfile = getEnv("USERPROFILE");

        if (userProfile.empty()) {
            auto homeDrive = getEnv("HOMEDRIVE");
            if (homeDrive.empty())
                homeDrive = "";

            auto envHomePath = getEnv("HOMEPATH");

            if (envHomePath.empty()) {
                throw std::runtime_error("Unable to find %HOMEPATH%. Specify the path explicitly instead.");
                //return "";
            }
            homePath = homeDrive + envHomePath;
        } else
            homePath = userProfile;

    } else {
        throw std::runtime_error("This doesn't work."
                     " Due to Windows having a very limited API for expanding user paths, and it relies on environment "
                     "variables and assumptions, me (the developer), has decided to not implement ~user expansion on "
                     "Windows. "
                     "I cannot easily test it, nor can I find any reassuring information for a universal pattern I can "
                     "use. "
                     "Replace your path with an absolute path instead. An implementation for this feature may be "
                     "available in the future.");
        //return "";
    }
    // Force forward slashes
    homePath = std::regex_replace(homePath, std::regex("\\\\"), "/");

#else
    /*
     The unixes are easier, because they should in theory share a single
     API that makes this a whole lot easier.
     The idea is checking for HOME if we're looking for the current user.
     If we can't find the home variable, fall back to a UNIX-specific^1
     function that retrieves the path, along with a few other details.
     see getpwnam(3) for more info on the functions, and passwd(5)
     for details on the struct. This code drops the HOME variable for
     various reasons.
     If a username has been supplied (~username/), getpwnam is used instead
     of getpwuid. This returns the same type of struct as getpwuid().
     The bonus with the UNIX approach is that it requires the user to exist,
     where as Windows for non-existent users with a custom specified path should
     in theory cause a bad path. This is on the user, however.
     ^1: untested on mac and other UNIX-based systems. Only verified on Linux.
    */
    struct passwd* passwdPtr = nullptr;

    if (!username.has_value()) {
        // While the home environment variable can be used,
        // getenv() is considered insecure
        // secure_getenv is only available on GNU/Linux, and not Mac.
        // Mac is still compatible with the rest of the code,
        // so no environment variables are used
        passwdPtr = getpwuid(getuid());
    } else {
        auto& name = *username;
        passwdPtr = getpwnam(name.c_str());
    }

    if (passwdPtr == nullptr) {
        throw std::runtime_error(std::string("Failed to expand the user path for ") + rawPath + ". The system seems to think you don't exist. "
                     "Please specify the path to use - don't abbreviate it with ~.\n");
    }
    homePath = passwdPtr->pw_dir;
#endif
    return std::filesystem::path{homePath} / remainingPath;
}

inline std::filesystem::path getHome() {

    std::optional<std::string> username;
    std::string remainingPath;

    std::string homePath = "";
#if defined(_WIN32) || defined(_WIN64)
    auto userProfile = getEnv("USERPROFILE");

    if (userProfile.empty()) {
        auto homeDrive = getEnv("HOMEDRIVE");
        if (homeDrive.empty())
            homeDrive = "";

        auto envHomePath = getEnv("HOMEPATH");

        if (envHomePath.empty()) {
            throw std::runtime_error("Failed to find home path");
        }
        homePath = homeDrive + envHomePath;
    } else
        homePath = userProfile;

    // Force forward slashes
    homePath = std::regex_replace(homePath, std::regex("\\\\"), "/");

#else
    /*
     The unixes are easier, because they should in theory share a single
     API that makes this a whole lot easier.
     The idea is checking for HOME if we're looking for the current user.
     If we can't find the home variable, fall back to a UNIX-specific^1
     function that retrieves the path, along with a few other details.
     see getpwnam(3) for more info on the functions, and passwd(5)
     for details on the struct. This code drops the HOME variable for
     various reasons.
     If a username has been supplied (~username/), getpwnam is used instead
     of getpwuid. This returns the same type of struct as getpwuid().
     The bonus with the UNIX approach is that it requires the user to exist,
     where as Windows for non-existent users with a custom specified path should
     in theory cause a bad path. This is on the user, however.
     ^1: untested on mac and other UNIX-based systems. Only verified on Linux.
    */
    struct passwd* passwdPtr = nullptr;

    // While the home environment variable can be used,
    // getenv() is considered insecure
    // secure_getenv is only available on GNU/Linux, and not Mac.
    // Mac is still compatible with the rest of the code,
    // so no environment variables are used
    //
    // The odds that this presents a threat are probably tiny,
    // but hey, if I'm putting shit in a ton of projects, I'm
    // gonna make sure I do it properly.
    //
    // Also, LOOK AT HOW EASY THIS IS! It's literally 6 lines
    // of code, and one API call, instead of that if-statement cascade bullshit Windows needs
    passwdPtr = getpwuid(getuid());

    if (passwdPtr == nullptr) {
        throw std::runtime_error("Failed to find home directory");
    }
    homePath = passwdPtr->pw_dir;
#endif
    return std::filesystem::path{homePath};
}

inline std::string syscommand(const std::string& command, int* codeOutput = nullptr) {
    std::array<char, 128> buffer;
    std::string res;

    std::unique_ptr<std::FILE, void(*)(FILE*)> fd {
        popen(command.c_str(), "r"),
        [](FILE* fd) {
            std::ignore = pclose(fd);
        }
    };
    if (!fd) {
        throw std::runtime_error("Failed to run " + command);
    }
    size_t bytes = 0;
    while ((bytes = fread(
        buffer.data(),
        sizeof(buffer[0]),
        static_cast<int>(buffer.size()),
        fd.get())
    ) > 0) {
        res.insert(res.end(), buffer.begin(), buffer.begin() + bytes);
    }

    if (codeOutput != nullptr) {
        int r = pclose(fd.release());
        int exitCode = WEXITSTATUS(r);
        *codeOutput = exitCode;
    }
    return res;
}

#ifndef _WIN32
/**
 * Low-level command execution, bypassing shell evaluation. For shell evaluation, use syscommand(string, int*), or pass
 * a shell directly to this command.
 */
inline std::string syscommand(std::vector<const char*> command, int* codeOutput = nullptr) {
    command.push_back(nullptr);
    std::array<char, 256> buffer;
    std::string res;


//#ifndef _WIN32
    int fd[2];
    pipe(fd);

    auto pid = fork();

    if (pid < 0) {
        throw std::runtime_error("Fork error");
    } else if (pid == 0) {
        // Child process

        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        close(fd[0]);
        close(fd[1]);

        execv(command.at(0), (char**) command.data());
        exit(1);
    } else {
        close(fd[1]);
        size_t bytes = 0;
        while ((bytes = read(
            fd[0],
            buffer.data(), buffer.size()
        )) > 0) {
            res.insert(res.end(), buffer.begin(), buffer.begin() + bytes);
        }

        int status;
        waitpid(pid, &status, 0);

        if (codeOutput != nullptr) {
            int exitCode = WEXITSTATUS(status);
            *codeOutput = exitCode;
        }

    }
//#else
    //using pipe_handle = void*;

    //auto stdoutRead = pipe_handle();
    //auto stdoutWrite = pipe_handle();

    //auto secAttr = SECURITY_ATTRIBUTES {
        //.nLength = sizeof(SECURITY_ATTRIBUTES),
        //.lpSecurityDescriptor = nullptr,
        //.bInheritHandle = true
    //};

    //CreatePipe(
        //&stdoutRead, 
        //&stdoutWrite, 
        //&secAttr,
        //0 
    //);

    //SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);

    //auto startInfo = STARTUPINFO {
        //.cb = sizeof(STARTUPINFOA),
        //.dwFlags = STARTF_USESTDHANDLES,
        //.hStdOutput = stdoutWrite,
        //.hStdError = stdoutWRite
    //};


    //auto procInfo = PROCESS_INFORMATION();

    //CreateProcess(
        //command.at(0),
        //nullptr,
        //nullptr,
        //nullptr,
        //true,
        //0, 
        //nullptr,
        //nullptr,
        //&startInfo,
        //&procInfo
    //);

    //CloseHandle(stdoutWrite);

    //while(true) {
        //auto readBytes = DWORD(0);

        //auto success =
            //ReadFile(
                //stdoutRead,
                //buffer.data(),
                //buffer.size(),
                //&readBytes,
                //nullptr 
            //);

        //// Is this correct?
        //if(!success) {
            //break;
        //}

        //if (readBytes > 0) {
            //res.insert(res.end(), buffer.begin(), buffer.begin() + readBytes);
        //}

    //}

    //WaitForSingleObject(procInfo.hProcess, INFINITY);
    //CloseHandle(stdoutRead);

    //if (codeOutput != nullptr) {
        //GetExitCodeProcess(procInfo.hProcess, (LPDWORD) codeOutput)
    //}
//#endif

    return res;
}

/**
 * (Theoretically) safe equivalent of std::system
 */
inline void syscommandNoCapture(std::vector<const char*> command, int* codeOutput = nullptr) {
    command.push_back(nullptr);

    auto pid = fork();

    if (pid < 0) {
        throw std::runtime_error("Fork error");
    } else if (pid == 0) {
        execv(command.at(0), (char**) command.data());
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (codeOutput != nullptr) {
            int exitCode = WEXITSTATUS(status);
            *codeOutput = exitCode;
        }

    }
}
#endif

inline std::optional<std::string> getHostname() {
    // According to the linux manpage, and the Windows docs page, it looks like approximately 256 bytes is the max length across all platforms.
#ifndef _WIN32
    constexpr size_t size = 256 + 2 /* + 2 for padding, just in case :) */;
#else
    constexpr size_t size = MAX_COMPUTERNAME_LENGTH + 1;
#endif

    char hostname[size];
#ifndef _WIN32
    int res = gethostname(hostname, size);
    if (res != 0) {
        return std::nullopt;
    }

    std::string str(hostname);
    return str;
#else
    DWORD _size = size;
    bool res = GetComputerNameEx(
        ComputerNamePhysicalDnsHostname, 
        hostname,
        &_size); 
    if (res == false) {
        return std::nullopt;
    }
    return std::string {
        hostname,
        (size_t) _size
    };
#endif

}

enum class StreamType {
    STDOUT,
    STDERR,
    OTHER
};  

/**
 * Utility wrapper around isatty (UNIX)/_isatty (Windows).
 */
inline bool isStreamTTY(StreamType type = StreamType::STDOUT) {
    if (type == StreamType::OTHER) {
        return false;
    }
    auto streamDescriptor = type == StreamType::STDOUT ? stdout : stderr;

#ifdef _WIN32
    return _isatty(_fileno(streamDescriptor));
#else
    return isatty(fileno(streamDescriptor));
#endif
}

template <typename CharT>
static constexpr StreamType getOutputStreamType(std::basic_ostream<CharT>& ss) {
    if constexpr(std::is_same_v<CharT, char>) {
        if (&ss == &std::cout) {
            return StreamType::STDOUT;
        } else if (&ss == &std::cerr) {
            return StreamType::STDERR;
        } 
    } else if constexpr (std::is_same_v<CharT, wchar_t>){
        if (&ss == &std::wcout) {
            return StreamType::STDOUT;
        } else if (&ss == &std::wcerr) {
            return StreamType::STDERR;
        } 
    }
    return StreamType::OTHER;
}

template <typename CharT>
inline bool isCppStreamTTY(std::basic_ostream<CharT>& ss) {
    return isStreamTTY(
        getOutputStreamType(ss)
    );
}

}
