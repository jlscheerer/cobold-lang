#ifndef COBOLD_CODEGEN_LLVM_STATEMENT_VISITOR
#define COBOLD_CODEGEN_LLVM_STATEMENT_VISITOR

#include "visitor/statement_visitor.h"

#include "cobold_build_context.h"

namespace Cobold {
class LLVMStatementVisitor : private StatementVisitor<true> {
public:
  static void Translate(CoboldBuildContext *context, const Statement *stmt) {
    LLVMStatementVisitor visitor(context);
    return visitor.Visit(stmt);
  }

  static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *function,
                                                  const std::string &var_name,
                                                  llvm::Type *type);

private:
  LLVMStatementVisitor(CoboldBuildContext *context) : context_(context) {}

  void DispatchReturn(const ReturnStatement *stmt) override;
  void DispatchAssignment(const AssignmentStatement *stmt) override;
  void DispatchCompound(const CompoundStatement *stmt) override;
  void DispatchExpression(const ExpressionStatement *stmt) override;
  void DispatchIf(const IfStatement *stmt) override;
  void DispatchFor(const ForStatement *stmt) override;
  void DispatchWhile(const WhileStatement *stmt) override;
  void DispatchDeclaration(const DeclarationStatement *stmt) override;
  void DispatchBreak(const BreakStatement *stmt) override;
  void DispatchContinue(const ContinueStatement *stmt) override;

  CoboldBuildContext *context_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_STATEMENT_VISITOR */
