#include "core/expression.h"

#include <cstdlib>
#include <memory>
#include <string>

namespace Cobold {
// `BinaryExpression` ===================================================
BinaryExpressionType BinaryExpression::TypeFromString(const std::string &type) {
  if (type == "||")
    return BinaryExpressionType::LOGICAL_OR;
  if (type == "&&")
    return BinaryExpressionType::LOGICAL_AND;
  if (type == "|")
    return BinaryExpressionType::BIT_OR;
  if (type == "^")
    return BinaryExpressionType::BIT_XOR;
  if (type == "&")
    return BinaryExpressionType::BIT_AND;
  if (type == "==")
    return BinaryExpressionType::EQUALS;
  if (type == "!=")
    return BinaryExpressionType::NOT_EQUALS;
  if (type == "<")
    return BinaryExpressionType::LESS_THAN;
  if (type == ">")
    return BinaryExpressionType::GREATER_THAN;
  if (type == "<=")
    return BinaryExpressionType::LESS_EQUAL;
  if (type == ">=")
    return BinaryExpressionType::GREATER_EQUAL;
  if (type == "<<")
    return BinaryExpressionType::SHIFT_LEFT;
  if (type == ">>")
    return BinaryExpressionType::SHIFT_RIGHT;
  if (type == "+")
    return BinaryExpressionType::ADD;
  if (type == "-")
    return BinaryExpressionType::SUBTRACT;
  if (type == "*")
    return BinaryExpressionType::MULTIPLY;
  if (type == "%")
    return BinaryExpressionType::MOD;
  assert(false);
}
// `BinaryExpression` ===================================================

// `UnaryExpression` ====================================================
UnaryExpressionType
UnaryExpression::TypeFromPrefixString(const std::string &type) {
  if (type == "++")
    return UnaryExpressionType::PRE_INCREMENT;
  if (type == "--")
    return UnaryExpressionType::PRE_DECREMENT;
  if (type == "&")
    return UnaryExpressionType::REFERENCE;
  if (type == ">>")
    return UnaryExpressionType::DEREFERENCE;
  if (type == "-")
    return UnaryExpressionType::NEGATIVE;
  if (type == "+")
    return UnaryExpressionType::POSITIVE;
  if (type == "~")
    return UnaryExpressionType::INVERT;
  if (type == "!")
    return UnaryExpressionType::NOT;
  assert(false);
}
// `UnaryExpression` ====================================================

// `ConstantExpression` =================================================
absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::String(const std::string &value) {
  return std::make_unique<ConstantExpression>(value);
}

absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::Integer(const std::string &value) {
  // TODO(jlscheerer) Improve this conversion
  char *end;
  long long i = std::strtoll(value.c_str(), &end, 10);
  return std::make_unique<ConstantExpression>((int64_t)i);
}

absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::Floating(const std::string &value) {
  // TODO(jlscheerer) Convert to float!
  return std::make_unique<ConstantExpression>(value);
}
// `ConstantExpression` =================================================
} // namespace Cobold