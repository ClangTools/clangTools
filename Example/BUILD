cc_binary(
    name = "kHttpd_demo",
    srcs = ["kHttpd_demo.cpp"],
    copts = [
        "-Isrc",
        "-Isrc/kHttpd",
        "-Isrc/logger",
        "-Isrc/thread_pool",
        "-Isrc/http",
        "-Isrc/UTF8Url",
        "-Isrc/socket",
        "-Isrc/JSON",
        "-Isrc/Base64",
    ],
    deps = [
        "//:Tools",
    ],
)
cc_binary(
    name = "logger_demo",
    srcs = ["logger_demo.cpp"],
    copts = [
        "-Isrc",
        "-Isrc/logger",
        "-Isrc/thread_pool",
    ],
    deps = [
        "//:Tools",
    ],
)