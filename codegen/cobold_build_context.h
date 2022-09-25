#ifndef COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT
#define COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT

#include <string>

#include "absl/container/flat_hash_map.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace Cobold {
struct CoboldBuildContext {
  llvm::LLVMContext *llvm_context() { return context.get(); }
  llvm::Module *llvm_module() { return module.get(); }
  llvm::IRBuilder<> *llvm_builder() { return builder.get(); }

  llvm::LLVMContext &operator*() { return *context; }

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

  absl::flat_hash_map<std::string, llvm::Function *> functions;

  // TODO(jlscheerer) Move this outside and into a separate class
  absl::flat_hash_map<std::string, llvm::AllocaInst *> named_vars;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT */
