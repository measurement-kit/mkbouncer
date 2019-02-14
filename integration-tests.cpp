#include "mkbouncer.hpp"

#include <iostream>

#define MKCURL_INLINE_IMPL
#include "mkcurl.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("We can successfully contact the bouncer") {
  mk::bouncer::Request request;
  request.ca_bundle_path = ".mkbuild/data/cacert.pem";
  request.name = "web_connectivity";
  request.version = "0.0.1";
  request.timeout = 7;
  request.helpers.push_back(MKBOUNCER_HELPER_TCP_ECHO);
  request.helpers.push_back(MKBOUNCER_HELPER_HTTP_RETURN_JSON_HEADERS);
  request.helpers.push_back(MKBOUNCER_HELPER_WEB_CONNECTIVITY);
  mk::bouncer::Response response = mk::bouncer::perform(request);
  std::clog << "Good: "
            << std::boolalpha
            << response.good
            << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl;
  for (auto &s : response.logs) std::clog << s << std::endl;
  std::clog << "=== END LOGS ==="
            << std::endl;
  std::clog << "=== BEGIN COLLECTORS ==="
            << std::endl;
  {
    for (auto &r : response.collectors) {
      std::clog << "- address=" << r.address << " type=" << r.type
                << " front=" << r.front << std::endl;
    }
  }
  std::clog << "=== END COLLECTORS ==="
            << std::endl;
  std::clog << "=== BEGIN HELPERS ==="
            << std::endl;
  {
    for (auto &p : response.helpers) {
      auto &k = p.first;
      for (auto &r : p.second) {
        std::clog << "- key=" << k << " address=" << r.address << " type="
                  << r.type << " front=" << r.front << std::endl;
      }
    }
  }
  std::clog << "=== END HELPERS ==="
            << std::endl;
  REQUIRE(response.good == true);
}
