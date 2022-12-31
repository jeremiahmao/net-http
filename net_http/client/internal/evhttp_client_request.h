//copyright information needs to be added

#ifndef TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_H_
#define TENSORFLOW_SERVING_UTIL_NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_H_

#include "absl/synchronization/notification.h"
#include "net_http/internal/net_logging.h"

#include "net_http/client/public/client_request_interface.h"

//needed for ClientOptions
#include "net_http/client/public/httpclient_interface.h"

struct evhttp_request;
struct evkeyvalq;
struct evbuffer;

//needed for Send()
struct evhttp_connection;
struct event_base;

namespace net_http {

// Headers only
struct ParsedEvResponse {
  public:
    // Doesn't take the ownership
    explicit ParsedEvResponse(ClientHandlerInterface* handler, 
                              ClientHandlerOptions* handler_options);
    ~ParsedEvResponse();

    // Decode and cache the result
    void decode_response();

    evhttp_request* request;  // raw request
    
    //request components
    absl::string_view method;  // from enum
    absl::string_view uri_path;
  
    //response components
    HTTPStatusCode response_status = HTTPStatusCode::UNDEFINED;
    evkeyvalq* response_headers = nullptr;  // owned by raw request

    ClientHandlerInterface* handler = nullptr; //callback handler
    ClientHandlerOptions* handler_options = nullptr;
};


class EvHTTPClientRequest : public ClientRequestInterface {
  public:
    virtual ~EvHTTPClientRequest(); 
    
    EvHTTPClientRequest(const EvHTTPClientRequest& other) = delete;
    EvHTTPClientRequest& operator=(const EvHTTPClientRequest& other) = delete;
    
    // Doesn't own the handler
    //evconnection and evbase are subject to being removed based on design
    EvHTTPClientRequest(ClientHandlerInterface* handler,
                        ClientHandlerOptions* handler_options,
                        evhttp_connection* ev_con);
    
    void SetUriPath(absl::string_view path) override;
    absl::string_view uri_path() const override;

    void SetHTTPMethod(absl::string_view method) override;
    absl::string_view http_method() const override;

    void RegisterResponseHandler(ClientHandlerInterface* handler, 
                                      ClientHandlerOptions* options) override;
   
    void WriteRequestBytes(const char* data, int64_t size) override;

    void WriteRequestString(absl::string_view data) override;
    
    std::unique_ptr<char[], ClientRequestInterface::BlockDeleter> ReadResponseBytes(int64_t* size) override;

    absl::string_view GetResponseHeader(
         absl::string_view header) const override;

    std::vector<absl::string_view> response_headers() const override;

    void OverwriteRequestHeader(absl::string_view header,
                                absl::string_view value) override;
    void AppendRequestHeader(absl::string_view header,
                             absl::string_view value) override;

    void Send() override;

    ParsedEvResponse* GetParsedResponse();

    void Shutdown() override;
    
  private:
  
    void Initialize();

    bool with_body_ = false;

    std::unique_ptr<ParsedEvResponse> parsed_response_;
    
    //for Send()
    evhttp_connection* ev_con_;
    event_base* ev_base_;

    evbuffer* output_buf;  // owned by raw request

};

} // namespace net_http

#endif //NET_HTTP_CLIENT_INTERNAL_CLIENT_EVHTTP_REQUEST_H_