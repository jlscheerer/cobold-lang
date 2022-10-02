#include "inference/type_inference_visitor.h"

#include <cassert>
#include <memory>
#include <optional>
#include <variant>

namespace Cobold {
namespace {
const IntegralType *PromoteIntegral(const Type *lhs, const Type *rhs) {
  assert(lhs->type_class() == TypeClass::Integral &&
         rhs->type_class() == TypeClass::Integral);
  int max_size = std::max(lhs->As<IntegralType>()->size(),
                          rhs->As<IntegralType>()->size());
  return IntegralType::OfSize(max_size);
}
// Pointer & Integral
bool ArePointerMathTypes(const Type *lhs, const Type *rhs) {
  if (lhs->type_class() == rhs->type_class())
    return false;
  if (lhs->type_class() == TypeClass::Integral) {
    return ArePointerMathTypes(rhs, lhs);
  }
  return lhs->type_class() == TypeClass::Pointer &&
         rhs->type_class() == TypeClass::Integral;
}
const PointerType *PromotePointer(const Type *lhs, const Type *rhs) {
  assert(lhs->type_class() != rhs->type_class());
  if (lhs->type_class() == TypeClass::Integral) {
    return PromotePointer(rhs, lhs);
  }
  assert(lhs->type_class() == TypeClass::Pointer);
  return lhs->As<PointerType>();
}
const FloatingType *PromoteFloating(const Type *lhs, const Type *rhs) {
  assert(lhs->type_class() == TypeClass::Floating &&
         rhs->type_class() == TypeClass::Floating);
  int max_size = std::max(lhs->As<FloatingType>()->size(),
                          rhs->As<FloatingType>()->size());
  return FloatingType::OfSize(max_size);
}
bool IsArithmetic(const Type *type) {
  return type->type_class() == TypeClass::Floating ||
         type->type_class() == TypeClass::Integral;
}
const Type *PromoteArithmetic(const Type *lhs, const Type *rhs) {
  if (lhs->type_class() == TypeClass::Integral &&
      rhs->type_class() == TypeClass::Integral) {
    return PromoteIntegral(lhs, rhs);
  } else if (lhs->type_class() == TypeClass::Floating &&
             rhs->type_class() == TypeClass::Floating) {
    return PromoteFloating(lhs, rhs);
  } else if (lhs->type_class() == TypeClass::Floating &&
             rhs->type_class() == TypeClass::Integral) {
    return lhs;
  } else if (lhs->type_class() == TypeClass::Integral &&
             rhs->type_class() == TypeClass::Floating) {
    return rhs;
  }
  assert(false);
}
} // namespace
// `TypeInferenceVisitor` ===============================================
void TypeInferenceVisitor::Annotate(SourceFile &file) {
  TypeInferenceVisitor visitor(file);
  for (const auto &fn : file.functions()) {
    if (!fn->external()) {
      visitor.AnnotateFunction(fn->As<DefinedFunction>());
    }
  }
}

void TypeInferenceVisitor::AnnotateFunction(DefinedFunction *function) {
  type_context_.PushFunctionReturn(function->return_type());
  type_context_.PushScope();
  for (const auto &arg : function->arguments()) {
    type_context_.StoreVar(arg.name, arg.type);
  }
  StatementVisitor::Visit(&function->mutable_body());
  type_context_.PopScope();
  type_context_.PopFunctionReturn();
}

bool TypeInferenceVisitor::CanCastExplicitTo(const Type *from, const Type *to) {
  // TODO(jlscheerer) Revise this once casting is settled
  assert(to != nullptr);
  if (from == nullptr)
    return true;
  if (to == from)
    return true;
  if (to == from)
    return true;
  TypeClass from_class = from->type_class(), to_class = to->type_class();
  switch (from_class) {
  case TypeClass::Nil:
    return to_class == TypeClass::Nil;
  case TypeClass::Dash:
    return true; // DashType can be cast to anything!
  case TypeClass::Bool:
    return to_class == TypeClass::Bool || to_class == TypeClass::Integral;
  case TypeClass::Char:
    return to_class == TypeClass::Char || to_class == TypeClass::Integral;
  case TypeClass::Integral:
    return to_class == TypeClass::Integral || to_class == TypeClass::Floating ||
           to_class == TypeClass::Bool || to_class == TypeClass::Char ||
           to_class == TypeClass::Pointer;
  case TypeClass::Floating:
    return to_class == TypeClass::Floating || to_class == TypeClass::Integral;
  case TypeClass::String:
    return to_class == TypeClass::String;
  case TypeClass::Array:
    // TODO(jlscheerer) We could rethink this. (For now only identity)
    return from == to;
  case TypeClass::Range:
    // TODO(jlscheerer) We could rethink this. (For now only identity)
    return from == to;
  case TypeClass::Pointer:
    return to_class == TypeClass::Pointer || to_class == TypeClass::Integral;
  }
  return false;
}

bool TypeInferenceVisitor::CanCastImplicitTo(const Type *from, const Type *to) {
  assert(to != nullptr && from != nullptr);
  if (to == from)
    return true;
  TypeClass from_class = from->type_class(), to_class = to->type_class();
  switch (from_class) {
  case TypeClass::Dash:
    return true; // DashType can be cast to anything!
  case TypeClass::Nil:
  case TypeClass::Bool:
  case TypeClass::Char:
  case TypeClass::String:
  case TypeClass::Array: // TODO(jlscheerer) [<?>] -> [] should be ok.
  case TypeClass::Range:
  case TypeClass::Pointer:
    return false;
  case TypeClass::Integral:
    return to_class == TypeClass::Integral &&
           from->As<IntegralType>()->size() <= to->As<IntegralType>()->size();
  case TypeClass::Floating:
    return to_class == TypeClass::Floating &&
           from->As<FloatingType>()->size() <= to->As<FloatingType>()->size();
  }
  return false;
}

// Used by Array, Range and Ternary
const Type *TypeInferenceVisitor::UnifyArrayTypes(
    std::vector<const Type *> &element_types) {
  // TODO(jlscheerer) Maybe we should revise this in the future
  const Type *array_type = nullptr;
  for (const auto &type : element_types) {
    if (array_type == nullptr) {
      array_type = type;
    } else if (CanCastImplicitTo(type, array_type)) {
      // keep the current array_type
    } else if (CanCastImplicitTo(array_type, type)) {
      array_type = type;
    } else {
      assert(false);
    }
  }
  return array_type;
}

const Type *TypeInferenceVisitor::IteratorType(const Type *type) {
  assert(type != nullptr);
  TypeClass type_class = type->type_class();
  if (type_class == TypeClass::Array) {
    return type->As<ArrayType>()->underlying_type();
  } else if (type_class == TypeClass::Range) {
    return type->As<RangeType>()->underlying_type();
  } else if (type_class == TypeClass::String) {
    return CharType::Get();
  }
  assert(false); // non-iterable type!
}

bool TypeInferenceVisitor::ArgumentTypesCompatible(
    std::vector<const Type *> expected_args,
    std::vector<const Type *> actual_args) {
  if (expected_args.size() != actual_args.size())
    return false;
  for (int i = 0; i < expected_args.size(); ++i) {
    if (!CanCastImplicitTo(actual_args[i], expected_args[i]))
      return false;
  }
  return true;
}

// Statements
void TypeInferenceVisitor::DispatchReturn(ReturnStatement *stmt) {
  const Type *fn_return = type_context_.FunctionReturnType();
  ExpressionVisitor::Visit(stmt->mutable_expression());
  if (fn_return != stmt->expression()->expr_type()) {
    assert(CanCastExplicitTo(stmt->expression()->expr_type(), fn_return));
    // Add the explicit cast. For future iterations.
    stmt->expression_ =
        WrapExplicitCast(fn_return, std::move(stmt->expression_));
  }
  // TODO(jlscheerer) Assert that the type is compatible with decl_type of
  // the surrounding function!
}

void TypeInferenceVisitor::DispatchAssignment(AssignmentStatement *stmt) {
  // TODO(jlscheerer) Check that the types are compatible by the operation!
  BinaryExpressionType bin_op;
  switch (stmt->assgn_type()) {
  case AssignmentType::EQ:
    ExpressionVisitor::Visit(stmt->mutable_lhs());
    ExpressionVisitor::Visit(stmt->mutable_rhs());
    assert(
        CanCastExplicitTo(stmt->rhs()->expr_type(), stmt->lhs()->expr_type()));
    stmt->rhs_ =
        WrapExplicitCast(stmt->lhs()->expr_type(), std::move(stmt->rhs_));
    return;
  // in the non-trivial case rewrite: a x= b to a = a x b
  case AssignmentType::MUL_EQ:
    bin_op = BinaryExpressionType::MULTIPLY;
    break;
  case AssignmentType::DIV_EQ:
    bin_op = BinaryExpressionType::DIVIDE;
    break;
  case AssignmentType::MOD_EQ:
    bin_op = BinaryExpressionType::MOD;
    break;
  case AssignmentType::ADD_EQ:
    bin_op = BinaryExpressionType::ADD;
    break;
  case AssignmentType::SUB_EQ:
    bin_op = BinaryExpressionType::SUBTRACT;
    break;
  case AssignmentType::SHL_EQ:
    bin_op = BinaryExpressionType::SHIFT_LEFT;
    break;
  case AssignmentType::SHR_EQ:
    bin_op = BinaryExpressionType::SHIFT_RIGHT;
    break;
  // TODO(jlscheerer) For bit operations they could be logical or bitwise!
  case AssignmentType::AND_EQ:
    bin_op = BinaryExpressionType::BIT_AND;
    break;
  case AssignmentType::XOR_EQ:
    bin_op = BinaryExpressionType::BIT_XOR;
    break;
  case AssignmentType::OR_EQ:
    bin_op = BinaryExpressionType::BIT_OR;
    break;
  }
  stmt->assgn_type_ = AssignmentType::EQ;
  stmt->rhs_ = std::make_unique<BinaryExpression>(SourceLocation::Generated(),
                                                  stmt->lhs()->Clone(), bin_op,
                                                  std::move(stmt->rhs_));
  return DispatchAssignment(stmt);
}

void TypeInferenceVisitor::DispatchCompound(CompoundStatement *stmt) {
  type_context_.PushScope();
  for (const auto &child : stmt->statements())
    StatementVisitor::Visit(child.get());
  type_context_.PopScope();
}

void TypeInferenceVisitor::DispatchExpression(ExpressionStatement *stmt) {
  ExpressionVisitor::Visit(stmt->mutable_expression());
}

void TypeInferenceVisitor::DispatchIf(IfStatement *stmt) {
  for (const auto &branch : stmt->branches()) {
    // TODO(jlscheerer) Check that the result is a boolean!
    ExpressionVisitor::Visit(branch.condition.get());
    StatementVisitor::Visit(branch.body.get());
  }
}

void TypeInferenceVisitor::DispatchFor(ForStatement *stmt) {
  // TODO(jlscheerer) Check that the iteration of "expression" is compatible
  // with the specified decl_type
  ExpressionVisitor::Visit(stmt->mutable_expression());
  if (stmt->decl_type() != nullptr) {
    // TODO(jlscheerer) This requires some more thought.
    assert(
        CanCastExplicitTo(stmt->expression()->expr_type(), stmt->decl_type()));
  } else {
    stmt->decl_type_ = IteratorType(stmt->expression()->expr_type());
  }
  type_context_.StoreVar(stmt->identifier(), stmt->decl_type());
  StatementVisitor::Visit(stmt->body().get());
}

void TypeInferenceVisitor::DispatchWhile(WhileStatement *stmt) {
  ExpressionVisitor::Visit(stmt->mutable_condition());
  // TODO(jlscheerer) Handle while [..] {}
  if (CanCastImplicitTo(stmt->condition()->expr_type(), BoolType::Get())) {
    // TODO(jlscheerer) Actually cast the expression to boolean!
  } else if (stmt->condition()->expr_type()->type_class() == TypeClass::Range) {
    // TODO(jlscheerer) Maybe we want to support:
    // var a = [..]; while a { ... }
    // in the future.
    // For now we only support an unbounded range literal
    assert(stmt->condition()->type() == ExpressionType::Range); // Range Literal
    const RangeExpression *range = stmt->condition()->As<RangeExpression>();
    assert(range->unbounded());
    // Replace the unbounded expression with `true` literal
    std::unique_ptr<Expression> true_literal =
        ConstantExpression::True(SourceLocation::Generated());
    true_literal->set_expr_type(BoolType::Get());
    std::swap(stmt->condition_, true_literal);
  } else {
    assert(false); // unsupported condition for while expression!
  }
  StatementVisitor::Visit(stmt->body().get());
}

void TypeInferenceVisitor::DispatchDeclaration(DeclarationStatement *stmt) {
  ExpressionVisitor::Visit(stmt->mutable_expression());
  // if the inferred type is nullptr (we could not infer it --)
  if (stmt->decl_type() == nullptr) {
    // we should not be getting an unassignable type (nil, --, etc.)!
    TypeClass type_class = stmt->expression()->expr_type()->type_class();
    assert(type_class != TypeClass::Nil && type_class != TypeClass::Dash);
    stmt->infer_type(stmt->expression()->expr_type());
  } else if (stmt->decl_type() != stmt->expression()->expr_type()) {
    if (stmt->decl_type()->type_class() == TypeClass::Array &&
        stmt->expression()->expr_type()->type_class() == TypeClass::Array &&
        stmt->expression()->type() == ExpressionType::Array) {
      // TODO(jlscheerer) We need to do the same for ranges...
      // Explicitly stated the type of the array i.e.,:
      // var x: [i8] = [1, 2, 3];
      // TODO(jlscheerer) Emit a warning here (in case of explicit casts) i.e.,:
      // var x: [i16] = [(i64) 1];
      const Type *actual =
          stmt->expression()->expr_type()->As<ArrayType>()->underlying_type();
      const Type *expected =
          stmt->decl_type()->As<ArrayType>()->underlying_type();
      std::cout << actual->DebugString() << std::endl;
      assert(CanCastExplicitTo(actual, expected));

      ArrayExpression *expr = stmt->mutable_expression()->As<ArrayExpression>();
      for (int i = 0; i < expr->elements().size(); ++i) {
        expr->mutable_elements()[i] =
            WrapExplicitCast(expected, std::move(expr->mutable_elements()[i]));
      }
    } else {
      assert(CanCastExplicitTo(stmt->expression()->expr_type(),
                               stmt->decl_type()));
      // Add the explicit cast. For future iterations.
      if (stmt->expression()->expr_type()->type_class() != TypeClass::Dash) {
        stmt->expression_ =
            WrapExplicitCast(stmt->decl_type(), std::move(stmt->expression_));
      }
    }
  }
  // stmt->decl_type() == stmt->expression()->expr_type() should be fine for
  // now (and does not require an explicit cast)!
  assert(type_context_.StoreVar(stmt->identifier(), stmt->decl_type()));
}

void TypeInferenceVisitor::DispatchBreak(BreakStatement *stmt) {
  // Nothing to do here...
}

void TypeInferenceVisitor::DispatchContinue(ContinueStatement *stmt) {
  // Nothing to do here...
}

// Expressions
void TypeInferenceVisitor::DispatchEmpty() {}

void TypeInferenceVisitor::DispatchTernary(TernaryExpression *expr) {
  // TODO(jlscheerer) Assert that the condition is a bool
  ExpressionVisitor::Visit(expr->mutable_condition());

  std::vector<const Type *> types;
  ExpressionVisitor::Visit(expr->mutable_true_case());
  types.push_back(expr->true_case()->expr_type());

  ExpressionVisitor::Visit(expr->mutable_false_case());
  types.push_back(expr->false_case()->expr_type());

  const Type *common_type = UnifyArrayTypes(types);
  // TODO(jlscheerer) Cast both to common_type
  expr->set_expr_type(common_type);
}

void TypeInferenceVisitor::DispatchBinary(BinaryExpression *expr) {
  ExpressionVisitor::Visit(expr->mutable_lhs());
  const Type *lhs_type = expr->lhs()->expr_type();
  const TypeClass lhs_tc = lhs_type->type_class();

  ExpressionVisitor::Visit(expr->mutable_rhs());
  const Type *rhs_type = expr->rhs()->expr_type();
  const TypeClass rhs_tc = rhs_type->type_class();

  switch (expr->op_type()) {
  case BinaryExpressionType::LOGICAL_OR:
  case BinaryExpressionType::LOGICAL_AND:
    assert(lhs_tc == TypeClass::Bool && rhs_tc == TypeClass::Bool);
    expr->set_expr_type(BoolType::Get());
    return;
  case BinaryExpressionType::BIT_OR:
  case BinaryExpressionType::BIT_XOR:
  case BinaryExpressionType::BIT_AND:
    if (lhs_tc == TypeClass::Integral && rhs_tc == TypeClass::Integral) {
      const Type *promoted_type = PromoteIntegral(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(promoted_type);
      return;
    } else {
      assert(false); // Bit operations only for Integral types!
    }
  case BinaryExpressionType::EQUALS:
  case BinaryExpressionType::NOT_EQUALS:
    if (lhs_type == rhs_type) {
      // always ok.
      expr->set_expr_type(BoolType::Get());
      return;
    } else if (IsArithmetic(lhs_type) && IsArithmetic(rhs_type)) {
      const Type *promoted_type = PromoteArithmetic(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(BoolType::Get());
      return;
    } else if (ArePointerMathTypes(lhs_type, rhs_type)) {
      const Type *promoted_type = PromotePointer(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(BoolType::Get());
      return;
    } else {
      assert(false);
    }
  case BinaryExpressionType::LESS_THAN:
  case BinaryExpressionType::GREATER_THAN:
  case BinaryExpressionType::LESS_EQUAL:
  case BinaryExpressionType::GREATER_EQUAL:
    // Floating & Integer & Pointer
    if (lhs_tc == TypeClass::Char && rhs_tc == TypeClass::Char) {
      expr->set_expr_type(BoolType::Get());
      return;
    } else if (lhs_tc == TypeClass::Bool && rhs_tc == TypeClass::Bool) {
      expr->set_expr_type(BoolType::Get());
      return;
    } else if (IsArithmetic(lhs_type) && IsArithmetic(rhs_type)) {
      const Type *promoted_type = PromoteArithmetic(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(BoolType::Get());
      return;
    } else if (ArePointerMathTypes(lhs_type, rhs_type)) {
      const Type *promoted_type = PromotePointer(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(BoolType::Get());
      return;
    } else if (lhs_tc == TypeClass::Pointer && lhs_type == rhs_type) {
      // can compare pointers to the same type
      expr->set_expr_type(BoolType::Get());
      return;
    } else {
      assert(false);
    }
  case BinaryExpressionType::ADD:
  case BinaryExpressionType::SUBTRACT:
  case BinaryExpressionType::MULTIPLY:
  case BinaryExpressionType::DIVIDE:
    if (IsArithmetic(lhs_type) && IsArithmetic(rhs_type)) {
      const Type *promoted_type = PromoteArithmetic(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(promoted_type);
      return;
    } else if (ArePointerMathTypes(lhs_type, rhs_type)) {
      const Type *promoted_type = PromotePointer(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(promoted_type);
      return;
    } else if (lhs_tc == TypeClass::Pointer && lhs_type == rhs_type) {
      // can compare pointers to the same type
      expr->set_expr_type(lhs_type);
      return;
    } else {
      assert(false);
    }
  case BinaryExpressionType::SHIFT_LEFT:
  case BinaryExpressionType::SHIFT_RIGHT:
    if (lhs_tc == TypeClass::Integral && rhs_tc == TypeClass::Integral) {
      const Type *promoted_type = PromoteArithmetic(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(promoted_type);
      return;
    } else {
      assert(false);
    }
  case BinaryExpressionType::MOD:
    if (lhs_tc == TypeClass::Integral && rhs_tc == TypeClass::Integral) {
      const Type *promoted_type = PromoteIntegral(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(promoted_type);
      return;
    } else if (ArePointerMathTypes(lhs_type, rhs_type)) {
      const Type *promoted_type = PromotePointer(lhs_type, rhs_type);
      expr->lhs_ = WrapExplicitCast(promoted_type, std::move(expr->lhs_));
      expr->rhs_ = WrapExplicitCast(promoted_type, std::move(expr->rhs_));
      expr->set_expr_type(promoted_type);
      return;
    } else {
      assert(false); // Bit operations only for Integral types!
    }
    break;
  }
}

void TypeInferenceVisitor::DispatchUnary(UnaryExpression *expr) {
  ExpressionVisitor::Visit(expr->mutable_expression());
  const Type *expr_type = expr->expression()->expr_type();
  TypeClass type_class = expr_type->type_class();
  switch (expr->op_type()) {
  case UnaryExpressionType::PRE_INCREMENT:
  case UnaryExpressionType::PRE_DECREMENT:
  case UnaryExpressionType::POST_INCREMENT:
  case UnaryExpressionType::POST_DECREMENT:
    assert(type_class == TypeClass::Integral ||
           type_class == TypeClass::Pointer);
    expr->set_expr_type(expr_type);
    return;
  case UnaryExpressionType::REFERENCE:
    assert(expr->expression()->type() == ExpressionType::Identifier);
    expr->set_expr_type(Type::PointerTo(expr_type));
    return;
  case UnaryExpressionType::DEREFERENCE:
    assert(type_class == TypeClass::Pointer);
    expr->set_expr_type(expr_type->As<PointerType>()->underlying_type());
    return;
  case UnaryExpressionType::NEGATIVE:
  case UnaryExpressionType::POSITIVE:
    assert(type_class == TypeClass::Integral ||
           type_class == TypeClass::Floating);
    expr->set_expr_type(expr_type);
    return;
  case UnaryExpressionType::INVERT:
    assert(type_class == TypeClass::Integral);
    expr->set_expr_type(expr_type);
    return;
  case UnaryExpressionType::NOT: // !x
    assert(type_class == TypeClass::Bool);
    expr->set_expr_type(BoolType::Get());
    return;
  }
}

void TypeInferenceVisitor::DispatchCall(CallExpression *expr) {
  assert(false); // this should now be redundant!
}

void TypeInferenceVisitor::DispatchRange(RangeExpression *expr) {
  std::vector<const Type *> element_types;
  if (expr->lhs()) {
    ExpressionVisitor::Visit(expr->mutable_lhs());
    element_types.push_back(expr->lhs()->expr_type());
  }
  if (expr->rhs()) {
    ExpressionVisitor::Visit(expr->mutable_rhs());
    element_types.push_back(expr->rhs()->expr_type());
  }
  const Type *range_elem_type = UnifyArrayTypes(element_types);
  expr->set_expr_type(RangeType::Of(range_elem_type));
}

void TypeInferenceVisitor::DispatchArray(ArrayExpression *expr) {
  std::vector<const Type *> element_types;
  element_types.reserve(expr->elements().size());
  for (const auto &element : expr->elements()) {
    ExpressionVisitor::Visit(element.get());
    element_types.push_back(element->expr_type());
  }
  const Type *array_elem_type = UnifyArrayTypes(element_types);
  expr->set_expr_type(Type::ArrayOf(array_elem_type));
}

void TypeInferenceVisitor::DispatchCast(CastExpression *expr) {
  ExpressionVisitor::Visit(expr->mutable_expression());
  assert(CanCastExplicitTo(expr->expression()->expr_type(), expr->cast_type()));
  expr->set_expr_type(expr->cast_type());
}

void TypeInferenceVisitor::DispatchConstant(ConstantExpression *expr) {
  if (std::holds_alternative<DashTypeTag>(expr->data())) {
    expr->set_expr_type(DashType::Get());
  } else if (std::holds_alternative<bool>(expr->data())) {
    expr->set_expr_type(BoolType::Get());
  } else if (std::holds_alternative<char>(expr->data())) {
    expr->set_expr_type(CharType::Get());
  } else if (std::holds_alternative<int64_t>(expr->data())) {
    expr->set_expr_type(IntegralType::OfSize(64));
  } else if (std::holds_alternative<double>(expr->data())) {
    expr->set_expr_type(FloatingType::OfSize(64));
  } else if (std::holds_alternative<std::string>(expr->data())) {
    expr->set_expr_type(StringType::Get());
  } else {
    // this should not happen (i.e. there are no other constant types!)
    assert(false);
  }
}

void TypeInferenceVisitor::DispatchIdentifier(IdentifierExpression *expr) {
  assert(type_context_.LookupVar(expr->identifier()) != std::nullopt);
  expr->set_expr_type(*type_context_.LookupVar(expr->identifier()));
}

void TypeInferenceVisitor::DispatchMemberAccess(MemberAccessExpression *expr) {}

void TypeInferenceVisitor::DispatchArrayAccess(ArrayAccessExpression *expr) {
  ExpressionVisitor::Visit(expr->mutable_index());
  assert(expr->index()->expr_type()->type_class() == TypeClass::Integral);
  ExpressionVisitor::Visit(expr->mutable_expression());
  const Type *expr_type = expr->expression()->expr_type();
  if (expr_type->type_class() == TypeClass::Array) {
    // TODO(jlscheerer) Not yet supported by the grammar (direct)
    expr->set_expr_type(expr_type->As<ArrayType>()->underlying_type());
  } else if (expr_type->type_class() == TypeClass::String) {
    expr->set_expr_type(CharType::Get());
  } else if (expr_type->type_class() == TypeClass::Range) {
    // TODO(jlscheerer) Not yet supported by the grammar (direct)
    expr->set_expr_type(expr_type->As<RangeType>()->underlying_type());
  } else {
    assert(false);
  }
}

void TypeInferenceVisitor::DispatchCallOp(CallOpExpression *expr) {
  assert(expr->expression()->type() == ExpressionType::Identifier);
  std::string identifier =
      expr->expression()->As<IdentifierExpression>()->identifier();
  const auto fn_declaration = type_context_.LookupFn(identifier);
  assert(fn_declaration != std::nullopt); // function does not exist!
  const auto &[fn_ret_type, fn_arg_types] = *fn_declaration;
  std::vector<const Type *> arg_types;
  arg_types.reserve(expr->args().size());
  for (const auto &arg : expr->args()) {
    ExpressionVisitor::Visit(arg.get());
    arg_types.push_back(arg->expr_type());
  }
  // TODO(jlscheerer) Cast all arguments to the expected types.
  assert(ArgumentTypesCompatible(fn_arg_types, arg_types));
  for (int i = 0; i < expr->args().size(); ++i) {
    expr->mutable_args()[i] =
        WrapExplicitCast(fn_arg_types[i], std::move(expr->mutable_args()[i]));
  }
  expr->set_expr_type(fn_ret_type);
}

void TypeInferenceVisitor::DispatchMalloc(MallocExpression *expr) {
  assert(false); // malloc should have been rewritten
}

void TypeInferenceVisitor::DispatchSizeof(SizeofExpression *expr) {
  // TODO(jlscheerer) Maybe we want to consider a different type here instead.
  // TODO(jlscheerer) Assert that we can get the size of the specified type.
  expr->set_expr_type(IntegralType::OfSize(64));
}

std::unique_ptr<Expression>
TypeInferenceVisitor::WrapExplicitCast(const Type *type,
                                       std::unique_ptr<Expression> &&expr) {
  if (type == expr->expr_type())
    return std::move(expr);
  auto new_expr = std::make_unique<CastExpression>(SourceLocation::Generated(),
                                                   type, std::move(expr));
  new_expr->set_expr_type(type);
  return new_expr;
}
// `TypeInferenceVisitor` ===============================================
} // namespace Cobold