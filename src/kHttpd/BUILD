cc_library(
    name = "kHttpd",
    srcs = [
        "kHttpd.cpp",
        "kHttpdClient.cpp",
        "kWebSocketClient.cpp",
        ],
    hdrs = [
        "HttpResponseCode.h",
        "kHttpd.h",
        "kHttpdClient.h",
        "kWebSocketClient.h",
        "URLParser.hpp"
        ],
    visibility = ["//visibility:public"],
    copts = [
        "-Isrc/logger",
        "-Isrc/thread_pool",
        "-Isrc/kHttpd",
        "-Isrc/socket",
        "-Isrc/http",
        "-Isrc/net_tool",
        "-Isrc/Base64",
        "-Isrc/SHA1",
        "-Isrc"
    ],
    deps = [
            "//src/logger:logger",
            "//src/thread_pool:thread_pool",
            "//src/socket:socket",
            "//src/Base64:base64",
            "//src/CGI:CGI",
            "//src/http:UTF8Url",
            "//src/net_tool:net_tool",
            "//src/SHA1:SHA1",
            ]
)


