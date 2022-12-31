
# Description: official client APIs are still heavily experimental

package(
    default_visibility = [
        ":http_client_users",
        "//net_http:__subpackages__",
    ],
)

package_group(
    name = "http_client_users",
    packages = [
        "//third_party/ecclesia/...",
    ],
)

licenses(["notice"])

cc_library(
    name = "http_client_api",
    srcs = [],
    hdrs = [
        "httpclient_interface.h",
        "client_request_interface.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//net_http/public:shared_files",
    ],
)

cc_library(
    name = "http_client",
    hdrs = [
        "httpclient.h",
    ],
    deps = [
        ":http_client_api",
        "//net_http/client/internal:evhttp_client",
        "@com_google_absl//absl/memory",
    ],
)

# currently not able to be used
