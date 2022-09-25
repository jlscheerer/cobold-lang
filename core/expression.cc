#include "core/expression.h"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <string>

#include "absl/strings/escaping.h"
#include "absl/strings/str_split.h"

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
  if (type == "/")
    return BinaryExpressionType::DIVIDE;
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
ConstantExpression::Bool(SourceLocation location, const std::string &value) {
  assert(value == "true" || value == "false");
  return std::make_unique<ConstantExpression>(location, value == "true");
}

absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::Char(SourceLocation location, const std::string &value) {
  // TODO(jlscheerer) Check the size constraints here...
  // TODO(jlscheerer) Turn the value into a character also..
  assert(absl::StartsWith(value, "'") && absl::EndsWith(value, "'"));
  std::string parsed_char, error;
  absl::CUnescape(value.substr(1, value.size() - 2), &parsed_char, &error);
  assert(error.size() == 0);
  assert(parsed_char.size() == 1);
  return std::make_unique<ConstantExpression>(location, parsed_char[0]);
}

absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::String(SourceLocation location, const std::string &value) {
  assert(absl::StartsWith(value, "\"") && absl::EndsWith(value, "\""));
  std::string parsed_string, error;
  absl::CUnescape(value.substr(1, value.size() - 2), &parsed_string, &error);
  assert(error.size() == 0);
  return std::make_unique<ConstantExpression>(location, parsed_string);
}

absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::Integer(SourceLocation location, const std::string &value) {
  // TODO(jlscheerer) Handle other formats 0b11, ...
  // TODO(jlscheerer) Improve this conversion
  char *end;
  long long i = std::strtoll(value.c_str(), &end, 10);
  return std::make_unique<ConstantExpression>(location,
                                              static_cast<int64_t>(i));
}

absl::StatusOr<std::unique_ptr<ConstantExpression>>
ConstantExpression::Floating(SourceLocation location,
                             const std::string &value) {
  // TODO(jlscheerer) Improve this conversion
  char *end;
  long double d = std::strtold(value.c_str(), &end);
  return std::make_unique<ConstantExpression>(location, static_cast<double>(d));
}
// `ConstantExpression` =================================================
} // namespace Cobold