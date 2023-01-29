#pragma once

#if __has_include(<format>)
    #include <format>
    namespace fmt = std;
#else
    #include "fmt/format.h"
#endif
