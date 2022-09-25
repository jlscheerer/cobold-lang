#include "util/expression_printer.h"

#include <sys/_types/_int64_t.h>
#include <vector>

#include "absl/strings/escaping.h"

namespace Cobold {
// `ExpressionPrinter` ==================================================
std::string ExpressionPrinter::Print(const Expression *expr) {
  ExpressionPrinter printer;
  printer.Visit(expr);
  return printer.buffer_;
}

void ExpressionPrinter::DispatchEmpty() {}

void ExpressionPrinter::DispatchTernary(const TernaryExpression *expr) {
  Visit(expr->condition());
  Append(" ? ");
  Visit(expr->true_case());
  Append(" : ");
  Visit(expr->false_case());
}

void ExpressionPrinter::DispatchBinary(const BinaryExpression *expr) {
  Visit(expr->lhs());
  switch (expr->op_type()) {
  case BinaryExpressionType::LOGICAL_OR:
    Append(" || ");
    break;
  case BinaryExpressionType::LOGICAL_AND:
    Append(" && ");
    break;
  case BinaryExpressionType::BIT_OR:
    Append(" | ");
    break;
  case BinaryExpressionType::BIT_XOR:
    Append(" ^ ");
    break;
  case BinaryExpressionType::BIT_AND:
    Append(" & ");
    break;
  case BinaryExpressionType::EQUALS:
    Append(" == ");
    break;
  case BinaryExpressionType::NOT_EQUALS:
    Append(" != ");
    break;
  case BinaryExpressionType::LESS_THAN:
    Append(" < ");
    break;
  case BinaryExpressionType::GREATER_THAN:
    Append(" > ");
    break;
  case BinaryExpressionType::LESS_EQUAL:
    Append(" <= ");
    break;
  case BinaryExpressionType::GREATER_EQUAL:
    Append(" >= ");
    break;
  case BinaryExpressionType::SHIFT_LEFT:
    Append(" << ");
    break;
  case BinaryExpressionType::SHIFT_RIGHT:
    Append(" >> ");
    break;
  case BinaryExpressionType::ADD:
    Append(" + ");
    break;
  case BinaryExpressionType::SUBTRACT:
    Append(" - ");
    break;
  case BinaryExpressionType::MULTIPLY:
    Append(" * ");
    break;
  case BinaryExpressionType::DIVIDE:
    Append(" / ");
    break;
  case BinaryExpressionType::MOD:
    Append(" % ");
    break;
  }
  Visit(expr->rhs());
}

void ExpressionPrinter::DispatchUnary(const UnaryExpression *expr) {
  const UnaryExpressionType type = expr->op_type();
  switch (type) {
  case UnaryExpressionType::PRE_INCREMENT:
    Append("++");
    break;
  case UnaryExpressionType::PRE_DECREMENT:
    Append("--");
    break;
  case UnaryExpressionType::POST_INCREMENT:
    break;
  case UnaryExpressionType::POST_DECREMENT:
    break;
  case UnaryExpressionType::REFERENCE:
    Append("&");
    break;
  case UnaryExpressionType::DEREFERENCE:
    Append(">>");
    break;
  case UnaryExpressionType::NEGATIVE:
    Append("-");
    break;
  case UnaryExpressionType::POSITIVE:
    Append("+");
    break;
  case UnaryExpressionType::INVERT:
    Append("~");
    break;
  case UnaryExpressionType::NOT:
    Append("!");
    break;
  }
  Visit(expr->expression());
  if (type == UnaryExpressionType::POST_INCREMENT) {
    Append("++");
  } else if (type == UnaryExpressionType::POST_DECREMENT) {
    Append("--");
  }
}

// TODO(jlscheerer) Call is probably redundant at this point...
void ExpressionPrinter::DispatchCall(const CallExpression *expr) {
  Append(expr->identifier(), "(");
  const int num_args = expr->args().size();
  for (int i = 0; i < num_args; ++i) {
    Visit(expr->args()[i].get());
    if (i != num_args - 1)
      Append(", ");
  }
  Append(")");
}

void ExpressionPrinter::DispatchRange(const RangeExpression *expr) {
  Append("[");
  if (expr->lhs())
    Visit(expr->lhs());
  Append(" .. ");
  if (expr->rhs())
    Visit(expr->rhs());
  Append("]");
}

void ExpressionPrinter::DispatchArray(const ArrayExpression *expr) {
  Append("[");
  const int num_elements = expr->elements().size();
  for (int i = 0; i < num_elements; ++i) {
    Visit(expr->elements()[i].get());
    if (i != num_elements - 1)
      Append(", ");
  }
  Append("]");
}

void ExpressionPrinter::DispatchCast(const CastExpression *expr) {
  Append("(", expr->cast_type()->DebugString(), ") ");
  Visit(expr->expression());
}

void ExpressionPrinter::DispatchConstant(const ConstantExpression *expr) {
  const auto &data = expr->data();
  if (const DashType *value = std::get_if<DashType>(&data)) {
    Append("--");
  } else if (const bool *value = std::get_if<bool>(&data)) {
    Append(*value ? "true" : "false");
  } else if (const int64_t *value = std::get_if<int64_t>(&data)) {
    Append(std::to_string(*value));
  } else if (const double *value = std::get_if<double>(&data)) {
    Append(std::to_string(*value));
  } else if (const char *value = std::get_if<char>(&data)) {
    Append("'", absl::CEscape(std::string(1, *value)), "'");
  } else if (const std::string *value = std::get_if<std::string>(&data)) {
    Append("\"", absl::CEscape(*value), "\"");
  } else {
    assert(false);
  }
}

void ExpressionPrinter::DispatchIdentifier(const IdentifierExpression *expr) {
  Append(expr->identifier());
}

void ExpressionPrinter::DispatchMemberAccess(
    const MemberAccessExpression *expr) {
  Visit(expr->expression());
  Append(expr->direct() ? "." : "->", expr->identifier());
}

void ExpressionPrinter::DispatchArrayAccess(const ArrayAccessExpression *expr) {
  Visit(expr->expression());
  Append("[");
  Visit(expr->index());
  Append("]");
}

void ExpressionPrinter::DispatchCallOp(const CallOpExpression *expr) {
  Visit(expr->expression());
  Append("(");
  const int num_args = expr->args().size();
  for (int i = 0; i < num_args; ++i) {
    Visit(expr->args()[i].get());
    if (i != num_args - 1)
      Append(", ");
  }
  Append(")");
}

void ExpressionPrinter::AppendLine(const std::string &line) {
  buffer_ = absl::StrCat(std::move(buffer_), line);
}
// `ExpressionPrinter` ==================================================
} // namespace Cobold