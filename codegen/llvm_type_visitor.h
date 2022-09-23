#ifndef COBOLD_CODEGEN_LLVM_TYPES
#define COBOLD_CODEGEN_LLVM_TYPES

#include "visitor/type_visitor.h"

#include "llvm/IR/Type.h"

namespace Cobold {
class LLVMTypeVisitor : private TypeVisitor<llvm::Type *> {
public:
  static llvm::Type *Translate(llvm::LLVMContext *context, const Type *type) {
    LLVMTypeVisitor visitor(context);
    return visitor.Visit(type);
  }

private:
  LLVMTypeVisitor(llvm::LLVMContext *context) : context_(context) {}

  llvm::Type *DispatchEmpty() override;
  llvm::Type *DispatchNil(const NilType *type) override;
  llvm::Type *DispatchIntegral(const IntegralType *type) override;
  llvm::Type *DispatchString(const StringType *type) override;
  llvm::Type *DispatchArray(const ArrayType *type) override;
  llvm::Type *DispatchPointer(const PointerType *type) override;

  llvm::LLVMContext *context_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_TYPES */
