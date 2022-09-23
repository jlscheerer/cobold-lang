#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMTypeVisitor` ====================================================
llvm::Type *LLVMTypeVisitor::DispatchEmpty() {
  return llvm::Type::getVoidTy(*context_);
}

llvm::Type *LLVMTypeVisitor::DispatchNil(const NilType *type) {
  return llvm::Type::getVoidTy(*context_);
}

llvm::Type *LLVMTypeVisitor::DispatchIntegral(const IntegralType *type) {
  return (llvm::Type *)llvm::Type::getIntNTy(*context_, type->size());
}

llvm::Type *LLVMTypeVisitor::DispatchString(const StringType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchArray(const ArrayType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchPointer(const PointerType *type) {
  assert(false);
}
// `LLVMTypeVisitor` ====================================================
} // namespace Cobold