// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKBOUNCER_H
#define MEASUREMENT_KIT_MKBOUNCER_H

/// @file mkbouncer.h. Measurement Kit OONI bouncer library.

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// mkbouncer_request_t is a bouncer request.
typedef struct mkbouncer_request mkbouncer_request_t;

/// mkbouncer_response_t is a bouncer response.
typedef struct mkbouncer_response mkbouncer_response_t;

/// mkbouncer_request_new_nonnull creates a new bouncer request.
mkbouncer_request_t *mkbouncer_request_new_nonnull(void);

/// mkbouncer_request_set_ca_bundle_path sets the CA bundle path.
void mkbouncer_request_set_ca_bundle_path(
    mkbouncer_request_t *request, const char *ca_bundle_path);

/// mkbouncer_request_set_timeout sets the timeout.
void mkbouncer_request_set_timeout(
    mkbouncer_request_t *request, int64_t timeout);

/// mkbouncer_request_perform_nonnull performs @p request and returns
/// the corresponding response.
mkbouncer_response_t *mkbouncer_request_perform_nonnull(
    const mkbouncer_request_t *request);

/// mkbouncer_response_good returns true if the response is good
/// and false in case there was an error.
int64_t mkbouncer_response_good(const mkbouncer_response_t *response);

/// mkbouncer_response_get_binary_logs returns the possibly non UTF-8 logs
/// collected when sending the request and processing the response.
void mkbouncer_response_get_binary_logs(const mkbouncer_response_t *response,
                                        const uint8_t **data, size_t *count);

/// mkbouncer_request_delete deletes @p request.
void mkbouncer_request_delete(mkbouncer_request_t *request);

/// mkbouncer_response_delete deletes @p response.
void mkbouncer_response_delete(mkbouncer_response_t *response);

#ifdef __cplusplus
}  // extern "C"

#include <memory>
#include <string>

/// mkbouncer_request_deleter is a deleter for mkbouncer_request_t.
struct mkbouncer_request_deleter {
  void operator()(mkbouncer_request_t *s) { mkbouncer_request_delete(s); }
};

/// mkbouncer_request_uptr is a unique pointer to a mkbouncer_request_t.
using mkbouncer_request_uptr = std::unique_ptr<
    mkbouncer_request_t, mkbouncer_request_deleter>;

/// mkbouncer_response_deleter is a deleter for mkbouncer_response_t.
struct mkbouncer_response_deleter {
  void operator()(mkbouncer_response_t *s) { mkbouncer_response_delete(s); }
};

/// mkbouncer_response_uptr is a unique pointer to a mkbouncer_response_t.
using mkbouncer_response_uptr = std::unique_ptr<
    mkbouncer_response_t, mkbouncer_response_deleter>;

/// mkbouncer_response_moveout_logs moves the logs out of @p response.
std::string mkbouncer_response_moveout_logs(mkbouncer_response_uptr &response);

// MKBOUNCER_INLINE_IMPL controls whether to include the implementation inline.
#ifdef MKBOUNCER_INLINE_IMPL

#include "mkcurl.h"

#include "json.hpp"

// MKBOUNCER_ABORT allows to override abort in unit tests.
#ifndef MKBOUNCER_ABORT
#define MKBOUNCER_ABORT abort
#endif

// mkbouncer_request is the private data of mkbouncer_request_t.
struct mkbouncer_request {
  // base_url is the bouncer base URL
  std::string base_url = "https://bouncer.ooni.io";

  // ca_bundle_path is the CA bundle path.
  std::string ca_bundle_path;

  // timeout is the timeout in seconds.
  int64_t timeout = 30;
};

mkbouncer_request_t *mkbouncer_request_new_nonnull() {
  return new mkbouncer_request_t;
}

void mkbouncer_request_set_ca_bundle_path(
    mkbouncer_request_t *request, const char *ca_bundle_path) {
  if (request == nullptr || ca_bundle_path == nullptr) {
    MKBOUNCER_ABORT();
  }
  request->ca_bundle_path = ca_bundle_path;
}

void mkbouncer_request_set_timeout(
    mkbouncer_request_t *request, int64_t timeout) {
  if (request == nullptr) {
    MKBOUNCER_ABORT();
  }
  request->timeout = timeout;
}

// mkbouncer_response is the private data of mkbouncer_response_t.
struct mkbouncer_response {
  // good indicates whether we good a good response.
  int64_t good = false;

  // logs contains the possibly binary logs.
  std::string logs;
};

mkbouncer_response_t *mkbouncer_request_perform_nonnull(
    const mkbouncer_request_t *request) {
  if (request == nullptr) {
    MKBOUNCER_ABORT();
  }
  mkbouncer_response_uptr response{new mkbouncer_response_t};
  mkcurl_request_uptr curl_request{mkcurl_request_new_nonnull()};
  mkcurl_request_set_ca_bundle_path_v2(
      curl_request.get(), request->ca_bundle_path.c_str());
  mkcurl_request_set_timeout_v2(curl_request.get(), request->timeout);
  mkcurl_request_set_method_post_v2(curl_request.get());
  {
    std::string url = request->base_url;
    url += "/bouncer/net-tests";
    mkcurl_request_set_url_v2(curl_request.get(), url.c_str());
  }
  {
    // Apparently the bouncer does not care about the test that we're running
    // as this is a relic of the input-hashes era. Hence, don't create extra
    // complexity by asking the caller to fill in the test name. Likewise don't
    // bother with asking for the right test helper, just ask all of them, and
    // let the caller decide what test helper they're interested to use.
    nlohmann::json doc;
    nlohmann::json nettest;
    nettest["input-hashes"] = nullptr;
    nettest["name"] = "web_connectivity";
    // These are all the test helpers that were ever used by MK.
    nettest["test-helpers"].push_back("web-connectivity");
    nettest["test-helpers"].push_back("http-return-json-headers");
    nettest["test-helpers"].push_back("tcp-echo");
    nettest["version"] = "0.0.1";
    doc["net-tests"].push_back(std::move(nettest));
    std::string body;
    try {
      body = doc.dump();
    } catch (const std::exception &exc) {
      response->logs += exc.what();
      response->logs += "\n";
      return response.release();
    }
    response->logs += "Request body: ";
    response->logs += body;
    response->logs += "\n";
    mkcurl_request_movein_body_v2(curl_request, std::move(body));
  }
  mkcurl_response_uptr curl_response{
      mkcurl_request_perform_nonnull(curl_request.get())};
  response->logs += mkcurl_response_moveout_logs_v2(curl_response);
  if (mkcurl_response_get_error_v2(curl_response.get()) != 0) {
    return response.release();
  }
  if (mkcurl_response_get_status_code_v2(curl_response.get()) != 200) {
    return response.release();
  }
  {
    std::string body = mkcurl_response_moveout_body_v2(curl_response);
    response->logs += "Response body: ";
    response->logs += body;
    response->logs += "\n";
    try {
      nlohmann::json doc = nlohmann::json::parse(body);
      // TODO(bassosimone): parse the body. This is a body sample:
      //
      // {"net-tests": [{"collector-alternate": [{"type": "https", "address": "https://c.collector.ooni.io:443"}, {"front": "a0.awsstatic.com", "type": "cloudfront", "address": "https://das0y2z2ribx3.cloudfront.net"}], "version": "0.0.1", "name": "web_connectivity", "test-helpers": {"web-connectivity": "httpo://y3zq5fwelrzkkv3s.onion"}, "test-helpers-alternate": {"web-connectivity": [{"type": "https", "address": "https://c.web-connectivity.th.ooni.io:443"}, {"front": "a0.awsstatic.com", "type": "cloudfront", "address": "https://d2vt18apel48hw.cloudfront.net"}]}, "collector": "httpo://42q7ug46dspcsvkw.onion", "input-hashes": null}]}
      //
    } catch (const std::exception &exc) {
      response->logs += exc.what();
      response->logs += "\n";
      return response.release();
    }
  }
  response->good = true;
  return response.release();
}

int64_t mkbouncer_response_good(const mkbouncer_response_t *response) {
  if (response == nullptr) {
    MKBOUNCER_ABORT();
  }
  return response->good;
}

void mkbouncer_response_get_binary_logs(const mkbouncer_response_t *response,
                                        const uint8_t **data, size_t *count) {
  if (response == nullptr || data == nullptr || count == nullptr) {
    MKBOUNCER_ABORT();
  }
  *data = (const uint8_t *)response->logs.c_str();
  *count = response->logs.size();
}

void mkbouncer_request_delete(mkbouncer_request_t *request) {
  delete request;
}

void mkbouncer_response_delete(mkbouncer_response_t *response) {
  delete response;
}

std::string mkbouncer_response_moveout_logs(mkbouncer_response_uptr &response) {
  if (response == nullptr) {
    MKBOUNCER_ABORT();
  }
  return std::move(response->logs);
}

#endif  // MKBOUNCER_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKBOUNCER_H
