/*add copyright information?*/

// APIs for the HTTP client.

//Client should own requests and send them at the user's command.

//client is request builder, but owns the request. the request can be accessed by user
// to change things, but ownership remains with the client.

//responses are also recieved to corresponding requests by the client, and accessed through the client.

#ifndef NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_
#define NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_

#include <cassert>

#include <functional>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/time/time.h"

#include "net_http/client/public/client_request_interface.h"

//Everything in this API is experimental and subject to change.

namespace net_http {

//may want to add more to ClientOptions than just the executor  
class ClientOptions{
 public: 
  class EventExecutor {
   public:
    virtual ~EventExecutor() = default;

    EventExecutor(const EventExecutor& other) = delete;
    EventExecutor& operator=(const EventExecutor& other) = delete;

    // Schedule the specified 'fn' for execution in this executor.
    // Must be non-blocking
    virtual void Schedule(std::function<void()> fn) = 0;

   protected:
    EventExecutor() = default;
  };

  void SetExecutor(std::unique_ptr<ClientOptions::EventExecutor> executor) {
    executor_ = std::move(executor);
  }
  ClientOptions::EventExecutor* executor() const { return executor_.get(); }

 private:
  std::unique_ptr<ClientOptions::EventExecutor> executor_;
};

class HTTPClientInterface {
 public:
  virtual ~HTTPClientInterface() = default;

  HTTPClientInterface(const HTTPClientInterface& other) = delete;
  HTTPClientInterface& operator=(const HTTPClientInterface& other) = delete;

  virtual void Connect(absl::string_view host, int port, int timeout_in_secs) = 0;

  //NOTE: requests are owned by the client
  virtual std::unique_ptr<ClientRequestInterface> MakeRequest(absl::string_view uri_path, 
                                              absl::string_view method,
                                              ClientHandlerInterface* handler,
                                              ClientHandlerOptions* handler_options) = 0;

  // Closes connection
  virtual void CloseConnection() = 0;

 protected:
  HTTPClientInterface() = default;

};

}  // namespace net_http


#endif  // NET_HTTP_CLIENT_PUBLIC_HTTPCLIENT_INTERFACE_H_
