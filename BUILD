load("@rules_cc//cc:defs.bzl", "cc_library")

cc_binary(
    name = "cobold",
    srcs = ["cobold.cc"],
    deps = [
        "//parser:parser",
        "//codegen:llvm_codegen",
        "//codegen:llvm_type_visitor",
    ],
)