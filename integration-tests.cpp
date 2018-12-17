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
  std::clog << "=== BEGIN COLLECTORS ==="
            << std::endl;
  {
    size_t n = mkbouncer_response_get_collectors_size(response.get());
    for (size_t i = 0; i < n; ++i) {
      const char *address = "";
      const char *type = "";
      const char *front = "";
      mkbouncer_response_get_collector_at(
          response.get(), i, &type, &address, &front);
      std::clog << "- address=" << address << " type=" << type
                << " front=" << front << std::endl;
    }
  }
  std::clog << "=== END COLLECTORS ==="
            << std::endl;
  std::clog << "=== BEGIN HELPERS ==="
            << std::endl;
  {
    size_t n = mkbouncer_response_get_testhelper_keys_size(response.get());
    for (size_t i = 0; i < n; ++i) {
      const char *key = mkbouncer_response_get_testhelper_key_at(
          response.get(), i);
      size_t m = mkbouncer_response_get_testhelpers_size(response.get(), key);
      for (size_t j = 0; j < m; ++j) {
        const char *address = "";
        const char *type = "";
        const char *front = "";
        mkbouncer_response_get_testhelper_at(
            response.get(), key, j, &type, &address, &front);
        std::clog << "- key=" << key << " address=" << address << " type="
                  << type << " front=" << front << std::endl;
      }
    }
  }
  std::clog << "=== END HELPERS ==="
            << std::endl;
}
