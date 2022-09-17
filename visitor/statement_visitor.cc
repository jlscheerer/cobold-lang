#include "visitor/statement_visitor.h"

namespace Cobold {
// `StatementVisitor` ===================================================
void StatementVisitor::Visit(const Statement *stmt) {
  assert(stmt != nullptr);
  StatementType type = stmt->type();
  switch (type) {
  case StatementType::Return:
    return DispatchReturn(stmt->As<ReturnStatement>());
  case StatementType::Assignment:
    return DispatchAssignment(stmt->As<AssignmentStatement>());
  case StatementType::Compound:
    return DispatchCompound(stmt->As<CompoundStatement>());
  case StatementType::Expression:
    return DispatchExpression(stmt->As<ExpressionStatement>());
  case StatementType::If:
    return DispatchIf(stmt->As<IfStatement>());
  case StatementType::For:
    return DispatchFor(stmt->As<ForStatement>());
  case StatementType::While:
    return DispatchWhile(stmt->As<WhileStatement>());
  case StatementType::Declaration:
    return DispatchDeclaration(stmt->As<DeclarationStatement>());
  }
}
// `StatementVisitor` ===================================================
} // namespace Cobold