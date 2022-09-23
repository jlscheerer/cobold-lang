#include "codegen/llvm_expression_visitor.h"
#include <variant>

namespace Cobold {
// `LLVMExpressionVisitor` ==============================================
llvm::Value *LLVMExpressionVisitor::Translate(llvm::LLVMContext *context,
                                              const Expression *expr) {
  LLVMExpressionVisitor visitor(context);
  return visitor.Visit(expr);
}

llvm::Value *LLVMExpressionVisitor::DispatchEmpty() { assert(false); }

llvm::Value *
LLVMExpressionVisitor::DispatchTernary(const TernaryExpression *expr) {
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchBinary(const BinaryExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchUnary(const UnaryExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchCall(const CallExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchRange(const RangeExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchArray(const ArrayExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchCast(const CastExpression *expr) {
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchConstant(const ConstantExpression *expr) {
  if (std::holds_alternative<int64_t>(expr->data())) {
    // TODO(jlscheerer) Improve the handling of this...
    return llvm::ConstantInt::get(
        *context_, llvm::APInt(32, std::get<int64_t>(expr->data()), true));
  }
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchIdentifier(const IdentifierExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchMemberAccess(
    const MemberAccessExpression *expr) {
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchArrayAccess(const ArrayAccessExpression *expr) {
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchCallOp(const CallOpExpression *expr) {
  assert(false);
}
// `LLVMExpressionVisitor` ==============================================
} // namespace Cobold