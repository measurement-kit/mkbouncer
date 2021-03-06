#include "mkmock.hpp"

MKMOCK_DEFINE_HOOK(curl_response_error, int64_t);
MKMOCK_DEFINE_HOOK(curl_response_status_code, int64_t);
MKMOCK_DEFINE_HOOK(curl_response_body, std::string);

#define MKCURL_INLINE_IMPL
#include "mkcurl.hpp"

#define MKBOUNCER_MOCK
#define MKBOUNCER_INLINE_IMPL
#include "mkbouncer.hpp"

#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// clang-format off
const uint8_t binary_input[] = {
  0x57, 0xe5, 0x79, 0xfb, 0xa6, 0xbb, 0x0d, 0xbc, 0xce, 0xbd, 0xa7, 0xa0,
  0xba, 0xa4, 0x78, 0x78, 0x12, 0x59, 0xee, 0x68, 0x39, 0xa4, 0x07, 0x98,
  0xc5, 0x3e, 0xbc, 0x55, 0xcb, 0xfe, 0x34, 0x3c, 0x7e, 0x1b, 0x5a, 0xb3,
  0x22, 0x9d, 0xc1, 0x2d, 0x6e, 0xca, 0x5b, 0xf1, 0x10, 0x25, 0x47, 0x1e,
  0x44, 0xe2, 0x2d, 0x60, 0x08, 0xea, 0xb0, 0x0a, 0xcc, 0x05, 0x48, 0xa0,
  0xf5, 0x78, 0x38, 0xf0, 0xdb, 0x3f, 0x9d, 0x9f, 0x25, 0x6f, 0x89, 0x00,
  0x96, 0x93, 0xaf, 0x43, 0xac, 0x4d, 0xc9, 0xac, 0x13, 0xdb, 0x22, 0xbe,
  0x7a, 0x7d, 0xd9, 0x24, 0xa2, 0x52, 0x69, 0xd8, 0x89, 0xc1, 0xd1, 0x57,
  0xaa, 0x04, 0x2b, 0xa2, 0xd8, 0xb1, 0x19, 0xf6, 0xd5, 0x11, 0x39, 0xbb,
  0x80, 0xcf, 0x86, 0xf9, 0x5f, 0x9d, 0x8c, 0xab, 0xf5, 0xc5, 0x74, 0x24,
  0x3a, 0xa2, 0xd4, 0x40, 0x4e, 0xd7, 0x10, 0x1f
};
// clang-format on

TEST_CASE("curl_reason_for_failure works") {
  SECTION("With a real curl error") {
    mk::curl::Response response;
    response.error = CURLE_AGAIN;
    std::string v = mk::bouncer::curl_reason_for_failure(response);
    REQUIRE(v == "bouncer: Socket not ready for send/recv");
  }
  SECTION("With an HTTP error") {
    mk::curl::Response response;
    response.status_code = 404;
    std::string v = mk::bouncer::curl_reason_for_failure(response);
    REQUIRE(v == "bouncer: HTTP response code said error");
  }
  SECTION("With an unexpected error") {
    mk::curl::Response response;
    response.status_code = 200;
    std::string v = mk::bouncer::curl_reason_for_failure(response);
    REQUIRE(v == "bouncer: unknown libcurl error");
  }
}

TEST_CASE("We deal with perform errors") {
  SECTION("On failure to serialize the request body") {
    mk::bouncer::Request request;
    request.name = std::string{(const char *)binary_input,
                               sizeof(binary_input)};
    auto response = mk::bouncer::perform(request);
    REQUIRE(!response.good);
  }

  SECTION("On network error") {
    MKMOCK_WITH_ENABLED_HOOK(curl_response_error, CURL_LAST, {
      mk::bouncer::Request request;
      auto response = mk::bouncer::perform(request);
      REQUIRE(!response.good);
    });
  }

  SECTION("On HTTP error") {
    MKMOCK_WITH_ENABLED_HOOK(curl_response_error, 0, {
      MKMOCK_WITH_ENABLED_HOOK(curl_response_status_code, 500, {
        mk::bouncer::Request request;
        auto response = mk::bouncer::perform(request);
        REQUIRE(!response.good);
      });
    });
  }

  SECTION("On invalid JSON body") {
    MKMOCK_WITH_ENABLED_HOOK(curl_response_error, 0, {
      MKMOCK_WITH_ENABLED_HOOK(curl_response_status_code, 200, {
        MKMOCK_WITH_ENABLED_HOOK(curl_response_body, "{", {
          mk::bouncer::Request request;
          auto response = mk::bouncer::perform(request);
          REQUIRE(!response.good);
        });
      });
    });
  }
}
