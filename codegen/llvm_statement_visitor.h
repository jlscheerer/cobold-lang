#ifndef COBOLD_CODEGEN_LLVM_STATEMENT_VISITOR
#define COBOLD_CODEGEN_LLVM_STATEMENT_VISITOR

#include "visitor/statement_visitor.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace Cobold {
class LLVMStatementVisitor : private StatementVisitor {
public:
  static void Translate(llvm::LLVMContext *context, llvm::Module *module,
                        llvm::IRBuilder<> *builder, const Statement *stmt) {
    LLVMStatementVisitor visitor(context, module, builder);
    return visitor.Visit(stmt);
  }

private:
  LLVMStatementVisitor(llvm::LLVMContext *context, llvm::Module *module,
                       llvm::IRBuilder<> *builder)
      : context_(context), module_(module), builder_(builder) {}

  void DispatchReturn(const ReturnStatement *stmt) override;
  void DispatchAssignment(const AssignmentStatement *stmt) override;
  void DispatchCompound(const CompoundStatement *stmt) override;
  void DispatchExpression(const ExpressionStatement *stmt) override;
  void DispatchIf(const IfStatement *stmt) override;
  void DispatchFor(const ForStatement *stmt) override;
  void DispatchWhile(const WhileStatement *stmt) override;
  void DispatchDeclaration(const DeclarationStatement *stmt) override;

  llvm::LLVMContext *context_;
  llvm::Module *module_;
  llvm::IRBuilder<> *builder_;
};
} // namespace Cobold

#endif /* COBOLD_CODEGEN_LLVM_STATEMENT_VISITOR */
