#ifndef COBOLD_UTIL_STATEMENT_PRINTER
#define COBOLD_UTIL_STATEMENT_PRINTER

#include <utility>

#include "absl/strings/str_cat.h"
#include "visitor/statement_visitor.h"

namespace Cobold {
class StatementPrinter : private StatementVisitor<true> {
public:
  static std::string Print(const Statement *stmt);

private:
  void DispatchReturn(const ReturnStatement *stmt) override;
  void DispatchAssignment(const AssignmentStatement *stmt) override;
  void DispatchCompound(const CompoundStatement *stmt) override;
  void DispatchExpression(const ExpressionStatement *stmt) override;
  void DispatchIf(const IfStatement *stmt) override;
  void DispatchFor(const ForStatement *stmt) override;
  void DispatchWhile(const WhileStatement *stmt) override;
  void DispatchDeclaration(const DeclarationStatement *stmt) override;
  void AppendLineIndented(const std::string &line);
  template <typename... Args> void AppendIndented(Args &&...args) {
    return AppendLineIndented(absl::StrCat(args...));
  }

  int indent_ = 0;
  std::string buffer_;
};
} // namespace Cobold

#endif /* COBOLD_UTIL_STATEMENT_PRINTER */
