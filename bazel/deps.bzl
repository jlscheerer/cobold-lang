load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_jar")

def antlr4_deps():
    """ANTLR dependency for the parser."""
    ANTLR4_VERSION = "4.10.1"
    http_archive(
        name = "antlr4_runtimes",
        build_file_content = """
package(default_visibility = ["//visibility:public"])
cc_library(
    name = "cpp",
    srcs = glob(["runtime/Cpp/runtime/src/**/*.cpp"]),
    hdrs = glob(["runtime/Cpp/runtime/src/**/*.h"]),
    includes = ["runtime/Cpp/runtime/src"],
)
  """,
        sha256 = "a320568b738e42735946bebc5d9d333170e14a251c5734e8b852ad1502efa8a2",
        strip_prefix = "antlr4-" + ANTLR4_VERSION,
        urls = ["https://github.com/antlr/antlr4/archive/v" + ANTLR4_VERSION + ".tar.gz"],
    )
    http_jar(
        name = "antlr4_jar",
        urls = ["https://www.antlr.org/download/antlr-" + ANTLR4_VERSION + "-complete.jar"],
        sha256 = "41949d41f20d31d5b8277187735dd755108df52b38db6c865108d3382040f918",
    )