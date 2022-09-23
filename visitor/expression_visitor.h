#ifndef COBOLD_VISITOR_EXPRESSION_VISITOR
#define COBOLD_VISITOR_EXPRESSION_VISITOR

#include "core/expression.h"

namespace Cobold {
template <typename RetType = void> class ExpressionVisitor {
public:
  RetType Visit(const Expression *expr) {
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

protected:
  virtual RetType DispatchEmpty() { assert(false); };
  virtual RetType DispatchTernary(const TernaryExpression *expr) = 0;
  virtual RetType DispatchBinary(const BinaryExpression *expr) = 0;
  virtual RetType DispatchUnary(const UnaryExpression *expr) = 0;
  virtual RetType DispatchCall(const CallExpression *expr) = 0;
  virtual RetType DispatchRange(const RangeExpression *expr) = 0;
  virtual RetType DispatchArray(const ArrayExpression *expr) = 0;
  virtual RetType DispatchCast(const CastExpression *expr) = 0;
  virtual RetType DispatchConstant(const ConstantExpression *expr) = 0;
  virtual RetType DispatchIdentifier(const IdentifierExpression *expr) = 0;
  virtual RetType DispatchMemberAccess(const MemberAccessExpression *expr) = 0;
  virtual RetType DispatchArrayAccess(const ArrayAccessExpression *expr) = 0;
  virtual RetType DispatchCallOp(const CallOpExpression *expr) = 0;
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_EXPRESSION_VISITOR */
