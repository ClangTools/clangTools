cc_library(
    name = "http",
    srcs = [
        "HTTP.cpp",
        ],
    hdrs = [
        "HTTP.h",
        ],
    visibility = ["//visibility:public"],
    copts = ["-Isrc/logger","-Isrc/socket","-Isrc/http","-Isrc/nlohmann","-Isrc/UTF8Url","-Isrc"],
    deps = [
            "//src/logger:logger",
            "//src/nlohmann:nlohmann_json",
            "//src/socket:socket",
            "//src/UTF8Url:UTF8Url",
            ]
)


