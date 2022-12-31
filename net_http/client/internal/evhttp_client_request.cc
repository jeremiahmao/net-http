//TODO(jeremiahmao): add copyright info

// libevent based request implementation

#include "evhttp_client_request.h"

//libevent
#include "libevent/include/event2/http.h"
#include "libevent/include/event2/buffer.h"
#include "libevent/include/event2/event.h"
#include "libevent/include/event2/keyvalq_struct.h"
#include "libevent/include/event2/util.h"

namespace net_http {

ParsedEvResponse::~ParsedEvResponse() {
  if (request && evhttp_request_is_owned(request)) {
    evhttp_request_free(request);
  }
}

ParsedEvResponse::ParsedEvResponse(ClientHandlerInterface* handler, 
                                   ClientHandlerOptions* handler_options)
    : request(nullptr),
      handler(handler),
      handler_options(handler_options)
     {}

void ParsedEvResponse::decode_response() { //similar to PopulateResponse in test_client
    response_status =
        static_cast<HTTPStatusCode>(evhttp_request_get_response_code(request));
    
    response_headers = evhttp_request_get_input_headers(request);
}


//---------------------------------------------------------------------------------------


EvHTTPClientRequest::EvHTTPClientRequest(ClientHandlerInterface* handler,
                                         ClientHandlerOptions* handler_options,
                                         ClientOptions* client_options,
                                         evhttp_connection* ev_con
                                         )
    : ev_con_(ev_con), 
      ev_base_(evhttp_connection_get_base(ev_con)),
      client_options_(client_options),
      parsed_response_(new ParsedEvResponse(handler, handler_options)),
      output_buf(nullptr) {Initialize();}


EvHTTPClientRequest::~EvHTTPClientRequest() {}

//general callback (formerly ResponseDone)
void OnResponse(evhttp_request* request, void* ctx){
  
    EvHTTPClientRequest* client_request = reinterpret_cast<EvHTTPClientRequest*>(ctx);

    ParsedEvResponse* parsed_response = client_request->GetParsedResponse();

    if (request == nullptr) {
     int errcode = EVUTIL_SOCKET_ERROR();
     NET_LOG(ERROR, "socket error = %s (%d)",
     evutil_socket_error_to_string(errcode), errcode);

     parsed_response->handler->OnTimeout(client_request);

     return;
    }

    parsed_response->decode_response();
    
    parsed_response->handler->OnResponse(client_request, parsed_response->response_status);
}


void EvHTTPClientRequest::Initialize(){
  parsed_response_->request = evhttp_request_new(OnResponse, this);
  if (parsed_response_->request == nullptr) {
    NET_LOG(FATAL, "Request not initialized.");
  }

  output_buf = evhttp_request_get_output_buffer(parsed_response_->request);
  if (output_buf == nullptr)
    NET_LOG(ERROR, "Request not initialized.");
}
    
void EvHTTPClientRequest::SetUriPath(absl::string_view path){
    parsed_response_->uri_path = path;
}

absl::string_view EvHTTPClientRequest::uri_path() const {
  return parsed_response_->uri_path;
}

void EvHTTPClientRequest::SetHTTPMethod(absl::string_view method){
  parsed_response_->method = method;
}

absl::string_view EvHTTPClientRequest::http_method() const{
  return parsed_response_->method;
}

void EvHTTPClientRequest::RegisterResponseHandler(ClientHandlerInterface* handler,
                                             ClientHandlerOptions* options){
  parsed_response_->handler = handler;
  parsed_response_->handler_options = options;
}

void EvHTTPClientRequest::WriteRequestBytes(const char* data, int64_t size){
  bool with_body_ = true;

  assert(size >= 0);
  if (output_buf == nullptr) {
    NET_LOG(FATAL, "Request not initialized.");
    return;
  }

  int ret = evbuffer_add(output_buf, data, static_cast<size_t>(size));
  if (ret == -1) {
    NET_LOG(ERROR, "Failed to write %zu bytes data to output buffer",
            static_cast<size_t>(size));
  }

  //different from server write, taken from test_client and modified
  //TODO (jeremiahmao): ask wenbozhu about this
  char length_header[16];
    evutil_snprintf(length_header, sizeof(length_header) - 1, "%lu",
                    static_cast<size_t>(size));
    AppendRequestHeader("Content-Length", length_header);
}

void EvHTTPClientRequest::WriteRequestString(absl::string_view data){
  WriteRequestBytes(data.data(), static_cast<int64_t>(data.size()));
}

//TODO (jeremiahmao): does not currently support Gzip content
//TODO (jeremiahmao): cannot happen until response is recieved
std::unique_ptr<char[], ClientRequestInterface::BlockDeleter> 
EvHTTPClientRequest::ReadResponseBytes(int64_t* size){
  evbuffer* input_buf =
      evhttp_request_get_input_buffer(parsed_response_->request);
  if (input_buf == nullptr) {
    *size = 0;
    return nullptr;  // no body
  }

  if (evbuffer_get_length(input_buf) == 0) {
    *size = 0;
    return nullptr;  // EOF
  }

  auto buf_size = reinterpret_cast<size_t*>(size);

  *buf_size = evbuffer_get_contiguous_space(input_buf);
  assert(*buf_size > 0);

  char* block = std::allocator<char>().allocate(*buf_size);
  int ret = evbuffer_remove(input_buf, block, *buf_size);

  if (ret != *buf_size) {
    NET_LOG(ERROR, "Unexpected: read less than specified num_bytes : %zu",
            *buf_size);
    std::allocator<char>().deallocate(block, *buf_size);
    *buf_size = 0;
    return nullptr;  // don't return corrupted buffer
  }

  return std::unique_ptr<char[], ClientRequestInterface::BlockDeleter>(block, ClientRequestInterface::BlockDeleter(*buf_size));
}

//TODO (jeremiahmao): cannot happen until the response has been recieved
absl::string_view EvHTTPClientRequest::GetResponseHeader(
    absl::string_view header) const{
  std::string header_str(header.data(), header.size());
  return absl::NullSafeStringView(
    evhttp_find_header(parsed_response_->response_headers, header_str.c_str()));
}

std::vector<absl::string_view> EvHTTPClientRequest::response_headers() const{
  auto result = std::vector<absl::string_view>();
  auto ev_headers = parsed_response_->response_headers;

  for (evkeyval* header = ev_headers->tqh_first; header;
       header = header->next.tqe_next) {
    result.emplace_back(header->key);
  }

  return result;
}

void EvHTTPClientRequest::OverwriteRequestHeader(absl::string_view header,
                                            absl::string_view value) {
  evkeyvalq* ev_headers =
      evhttp_request_get_output_headers(parsed_response_->request);

  std::string header_str = std::string(header.data(), header.size());
  const char* header_cstr = header_str.c_str();

  evhttp_remove_header(ev_headers, header_cstr);
  evhttp_add_header(ev_headers, header_cstr,
                    std::string(value.data(), value.size()).c_str());
}

void EvHTTPClientRequest::AppendRequestHeader(absl::string_view header,
                                         absl::string_view value) {
  evkeyvalq* ev_headers =
      evhttp_request_get_output_headers(parsed_response_->request);

  int ret = evhttp_add_header(ev_headers,
                              std::string(header.data(), header.size()).c_str(),
                              std::string(value.data(), value.size()).c_str());

  if (ret != 0) {
    NET_LOG(ERROR,
            "Unexpected: failed to set the request header"
            " %.*s: %.*s",
            static_cast<int>(header.size()), header.data(),
            static_cast<int>(value.size()), value.data());
  }
}

evhttp_cmd_type GetMethodEnum(absl::string_view method, bool with_body) {
  if (method.compare("GET") == 0) {
    return EVHTTP_REQ_GET;
  } else if (method.compare("POST") == 0) {
    return EVHTTP_REQ_POST;
  } else if (method.compare("HEAD") == 0) {
    return EVHTTP_REQ_HEAD;
  } else if (method.compare("PUT") == 0) {
    return EVHTTP_REQ_PUT;
  } else if (method.compare("DELETE") == 0) {
    return EVHTTP_REQ_DELETE;
  } else if (method.compare("OPTIONS") == 0) {
    return EVHTTP_REQ_OPTIONS;
  } else if (method.compare("TRACE") == 0) {
    return EVHTTP_REQ_TRACE;
  } else if (method.compare("CONNECT") == 0) {
    return EVHTTP_REQ_CONNECT;
  } else if (method.compare("PATCH") == 0) {
    return EVHTTP_REQ_PATCH;
  } else {
    if (with_body) {
      return EVHTTP_REQ_POST;
    } else {
      return EVHTTP_REQ_GET;
    }
  }
}

//asynchronously sends requests via event executor
void EvHTTPClientRequest::Send(){

 //This means the request is not constantly streaming something
 AppendRequestHeader("Connection", "close");

 std::string uri_path(parsed_response_->uri_path.data(), parsed_response_->uri_path.size());
 evhttp_cmd_type method = GetMethodEnum(parsed_response_->method, with_body_);

 int request_made = evhttp_make_request(
     ev_con_, parsed_response_->request, method, uri_path.c_str());

 if(request_made != 0){
   NET_LOG(ERROR, "evhttp_make_request() failed");
   return;
 }
  
  /*
   When the following code runs
  */
  client_options_->executor()->Schedule([this]() {
    client_options_->ResetNotification();
    event_base_dispatch(ev_base_);
    client_options_->Notify();
  });
 return;
}

ParsedEvResponse* EvHTTPClientRequest::GetParsedResponse(){
  return parsed_response_.get();
}

//TODO (jeremiahmao): figure out how to implement
void EvHTTPClientRequest::Shutdown(){
  ev_con_ = nullptr;
  NET_LOG(FATAL, "Shutdown() not implemented.");
}

} // namespace net_http