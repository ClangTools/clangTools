cc_library(
    name = "yaml-cpp_internal",
    visibility = ["//:__subpackages__"],
    strip_include_prefix = "src",
    hdrs = glob(["src/**/*.h"]),
    copts = ["-std=c++11"],
)

cc_library(
    name = "yaml-cpp",
    visibility = ["//visibility:public"],
    includes = ["include"],
    hdrs = glob(["include/**/*.h"]),
    srcs = glob(["src/**/*.cpp", "src/**/*.h"]),
    copts = ["-std=c++11"],
)

cc_test(
    name = "yaml-cpp-test",
    srcs = ["test/yaml-cpp-test.cc"],
    copts = ["-Iinclude","-std=c++11"],
    deps = [
        "@com_google_googletest//:gtest_main",
        ":yaml-cpp",
    ],
)
