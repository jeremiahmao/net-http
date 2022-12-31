/*add copyright information?*/

// Work In Progress: I do not fully understand what this file does yet. Performs as a wrapper?

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_H_

#include <memory>

#include "absl/memory/memory.h"

#include "net_http/client/internal/evhttp_client.h"
#include "net_http/client/public/httpclient_interface.h"


namespace net_http {

// Creates a client implemented based on the libevents library.
// Returns nullptr if there is any error

//TODO: needs to be completely redone after EvHTTPConnection is actually implemented
inline std::unique_ptr<HTTPClientInterface> CreateEvHTTPClient(
    std::unique_ptr<ClientOptions> options) {
  auto client = absl::make_unique<EvHTTPClient>(std::move(options));
  return std::move(client);
}


}  // namespace net_http


#endif  // NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_H_
