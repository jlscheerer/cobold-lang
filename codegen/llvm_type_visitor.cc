#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMTypeVisitor` ====================================================
llvm::Type *LLVMTypeVisitor::DispatchEmpty() {
  return llvm::Type::getVoidTy(**context_);
}

llvm::Type *LLVMTypeVisitor::DispatchNil(const NilType *type) {
  return llvm::Type::getVoidTy(**context_);
}

llvm::Type *LLVMTypeVisitor::DispatchDash(const DashType *type) {
  assert(false); // we should never be emitting DashType!
}

llvm::Type *LLVMTypeVisitor::DispatchBool(const BoolType *type) {
  return llvm::Type::getIntNTy(**context_, 1);
}

llvm::Type *LLVMTypeVisitor::DispatchChar(const CharType *type) {
  return llvm::Type::getIntNTy(**context_, 8);
}

llvm::Type *LLVMTypeVisitor::DispatchIntegral(const IntegralType *type) {
  return (llvm::Type *)llvm::Type::getIntNTy(**context_, type->size());
}

llvm::Type *LLVMTypeVisitor::DispatchFloating(const FloatingType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchString(const StringType *type) {
  return llvm::StructType::getTypeByName(**context_, "string");
}

llvm::Type *LLVMTypeVisitor::DispatchArray(const ArrayType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchRange(const RangeType *type) {
  assert(false);
}

llvm::Type *LLVMTypeVisitor::DispatchPointer(const PointerType *type) {
  // "Invalid type for pointer element!"
  if (type->underlying_type() == nullptr ||
      type->underlying_type() == NilType::Get()) {
    return llvm::PointerType::get(llvm::Type::getIntNTy(**context_, 8), 0);
  }
  return llvm::PointerType::get(Visit(type->underlying_type()), 0);
}
// `LLVMTypeVisitor` ====================================================
} // namespace Cobold