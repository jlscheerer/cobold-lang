load("//bazel:antlr4.bzl", "antlr_cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "options",
    hdrs = ["options.h"],
)

antlr_cc_library(
    name = "cobold",
    src = "Cobold.g4",
    package = "Cobold",
)
