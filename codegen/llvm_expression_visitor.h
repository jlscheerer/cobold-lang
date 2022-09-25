#ifndef COBOLD_CODEGEN_LLVM_EXPRESSION_VISITOR
#define COBOLD_CODEGEN_LLVM_EXPRESSION_VISITOR

#include <string>

#include "codegen/cobold_build_context.h"
#include "core/expression.h"
#include "visitor/expression_visitor.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"

namespace Cobold {
class LLVMExpressionVisitor : private ExpressionVisitor<true, llvm::Value *> {
public:
  static llvm::Value *Translate(CoboldBuildContext *context,
                                const Expression *expr);

private:
  LLVMExpressionVisitor(CoboldBuildContext *context) : context_(context) {}

  llvm::Value *DispatchEmpty() override;
  llvm::Value *DispatchTernary(const TernaryExpression *expr) override;
  llvm::Value *DispatchBinary(const BinaryExpression *expr) override;
  llvm::Value *DispatchUnary(const UnaryExpression *expr) override;
  llvm::Value *DispatchCall(const CallExpression *expr) override;
  llvm::Value *DispatchRange(const RangeExpression *expr) override;
  llvm::Value *DispatchArray(const ArrayExpression *expr) override;
  llvm::Value *DispatchCast(const CastExpression *expr) override;
  llvm::Value *DispatchConstant(const ConstantExpression *expr) override;
  llvm::Value *DispatchIdentifier(const IdentifierExpression *expr) override;
  llvm::Value *
  DispatchMemberAccess(const MemberAccessExpression *expr) override;
  llvm::Value *DispatchArrayAccess(const ArrayAccessExpression *expr) override;
  llvm::Value *DispatchCallOp(const CallOpExpression *expr) override;

  llvm::Value *IntegralBinaryExpression(BinaryExpressionType op_type,
                                        llvm::Value *lhs, llvm::Value *rhs);

  CoboldBuildContext *context_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_EXPRESSION_VISITOR */
