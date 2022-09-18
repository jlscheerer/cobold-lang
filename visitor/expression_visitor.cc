#include "visitor/expression_visitor.h"

namespace Cobold {
// `ExpressionVisitor` ==================================================
void ExpressionVisitor::Visit(const Expression *expr) {
  if (expr == nullptr)
    return DispatchEmpty();
  switch (expr->type()) {
  case ExpressionType::Ternary:
    return DispatchTernary(expr->As<TernaryExpression>());
  case ExpressionType::Binary:
    return DispatchBinary(expr->As<BinaryExpression>());
  case ExpressionType::Unary:
    return DispatchUnary(expr->As<UnaryExpression>());
  case ExpressionType::Call:
    return DispatchCall(expr->As<CallExpression>());
  case ExpressionType::Range:
    return DispatchRange(expr->As<RangeExpression>());
  case ExpressionType::Array:
    return DispatchArray(expr->As<ArrayExpression>());
  case ExpressionType::Cast:
    return DispatchCast(expr->As<CastExpression>());
  case ExpressionType::Constant:
    return DispatchConstant(expr->As<ConstantExpression>());
  case ExpressionType::Identifier:
    return DispatchIdentifier(expr->As<IdentifierExpression>());
  case ExpressionType::MemberAccess:
    return DispatchMemberAccess(expr->As<MemberAccessExpression>());
  case ExpressionType::ArrayAccess:
    return DispatchArrayAccess(expr->As<ArrayAccessExpression>());
  case ExpressionType::CallOp:
    return DispatchCallOp(expr->As<CallOpExpression>());
  }
}
// `ExpressionVisitor` ==================================================
} // namespace Cobold