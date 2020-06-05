cc_library(
    name = "UTF8Url",
    srcs = [
        "UTF8Url.cpp",
        ],
    hdrs = [
        "UTF8Url.h",
        ],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "http",
    srcs = [
        "HTTP.cpp",
        ],
    hdrs = [
        "HTTP.h",
        ],
    visibility = ["//visibility:public"],
    copts = ["-Isrc/logger","-Isrc/socket","-Isrc/http","-Isrc/nlohmann","-Isrc"],
    deps = [
            "//src/logger:logger",
            "//src/nlohmann:nlohmann_json",
            "//src/socket:socket",
            ":UTF8Url",
            ]
)


