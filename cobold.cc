#include <iostream>

#include "absl/status/statusor.h"
#include "codegen/llvm_codegen.h"
#include "parser/parser.h"

int main(int argc, char **argv) {
#if 0
  const std::string filename = "test/example.cb";
  absl::StatusOr<Cobold::SourceFile> source = Cobold::Parser::Parse(filename);
  std::cout << source.value().DebugString() << std::endl;
#endif
  auto _ = Cobold::LLVMCodeGen::Generate();
}