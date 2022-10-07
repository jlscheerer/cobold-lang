#include "util/statement_printer.h"

#include "absl/strings/str_cat.h"
#include "util/expression_printer.h"

namespace Cobold {
// `StatementPrinter` ===================================================
std::string StatementPrinter::Print(const Statement *stmt) {
  StatementPrinter printer;
  printer.Visit(stmt);
  return printer.buffer_;
}

void StatementPrinter::DispatchReturn(const ReturnStatement *stmt) {
  AppendIndented("return ", ExpressionPrinter::Print(stmt->expression()), ";");
}

void StatementPrinter::DispatchDeinit(const DeinitStatement *stmt) {
  AppendIndented("deinit ", ExpressionPrinter::Print(stmt->expression()), ";");
}

void StatementPrinter::DispatchAssignment(const AssignmentStatement *stmt) {
  AppendIndented(ExpressionPrinter::Print(stmt->lhs()), " ",
                 AssignmentStatement::TypeToString(stmt->assgn_type()), " ",
                 ExpressionPrinter::Print(stmt->rhs()));
}

void StatementPrinter::DispatchCompound(const CompoundStatement *stmt) {
  AppendIndented("{");
  ++indent_;
  for (const auto &child : stmt->statements()) {
    Visit(child.get());
  }
  --indent_;
  AppendIndented("}");
}

void StatementPrinter::DispatchExpression(const ExpressionStatement *stmt) {
  AppendIndented(ExpressionPrinter::Print(stmt->expression()), ";");
}

void StatementPrinter::DispatchIf(const IfStatement *stmt) {
  const int n = stmt->branches().size();
  for (int i = 0; i < n; ++i) {
    AppendIndented(i != 0 ? "else " : "", i != n - 1 ? "if " : "",
                   i != n - 1 ? ExpressionPrinter::Print(
                                    stmt->branches()[i].condition.get())
                              : "");
    Visit(stmt->branches()[i].body.get());
  }
}

void StatementPrinter::DispatchFor(const ForStatement *stmt) {
  AppendIndented("for ", stmt->identifier(), ": ",
                 stmt->decl_type() ? stmt->decl_type()->DebugString() : "<?>",
                 " in ", ExpressionPrinter::Print(stmt->expression()));
  Visit(stmt->body().get());
}

void StatementPrinter::DispatchWhile(const WhileStatement *stmt) {
  AppendIndented("while ", ExpressionPrinter::Print(stmt->condition()));
  Visit(stmt->body().get());
}

void StatementPrinter::DispatchDeclaration(const DeclarationStatement *stmt) {
  AppendIndented(stmt->is_const() ? "let" : "var", " ", stmt->identifier(),
                 ": ",
                 stmt->decl_type() ? stmt->decl_type()->DebugString() : "<?>",
                 " = ", ExpressionPrinter::Print(stmt->expression()), ";");
}

void StatementPrinter::DispatchBreak(const BreakStatement *stmt) {
  AppendIndented("break;");
}

void StatementPrinter::DispatchContinue(const ContinueStatement *stmt) {
  AppendIndented("continue;");
}

void StatementPrinter::AppendLineIndented(const std::string &line) {
  buffer_ = absl::StrCat(buffer_, std::string(indent_, ' '), line, "\n");
}
// `StatementPrinter` ===================================================
} // namespace Cobold