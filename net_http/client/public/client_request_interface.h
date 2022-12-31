/*add copyright information*/

// net_http::ClientRequestInterface defines a pure interface class for handling 
// an HTTP request/response on the client-side. It is designed...

#ifndef NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_
#define NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_

#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "net_http/public/response_code_enum.h"

namespace net_http {

// Options to specify when registering a handler (given a uri pattern).
// This should be a value type.
class ClientHandlerOptions {
 public:
  ClientHandlerOptions() = default;

  ClientHandlerOptions(const ClientHandlerOptions&) = default;
  ClientHandlerOptions& operator=(const ClientHandlerOptions&) = default;

  // Sets the max length of uncompressed data when uncompressing a request body
  inline ClientHandlerOptions& set_auto_uncompress_max_size(int64_t size) {
    auto_uncompress_max_size_ = size;
    return *this;
  }

  // The max length of uncompressed data when doing uncompress. Returns 0 if
  // not set. See Zlib::kMaxUncompressedBytes for the default config.
  inline int64_t auto_uncompress_max_size() const {
    return auto_uncompress_max_size_;
  }

  // The auto_uncompress_input option specifies whether the request
  // input data should be uncompressed if the request has the
  // Content-Encoding: .*gzip.* header. The option defaults to true.
  inline ClientHandlerOptions& set_auto_uncompress_input(
      bool should_uncompress) {
    auto_uncompress_input_ = should_uncompress;
    return *this;
  }

  inline bool auto_uncompress_input() const { return auto_uncompress_input_; }

 private:
  // To be added: compression, CORS rules, streaming control
  // thread executor, admission control, limits ...

  bool auto_uncompress_input_ = true;

  int64_t auto_uncompress_max_size_ = 0;
};

struct ClientRequestInterface;

class ClientHandlerInterface{
 public:
  virtual ~ClientHandlerInterface() = default;

  ClientHandlerInterface(const ClientHandlerInterface& other) = delete;
  ClientHandlerInterface& operator=(const ClientHandlerInterface& other) = delete;

  virtual void OnResponse(ClientRequestInterface* request, HTTPStatusCode status) = 0;

  virtual void OnTimeout(ClientRequestInterface* request) = 0;

 protected:
  ClientHandlerInterface() = default;
};

// A request handler is registered by the application to handle a request
// based on the request Uri path, available via ServerRequestInterface.
//
// Request handlers need be completely non-blocking. And handlers may add
// callbacks to a thread-pool that is managed by the application itself.

class ClientRequestInterface {
 public:
  // To be used with memory blocks returned via std::unique_ptr<char[]>
  struct BlockDeleter {
   public:
    BlockDeleter() : size_(0) {}  // nullptr
    explicit BlockDeleter(int64_t size) : size_(size) {}
    inline void operator()(char* ptr) const {
      // TODO: c++14 ::operator delete[](ptr, size_t)
      std::allocator<char>().deallocate(ptr, static_cast<std::size_t>(size_));
    }

   private:
    int64_t size_;
  };

  virtual ~ClientRequestInterface() = default;

  ClientRequestInterface(const ClientRequestInterface& other) = delete;
  ClientRequestInterface& operator=(const ClientRequestInterface& other) = 
     delete;
 
  // The portion of the request URI after the host and port.
  // E.g. "/path/to/resource?param=value&param=value#fragment".
  // Doesn't unescape the contents; returns "/" at least.

  virtual void SetUriPath(absl::string_view path) = 0; //setter for uri
  virtual absl::string_view uri_path() const = 0; 

  // HTTP request method.
  // Must be in Upper Case.

  virtual void SetHTTPMethod(absl::string_view method) = 0;//setter for uri
  virtual absl::string_view http_method() const = 0;

  virtual void RegisterResponseHandler(ClientHandlerInterface* handler, 
                                      ClientHandlerOptions* options) = 0;

  // Appends the data block of the specified size to the request body.
  // This request object takes the ownership of the data block.
  virtual void WriteRequestBytes(const char* data, int64_t size) = 0;

  // Appends (by coping) the data of string_view to the end of
  // the response body.
  virtual void WriteRequestString(absl::string_view data) = 0;
  
  virtual std::unique_ptr<char[], ClientRequestInterface::BlockDeleter> ReadResponseBytes(
      int64_t* size) = 0;

  virtual absl::string_view GetResponseHeader(
       absl::string_view header) const = 0;

  // Returns all the response header names.
  // This is not an efficient way to access headers, mainly for debugging uses.
  virtual std::vector<absl::string_view> response_headers() const = 0; 

  virtual void OverwriteRequestHeader(absl::string_view header,
                                       absl::string_view value) = 0;
  virtual void AppendRequestHeader(absl::string_view header,
                                    absl::string_view value) = 0;

  //sends request
  virtual void Send() = 0;

  //Shuts down request processes, request can be safely deleted after calling
  virtual void Shutdown() = 0; //new method for request internal shutdown

 protected:
  ClientRequestInterface() = default;

 private:
  // Do not add any data members to this class.
};

}  // namespace net_http

#endif  // NET_HTTP_CLIENT_PUBLIC_CLIENT_REQUEST_INTERFACE_H_
