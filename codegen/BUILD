package(default_visibility = ["//visibility:public"])

cc_library(
    name = "llvm_codegen",
    hdrs = ["llvm_codegen.h"],
    srcs = ["llvm_codegen.cc"],
    deps = [
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