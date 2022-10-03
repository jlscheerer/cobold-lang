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

class BuildContext {
public:
  BuildContext() {}
  BuildContext(std::unique_ptr<llvm::LLVMContext> &&context,
               std::unique_ptr<llvm::Module> &&module,
               std::unique_ptr<llvm::IRBuilder<>> &&builder,
               std::unique_ptr<llvm::legacy::FunctionPassManager>
                   &&function_pass_manager)
      : context_(std::move(context)), module_(std::move(module)),
        builder_(std::move(builder)),
        function_pass_manager_(std::move(function_pass_manager)) {}

  llvm::LLVMContext &operator*() { return *context_; }

  llvm::Constant *AddStringConstant(std::string &value);

  llvm::LLVMContext *llvm_context() { return context_.get(); }
  llvm::Module *llvm_module() { return module_.get(); }
  llvm::IRBuilder<> *llvm_builder() { return builder_.get(); }
  llvm::legacy::FunctionPassManager *function_pass_manager() {
    return function_pass_manager_.get();
  }

  std::stack<LoopInstructionBlock> &loop_instruction_stack() {
    return loop_instruction_stack_;
  }

  bool HasNamedVar(const std::string &identifier) {
    return named_vars_.contains(identifier);
  }
  llvm::AllocaInst *AllocaForNamedVar(const std::string &identifier) {
    return named_vars_[identifier];
  }
  bool PutNamedVar(const std::string &identifier, llvm::AllocaInst *alloca) {
    if (!HasNamedVar(identifier)) {
      named_vars_[identifier] = alloca;
      return true;
    }
    return false;
  }

  bool HasFunction(const std::string &name) {
    return functions_.contains(name);
  }
  llvm::Function *FunctionForName(const std::string &name) {
    return functions_[name];
  }
  bool PutFunction(const std::string &name, llvm::Function *function) {
    if (!HasFunction(name)) {
      functions_[name] = function;
      return true;
    }
    return false;
  }

private:
  std::unique_ptr<llvm::LLVMContext> context_;
  std::unique_ptr<llvm::Module> module_;
  std::unique_ptr<llvm::IRBuilder<>> builder_;

  // Pass Managers
  std::unique_ptr<llvm::legacy::FunctionPassManager> function_pass_manager_;

  absl::flat_hash_map<std::string, llvm::Function *> functions_;

  // TODO(jlscheerer) Move this outside and into a separate class
  absl::flat_hash_map<std::string, llvm::AllocaInst *> named_vars_;

  std::stack<LoopInstructionBlock> loop_instruction_stack_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_COBOLD_BUILD_CONTEXT */
