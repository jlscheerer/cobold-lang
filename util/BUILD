package(default_visibility = ["//visibility:public"])

cc_library(
    name = "type_traits",
    hdrs = ["type_traits.h"],
    deps = [],
)

cc_library(
    name = "scoped_map",
    hdrs = ["scoped_map.h"],
    deps = ["@com_google_absl//absl/container:flat_hash_map"],
)

cc_library(
    name = "expression_printer",
    srcs = ["expression_printer.cc"],
    hdrs = ["expression_printer.h"],
    deps = [
        "//visitor:expression_visitor"
    ],
)

cc_library(
    name = "statement_printer",
    srcs = ["statement_printer.cc"],
    hdrs = ["statement_printer.h"],
    deps = [
        ":expression_printer",
        "//visitor:statement_visitor"
    ],
)