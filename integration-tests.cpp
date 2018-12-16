#include "mkbouncer.h"

#include <iostream>

//#define MKCURL_INLINE_IMPL
//#include "mkcurl.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("We can successfully contact the bouncer") {
  mkbouncer_request_uptr request{mkbouncer_request_new_nonnull()};
  mkbouncer_response_uptr response{mkbouncer_request_perform_nonnull(
      request.get())};
  std::clog << "Good: "
            << std::boolalpha
            << (bool)mkbouncer_response_good(response.get())
            << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mkbouncer_response_moveout_logs(response)
            << "=== END LOGS ==="
            << std::endl;
}
