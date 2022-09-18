#ifndef COBOLD_VISITOR_EXPRESSION_VISITOR
#define COBOLD_VISITOR_EXPRESSION_VISITOR

#include "core/expression.h"

namespace Cobold {
class ExpressionVisitor {
public:
  void Visit(const Expression *expr);

protected:
  virtual void DispatchEmpty() { assert(false); };
  virtual void DispatchTernary(const TernaryExpression *expr) = 0;
  virtual void DispatchBinary(const BinaryExpression *expr) = 0;
  virtual void DispatchUnary(const UnaryExpression *expr) = 0;
  virtual void DispatchCall(const CallExpression *expr) = 0;
  virtual void DispatchRange(const RangeExpression *expr) = 0;
  virtual void DispatchArray(const ArrayExpression *expr) = 0;
  virtual void DispatchCast(const CastExpression *expr) = 0;
  virtual void DispatchConstant(const ConstantExpression *expr) = 0;
  virtual void DispatchIdentifier(const IdentifierExpression *expr) = 0;
  virtual void DispatchMemberAccess(const MemberAccessExpression *expr) = 0;
  virtual void DispatchArrayAccess(const ArrayAccessExpression *expr) = 0;
  virtual void DispatchCallOp(const CallOpExpression *expr) = 0;
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_EXPRESSION_VISITOR */
