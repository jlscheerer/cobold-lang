package(default_visibility = ["//visibility:public"])

cc_library(
    name = "source_location",
    srcs = ["source_location.cc"],
    hdrs = ["source_location.h"],
    deps = [],
)

cc_library(
    name = "source_file",
    srcs = ["source_file.cc"],
    hdrs = ["source_file.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "//core:function",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    copts = [
        "-fexceptions",
    ],
    deps = [
        ":source_file",
        "//core:function",
        "//inference:type_inference_visitor",
        "//reporting:error_context",
        "//parser/internal:options",
        "//parser/internal:cobold_cc_parser",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/status",
    ]
)