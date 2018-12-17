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

/// mkbouncer_request_set_nettest_name sets the nettest name.
void mkbouncer_request_set_nettest_name(
    mkbouncer_request_t *request, const char *name);

/// mkbouncer_request_set_nettest_version sets the nettest version.
void mkbouncer_request_set_nettest_version(
    mkbouncer_request_t *request, const char *version);

/// mkbouncer_helper_web_connectivity returns the web connectivity helper name.
const char *mkbouncer_helper_web_connectivity(void);

/// mkbouncer_helper_tcp_echo returns the TCP echo helper name.
const char *mkbouncer_helper_tcp_echo(void);

/// mkbouncer_helper_http_return_json_headers returns the HTTP return
/// JSON headers helper name.
const char *mkbouncer_helper_http_return_json_headers(void);

/// mkbouncer_request_add_helper adds a helper to the request.
void mkbouncer_request_add_helper(
    mkbouncer_request_t *request, const char *helper);

/// mkbouncer_request_perform_nonnull performs @p request and returns
/// the corresponding response.
mkbouncer_response_t *mkbouncer_request_perform_nonnull(
    const mkbouncer_request_t *request);

/// mkbouncer_response_good returns true if the response is good
/// and false in case there was an error.
int64_t mkbouncer_response_good(const mkbouncer_response_t *response);

/// mkbouncer_response_get_collectors_size returns the number
/// of collectors contained in @p response.
size_t mkbouncer_response_get_collectors_size(
    const mkbouncer_response_t *response);

/// mkbouncer_response_get_collector_at returns the collector with the given
/// @p index. The strings returned in @p type, @p address and @p front are
/// always valid. They're set to the empty string when no specific value is
/// actually available. This function aborts if passed any null pointer
/// as well as if the @p index argument is out of bounds.
void mkbouncer_response_get_collector_at(
    const mkbouncer_response_t *response, size_t index, const char **type,
    const char **address, const char **front);

/// mkbouncer_response_get_testhelper_keys_size returns the number of
/// test helper keys contained in @p response.
size_t mkbouncer_response_get_testhelper_keys_size(
    const mkbouncer_response_t *response);

/// mkbouncer_response_get_testhelper_key_at returns the test helper key
/// in @p response at index @p index. This functiona aborts if @p response
/// is null as well as if @p index is out of bounds.
const char *mkbouncer_response_get_testhelper_key_at(
    const mkbouncer_response_t *response, size_t index);

/// mkbouncer_response_get_testhelpers_size returns the number of records
/// for the test helper identified by @p key.
size_t mkbouncer_response_get_testhelpers_size(
    const mkbouncer_response_t *response, const char *key);

/// mkbouncer_response_get_testhelper_at returns the testhelper record
/// for the test helper identified by @p key and at index @p index. This
/// function aborts if passed null pointers and if @p index is out of bounds
/// or if the @p key does not exist.
void mkbouncer_response_get_testhelper_at(
    const mkbouncer_response_t *response, const char *key, size_t index,
    const char **type, const char **address, const char **front);

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

  // helpers contains the list of test helpers to request.
  std::vector<std::string> helpers;

  // name is the nettest name.
  std::string name;

  // timeout is the timeout in seconds.
  int64_t timeout = 30;

  // version is the nettest version.
  std::string version;
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

void mkbouncer_request_set_nettest_name(
    mkbouncer_request_t *request, const char *name) {
  if (request == nullptr || name == nullptr) {
    MKBOUNCER_ABORT();
  }
  request->name = name;
}

void mkbouncer_request_set_nettest_version(
    mkbouncer_request_t *request, const char *version) {
  if (request == nullptr || version == nullptr) {
    MKBOUNCER_ABORT();
  }
  request->version = version;
}

const char *mkbouncer_helper_web_connectivity() {
  return "web-connectivity";
}

const char *mkbouncer_helper_tcp_echo() {
  return "tcp-echo";
}

const char *mkbouncer_helper_http_return_json_headers() {
  return "http-return-json-headers";
}

void mkbouncer_request_add_helper(
    mkbouncer_request_t *request, const char *helper) {
  if (request == nullptr || helper == nullptr) {
    MKBOUNCER_ABORT();
  }
  request->helpers.push_back(helper);
}

// mkbouncer_record is a collector or test-helper record.
struct mkbouncer_record {
  // type is the record type.
  std::string type;

  // address is the record address.
  std::string address;

  // front is the front to use in case of domain fronting.
  std::string front;
};

// mkbouncer_response is the private data of mkbouncer_response_t.
struct mkbouncer_response {
  // good indicates whether we good a good response.
  int64_t good = false;

  // collectors lists all available collectors.
  std::vector<mkbouncer_record> collectors;

  // helpers lists all available test-helpers.
  std::map<std::string, std::vector<mkbouncer_record>> helpers;

  // keys contains the keys in helpers.
  std::vector<std::string> keys;

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
    nlohmann::json doc;
    nlohmann::json nettest;
    nettest["input-hashes"] = nullptr;
    nettest["name"] = request->name;
    nettest["test-helpers"] = request->helpers;
    nettest["version"] = request->version;
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
      const nlohmann::json &net_tests = doc.at("net-tests")[0];
      if (net_tests.count("collector") > 0) {
        mkbouncer_record record;
        record.address = net_tests.at("collector");
        record.type = "onion";
        response->collectors.push_back(std::move(record));
      }
      if (net_tests.count("collector-alternate") > 0) {
        for (const nlohmann::json &e : net_tests.at("collector-alternate")) {
          mkbouncer_record record;
          record.address = e.at("address");
          record.type = e.at("type");
          if (e.count("front") > 0) record.front = e.at("front");
          response->collectors.push_back(std::move(record));
        }
      }
      if (net_tests.count("test-helpers") > 0) {
        for (auto &e : net_tests.at("test-helpers").items()) {
          mkbouncer_record record;
          record.address = e.value();
          response->helpers[e.key()].push_back(std::move(record));
        }
      }
      if (net_tests.count("test-helpers-alternate") > 0) {
        for (auto &e : net_tests.at("test-helpers-alternate").items()) {
          for (auto &v : e.value()) {
            mkbouncer_record record;
            record.address = v.at("address");
            record.type = v.at("type");
            if (v.count("front") > 0) {
              record.front = v.at("front");
            }
            response->helpers[e.key()].push_back(std::move(record));
          }
        }
      }
    } catch (const std::exception &exc) {
      response->logs += exc.what();
      response->logs += "\n";
      return response.release();
    }
  }
  for (auto &pair : response->helpers) response->keys.push_back(pair.first);
  response->good = true;
  return response.release();
}

int64_t mkbouncer_response_good(const mkbouncer_response_t *response) {
  if (response == nullptr) {
    MKBOUNCER_ABORT();
  }
  return response->good;
}

size_t mkbouncer_response_get_collectors_size(
    const mkbouncer_response_t *response) {
  if (response == nullptr) {
    MKBOUNCER_ABORT();
  }
  return response->collectors.size();
}

void mkbouncer_response_get_collector_at(
    const mkbouncer_response_t *response, size_t index, const char **type,
    const char **address, const char **front) {
  if (response == nullptr || type == nullptr || address == nullptr ||
      front == nullptr) {
    MKBOUNCER_ABORT();
  }
  if (index >= response->collectors.size()) {
    MKBOUNCER_ABORT();
  }
  const mkbouncer_record &record = response->collectors[index];
  *type = record.type.c_str();
  *address = record.address.c_str();
  *front = record.front.c_str();
}

size_t mkbouncer_response_get_testhelper_keys_size(
    const mkbouncer_response_t *response) {
  if (response == nullptr) {
    MKBOUNCER_ABORT();
  }
  return response->keys.size();
}

const char *mkbouncer_response_get_testhelper_key_at(
    const mkbouncer_response_t *response, size_t index) {
  if (response == nullptr || index >= response->keys.size()) {
    MKBOUNCER_ABORT();
  }
  return response->keys[index].c_str();
}

size_t mkbouncer_response_get_testhelpers_size(
    const mkbouncer_response_t *response, const char *key) {
  if (response == nullptr || key == nullptr) {
    MKBOUNCER_ABORT();
  }
  if (response->helpers.count(key) <= 0) {
    return 0;
  }
  return response->helpers.at(key).size();
}

void mkbouncer_response_get_testhelper_at(
    const mkbouncer_response_t *response, const char *key, size_t index,
    const char **type, const char **address, const char **front) {
  if (response == nullptr || key == nullptr || type == nullptr ||
      address == nullptr || front == nullptr) {
    MKBOUNCER_ABORT();
  }
  if (response->helpers.count(key) <= 0 ||
      index >= response->helpers.at(key).size()) {
    MKBOUNCER_ABORT();
  }
  const mkbouncer_record &record = response->helpers.at(key)[index];
  *type = record.type.c_str();
  *address = record.address.c_str();
  *front = record.front.c_str();
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
