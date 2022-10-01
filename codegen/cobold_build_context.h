#ifndef COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT
#define COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT

#include <memory>
#include <stack>
#include <string>

#include "absl/container/flat_hash_map.h"

#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"

namespace Cobold {
struct LoopInstructionBlock {
  llvm::BasicBlock *break_bb, *continue_bb;
};

struct CoboldBuildContext {
  llvm::LLVMContext *llvm_context() { return context.get(); }
  llvm::Module *llvm_module() { return module.get(); }
  llvm::IRBuilder<> *llvm_builder() { return builder.get(); }

  llvm::LLVMContext &operator*() { return *context; }

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

  // Pass Managers
  std::unique_ptr<llvm::legacy::FunctionPassManager> function_pass_manager;

  absl::flat_hash_map<std::string, llvm::Function *> functions;

  // TODO(jlscheerer) Move this outside and into a separate class
  absl::flat_hash_map<std::string, llvm::AllocaInst *> named_vars;

  std::stack<LoopInstructionBlock> loop_instruction_stack;

  llvm::Constant *AddStringConstant(std::string &value);
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT */
