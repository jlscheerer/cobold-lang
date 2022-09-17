#ifndef COBOLD_CORE_EXPRESSION
#define COBOLD_CORE_EXPRESSION

#include <memory>
#include <type_traits>

namespace Cobold {
enum class ExpressionType {
  Ternary,   // x ? a : b
  Binary,    // a + b, a == b
  Unary,     // ++a, a++, >>a
  Call,      // Test("a", 2, b)
  Range,     // [1..], [..2], [..]
  Array,     // [1, a, 1.2]
  Cast,      // (i32*) x
  Constant,  // 1, "a", 1.0
  Identifier // a
};
class Expression {
public:
  virtual const ExpressionType type() const = 0;

  template <typename T> const T *As() const {
    static_assert(std::is_base_of_v<Expression, T>,
                  "Attempting to cast to non-derived class.");
    // TODO(jlscheerer) check for `nullptr`
    return dynamic_cast<const T *>(this);
  }

  virtual ~Expression() = default;
};

class TernaryExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Ternary; }
};

class BinaryExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Binary; }
};

class UnaryExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Unary; }
};

class CallExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Call; }
};

class RangeExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Range; }
};

class ArrayExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Array; }
};

class CastExpression : public Expression {
public:
  virtual const ExpressionType type() const { return ExpressionType::Cast; }
};

class ConstantExpression : public Expression {
public:
  static std::unique_ptr<ConstantExpression> True() { return nullptr; }
  static std::unique_ptr<ConstantExpression> DashInit() { return nullptr; }

  virtual const ExpressionType type() const { return ExpressionType::Constant; }
};

class IdentifierExpression : public Expression {
public:
  virtual const ExpressionType type() const {
    return ExpressionType::Identifier;
  }
};
} // namespace Cobold

#endif /* COBOLD_CORE_EXPRESSION */
