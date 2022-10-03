#include "codegen/llvm_codegen.h"

#include <cstdlib>
#include <memory>
#include <mutex>
#include <system_error>
#include <vector>

#include "build_context.h"
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
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"

namespace Cobold {
// `LLVMCodeGen` ========================================================
absl::Status LLVMCodeGen::Generate(const SourceFile &file) {
  LLVMCodeGen codegen;
  codegen.GenerateLLVM(file);
  return codegen.Build("output");
}

LLVMCodeGen::LLVMCodeGen() {
  auto llvm_context = std::make_unique<llvm::LLVMContext>();
  auto llvm_module =
      std::make_unique<llvm::Module>("Cobold::Module", *llvm_context);
  auto llvm_builder = std::make_unique<llvm::IRBuilder<>>(*llvm_context);

  // Some optimizations on the functions.
  // Create a new pass manager attached to it.
  auto function_pass_manager =
      std::make_unique<llvm::legacy::FunctionPassManager>(llvm_module.get());
  // Promote allocas to registers.
  function_pass_manager->add(llvm::createPromoteMemoryToRegisterPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  function_pass_manager->add(llvm::createInstructionCombiningPass());
  // Reassociate expressions.
  function_pass_manager->add(llvm::createReassociatePass());
  // Eliminate Common SubExpressions.
  function_pass_manager->add(llvm::createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  function_pass_manager->add(llvm::createCFGSimplificationPass());
  function_pass_manager->doInitialization();

  context_ =
      BuildContext(std::move(llvm_context), std::move(llvm_module),
                   std::move(llvm_builder), std::move(function_pass_manager));

  CreateBuiltinTypes();
}

void LLVMCodeGen::CreateBuiltinTypes() {
  // size: i64 + data: char*
  llvm::StructType::create(
      *context_,
      {llvm::Type::getIntNTy(*context_, 64),
       llvm::PointerType::get(llvm::Type::getIntNTy(*context_, 8), 0)},
      "string", /*isPacked=*/false);
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
  llvm::Function *function =
      llvm::Function::Create(function_type, llvm::Function::ExternalLinkage,
                             "main", context_.llvm_module());

  llvm::BasicBlock *basic_block =
      llvm::BasicBlock::Create(*context_, "entry", function);
  context_.llvm_builder()->SetInsertPoint(basic_block);

  // Call the user provided "fn Main()"
  llvm::Value *ret_value =
      context_.llvm_builder()->CreateCall(context_.FunctionForName("Main"), {});

  context_.llvm_builder()->CreateRet(ret_value);
  llvm::verifyFunction(*function);

  AddFunctionDefinitions(file);
}

void LLVMCodeGen::AddFunctionDeclarations(const SourceFile &file) {
  for (const std::unique_ptr<Function> &fn : file.functions()) {
    // TODO(jlscheerer) Handle overloading functions.
    std::vector<llvm::Type *> args;
    args.reserve(fn->arguments().size());
    for (const auto &argument : fn->arguments()) {
      args.push_back(LLVMTypeVisitor::Translate(&context_, argument.type));
    }
    llvm::Type *return_type =
        LLVMTypeVisitor::Translate(&context_, fn->return_type());
    llvm::FunctionType *function_type =
        llvm::FunctionType::get(return_type, args, false);

    // llvm::Function::ExternalLinkage for external functions...
    llvm::Function *function;
    if (fn->external()) {
      function =
          llvm::Function::Create(function_type, llvm::Function::ExternalLinkage,
                                 fn->name(), context_.llvm_module());
    } else {
      function =
          llvm::Function::Create(function_type, llvm::Function::PrivateLinkage,
                                 fn->name(), context_.llvm_module());
    }
    context_.PutFunction(fn->name(), function);
  }
}

void LLVMCodeGen::AddFunctionDefinitions(const SourceFile &file) {
  for (const std::unique_ptr<Function> &fn : file.functions()) {
    if (!fn->external()) {
      llvm::Function *function = context_.FunctionForName(fn->name());
      llvm::BasicBlock *basic_block =
          llvm::BasicBlock::Create(*context_, "entry", function);
      context_.llvm_builder()->SetInsertPoint(basic_block);

      int index = 0; // we need to iterate over the declared and llvms args.
      for (auto &argument : function->args()) {
        const auto &decl_arg = fn->arguments()[index++];
        llvm::AllocaInst *alloca = LLVMStatementVisitor::CreateEntryBlockAlloca(
            function, decl_arg.name,
            LLVMTypeVisitor::Translate(&context_, decl_arg.type));
        context_.llvm_builder()->CreateStore(&argument, alloca);
        // TODO(jlscheerer) Improve this...
        context_.PutNamedVar(decl_arg.name, alloca);
      }

      LLVMStatementVisitor::Translate(&context_,
                                      &fn->As<DefinedFunction>()->body());
      llvm::verifyFunction(*function);

      context_.function_pass_manager()->run(*function);
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
  context_.llvm_module()->setTargetTriple(target_triple);

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
  context_.llvm_module()->setDataLayout(target_machine->createDataLayout());

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

  pass.run(*context_.llvm_module());
  out_file.flush();

  context_.llvm_module()->dump();
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