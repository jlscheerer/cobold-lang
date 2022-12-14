#include "codegen/llvm_statement_visitor.h"

#include "codegen/build_context.h"
#include "codegen/llvm_expression_visitor.h"
#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMStatementVisitor` ===============================================
void LLVMStatementVisitor::DispatchReturn(const ReturnStatement *stmt) {
  llvm::Value *ret_value =
      LLVMExpressionVisitor::Translate(context_, stmt->expression());
  context_->llvm_builder()->CreateRet(ret_value);
}

void LLVMStatementVisitor::DispatchDeinit(const DeinitStatement *stmt) {
  assert(false);
}

void LLVMStatementVisitor::DispatchAssignment(const AssignmentStatement *stmt) {
  assert(stmt->assgn_type() == AssignmentType::EQ);
  if (stmt->lhs()->type() == ExpressionType::Identifier) {
    std::string identifier =
        stmt->lhs()->As<IdentifierExpression>()->identifier();
    assert(context_->HasNamedVar(identifier));

    llvm::AllocaInst *alloca = context_->AllocaForNamedVar(identifier);
    llvm::Value *value =
        LLVMExpressionVisitor::Translate(context_, stmt->rhs());
    context_->llvm_builder()->CreateStore(value, alloca);
  } else if (stmt->lhs()->type() == ExpressionType::Unary &&
             stmt->lhs()->As<UnaryExpression>()->op_type() ==
                 UnaryExpressionType::DEREFERENCE) {
    llvm::Value *value =
        LLVMExpressionVisitor::Translate(context_, stmt->rhs());
    context_->llvm_builder()->CreateStore(
        value, LLVMExpressionVisitor::Translate(
                   context_, stmt->lhs()->As<UnaryExpression>()->expression()));
  }
}

void LLVMStatementVisitor::DispatchCompound(const CompoundStatement *stmt) {
  // TODO(jlscheerer) Create a "compilation" scope similar to the type_context
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
  llvm::Function *function =
      context_->llvm_builder()->GetInsertBlock()->getParent();

  const std::vector<IfBranch> &if_branches = stmt->branches();
  const int num_branches = if_branches.size();

  // TODO(jlscheerer) Rename these to be more uniform (i.e., prelude, ...)
  std::vector<llvm::BasicBlock *> condition_bbs, body_bbs;
  condition_bbs.reserve(num_branches), body_bbs.reserve(num_branches);
  for (int i = 0; i < num_branches; ++i) {
    condition_bbs.push_back(
        llvm::BasicBlock::Create(**context_, "if-condition", function));
    body_bbs.push_back(
        llvm::BasicBlock::Create(**context_, "if-body", function));
  }

  llvm::BasicBlock *after_if =
      llvm::BasicBlock::Create(**context_, "after-if", function);

  context_->llvm_builder()->CreateBr(condition_bbs[0]);
  for (int i = 0; i < num_branches; ++i) {
    context_->llvm_builder()->SetInsertPoint(condition_bbs[i]);
    llvm::Value *condition = LLVMExpressionVisitor::Translate(
        context_, stmt->branches()[i].condition.get());
    context_->llvm_builder()->CreateCondBr(
        condition, body_bbs[i],
        i != num_branches - 1 ? condition_bbs[i + 1] : after_if);

    context_->llvm_builder()->SetInsertPoint(body_bbs[i]);
    Visit(stmt->branches()[i].body.get());
    context_->llvm_builder()->CreateBr(after_if);
  }

  context_->llvm_builder()->SetInsertPoint(after_if);
}

void LLVMStatementVisitor::DispatchFor(const ForStatement *stmt) {
  llvm::Function *function =
      context_->llvm_builder()->GetInsertBlock()->getParent();

  llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
      context_->llvm_builder()->GetInsertBlock()->getParent(),
      stmt->identifier(),
      LLVMTypeVisitor::Translate(context_, stmt->decl_type()));

  context_->PutNamedVar(stmt->identifier(), alloca);

  // TODO(jlscheerer) Generalize this.
  assert(stmt->expression()->expr_type()->type_class() == TypeClass::Range);

  const RangeExpression *range = stmt->expression()->As<RangeExpression>();
  // TODO(jlscheerer) Supported unbounded sides.
  assert(range->lhs() && range->rhs());

  // TODO(jlscheerer) Reformat to include prelude / postlude (for increment)
  llvm::BasicBlock *loop_condition =
      llvm::BasicBlock::Create(**context_, "loop_condition", function);

  llvm::BasicBlock *loop_body =
      llvm::BasicBlock::Create(**context_, "loop_body", function);

  // this is referenced by potential `continue`s
  llvm::BasicBlock *loop_increment =
      llvm::BasicBlock::Create(**context_, "loop_increment", function);

  llvm::BasicBlock *after_loop =
      llvm::BasicBlock::Create(**context_, "after_loop", function);

  context_->loop_instruction_stack().push(LoopInstructionBlock{
      .break_bb = after_loop, .continue_bb = loop_increment});

  // TODO(jlscheerer) Generalize this for different "iterators"

  // Prepare for the loop (i.e., init start)
  llvm::Value *start = LLVMExpressionVisitor::Translate(context_, range->lhs());
  context_->llvm_builder()->CreateStore(start, alloca);

  // Insert explicit fall-through from the current block to the loop
  context_->llvm_builder()->CreateBr(loop_condition);

  // Compute the end condition.
  context_->llvm_builder()->SetInsertPoint(loop_condition);
  llvm::Value *end = LLVMExpressionVisitor::Translate(context_, range->rhs());
  // *alloca >= end ?
  llvm::Value *end_cond = context_->llvm_builder()->CreateICmpUGE(
      end, context_->llvm_builder()->CreateLoad(
               alloca->getAllocatedType(), alloca, stmt->identifier().c_str()));

  // Branch based on the computed condition
  context_->llvm_builder()->CreateCondBr(end_cond, loop_body, after_loop);

  context_->llvm_builder()->SetInsertPoint(loop_body);

  // TODO(jlscheerer) Handle shadowing of the loop variable
  Visit(stmt->body().get());

  context_->llvm_builder()->CreateBr(loop_increment);

  context_->llvm_builder()->SetInsertPoint(loop_increment);
  // Loop Increment
  // TODO(jlscheerer) Assumes that this is a i64 for the addition.
  llvm::Value *step_increment = llvm::ConstantInt::get(
      **context_,
      llvm::APInt(64, 1)); // TODO(jlscheerer) should probably be a APSInt

  // Reload, increment, and restore the alloca.  This handles the case where
  // the body of the loop mutates the variable.
  llvm::Value *current = context_->llvm_builder()->CreateLoad(
      alloca->getAllocatedType(), alloca, stmt->identifier().c_str());
  llvm::Value *next = context_->llvm_builder()->CreateAdd(
      current, step_increment, "next_loop_var");
  context_->llvm_builder()->CreateStore(next, alloca);

  // Insert the conditional branch into the end of LoopEndBB.
  context_->llvm_builder()->CreateBr(loop_condition);

  context_->loop_instruction_stack().pop();

  // Any new code will be inserted in AfterBB.
  context_->llvm_builder()->SetInsertPoint(after_loop);
}

void LLVMStatementVisitor::DispatchWhile(const WhileStatement *stmt) {
  llvm::Function *function =
      context_->llvm_builder()->GetInsertBlock()->getParent();

  // TODO(jlscheerer) Rename these to be more uniform (i.e., prelude, ...)
  llvm::BasicBlock *while_condition =
      llvm::BasicBlock::Create(**context_, "while-condition", function);

  llvm::BasicBlock *while_body =
      llvm::BasicBlock::Create(**context_, "while-body", function);

  llvm::BasicBlock *after_while =
      llvm::BasicBlock::Create(**context_, "after-while", function);

  context_->loop_instruction_stack().push(LoopInstructionBlock{
      .break_bb = after_while, .continue_bb = while_condition});

  context_->llvm_builder()->CreateBr(while_condition);

  context_->llvm_builder()->SetInsertPoint(while_condition);
  llvm::Value *condition =
      LLVMExpressionVisitor::Translate(context_, stmt->condition());
  context_->llvm_builder()->CreateCondBr(condition, while_body, after_while);

  context_->llvm_builder()->SetInsertPoint(while_body);
  Visit(stmt->body().get());
  context_->llvm_builder()->CreateBr(while_condition);

  context_->loop_instruction_stack().pop();

  context_->llvm_builder()->SetInsertPoint(after_while);
}

void LLVMStatementVisitor::DispatchDeclaration(
    const DeclarationStatement *stmt) {
  llvm::AllocaInst *alloca = CreateEntryBlockAlloca(
      context_->llvm_builder()->GetInsertBlock()->getParent(),
      stmt->identifier(),
      LLVMTypeVisitor::Translate(context_, stmt->decl_type()));

  // TODO(jlscheerer) Clean this up.
  assert(!context_->HasNamedVar(stmt->identifier()));

  if (stmt->expression()->expr_type()->type_class() == TypeClass::Dash) {
    // we only need to do initialization for "complex" types here.
    // TODO(jlscheerer) move these semantics into the type.
    if (stmt->decl_type()->type_class() == TypeClass::String) {
      context_->llvm_builder()->CreateStore(
          llvm::ConstantAggregateZero::get(
              LLVMTypeVisitor::Translate(context_, stmt->decl_type())),
          alloca);
    }
  } else {
    context_->llvm_builder()->CreateStore(
        LLVMExpressionVisitor::Translate(context_, stmt->expression()), alloca);
  }

  context_->PutNamedVar(stmt->identifier(), alloca);
}

void LLVMStatementVisitor::DispatchBreak(const BreakStatement *stmt) {
  assert(context_->loop_instruction_stack().size() > 0);
  llvm::Function *function =
      context_->llvm_builder()->GetInsertBlock()->getParent();

  // TODO(jlscheerer) Rename these to be more uniform (i.e., prelude, ...)
  llvm::BasicBlock *break_block =
      llvm::BasicBlock::Create(**context_, "break-block", function);
  llvm::BasicBlock *after_break =
      llvm::BasicBlock::Create(**context_, "after-break", function);

  // Wrap the break in a separate BasicBlock
  context_->llvm_builder()->CreateBr(break_block);
  context_->llvm_builder()->SetInsertPoint(break_block);
  context_->llvm_builder()->CreateBr(
      context_->loop_instruction_stack().top().break_bb);

  context_->llvm_builder()->SetInsertPoint(after_break);
}

void LLVMStatementVisitor::DispatchContinue(const ContinueStatement *stmt) {
  assert(context_->loop_instruction_stack().size() > 0);
  llvm::Function *function =
      context_->llvm_builder()->GetInsertBlock()->getParent();

  // TODO(jlscheerer) Rename these to be more uniform (i.e., prelude, ...)
  llvm::BasicBlock *continue_block =
      llvm::BasicBlock::Create(**context_, "continue-block", function);
  llvm::BasicBlock *after_continue =
      llvm::BasicBlock::Create(**context_, "after-continue", function);

  // Wrap the continue in a separate BasicBlock
  context_->llvm_builder()->CreateBr(continue_block);
  context_->llvm_builder()->SetInsertPoint(continue_block);
  context_->llvm_builder()->CreateBr(
      context_->loop_instruction_stack().top().continue_bb);

  context_->llvm_builder()->SetInsertPoint(after_continue);
}

llvm::AllocaInst *LLVMStatementVisitor::CreateEntryBlockAlloca(
    llvm::Function *function, const std::string &var_name, llvm::Type *type) {
  llvm::IRBuilder<> tmp_builder(&function->getEntryBlock(),
                                function->getEntryBlock().begin());
  return tmp_builder.CreateAlloca(type, 0, var_name);
}
// `LLVMStatementVisitor` ===============================================
} // namespace Cobold