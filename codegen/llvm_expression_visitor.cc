#include "codegen/llvm_expression_visitor.h"

#include <variant>

#include "codegen/build_context.h"
#include "codegen/llvm_type_visitor.h"

namespace Cobold {
// `LLVMExpressionVisitor` ==============================================
llvm::Value *LLVMExpressionVisitor::Translate(BuildContext *context,
                                              const Expression *expr) {
  LLVMExpressionVisitor visitor(context);
  return visitor.Visit(expr);
}

llvm::Value *LLVMExpressionVisitor::DispatchEmpty() { assert(false); }

llvm::Value *
LLVMExpressionVisitor::DispatchTernary(const TernaryExpression *expr) {
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchBinary(const BinaryExpression *expr) {
  llvm::Value *lhs = Visit(expr->lhs());
  const Type *lhs_type = expr->lhs()->expr_type();
  const TypeClass lhs_tc = lhs_type->type_class();

  llvm::Value *rhs = Visit(expr->rhs());
  const Type *rhs_type = expr->rhs()->expr_type();
  const TypeClass rhs_tc = rhs_type->type_class();

  if (lhs_tc == TypeClass::Integral && rhs_tc == TypeClass::Integral) {
    assert(lhs_type == rhs_type ||
           (expr->op_type() == BinaryExpressionType::SHIFT_LEFT ||
            expr->op_type() ==
                BinaryExpressionType::SHIFT_RIGHT)); // Type inference should
                                                     // have added the casts!
    return IntegralBinaryExpression(expr->op_type(), lhs, rhs);
  } else {
    assert(false);
  }
}

llvm::Value *LLVMExpressionVisitor::DispatchUnary(const UnaryExpression *expr) {
  const Type *type = expr->expression()->expr_type();
  const TypeClass type_class = type->type_class();
  if (type_class == TypeClass::Pointer) {
    return PointerUnaryExpression(expr, Visit(expr->expression()));
  }
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchCall(const CallExpression *expr) {
  assert(context_->HasFunction(expr->identifier()));
  llvm::Function *function = context_->FunctionForName(expr->identifier());
  std::vector<llvm::Value *> args;
  args.reserve(expr->args().size());
  for (const auto &arg : expr->args()) {
    args.push_back(Visit(arg.get()));
  }
  return context_->llvm_builder()->CreateCall(function, args);
}

llvm::Value *LLVMExpressionVisitor::DispatchRange(const RangeExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchArray(const ArrayExpression *expr) {
  assert(false);
}

llvm::Value *LLVMExpressionVisitor::DispatchCast(const CastExpression *expr) {
  const Type *from_type = expr->expression()->expr_type();
  TypeClass from_tc = from_type->type_class();
  const Type *to_type = expr->cast_type();
  TypeClass to_tc = to_type->type_class();

  if (from_tc == TypeClass::Integral && to_tc == TypeClass::Integral) {
    return context_->llvm_builder()->CreateSExtOrTrunc(
        Visit(expr->expression()),
        LLVMTypeVisitor::Translate(context_, expr->cast_type()));
  } else if (from_tc == TypeClass::Integral && to_tc == TypeClass::Bool) {
    // x != 0
    llvm::Value *zero = llvm::ConstantInt::get(
        **context_,
        llvm::APInt(expr->expression()->expr_type()->As<IntegralType>()->size(),
                    0));
    return context_->llvm_builder()->CreateICmpNE(Visit(expr->expression()),
                                                  zero);
  } else if (from_tc == TypeClass::Char && to_tc == TypeClass::Integral) {
    return context_->llvm_builder()->CreateSExtOrTrunc(
        Visit(expr->expression()),
        LLVMTypeVisitor::Translate(context_, expr->cast_type()));
  } else if (from_tc == TypeClass::Pointer && to_tc == TypeClass::Pointer) {
    return context_->llvm_builder()->CreateBitCast(
        Visit(expr->expression()),
        LLVMTypeVisitor::Translate(context_, expr->cast_type()),
        "pointer_cast");
  }
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchConstant(const ConstantExpression *expr) {
  if (std::holds_alternative<int64_t>(expr->data())) {
    assert(expr->expr_type()->type_class() == TypeClass::Integral);
    // TODO(jlscheerer) Improve the handling of this...
    return llvm::ConstantInt::get(
        **context_, llvm::APInt(expr->expr_type()->As<IntegralType>()->size(),
                                std::get<int64_t>(expr->data()), true));
  } else if (std::holds_alternative<bool>(expr->data())) {
    assert(expr->expr_type()->type_class() == TypeClass::Bool);
    return llvm::ConstantInt::get(
        **context_, llvm::APInt(1, std::get<bool>(expr->data()), true));
  } else if (std::holds_alternative<char>(expr->data())) {
    assert(expr->expr_type()->type_class() == TypeClass::Char);
    return llvm::ConstantInt::get(
        **context_, llvm::APInt(8, std::get<char>(expr->data()), true));
  } else if (std::holds_alternative<std::string>(expr->data())) {
    assert(expr->expr_type()->type_class() == TypeClass::String);
    std::string str = std::get<std::string>(expr->data());
    llvm::StructType *str_type =
        llvm::StructType::getTypeByName(**context_, "string");
    llvm::Constant *size =
        llvm::ConstantInt::get(**context_, llvm::APInt(64, str.size(), true));
    llvm::Constant *data = context_->AddStringConstant(str);
    return llvm::ConstantStruct::get(str_type, {size, data});
  }
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchIdentifier(const IdentifierExpression *expr) {
  assert(context_->HasNamedVar(expr->identifier()));
  llvm::AllocaInst *var = context_->AllocaForNamedVar(expr->identifier());
  return context_->llvm_builder()->CreateLoad(
      LLVMTypeVisitor::Translate(context_, expr->expr_type()), var,
      expr->identifier().c_str());
}

llvm::Value *LLVMExpressionVisitor::DispatchMemberAccess(
    const MemberAccessExpression *expr) {
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchArrayAccess(const ArrayAccessExpression *expr) {
  llvm::Value *expression = Visit(expr->expression());
  llvm::Value *index = Visit(expr->index());
  if (expr->expression()->expr_type()->type_class() == TypeClass::String) {
    llvm::Value *data = context_->llvm_builder()->CreateExtractValue(
        expression, /*member_index=*/1, "string::data");
    llvm::Type *char_ty = llvm::Type::getIntNTy(**context_, 8);
    llvm::Type *char_ptr = llvm::PointerType::get(char_ty, 0);
    llvm::Value *char_at_index =
        context_->llvm_builder()->CreateGEP(char_ptr, data, {index});
    return context_->llvm_builder()->CreateLoad(char_ty, char_at_index,
                                                ">>string::data");
  }
  assert(false);
}

llvm::Value *
LLVMExpressionVisitor::DispatchCallOp(const CallOpExpression *expr) {
  assert(expr->expression()->type() == ExpressionType::Identifier);
  const std::string &identifier =
      expr->expression()->As<IdentifierExpression>()->identifier();
  assert(context_->HasFunction(identifier));
  llvm::Function *function = context_->FunctionForName(identifier);
  std::vector<llvm::Value *> args;
  args.reserve(expr->args().size());
  for (const auto &arg : expr->args()) {
    args.push_back(Visit(arg.get()));
  }
  return context_->llvm_builder()->CreateCall(function, args);
}

llvm::Value *
LLVMExpressionVisitor::DispatchMalloc(const MallocExpression *expr) {
  assert(false); // should be rewritten! to fn call.
}

llvm::Value *
LLVMExpressionVisitor::DispatchSizeof(const SizeofExpression *expr) {
  return llvm::ConstantInt::get(
      llvm::Type::getInt64Ty(**context_),
      context_->llvm_module()->getDataLayout().getTypeAllocSize(
          LLVMTypeVisitor::Translate(context_, expr->decl_type())));
}

llvm::Value *LLVMExpressionVisitor::IntegralBinaryExpression(
    BinaryExpressionType op_type, llvm::Value *lhs, llvm::Value *rhs) {
  switch (op_type) {
  case BinaryExpressionType::LOGICAL_OR:
  case BinaryExpressionType::LOGICAL_AND:
    assert(false); // Not supported for integral types!
  case BinaryExpressionType::BIT_OR:
    return context_->llvm_builder()->CreateOr(lhs, rhs);
  case BinaryExpressionType::BIT_XOR:
    return context_->llvm_builder()->CreateXor(lhs, rhs);
  case BinaryExpressionType::BIT_AND:
    return context_->llvm_builder()->CreateAnd(lhs, rhs);
  case BinaryExpressionType::EQUALS:
    return context_->llvm_builder()->CreateICmpEQ(lhs, rhs);
  case BinaryExpressionType::NOT_EQUALS:
    return context_->llvm_builder()->CreateICmpNE(lhs, rhs);
  case BinaryExpressionType::LESS_THAN:
    return context_->llvm_builder()->CreateICmpSLT(lhs, rhs);
  case BinaryExpressionType::GREATER_THAN:
    return context_->llvm_builder()->CreateICmpSGT(lhs, rhs);
  case BinaryExpressionType::LESS_EQUAL:
    return context_->llvm_builder()->CreateICmpSLE(lhs, rhs);
  case BinaryExpressionType::GREATER_EQUAL:
    return context_->llvm_builder()->CreateICmpSGE(lhs, rhs);
  case BinaryExpressionType::SHIFT_LEFT:
    return context_->llvm_builder()->CreateShl(lhs, rhs);
  case BinaryExpressionType::SHIFT_RIGHT:
    return context_->llvm_builder()->CreateAShr(lhs, rhs);
  case BinaryExpressionType::ADD:
    return context_->llvm_builder()->CreateAdd(lhs, rhs);
  case BinaryExpressionType::SUBTRACT:
    return context_->llvm_builder()->CreateSub(lhs, rhs);
  case BinaryExpressionType::MULTIPLY:
    return context_->llvm_builder()->CreateMul(lhs, rhs);
  case BinaryExpressionType::DIVIDE:
    return context_->llvm_builder()->CreateSDiv(lhs, rhs);
  case BinaryExpressionType::MOD:
    return context_->llvm_builder()->CreateSRem(
        lhs, rhs); // TODO(jlscheerer) maybe support positive mod?
    break;
  }
}

llvm::Value *
LLVMExpressionVisitor::PointerUnaryExpression(const UnaryExpression *expr,
                                              llvm::Value *value) {
  UnaryExpressionType op_type = expr->op_type();
  switch (op_type) {
  case UnaryExpressionType::PRE_INCREMENT:
  case UnaryExpressionType::PRE_DECREMENT:
  case UnaryExpressionType::POST_INCREMENT:
  case UnaryExpressionType::POST_DECREMENT:
  case UnaryExpressionType::REFERENCE:
    assert(false);
  case UnaryExpressionType::DEREFERENCE:
    return context_->llvm_builder()->CreateLoad(
        LLVMTypeVisitor::Translate(context_, expr->expr_type()), value, "");
  case UnaryExpressionType::NEGATIVE:
  case UnaryExpressionType::POSITIVE:
  case UnaryExpressionType::INVERT:
  case UnaryExpressionType::NOT:
    assert(false);
    break;
  }
}

// `LLVMExpressionVisitor` ==============================================
} // namespace Cobold