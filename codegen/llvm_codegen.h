#ifndef COBOLD_CODEGEN_LLVM_CODEGEN
#define COBOLD_CODEGEN_LLVM_CODEGEN

#include <memory>

#include "parser/source_file.h"

#include "absl/status/status.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace Cobold {
class LLVMCodeGen {
public:
  static absl::Status Generate(const SourceFile &file);

private:
  LLVMCodeGen();
  void GenerateLLVM(const SourceFile &file);
  void AddFunctionDeclarations(const SourceFile &file);
  void AddFunctionDefinitions(const SourceFile &file);

  absl::Status Emit(const std::string &filename);
  absl::Status Build(const std::string &filename);

  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;

  absl::flat_hash_map<std::string, llvm::Function *> functions_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_CODEGEN */
