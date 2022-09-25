#include "codegen/llvm_expression_visitor.h"

#include <variant>

#include "codegen/cobold_build_context.h"
#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMExpressionVisitor` ==============================================
llvm::Value *LLVMExpressionVisitor::Translate(CoboldBuildContext *context,
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
  assert(context_->functions.contains(expr->identifier()));
  llvm::Function *function = context_->functions[expr->identifier()];
  std::vector<llvm::Value *> args;
  args.reserve(expr->args().size());
  for (const auto &arg : expr->args()) {
    args.push_back(Visit(arg.get()));
  }
  return context_->llvm_builder()->CreateCall(function, args);
}

llvm::Value *LLVMExpressionVisitor::DispatchRange(const RangeExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchArray(const ArrayExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchCast(const CastExpression *expr) {
  assert(expr->expr_type()->type_class() == TypeClass::Integral &&
         expr->cast_type()->type_class() == TypeClass::Integral);
  return context_->llvm_builder()->CreateSExtOrTrunc(
      Visit(expr->expression()),
      LLVMTypeVisitor::Translate(context_, expr->cast_type()));
}

llvm::Value *
LLVMExpressionVisitor::DispatchConstant(const ConstantExpression *expr) {
  if (std::holds_alternative<int64_t>(expr->data())) {
    assert(expr->expr_type()->type_class() == TypeClass::Integral);
    // TODO(jlscheerer) Improve the handling of this...
    return llvm::ConstantInt::get(
        **context_, llvm::APInt(expr->expr_type()->As<IntegralType>()->size(),
                                std::get<int64_t>(expr->data()), true));
  } else if (std::holds_alternative<bool>(expr->data())) {
    assert(expr->expr_type()->type_class() == TypeClass::Bool);
    return llvm::ConstantInt::get(
        **context_, llvm::APInt(1, std::get<bool>(expr->data()), true));
  }
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchIdentifier(const IdentifierExpression *expr) {
  assert(context_->named_vars.find(expr->identifier()) !=
         context_->named_vars.end());
  llvm::AllocaInst *var = context_->named_vars[expr->identifier()];
  return context_->llvm_builder()->CreateLoad(
      LLVMTypeVisitor::Translate(context_, expr->expr_type()), var,
      expr->identifier().c_str());
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
  assert(expr->expression()->type() == ExpressionType::Identifier);
  const std::string &identifier =
      expr->expression()->As<IdentifierExpression>()->identifier();
  assert(context_->functions.contains(identifier));
  llvm::Function *function = context_->functions[identifier];
  std::vector<llvm::Value *> args;
  args.reserve(expr->args().size());
  for (const auto &arg : expr->args()) {
    args.push_back(Visit(arg.get()));
  }
  return context_->llvm_builder()->CreateCall(function, args);
}
// `LLVMExpressionVisitor` ==============================================
} // namespace Cobold