#ifndef COBOLD_CODEGEN_LLVM_CODEGEN
#define COBOLD_CODEGEN_LLVM_CODEGEN

#include <memory>

#include "codegen/cobold_build_context.h"
#include "parser/source_file.h"

#include "absl/status/status.h"

namespace Cobold {
class LLVMCodeGen {
public:
  static absl::Status Generate(const SourceFile &file);

private:
  LLVMCodeGen();
  void CreateBuiltinTypes();

  void GenerateLLVM(const SourceFile &file);
  void AddFunctionDeclarations(const SourceFile &file);
  void AddFunctionDefinitions(const SourceFile &file);

  absl::Status Emit(const std::string &filename);
  absl::Status Build(const std::string &filename);

  CoboldBuildContext context_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_CODEGEN */
