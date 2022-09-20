#include "codegen/llvm_codegen.h"

#include <memory>
#include <system_error>

#include "absl/strings/str_cat.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace Cobold {
// `LLVMCodeGen` ========================================================
absl::Status LLVMCodeGen::Generate() {
  LLVMCodeGen codegen;
  codegen.GenerateLLVM();
  return codegen.Emit("output.o");
}

LLVMCodeGen::LLVMCodeGen() {
  context_ = std::make_unique<llvm::LLVMContext>();
  module_ = std::make_unique<llvm::Module>("Cobold::Module", *context_);
  builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

void LLVMCodeGen::GenerateLLVM() {
  // Generate the entry point for the module: int main(int argc, char **argv)
  std::vector<llvm::Type *> args{
      llvm::Type::getIntNTy(*context_, 32),
      llvm::PointerType::get(
          llvm::PointerType::get(llvm::Type::getIntNTy(*context_, 8), 0), 0)};
  llvm::FunctionType *function_type = llvm::FunctionType::get(
      llvm::Type::getIntNTy(*context_, 32), args, false);
  llvm::Function *function = llvm::Function::Create(
      function_type, llvm::Function::ExternalLinkage, "main", module_.get());
  llvm::BasicBlock *basic_block =
      llvm::BasicBlock::Create(*context_, "entry", function);
  builder_->SetInsertPoint(basic_block);
  llvm::Value *ret_value =
      llvm::ConstantInt::get(*context_, llvm::APInt(32, 17, true));
  builder_->CreateRet(ret_value);
  llvm::verifyFunction(*function);
}

absl::Status LLVMCodeGen::Emit(const std::string &filename) {
  // Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto target_triple = llvm::sys::getDefaultTargetTriple();
  module_->setTargetTriple(target_triple);

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
  if (!target)
    return absl::InternalError(error);

  constexpr char CPU[] = "generic";
  constexpr char features[] = "";

  llvm::TargetOptions options;
  auto rm = llvm::Optional<llvm::Reloc::Model>();
  auto target_machine =
      target->createTargetMachine(target_triple, CPU, features, options, rm);
  module_->setDataLayout(target_machine->createDataLayout());

  std::error_code error_code;
  llvm::raw_fd_ostream out_file(filename, error_code, llvm::sys::fs::OF_None);
  if (error_code) {
    return absl::InternalError(
        absl::StrCat("Could not open file: ", error_code.message()));
  }
  llvm::legacy::PassManager pass;
  auto file_type = llvm::CGFT_ObjectFile;

  if (target_machine->addPassesToEmitFile(pass, out_file, nullptr, file_type)) {
    return absl::InternalError("Target cannot emit a file of this type.");
  }

  pass.run(*module_);
  out_file.flush();

  return absl::OkStatus();
}
// `LLVMCodeGen` ===================================================
} // namespace Cobold