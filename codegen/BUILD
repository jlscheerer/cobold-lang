package(default_visibility = ["//visibility:public"])

cc_library(
    name = "llvm_type_visitor",
    hdrs = ["llvm_type_visitor.h"],
    srcs = ["llvm_type_visitor.cc"],
    deps = [
        "//visitor:type_visitor",
        "@llvm-project//llvm:Core",
    ],
)

cc_library(
    name = "llvm_expression_visitor",
    hdrs = ["llvm_expression_visitor.h"],
    srcs = ["llvm_expression_visitor.cc"],
    deps = [
        "//visitor:expression_visitor",
        "@llvm-project//llvm:Core",
    ],
)

cc_library(
    name = "llvm_statement_visitor",
    hdrs = ["llvm_statement_visitor.h"],
    srcs = ["llvm_statement_visitor.cc"],
    deps = [
        ":llvm_expression_visitor",
        "//visitor:statement_visitor",
        "@llvm-project//llvm:Core",
    ],
)

cc_library(
    name = "llvm_codegen",
    hdrs = ["llvm_codegen.h"],
    srcs = ["llvm_codegen.cc"],
    deps = [
        ":llvm_type_visitor",
        ":llvm_statement_visitor",
        "//parser:source_file",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:Target",
        "@llvm-project//llvm:CodeGen",
        "@llvm-project//llvm:AllTargetsMCAs",
        "@llvm-project//llvm:AllTargetsCodeGens",
        "@llvm-project//llvm:AllTargetsAsmParsers",
        "@llvm-project//llvm:AsmParser",
        "@com_google_absl//absl/status",
    ],
)