#include "codegen/llvm_statement_visitor.h"

#include "codegen/llvm_expression_visitor.h"
#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMStatementVisitor` ===============================================
void LLVMStatementVisitor::DispatchReturn(const ReturnStatement *stmt) {
  llvm::Value *ret_value =
      LLVMExpressionVisitor::Translate(context_, stmt->expression());
  context_->llvm_builder()->CreateRet(ret_value);
}

void LLVMStatementVisitor::DispatchAssignment(const AssignmentStatement *stmt) {
  assert(stmt->assgn_type() == AssignmentType::EQ);
  assert(stmt->lhs()->type() == ExpressionType::Identifier);
  std::string identifier =
      stmt->lhs()->As<IdentifierExpression>()->identifier();
  assert(context_->named_vars.contains(identifier));

  llvm::AllocaInst *alloca = context_->named_vars[identifier];
  llvm::Value *value = LLVMExpressionVisitor::Translate(context_, stmt->rhs());
  context_->llvm_builder()->CreateStore(value, alloca);
}

void LLVMStatementVisitor::DispatchCompound(const CompoundStatement *stmt) {
  for (const auto &child : stmt->statements())
    Visit(child.get());
}

void LLVMStatementVisitor::DispatchExpression(const ExpressionStatement *stmt) {
  // TODO(jlscheerer) check that expression has non-void value
  // TODO(jlscheerer) This assumes that the return value is void!
  LLVMExpressionVisitor::Translate(context_, stmt->expression());
  // context_->llvm_builder()->Insert(
  //     LLVMExpressionVisitor::Translate(context_, stmt->expression()));
}

void LLVMStatementVisitor::DispatchIf(const IfStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchFor(const ForStatement *stmt) {
  llvm::Function *function =
      context_->llvm_builder()->GetInsertBlock()->getParent();

  llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
      context_->llvm_builder()->GetInsertBlock()->getParent(),
      stmt->identifier(),
      LLVMTypeVisitor::Translate(context_, stmt->decl_type()));

  context_->named_vars[stmt->identifier()] = alloca;

  // TODO(jlscheerer) Generalize this.
  assert(stmt->expression()->expr_type()->type_class() == TypeClass::Range);

  const RangeExpression *range = stmt->expression()->As<RangeExpression>();

  // TODO(jlscheerer) Supported unbounded sides.
  assert(range->lhs() && range->rhs());
  llvm::Value *start = LLVMExpressionVisitor::Translate(context_, range->lhs());
  context_->llvm_builder()->CreateStore(start, alloca);

  // New basic block for the loop header, inserting after current block.
  llvm::BasicBlock *loop =
      llvm::BasicBlock::Create(**context_, "loop", function);

  // Insert an explicit fall through from the current block to the LoopBB.
  context_->llvm_builder()->CreateBr(loop);

  // Start insertion in LoopBB.
  context_->llvm_builder()->SetInsertPoint(loop);

  // TODO(jlscheerer) Handle shadowing of the loop variable
  Visit(stmt->body().get());

  // TODO(jlscheerer) Assumes that this is a i64 for the addition.
  llvm::Value *step_increment = llvm::ConstantInt::get(
      **context_,
      llvm::APInt(64, 1)); // TODO(jlscheerer) should probably be a APSInt

  // Compute the end condition.
  llvm::Value *end = LLVMExpressionVisitor::Translate(context_, range->rhs());
  std::cout << range->rhs()->expr_type()->DebugString() << std::endl;
  // *alloca > end
  llvm::Value *end_cond = context_->llvm_builder()->CreateICmpUGT(
      end, context_->llvm_builder()->CreateLoad(
               alloca->getAllocatedType(), alloca, stmt->identifier().c_str()));

  // Reload, increment, and restore the alloca.  This handles the case where
  // the body of the loop mutates the variable.
  llvm::Value *current = context_->llvm_builder()->CreateLoad(
      alloca->getAllocatedType(), alloca, stmt->identifier().c_str());
  llvm::Value *next = context_->llvm_builder()->CreateAdd(
      current, step_increment, "next_loop_var");
  context_->llvm_builder()->CreateStore(next, alloca);

  // Create the "after loop" block and insert it.
  llvm::BasicBlock *after_loop =
      llvm::BasicBlock::Create(**context_, "after_loop", function);

  // Insert the conditional branch into the end of LoopEndBB.
  context_->llvm_builder()->CreateCondBr(end_cond, loop, after_loop);

  // Any new code will be inserted in AfterBB.
  context_->llvm_builder()->SetInsertPoint(after_loop);
}

void LLVMStatementVisitor::DispatchWhile(const WhileStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchDeclaration(
    const DeclarationStatement *stmt) {
  llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
      context_->llvm_builder()->GetInsertBlock()->getParent(),
      stmt->identifier(),
      LLVMTypeVisitor::Translate(context_, stmt->decl_type()));

  // TODO(jlscheerer) Clean this up.
  assert(context_->named_vars.find(stmt->identifier()) ==
         context_->named_vars.end());

  if (stmt->expression() != nullptr) {
    context_->llvm_builder()->CreateStore(
        LLVMExpressionVisitor::Translate(context_, stmt->expression()), alloca);
  }

  context_->named_vars[stmt->identifier()] = alloca;
}

llvm::AllocaInst *LLVMStatementVisitor::CreateEntryBlockAlloca(
    llvm::Function *function, const std::string &var_name, llvm::Type *type) {
  llvm::IRBuilder<> tmp_builder(&function->getEntryBlock(),
                                function->getEntryBlock().begin());
  return tmp_builder.CreateAlloca(type, 0, var_name);
}
// `LLVMStatementVisitor` ===============================================
} // namespace Cobold