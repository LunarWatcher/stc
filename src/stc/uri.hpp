#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <sstream>
#include <vector>

namespace stc {

/**
 * Generic struct that actually contains the path components. Exists separately from the main URI class to allow this
 * class to have a struct initializer, since { .host = "example.com" } in initialization is much more readable  
 */
struct URIComponents {
    std::optional<std::string> scheme = std::nullopt;

    /**
     * The userinfo part. This is stored as a vector instead of the conventional username:password, because that combo
     * is deprecated: https://www.rfc-editor.org/info/rfc3986/#section-3.2.1
     * Additionally, according to the URI standard, this is more of a suggestion than a requirement. `p1:p2:p3:p4:p5` is
     * also valid userinfo
     */
    std::optional<std::vector<std::string>> userinfo = std::nullopt;
    std::optional<std::string> host = std::nullopt;
    uint16_t port = 0;

    std::optional<std::string> path = std::nullopt;
    std::optional<std::string> query = std::nullopt;
    std::optional<std::string> fragment = std::nullopt;

    bool operator==(const URIComponents& other) const = default;

    std::string to_string() const {
        std::stringstream ss;

        if (scheme) {
            ss << *scheme << ":";
        }

        if (host) {
            ss << "//";
            if (userinfo) {
                for (size_t i = 0; i < userinfo->size(); ++i) {
                    if (i != 0) {
                        ss << ":";
                    }
                    ss << userinfo->at(i);
                }
                ss << "@";
            }

            ss << *host;
            if (port != 0) {
                ss << ":" << port;
            }
        }

        if (path) {
            ss << *path;
        }

        if (query) {
            ss << "?" << *query;
        }
        if (fragment) {
            ss << "#" << *fragment;
        }

        return ss.str();
    }
};

/**
 * Generic URI parser.
 *
 * Please note that this URI parser is meant to be much more flexible than many other URI parser implementations; not
 * notably makes the scheme optional (in order to support //example.com-style URLs), and it does no validation on
 * whether the URI is valid or not. No normalization or or from URL encoding is done; it's the responsibility of the
 * application using this class to handle URL encoding appropriately. Or not, I don't care.
 */
class uri {
private:
    URIComponents uriData = {};

    void parseAuthority(const std::string& authority) {
        auto at = authority.find('@');
        size_t next = 0;

        if (at != std::string::npos) {
            auto userInfoPart = authority.substr(0, at);
            next = at + 1;

            uriData.userinfo = std::vector<std::string>{};

            size_t next = 0;
            do {
                auto end = userInfoPart.find(':', next);

                auto part = userInfoPart.substr(
                    next,
                    end == std::string::npos ? end : (end - next)
                );
                uriData.userinfo->push_back(part);

                next = end == std::string::npos ? end : (end + 1);
            } while (next != std::string::npos);
        }

        size_t end = std::string::npos;
        if (authority.at(next) == '[') {
            end = authority.find(']', next);
            if (end == std::string::npos) {
                throw std::runtime_error("Malformed IPv6 URI");
            }
            end += 1;
        } else {
            end = authority.find(':', next);
        }
        uriData.host = authority.substr(next, end == std::string::npos ? end : end - next);

        if (end != std::string::npos) {
            auto colon = authority.find(':', end);
            if (colon != std::string::npos) {
                if (colon != authority.size() - 1) {
                    auto portPart = authority.substr(colon + 1);
                    if (portPart.empty()) {
                        return;
                    }

                    // port
                    auto port = std::stoi(portPart);

                    if (port < 0 || port > (int) std::numeric_limits<uint16_t>::max()) {
                        throw std::range_error("Illegal port");
                    }

                    uriData.port = port;
                }
            }
        }
    }

    void parse(const std::string& source) {
        // reset all the things so this can be used for copies as well
        uriData = {};

        if (source.size() == 0) {
            return;
        }

        size_t next = 0;
        auto colon = source.find(':', next);

        // URI parser extension: allow implicit scheme
        // Ignore `:` if it is after a `/` (parser extension: support optional schemes)
        auto firstSpecial = source.find_first_of("?#/");
        if (colon != 0 && colon < firstSpecial) {
           uriData.scheme = source.substr(0, colon);
        } else if (colon > firstSpecial) {
            colon = std::string::npos;
        }
        next = colon == std::string::npos ? 0 : (colon + 1);

        if (
            source.size() > 2
            && next < source.size() - 2
            && source.at(next) == '/'
            && source.at(next + 1) == '/'
        ) {
            // Authority part
            next += 2;
            auto end = source.find_first_of("?#/", next);
            
            std::string authority = source.substr(next, end == std::string::npos ? end : (end - next));
            parseAuthority(authority);

            next = end;
        }

        if (next != std::string::npos && source.at(next) != '#' && source.at(next) != '?') {
            // Path
            auto end = source.find_first_of("?#", next);
            // next += 1;

            auto sizeEnd = end;
            if (sizeEnd != std::string::npos) {
                sizeEnd -= next;
            }

            if (next < source.size() && sizeEnd != 0) {
                uriData.path = source.substr(next, sizeEnd);
            } else {
                return;
            }
            
            next = end;
        }

        if (next != std::string::npos && next < source.size() && source.at(next) == '?') {
            auto end = source.find('#', next);
            next += 1;
            if (end - next != 0) {
                uriData.query = source.substr(next, end - next);
            }
            next = end;
        }

        if (next != std::string::npos && source.at(next) == '#') {
            if (next + 1 != source.size()) {
                uriData.fragment = source.substr(next + 1);
            }
        }
    }

public:
    uri(const char* source) : uri(std::string(source)) {}
    uri(const std::string& source) {
        parse(source);
    }
    uri(const URIComponents& source)
        : uriData(source) {}
    uri(const uri& other)
        : uriData(other.uriData) {}
    uri(uri&& other) noexcept
        : uriData(std::move(other.uriData)) {
        other.uriData = {};
    }

    uri& operator=(uri&& other) noexcept = default;
    uri& operator=(const uri& other) noexcept = default;

    std::string to_string() const {
        return uriData.to_string();
    }

    bool operator==(const URIComponents& b) const {
        return uriData == b;
    }
    bool operator==(const uri& b) const {
        return uriData == b.uriData;
    }

    const URIComponents& components() const { return uriData; }

    /**
     * Can be used for URI modification.
     */
    URIComponents& components() { return uriData; }
};

inline std::ostream& operator<<(std::ostream& o, const uri& data) {
    return o << data.to_string();
}
inline std::ostream& operator<<(std::ostream& o, const URIComponents& data) {
    return o << data.to_string();
}

}
