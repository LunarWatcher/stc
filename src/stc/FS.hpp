#pragma once

#if __has_include(<filesystem>)
    #include <filesystem>
    namespace stcfs = std::filesystem;
#else
    #include "experimental/filesystem"
    namespace stcfs = std::experimental::filesystem;
#endif
