#ifndef COBOLD_VISITOR_STATEMENT_VISITOR
#define COBOLD_VISITOR_STATEMENT_VISITOR

#include <memory>

#include "core/statement.h"
#include "util/type_traits.h"

namespace Cobold {
template <bool ConstVisit = true> class StatementVisitor {
public:
  void Visit(enable_const_if_t<ConstVisit, Statement> *stmt) {
    assert(stmt != nullptr);
    StatementType type = stmt->type();
    switch (type) {
    case StatementType::Return:
      return DispatchReturn(stmt->template As<ReturnStatement>());
    case StatementType::Deinit:
      return DispatchDeinit(stmt->template As<DeinitStatement>());
    case StatementType::Assignment:
      return DispatchAssignment(stmt->template As<AssignmentStatement>());
    case StatementType::Compound:
      return DispatchCompound(stmt->template As<CompoundStatement>());
    case StatementType::Expression:
      return DispatchExpression(stmt->template As<ExpressionStatement>());
    case StatementType::If:
      return DispatchIf(stmt->template As<IfStatement>());
    case StatementType::For:
      return DispatchFor(stmt->template As<ForStatement>());
    case StatementType::While:
      return DispatchWhile(stmt->template As<WhileStatement>());
    case StatementType::Declaration:
      return DispatchDeclaration(stmt->template As<DeclarationStatement>());
    case StatementType::Break:
      return DispatchBreak(stmt->template As<BreakStatement>());
    case StatementType::Continue:
      return DispatchContinue(stmt->template As<ContinueStatement>());
    }
  }

protected:
  virtual void
  DispatchReturn(enable_const_if_t<ConstVisit, ReturnStatement> *stmt) {}
  virtual void
  DispatchDeinit(enable_const_if_t<ConstVisit, DeinitStatement> *stmt) {}
  virtual void
  DispatchAssignment(enable_const_if_t<ConstVisit, AssignmentStatement> *stmt) {
  }
  virtual void
  DispatchCompound(enable_const_if_t<ConstVisit, CompoundStatement> *stmt) {}
  virtual void
  DispatchExpression(enable_const_if_t<ConstVisit, ExpressionStatement> *stmt) {
  }
  virtual void DispatchIf(enable_const_if_t<ConstVisit, IfStatement> *stmt) {}
  virtual void DispatchFor(enable_const_if_t<ConstVisit, ForStatement> *stmt) {}
  virtual void
  DispatchWhile(enable_const_if_t<ConstVisit, WhileStatement> *stmt) {}
  virtual void DispatchDeclaration(
      enable_const_if_t<ConstVisit, DeclarationStatement> *stmt) {}
  virtual void
  DispatchBreak(enable_const_if_t<ConstVisit, BreakStatement> *stmt){};
  virtual void
  DispatchContinue(enable_const_if_t<ConstVisit, ContinueStatement> *stmt){};
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_STATEMENT_VISITOR */
