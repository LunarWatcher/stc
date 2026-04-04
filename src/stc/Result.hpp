#pragma once

#include <stdexcept>
#include <variant>

namespace stc {

/**
 * Contains a generic Result type, a type that holds either a result or an error.
 *
 * This can be treated as a specialised wrapper around std::variant<T, ERR> with some QOL features.
 *
 * ## Constructing an OK result
 * ```cpp
 * // You do not have to `using` the type, but it makes for significantly shorter commands
 * using ResultImpl = stc::Result<int, std::string_view>;
 * ResultImpl someFunc(...) {
 *     return ResultImpl::ok(0);
 * }
 * ```
 * ## Constructing an error result
 * ```cpp
 * // You do not have to `using` the type, but it makes for significantly shorter commands
 * using ResultImpl = stc::Result<int, std::string_view>;
 * ResultImpl someFunc(...) {
 *     return ResultImpl::err("Message");
 * }
 * ```
 * ## Checking and accessing results
 * ```
 * using ResultImpl = stc::Result<int, std::string_view>;
 * ResultImpl someResult = func();
 * if (someResult.isOk()) {
 *     auto& okType = someResult.unwrap();
 *     // This is also legal for access in non-primitive types:
 *     auto field = someResult->someField;
 *     // ... use field
 * } else if (someResult.isError()){ // This check is redundant, since isError = !isOk, but is included here to show
 *                                   // that it exists
 *     auto& errType = someResult.error();
 * }
 * ```
 *
 * Note that using operator-> or unwrap() when isError()
 */
template <typename T, typename ERR>
struct Result {
private:
    using InternalType = std::variant<T, ERR>;
    const InternalType held;

    Result(InternalType&& value)
        : held(std::move(value)) {}

public:
    const T* operator->() const {
        if (held.index() != 0) {
            throw std::runtime_error("Illegal access into Result type when an error is held");
        }

        return &(std::get<0>(held));
    }

    bool isOk() const { return held.index() == 0; }
    bool isError() const { return held.index() == 1; }

    const T& unwrap() { return std::get<0>(held); }
    const ERR& error() { return std::get<1>(held); }

    static Result<T, ERR> ok(T&& value) {
        return Result(InternalType(std::in_place_index<0>, std::move(value)));
    }
    static Result<T, ERR> err(ERR&& value) {
        return Result(InternalType(std::in_place_index<1>, std::move(value)));
    }
};

}
