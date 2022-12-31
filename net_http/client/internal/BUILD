# Description: implementations of HTTP Client

package(
    default_visibility = [
        "//net_http:__subpackages__",
    ],
)

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "evhttp_client",
    srcs = [
        "evhttp_client_request.cc",
        "evhttp_client.cc",
    ],
    hdrs = [
        "evhttp_client_request.h",
        "evhttp_client.h",
    ],
    deps = [
        "//net_http/compression:gzip_zlib",
        "//net_http/internal:net_logging",
        "//net_http/client/public:http_client_api",
        "@com_github_libevent_libevent//:libevent",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:span",
    ],
)