#ifndef COBOLD_VISITOR_STATEMENT_VISITOR
#define COBOLD_VISITOR_STATEMENT_VISITOR

#include <memory>

#include "core/statement.h"

namespace Cobold {
class StatementVisitor {
public:
  void Visit(const Statement *stmt);

protected:
  virtual void DispatchReturn(const ReturnStatement *stmt) {}
  virtual void DispatchAssignment(const AssignmentStatement *stmt) {}
  virtual void DispatchCompound(const CompoundStatement *stmt) {}
  virtual void DispatchExpression(const ExpressionStatement *stmt) {}
  virtual void DispatchIf(const IfStatement *stmt) {}
  virtual void DispatchFor(const ForStatement *stmt) {}
  virtual void DispatchWhile(const WhileStatement *stmt) {}
  virtual void DispatchDeclaration(const DeclarationStatement *stmt) {}
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_STATEMENT_VISITOR */
