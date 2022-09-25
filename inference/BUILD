package(default_visibility = ["//visibility:public"])

cc_library(
    name = "type_context",
    hdrs = ["type_context.h"],
    srcs = ["type_context.cc"],
    deps = [
        "//core:type",
        "//parser:source_file",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "type_inference_visitor",
    hdrs = ["type_inference_visitor.h"],
    srcs = ["type_inference_visitor.cc"],
    deps = [
        ":type_context",
        "//core:function",
        "//parser:source_file",
        "//visitor:expression_visitor",
    ],
)