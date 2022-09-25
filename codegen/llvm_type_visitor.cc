#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMTypeVisitor` ====================================================
llvm::Type *LLVMTypeVisitor::DispatchEmpty() {
  return llvm::Type::getVoidTy(**context_);
}

llvm::Type *LLVMTypeVisitor::DispatchNil(const NilType *type) {
  return llvm::Type::getVoidTy(**context_);
}

llvm::Type *LLVMTypeVisitor::DispatchBool(const BoolType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchChar(const CharType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchIntegral(const IntegralType *type) {
  return (llvm::Type *)llvm::Type::getIntNTy(**context_, type->size());
}

llvm::Type *LLVMTypeVisitor::DispatchFloating(const FloatingType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchString(const StringType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchArray(const ArrayType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchRange(const RangeType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchPointer(const PointerType *type) {
  assert(false);
}
// `LLVMTypeVisitor` ====================================================
} // namespace Cobold