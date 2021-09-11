#pragma once

#if __has_include(<optional>)
    #include <optional>
    template <typename T>
    using StdOptional = std::optional<T>;
#else
    #include <experimental/optional>
    template <typename T>
    using StdOptional = std::experimental::optional<T>;
#endif
