//copy right information needs to be added

#ifndef NET_HTTP_CLIENT_INTERNAL_EVHTTP_CLIENT_H_
#define NET_HTTP_CLIENT_INTERNAL_EVHTTP_CLIENT_H_

#include "net_http/client/public/httpclient_interface.h"
#include "net_http/client/public/client_request_interface.h"

struct event_base;
struct evhttp_connection;

namespace net_http {

class EvHTTPClient final: public HTTPClientInterface {
  public:
    virtual ~EvHTTPClient();
    
    EvHTTPClient(const EvHTTPClient& other) = delete;
    EvHTTPClient& operator=(const EvHTTPClient& other) = delete;
    
    explicit EvHTTPClient(std::unique_ptr<ClientOptions> options);

    void Connect(absl::string_view host, int port, int timeout_in_secs = 5) override;
    
    //must be called after Connect as of right now
    std::unique_ptr<ClientRequestInterface> MakeRequest(absl::string_view uri_path, 
                                              absl::string_view method,
                                              ClientHandlerInterface* handler,
                                              ClientHandlerOptions* handler_options) override;

    void CloseConnection();
    
 private:

  std::unique_ptr<ClientOptions> client_options_;

  // ev instances
  event_base* ev_base_ = nullptr;
  evhttp_connection* ev_con_ = nullptr;
  
};

}  // namespace net_http


#endif //NET_HTTP_CLIENT_INTERNAL_EVHTTP_CLIENT_H_