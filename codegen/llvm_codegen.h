#ifndef COBOLD_CODEGEN_COBOLD_CODEGEN
#define COBOLD_CODEGEN_COBOLD_CODEGEN

#include <memory>

#include "absl/status/status.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace Cobold {
class LLVMCodeGen {
public:
  static absl::Status Generate();

private:
  LLVMCodeGen();
  void GenerateLLVM();

  absl::Status Emit(const std::string &filename);
  absl::Status Build(const std::string &filename);
  llvm::Function *GeneratePrintFn();

  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_COBOLD_CODEGEN */
