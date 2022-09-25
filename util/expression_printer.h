#ifndef COBOLD_UTIL_EXPRESSION_PRINTER
#define COBOLD_UTIL_EXPRESSION_PRINTER

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "visitor/expression_visitor.h"

namespace Cobold {
class ExpressionPrinter : private ExpressionVisitor<true, void> {
public:
  static std::string Print(const Expression *expr);

private:
  void DispatchEmpty() override;
  void DispatchTernary(const TernaryExpression *expr) override;
  void DispatchBinary(const BinaryExpression *expr) override;
  void DispatchUnary(const UnaryExpression *expr) override;
  void DispatchCall(const CallExpression *expr) override;
  void DispatchRange(const RangeExpression *expr) override;
  void DispatchArray(const ArrayExpression *expr) override;
  void DispatchCast(const CastExpression *expr) override;
  void DispatchConstant(const ConstantExpression *expr) override;
  void DispatchIdentifier(const IdentifierExpression *expr) override;
  void DispatchMemberAccess(const MemberAccessExpression *expr) override;
  void DispatchArrayAccess(const ArrayAccessExpression *expr) override;
  void DispatchCallOp(const CallOpExpression *expr) override;
  void DispatchMalloc(const MallocExpression *expr) override;
  void DispatchSizeof(const SizeofExpression *expr) override;

  void AppendLine(const std::string &line);
  template <typename... Args> void Append(Args &&...args) {
    return AppendLine(absl::StrCat(args...));
  }

  std::string buffer_;
};
} // namespace Cobold

#endif /* COBOLD_UTIL_EXPRESSION_PRINTER */
