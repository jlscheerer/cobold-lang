#ifndef COBOLD_CODEGEN_LLVM_TYPES
#define COBOLD_CODEGEN_LLVM_TYPES

#include "visitor/type_visitor.h"

#include "cobold_build_context.h"

#include "llvm/IR/Type.h"

namespace Cobold {
class LLVMTypeVisitor : private TypeVisitor<llvm::Type *> {
public:
  static llvm::Type *Translate(CoboldBuildContext *context, const Type *type) {
    LLVMTypeVisitor visitor(context);
    return visitor.Visit(type);
  }

private:
  LLVMTypeVisitor(CoboldBuildContext *context) : context_(context) {}

  llvm::Type *DispatchEmpty() override;
  llvm::Type *DispatchNil(const NilType *type) override;
  llvm::Type *DispatchDash(const DashType *type) override;
  llvm::Type *DispatchBool(const BoolType *type) override;
  llvm::Type *DispatchChar(const CharType *type) override;
  llvm::Type *DispatchIntegral(const IntegralType *type) override;
  llvm::Type *DispatchFloating(const FloatingType *type) override;
  llvm::Type *DispatchString(const StringType *type) override;
  llvm::Type *DispatchArray(const ArrayType *type) override;
  llvm::Type *DispatchRange(const RangeType *type) override;
  llvm::Type *DispatchPointer(const PointerType *type) override;

  CoboldBuildContext *context_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_TYPES */
