#include <iostream>

#include "absl/status/statusor.h"
#include "codegen/llvm_codegen.h"
#include "codegen/llvm_type_visitor.h"
#include "core/type.h"
#include "parser/parser.h"

int main(int argc, char **argv) {
  const std::string filename = "test/simple.cb";
  absl::StatusOr<Cobold::SourceFile> source = Cobold::Parser::Parse(filename);
  std::cout << source.value().DebugString() << std::endl;
  // auto _ = Cobold::LLVMCodeGen::Generate(source.value());
}