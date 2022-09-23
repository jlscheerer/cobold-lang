#include "codegen/llvm_codegen.h"

#include <cstdlib>
#include <memory>
#include <mutex>
#include <system_error>
#include <vector>

#include "codegen/llvm_statement_visitor.h"
#include "codegen/llvm_type_visitor.h"
#include "parser/source_file.h"

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
absl::Status LLVMCodeGen::Generate(const SourceFile &file) {
  LLVMCodeGen codegen;
  codegen.GenerateLLVM(file);
  return codegen.Build("output");
}

LLVMCodeGen::LLVMCodeGen() {
  context_ = std::make_unique<llvm::LLVMContext>();
  module_ = std::make_unique<llvm::Module>("Cobold::Module", *context_);
  builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

void LLVMCodeGen::GenerateLLVM(const SourceFile &file) {
  AddFunctionDeclarations(file);

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

  // Call the user provided "fn Main()"
  llvm::Value *ret_value = builder_->CreateCall(functions_["Main"], {});

  builder_->CreateRet(ret_value);
  llvm::verifyFunction(*function);

  AddFunctionDefinitions(file);
}

void LLVMCodeGen::AddFunctionDeclarations(const SourceFile &file) {
  for (const std::unique_ptr<Function> &fn : file.functions()) {
    // TODO(jlscheerer) Handle overloading functions.
    std::vector<llvm::Type *> args;
    args.reserve(fn->arguments().size());
    for (const auto &argument : fn->arguments()) {
      args.push_back(LLVMTypeVisitor::Translate(context_.get(), argument.type));
    }
    llvm::Type *return_type =
        LLVMTypeVisitor::Translate(context_.get(), fn->return_type());
    llvm::FunctionType *function_type =
        llvm::FunctionType::get(return_type, args, false);

    llvm::Function *function =
        llvm::Function::Create(function_type, llvm::Function::ExternalLinkage,
                               fn->name(), module_.get());
    functions_[fn->name()] = function;
  }
}

void LLVMCodeGen::AddFunctionDefinitions(const SourceFile &file) {
  for (const std::unique_ptr<Function> &fn : file.functions()) {
    if (!fn->external()) {
      llvm::Function *function = functions_[fn->name()];

      llvm::BasicBlock *basic_block =
          llvm::BasicBlock::Create(*context_, "entry", function);
      builder_->SetInsertPoint(basic_block);

      LLVMStatementVisitor::Translate(context_.get(), module_.get(),
                                      builder_.get(),
                                      &fn->As<DefinedFunction>()->body());
      llvm::verifyFunction(*function);
    }
  }
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

  // TODO(jlscheerer) Add Function Pass Manager for Optimizations

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

absl::Status LLVMCodeGen::Build(const std::string &filename) {
  absl::Status status = Emit(absl::StrCat(filename, ".o"));
  if (!status.ok())
    return status;
  std::system(absl::StrCat("gcc ", absl::StrCat(filename, ".o"),
                           " std/bin/cobold_io.o -o ", filename)
                  .c_str());
  return absl::OkStatus();
}
// `LLVMCodeGen` ===================================================
} // namespace Cobold