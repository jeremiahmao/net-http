#include "evhttp_client.h"

//Need to add copyright info

#include "evhttp_client_request.h"

#include "libevent/include/event2/http.h"
#include "libevent/include/event2/event.h"

namespace net_http {

EvHTTPClient::EvHTTPClient(std::unique_ptr<ClientOptions> options)
    : client_options_(std::move(options)) {}

EvHTTPClient::~EvHTTPClient() {
    if (ev_con_ != nullptr) {
      evhttp_connection_free(ev_con_);
    }

    event_base_free(ev_base_);
}

void EvHTTPClient::Connect(absl::string_view host, int port, int timeout_in_secs){
    
    this->ev_base_ = event_base_new();
    if (this->ev_base_ == nullptr) {
      NET_LOG(ERROR, "Failed to connect : event_base_new()");
      return;
    }

    // blocking call (DNS resolution)
    std::string host_str(host.data(), host.size());
    this->ev_con_ = evhttp_connection_base_bufferevent_new(
        this->ev_base_, nullptr, nullptr, host_str.c_str(),
        static_cast<uint16_t>(port));
    if (this->ev_con_ == nullptr) {
      NET_LOG(ERROR,
              "Failed to connect : evhttp_connection_base_bufferevent_new()");
      return;
    }

    evhttp_connection_set_retries(this->ev_con_, 0);

    evhttp_connection_set_timeout(this->ev_con_, timeout_in_secs);

    return;
}

std::unique_ptr<ClientRequestInterface> EvHTTPClient::MakeRequest(absl::string_view uri_path, 
                                              absl::string_view method,
                                              ClientHandlerInterface* handler,
                                              ClientHandlerOptions* handler_options)
{
 std::unique_ptr<ClientRequestInterface> request(new EvHTTPClientRequest
                                    (handler, handler_options,this->ev_con_));
    
    absl::string_view path = uri_path;
    
    request->SetUriPath(path);
    
    request->SetHTTPMethod(method);

    return request;
}

void EvHTTPClient::CloseConnection(){
    if (ev_con_ != nullptr) {
      evhttp_connection_free(ev_con_);
    }

    event_base_free(ev_base_);

}


} // net_http
