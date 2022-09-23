#include "codegen/llvm_statement_visitor.h"

#include "codegen/llvm_expression_visitor.h"

namespace Cobold {
// `LLVMStatementVisitor` ===============================================
void LLVMStatementVisitor::DispatchReturn(const ReturnStatement *stmt) {
  llvm::Value *ret_value =
      LLVMExpressionVisitor::Translate(context_, stmt->expression());
  builder_->CreateRet(ret_value);
}

void LLVMStatementVisitor::DispatchAssignment(const AssignmentStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchCompound(const CompoundStatement *stmt) {
  for (const auto &child : stmt->statements())
    Visit(child.get());
}

void LLVMStatementVisitor::DispatchExpression(const ExpressionStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchIf(const IfStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchFor(const ForStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchWhile(const WhileStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchDeclaration(
    const DeclarationStatement *stmt) {
  assert(false);
}
// `LLVMStatementVisitor` ===============================================
} // namespace Cobold