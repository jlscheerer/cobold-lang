#ifndef COBOLD_VISITOR_EXPRESSION_VISITOR
#define COBOLD_VISITOR_EXPRESSION_VISITOR

#include "core/expression.h"
#include "util/type_traits.h"

namespace Cobold {
template <bool ConstVisit = true, typename RetType = void>
class ExpressionVisitor {
public:
  RetType Visit(enable_const_if_t<ConstVisit, Expression> *expr) {
    if (expr == nullptr)
      return DispatchEmpty();
    switch (expr->type()) {
    case ExpressionType::Ternary:
      return DispatchTernary(expr->template As<TernaryExpression>());
    case ExpressionType::Binary:
      return DispatchBinary(expr->template As<BinaryExpression>());
    case ExpressionType::Unary:
      return DispatchUnary(expr->template As<UnaryExpression>());
    case ExpressionType::Call:
      return DispatchCall(expr->template As<CallExpression>());
    case ExpressionType::Range:
      return DispatchRange(expr->template As<RangeExpression>());
    case ExpressionType::Array:
      return DispatchArray(expr->template As<ArrayExpression>());
    case ExpressionType::Cast:
      return DispatchCast(expr->template As<CastExpression>());
    case ExpressionType::Constant:
      return DispatchConstant(expr->template As<ConstantExpression>());
    case ExpressionType::Identifier:
      return DispatchIdentifier(expr->template As<IdentifierExpression>());
    case ExpressionType::MemberAccess:
      return DispatchMemberAccess(expr->template As<MemberAccessExpression>());
    case ExpressionType::ArrayAccess:
      return DispatchArrayAccess(expr->template As<ArrayAccessExpression>());
    case ExpressionType::CallOp:
      return DispatchCallOp(expr->template As<CallOpExpression>());
    case ExpressionType::Malloc:
      return DispatchMalloc(expr->template As<MallocExpression>());
    case ExpressionType::Sizeof:
      return DispatchSizeof(expr->template As<SizeofExpression>());
    }
  }

protected:
  virtual RetType DispatchEmpty() { assert(false); };
  virtual RetType
  DispatchTernary(enable_const_if_t<ConstVisit, TernaryExpression> *expr) = 0;
  virtual RetType
  DispatchBinary(enable_const_if_t<ConstVisit, BinaryExpression> *expr) = 0;
  virtual RetType
  DispatchUnary(enable_const_if_t<ConstVisit, UnaryExpression> *expr) = 0;
  virtual RetType
  DispatchCall(enable_const_if_t<ConstVisit, CallExpression> *expr) = 0;
  virtual RetType
  DispatchRange(enable_const_if_t<ConstVisit, RangeExpression> *expr) = 0;
  virtual RetType
  DispatchArray(enable_const_if_t<ConstVisit, ArrayExpression> *expr) = 0;
  virtual RetType
  DispatchCast(enable_const_if_t<ConstVisit, CastExpression> *expr) = 0;
  virtual RetType
  DispatchConstant(enable_const_if_t<ConstVisit, ConstantExpression> *expr) = 0;
  virtual RetType DispatchIdentifier(
      enable_const_if_t<ConstVisit, IdentifierExpression> *expr) = 0;
  virtual RetType DispatchMemberAccess(
      enable_const_if_t<ConstVisit, MemberAccessExpression> *expr) = 0;
  virtual RetType DispatchArrayAccess(
      enable_const_if_t<ConstVisit, ArrayAccessExpression> *expr) = 0;
  virtual RetType
  DispatchCallOp(enable_const_if_t<ConstVisit, CallOpExpression> *expr) = 0;
  virtual RetType
  DispatchMalloc(enable_const_if_t<ConstVisit, MallocExpression> *expr) = 0;
  virtual RetType
  DispatchSizeof(enable_const_if_t<ConstVisit, SizeofExpression> *expr) = 0;
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_EXPRESSION_VISITOR */
