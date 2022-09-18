#ifndef COBOLD_CORE_EXPRESSION
#define COBOLD_CORE_EXPRESSION

#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "absl/status/statusor.h"
#include "core/type.h"

namespace Cobold {
enum class ExpressionType {
  Ternary,      // x ? a : b
  Binary,       // a + b, a == b
  Unary,        // ++a, a++, >>a
  Call,         // Test("a", 2, b)
  Range,        // [1..], [..2], [..]
  Array,        // [1, a, 1.2]
  Cast,         // (i32*) x
  Constant,     // 1, "a", 1.0
  Identifier,   // a
  MemberAccess, // a.identifier, a->identifier
  ArrayAccess,  // a[...]
  CallOp,       // a(...)
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
  TernaryExpression(std::unique_ptr<Expression> &&condition,
                    std::unique_ptr<Expression> &&true_case,
                    std::unique_ptr<Expression> &&false_case)
      : condition_(std::move(condition)), true_case_(std::move(true_case)),
        false_case_(std::move(false_case)) {}

  const Expression *condition() const { return condition_.get(); }
  const Expression *true_case() const { return true_case_.get(); }
  const Expression *false_case() const { return false_case_.get(); }
  virtual const ExpressionType type() const { return ExpressionType::Ternary; }

private:
  std::unique_ptr<Expression> condition_, true_case_, false_case_;
};

enum class BinaryExpressionType {
  LOGICAL_OR,    // a || b
  LOGICAL_AND,   // a && b
  BIT_OR,        // a | b
  BIT_XOR,       // a ^ b
  BIT_AND,       // a & b
  EQUALS,        // a == b
  NOT_EQUALS,    // a != b
  LESS_THAN,     // a < b
  GREATER_THAN,  // a > b
  LESS_EQUAL,    // a <= b
  GREATER_EQUAL, // a >= b
  SHIFT_LEFT,    // a << b
  SHIFT_RIGHT,   // a >> b
  ADD,           // a + b
  SUBTRACT,      // a - b
  MULTIPLY,      // a * b
  DIVIDE,        // a / b
  MOD,           // a % b
};

class BinaryExpression : public Expression {
public:
  BinaryExpression(std::unique_ptr<Expression> &&lhs,
                   BinaryExpressionType op_type,
                   std::unique_ptr<Expression> &&rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)), op_type_(op_type) {}

  const Expression *lhs() const { return lhs_.get(); }
  const BinaryExpressionType op_type() const { return op_type_; }
  const Expression *rhs() const { return rhs_.get(); }

  static BinaryExpressionType TypeFromString(const std::string &type);

  virtual const ExpressionType type() const { return ExpressionType::Binary; }

private:
  std::unique_ptr<Expression> lhs_, rhs_;
  BinaryExpressionType op_type_;
};

enum class UnaryExpressionType {
  PRE_INCREMENT,  //++a
  PRE_DECREMENT,  // --a
  POST_INCREMENT, // a++
  POST_DECREMENT, // a--
  REFERENCE,      // &a
  DEREFERENCE,    // >>a
  NEGATIVE,       // -a
  POSITIVE,       // +a
  INVERT,         // ~a
  NOT,            // !a
};

class UnaryExpression : public Expression {
public:
  UnaryExpression(UnaryExpressionType op_type,
                  std::unique_ptr<Expression> &&expr)
      : op_type_(op_type), expr_(std::move(expr)) {}

  const UnaryExpressionType op_type() const { return op_type_; }
  const Expression *expression() const { return expr_.get(); }

  static UnaryExpressionType TypeFromPrefixString(const std::string &type);

  virtual const ExpressionType type() const { return ExpressionType::Unary; }

private:
  UnaryExpressionType op_type_;
  std::unique_ptr<Expression> expr_;
};

class CallExpression : public Expression {
public:
  CallExpression(std::string &&identifier,
                 std::vector<std::unique_ptr<Expression>> &&args)
      : identifier_(std::move(identifier)), args_(std::move(args)) {}

  const std::string &identifier() const { return identifier_; }
  const std::vector<std::unique_ptr<Expression>> &args() const { return args_; }
  virtual const ExpressionType type() const { return ExpressionType::Call; }

private:
  std::string identifier_;
  std::vector<std::unique_ptr<Expression>> args_;
};

class RangeExpression : public Expression {
public:
  RangeExpression(std::unique_ptr<Expression> &&lhs,
                  std::unique_ptr<Expression> &&rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  const Expression *lhs() const { return lhs_.get(); }
  const Expression *rhs() const { return rhs_.get(); }
  virtual const ExpressionType type() const { return ExpressionType::Range; }

private:
  std::unique_ptr<Expression> lhs_, rhs_;
};

class ArrayExpression : public Expression {
public:
  ArrayExpression(std::vector<std::unique_ptr<Expression>> &&elements)
      : elements_(std::move(elements)) {}

  const std::vector<std::unique_ptr<Expression>> &elements() const {
    return elements_;
  }
  virtual const ExpressionType type() const { return ExpressionType::Array; }

private:
  std::vector<std::unique_ptr<Expression>> elements_;
};

class CastExpression : public Expression {
public:
  CastExpression(const Type *cast_type, std::unique_ptr<Expression> &&expr)
      : cast_type_(cast_type), expr_(std::move(expr)) {}

  const Type *cast_type() const { return cast_type_; }
  const Expression *expression() const { return expr_.get(); }
  virtual const ExpressionType type() const { return ExpressionType::Cast; }

private:
  const Type *cast_type_;
  std::unique_ptr<Expression> expr_;
};

struct DashType {};

class ConstantExpression : public Expression {
public:
  using data_type = std::variant<DashType, bool, int64_t, double, std::string>;

  ConstantExpression(data_type data) : data_(data) {}
  const data_type &data() const { return data_; }

  static std::unique_ptr<ConstantExpression> True() {
    return std::make_unique<ConstantExpression>(true);
  }

  static std::unique_ptr<ConstantExpression> DashInit() {
    return std::make_unique<ConstantExpression>(DashType{});
  }

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  String(const std::string &value);

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  Integer(const std::string &value);

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  Floating(const std::string &value);

  virtual const ExpressionType type() const { return ExpressionType::Constant; }

private:
  data_type data_;
};

class IdentifierExpression : public Expression {
public:
  IdentifierExpression(const std::string &identifier)
      : identifier_(identifier) {}

  const std::string identifier() const { return identifier_; }
  virtual const ExpressionType type() const {
    return ExpressionType::Identifier;
  }

private:
  std::string identifier_;
};

class MemberAccessExpression : public Expression {
public:
  MemberAccessExpression(std::unique_ptr<Expression> &&expr, bool direct,
                         const std::string &identifier)
      : expr_(std::move(expr)), direct_(direct), identifier_(identifier) {}

  const Expression *expression() const { return expr_.get(); }
  const bool direct() const { return direct_; }
  const std::string &identifier() const { return identifier_; }

  virtual const ExpressionType type() const {
    return ExpressionType::MemberAccess;
  }

private:
  std::unique_ptr<Expression> expr_;
  bool direct_; //  "->" -> !direct_, "." -> direct
  std::string identifier_;
};

class ArrayAccessExpression : public Expression {
public:
  ArrayAccessExpression(std::unique_ptr<Expression> &&expr,
                        std::unique_ptr<Expression> &&index)
      : expr_(std::move(expr)), index_(std::move(index)) {}

  const Expression *expression() const { return expr_.get(); }
  const Expression *index() const { return index_.get(); }

  virtual const ExpressionType type() const {
    return ExpressionType::ArrayAccess;
  }

private:
  std::unique_ptr<Expression> expr_, index_;
};

class CallOpExpression : public Expression {
public:
  CallOpExpression(std::unique_ptr<Expression> &&expr,
                   std::vector<std::unique_ptr<Expression>> &&args)
      : expr_(std::move(expr)), args_(std::move(args)) {}

  const Expression *expression() const { return expr_.get(); }
  const std::vector<std::unique_ptr<Expression>> &args() const { return args_; }

  virtual const ExpressionType type() const { return ExpressionType::CallOp; }

private:
  std::unique_ptr<Expression> expr_;
  std::vector<std::unique_ptr<Expression>> args_;
};
} // namespace Cobold

#endif /* COBOLD_CORE_EXPRESSION */
