# Description: a lightweight http server and related utils to support Web clients

package(
    default_visibility = [
        ":http_server_clients",
        "//net_http:__subpackages__",
    ],
)

package_group(name = "http_server_clients")

licenses(["notice"])

cc_library(
    name = "http_server_api",
    hdrs = [
        "httpserver_interface.h",
        "server_request_interface.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//net_http/public:shared_files",
    ],
)

cc_library(
    name = "http_server",
    hdrs = [
        "httpserver.h",
    ],
    deps = [
        ":http_server_api",
        "//net_http/server/internal:evhttp_server",
        "@com_google_absl//absl/memory",
    ],
)
