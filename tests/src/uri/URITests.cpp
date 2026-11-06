#include <catch2/catch_test_macros.hpp>

#include <stc/uri.hpp>

namespace {

/**
 * Utility for making it more obvious what failing assertions are  
 */
void requireEqualUrl(
    const std::string& raw,
    const stc::URIComponents& expected,
    const std::optional<std::string>& expectedStringTranslation = std::nullopt
) {
    stc::uri actual(raw);
    auto actualComponents = actual.components();

    INFO(
        "Actual URL (serialized): "
        << actualComponents.to_string()
        << ", expected " << expected.to_string()
    );
    INFO(
        "Scheme: " << actualComponents.scheme.value_or("<undefined>") << ", expected " << expected.scheme.value_or("<undefined>")
    );
    REQUIRE(actualComponents.scheme == expected.scheme);
    REQUIRE(actualComponents.userinfo == expected.userinfo);
    INFO(
        "Port: " << actualComponents.port << ", expected " << expected.port
    );
    REQUIRE(actualComponents.port == expected.port);
    INFO(
        "Host: " << actualComponents.host.value_or("<undefined>") << ", expected " << expected.host.value_or("<undefined>")
    );
    REQUIRE(actualComponents.host == expected.host);
    INFO(
        "Path: " << actualComponents.path.value_or("<undefined>") << ", expected " << expected.path.value_or("<undefined>")
    );
    REQUIRE(actualComponents.path == expected.path);
    INFO(
        "Query: " << actualComponents.query.value_or("<undefined>") << ", expected " << expected.query.value_or("<undefined>")
    );
    REQUIRE(actualComponents.query == expected.query);
    INFO(
        "Fragment: " << actualComponents.fragment.value_or("<undefined>") << ", expected " << expected.fragment.value_or("<undefined>")
    );
    REQUIRE(actualComponents.fragment == expected.fragment);

    REQUIRE(actual == expected);

    REQUIRE(
        actual.to_string() == expected.to_string()
    );
    REQUIRE(
        actual.to_string() == expectedStringTranslation.value_or(raw)
    );
}

TEST_CASE("URI with No port, no query, no fragment", "[uri]") {
    requireEqualUrl(
        "https://example.com",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
        }
    );
    requireEqualUrl(
        "https://example.com/",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .path = "/",
        }
    );
    requireEqualUrl(
        "https://example.com/path",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .path = "/path",
        }
    );
}
TEST_CASE("URI with No port, query, no fragment", "[uri]") {
    requireEqualUrl(
        "https://example.com?a=b",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .query = "a=b",
        }
    );
    requireEqualUrl(
        "https://example.com/?a=b",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .path = "/",
            .query = "a=b",
        }
    );
    requireEqualUrl(
        "https://example.com/path?a=b",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .path = "/path",
            .query = "a=b",
        }
    );
}
TEST_CASE("URI with No port, no query, fragment", "[uri]") {
    requireEqualUrl(
        "https://example.com#fragment",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .fragment = "fragment",
        }
    );
    requireEqualUrl(
        "https://example.com/#fragment",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .path = "/",
            .fragment = "fragment",
        }
    );
    requireEqualUrl(
        "https://example.com/path#fragment",
        stc::URIComponents {
            .scheme = "https",
            .host = "example.com",
            .path = "/path",
            .fragment = "fragment",
        }
    );
}

TEST_CASE("URI with Port, no query, no fragment", "[uri]") {
    SECTION("Defined port") {
        requireEqualUrl(
            "https://example.com:443",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
            }
        );
        requireEqualUrl(
            "https://example.com:443/",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .path = "/",
            }
        );
        requireEqualUrl(
            "https://example.com:443/path",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .path = "/path",
            }
        );

    }
    SECTION("Empty port") {
        requireEqualUrl(
            "https://example.com:",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
            },
            "https://example.com"
        );
        requireEqualUrl(
            "https://example.com:/",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .path = "/",
            },
            "https://example.com/"
        );
        requireEqualUrl(
            "https://example.com:/path",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .path = "/path",
            },
            "https://example.com/path"
        );
    }
}
TEST_CASE("URI with Port, query, no fragment", "[uri]") {
    SECTION("Defined port") {
        requireEqualUrl(
            "https://example.com:443?v=69",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .query = "v=69",
            }
        );
        requireEqualUrl(
            "https://example.com:443/?v=69",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .path = "/",
                .query = "v=69",
            }
        );
        requireEqualUrl(
            "https://example.com:443/path?v=69",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .path = "/path",
                .query = "v=69",
            }
        );

    }
    SECTION("Empty port") {
        requireEqualUrl(
            "https://example.com:?v=69",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .query = "v=69",
            },
            "https://example.com?v=69"
        );
        requireEqualUrl(
            "https://example.com:/?v=69",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .path = "/",
                .query = "v=69",
            },
            "https://example.com/?v=69"
        );
        requireEqualUrl(
            "https://example.com:/path?v=69",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .path = "/path",
                .query = "v=69",
            },
            "https://example.com/path?v=69"
        );
    }
}
TEST_CASE("URI with Port, query, fragment", "[uri]") {
    SECTION("Defined port") {
        requireEqualUrl(
            "https://example.com:443?v=69#fragment",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .query = "v=69",
                .fragment = "fragment",
            }
        );
        requireEqualUrl(
            "https://example.com:443/?v=69#fragment",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .path = "/",
                .query = "v=69",
                .fragment = "fragment",
            }
        );
        requireEqualUrl(
            "https://example.com:443/path?v=69#fragment",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .port = 443,
                .path = "/path",
                .query = "v=69",
                .fragment = "fragment",
            }
        );

    }
    SECTION("Empty port") {
        requireEqualUrl(
            "https://example.com:?v=69#fragment",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .query = "v=69",
                .fragment = "fragment",
            },
            "https://example.com?v=69#fragment"
        );
        requireEqualUrl(
            "https://example.com:/?v=69#fragment",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .path = "/",
                .query = "v=69",
                .fragment = "fragment",
            },
            "https://example.com/?v=69#fragment"
        );
        requireEqualUrl(
            "https://example.com:/path?v=69#fragment",
            stc::URIComponents {
                .scheme = "https",
                .host = "example.com",
                .path = "/path",
                .query = "v=69",
                .fragment = "fragment",
            },
            "https://example.com/path?v=69#fragment"
        );
    }
}

TEST_CASE("URI with Userinfo", "[uri]") {
    requireEqualUrl(
        "https://user:1234@example.com:/path?v=69#fragment",
        stc::URIComponents {
            .scheme = "https",
            .userinfo = std::vector<std::string>{"user", "1234"},
            .host = "example.com",
            .path = "/path",
            .query = "v=69",
            .fragment = "fragment",
        },
        "https://user:1234@example.com/path?v=69#fragment"
    );
    requireEqualUrl(
        "https://user:1234@example.com/path?v=69#fragment",
        stc::URIComponents {
            .scheme = "https",
            .userinfo = std::vector<std::string>{"user", "1234"},
            .host = "example.com",
            .path = "/path",
            .query = "v=69",
            .fragment = "fragment",
        }
    );
    requireEqualUrl(
        "https://user:1234@example.com/path?v=69#fra:gment",
        stc::URIComponents {
            .scheme = "https",
            .userinfo = std::vector<std::string>{"user", "1234"},
            .host = "example.com",
            .path = "/path",
            .query = "v=69",
            .fragment = "fra:gment",
        }
    );
    requireEqualUrl(
        "https://user:1234:5678:@example.com:255/path?v=69#fra:gment",
        stc::URIComponents {
            .scheme = "https",
            .userinfo = std::vector<std::string>{"user", "1234", "5678", ""},
            .host = "example.com",
            .port = 255,
            .path = "/path",
            .query = "v=69",
            .fragment = "fra:gment",
        }
    );
    requireEqualUrl(
        "https://:::@example.com:255/path?v=69#fra:gment",
        stc::URIComponents {
            .scheme = "https",
            .userinfo = std::vector<std::string>{"", "", "", ""},
            .host = "example.com",
            .port = 255,
            .path = "/path",
            .query = "v=69",
            .fragment = "fra:gment",
        }
    );
}

TEST_CASE("URI with Examples from RFC 3986", "[uri]") {
    requireEqualUrl(
        "ftp://ftp.is.co.za/rfc/rfc1808.txt",
        {
            .scheme = "ftp",
            .host = "ftp.is.co.za",
            .path = "/rfc/rfc1808.txt",
        }
    );
    requireEqualUrl(
        "http://www.ietf.org/rfc/rfc2396.txt",
        {
            .scheme = "http",
            .host = "www.ietf.org",
            .path = "/rfc/rfc2396.txt",
        }
    );
    requireEqualUrl(
        "ldap://[2001:db8::7]/c=GB?objectClass?one",
        {
            .scheme = "ldap",
            .host = "[2001:db8::7]",
            .path = "/c=GB",
            .query = "objectClass?one",
        }
    );
    requireEqualUrl(
        "mailto:John.Doe@example.com",
        {
            .scheme = "mailto",
            // Per p. 22 of RFC 3986, this is a path, for whatever reason (I mean, good, no special case where the
            // authority is null, but like, really? example.com is the host in this case, you'd _want_ to know it
            // and the user info separately, right? So an email system has to parse out that info from the _path_ to
            // tell where to send the email?? But whatever)
            .path = "John.Doe@example.com",
        }
    );
    requireEqualUrl(
        "news:comp.infosystems.www.servers.unix",
        {
            .scheme = "news",
            .path = "comp.infosystems.www.servers.unix",
        }
    );
    requireEqualUrl(
        "tel:+1-816-555-1212",
        {
            .scheme = "tel",
            .path = "+1-816-555-1212",
        }
    );
    requireEqualUrl(
        "telnet://192.0.2.16:80/",
        {
            .scheme = "telnet",
            .host = "192.0.2.16",
            .port = 80,
            .path = "/"
        }
    );

    requireEqualUrl(
        "urn:oasis:names:specification:docbook:dtd:xml:4.1.2",
        {
            .scheme = "urn",
            .path = "oasis:names:specification:docbook:dtd:xml:4.1.2"
        }
    );
}

TEST_CASE("Gemini URLs", "[uri]") {
    // gemini protocol, not to be confused with google's AI slop machine that's contributing to fucking over the
    // environment: https://en.wikipedia.org/wiki/Gemini_(protocol)
    requireEqualUrl(
        "gemini://example.com/",
        {
            .scheme = "gemini",
            .host = "example.com",
            .path = "/",
        }
    );
    requireEqualUrl(
        "gemini://link.to/another/resource",
        {
            .scheme = "gemini",
            .host = "link.to",
            .path = "/another/resource"
        }
    );
}

TEST_CASE("Scheme-less URIs", "[uri]") {
    requireEqualUrl(
        "//example.com/",
        {
            .host = "example.com",
            .path = "/",
        }
    );
    requireEqualUrl(
        "//user:data@example.com/",
        {
            .userinfo = std::vector<std::string> { "user", "data" },
            .host = "example.com",
            .path = "/",
        }
    );
    requireEqualUrl(
        "//user:data@example.com/",
        {
            .userinfo = std::vector<std::string> { "user", "data" },
            .host = "example.com",
            .path = "/",
        }
    );
    requireEqualUrl(
        "//example.com:/",
        {
            .host = "example.com",
            .path = "/",
        },
        "//example.com/"
    );
    requireEqualUrl(
        "//example.com:621/",
        {
            .host = "example.com",
            .port = 621,
            .path = "/",
        }
    );
}

TEST_CASE("Port ranges", "[uri]") {
    REQUIRE_THROWS_AS(
        stc::uri("//example.com:100000"),
        std::range_error
    );
    REQUIRE_THROWS_AS(
        stc::uri("//example.com:65536"),
        std::range_error
    );
    REQUIRE_THROWS_AS(
        stc::uri("//example.com:-55"),
        std::range_error
    );
    REQUIRE_THROWS_AS(
        stc::uri("//example.com:potato"),
        std::invalid_argument
    );
    requireEqualUrl(
        "//example.com:65535",
        {
            .host = "example.com",
            .port = 65535
        }
    );
}

TEST_CASE("Fragment before query", "[uri]") {
    requireEqualUrl(
        "https://example.com/index.html#fragment?v=69",
        {
            .scheme = "https",
            .host = "example.com",
            .path = "/index.html",
            .fragment = "fragment?v=69"
        }
    );
}

TEST_CASE("Path only", "[uri]") {
    requireEqualUrl(
        "/test",
        {
            .path = "/test"
        }
    );
    requireEqualUrl(
        "/",
        {
            .path = "/"
        }
    );
    requireEqualUrl(
        "",
        {
        }
    );
    requireEqualUrl(
        "/test?v=69",
        {
            .path = "/test",
            .query = "v=69"
        }
    );
    requireEqualUrl(
        "/path#fragment:",
        {
            .path = "/path",
            .fragment = "fragment:"
        }
    );
    requireEqualUrl(
        "path.md#fragment:",
        {
            .path = "path.md",
            .fragment = "fragment:"
        }
    );
    requireEqualUrl(
        "path.md?fragment:",
        {
            .path = "path.md",
            .query = "fragment:"
        }
    );
}

TEST_CASE("URI assignment", "[uri]") {
    stc::uri base = "https://example.com:621";
    stc::uri alt = "https://sub.example.com:621/path";
    stc::URIComponents expected = {
        .scheme = "https",
        .host = "example.com",
        .port = 621,
    };
    stc::URIComponents altExpected = {
        .scheme = "https",
        .host = "sub.example.com",
        .port = 621,
        .path = "/path",
    };

    REQUIRE(base == expected);
    REQUIRE(alt == altExpected);

    stc::uri baseCopy = base;
    REQUIRE(base == baseCopy);
    REQUIRE(base == expected);

    stc::uri altCopy = alt;
    REQUIRE(altCopy == alt);
    altCopy = std::move(baseCopy);
    REQUIRE(altCopy == expected);
    REQUIRE(
        (baseCopy != expected && baseCopy != altExpected)
    );

    baseCopy = base;
    REQUIRE(base == baseCopy);
    REQUIRE(base == expected);

}

TEST_CASE("URI modification", "[uri]") {
    stc::uri uri("https://example.com");

    uri.components().host = "lunarwatcher.codeberg.page";
    uri.components().path = "/violet";

    REQUIRE(uri.to_string() == "https://lunarwatcher.codeberg.page/violet");

    uri.components().host = "";
    REQUIRE(uri.to_string() == "https:///violet");

    uri.components().scheme = "file";
    REQUIRE(uri.to_string() == "file:///violet");
}

}
