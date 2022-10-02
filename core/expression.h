#ifndef COBOLD_CORE_EXPRESSION
#define COBOLD_CORE_EXPRESSION

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "absl/status/statusor.h"
#include "core/type.h"
#include "parser/source_location.h"
#include "util/type_traits.h"

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
  CallOp,       // a(...),
  Malloc,       // malloc(i32)(...)
  Sizeof,       // sizeof(i32)
};
class Expression {
public:
  Expression(SourceLocation location) : location_(location) {}

  template <typename T> const T *As() const {
    static_assert(std::is_base_of_v<Expression, T>,
                  "Attempting to cast to non-derived class.");
    // TODO(jlscheerer) check for `nullptr`
    return dynamic_cast<const T *>(this);
  }

  template <typename T> T *As() {
    static_assert(std::is_base_of_v<Expression, T>,
                  "Attempting to cast to non-derived class.");
    // TODO(jlscheerer) check for `nullptr`
    return dynamic_cast<T *>(this);
  }

  virtual const ExpressionType type() const = 0;
  virtual std::unique_ptr<Expression> Clone() const = 0;

  const SourceLocation location() const { return location_; }
  const Type *expr_type() const { return expr_type_; }

  virtual ~Expression() = default;

protected:
  static std::vector<std::unique_ptr<Expression>>
  CloneVector(const std::vector<std::unique_ptr<Expression>> &v) {
    std::vector<std::unique_ptr<Expression>> cloned_args;
    cloned_args.reserve(v.size());
    for (int i = 0; i < v.size(); ++i) {
      cloned_args.push_back(v[i]->Clone());
    }
    return cloned_args;
  }

  SourceLocation location_;

private:
  void set_expr_type(const Type *type) { expr_type_ = type; }

  const Type *expr_type_;

  friend class TypeInferenceVisitor;
};

class TernaryExpression : public Expression {
public:
  TernaryExpression(SourceLocation location,
                    std::unique_ptr<Expression> &&condition,
                    std::unique_ptr<Expression> &&true_case,
                    std::unique_ptr<Expression> &&false_case)
      : Expression(location), condition_(std::move(condition)),
        true_case_(std::move(true_case)), false_case_(std::move(false_case)) {}

  const Expression *condition() const { return condition_.get(); }
  Expression *mutable_condition() { return condition_.get(); }

  const Expression *true_case() const { return true_case_.get(); }
  Expression *mutable_true_case() { return true_case_.get(); }

  const Expression *false_case() const { return false_case_.get(); }
  Expression *mutable_false_case() { return false_case_.get(); }

  virtual const ExpressionType type() const override {
    return ExpressionType::Ternary;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<TernaryExpression>(location_, condition_->Clone(),
                                               true_case_->Clone(),
                                               false_case_->Clone());
  }

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
  BinaryExpression(SourceLocation location, std::unique_ptr<Expression> &&lhs,
                   BinaryExpressionType op_type,
                   std::unique_ptr<Expression> &&rhs)
      : Expression(location), lhs_(std::move(lhs)), rhs_(std::move(rhs)),
        op_type_(op_type) {}

  const Expression *lhs() const { return lhs_.get(); }
  Expression *mutable_lhs() { return lhs_.get(); }

  const BinaryExpressionType op_type() const { return op_type_; }

  const Expression *rhs() const { return rhs_.get(); }
  Expression *mutable_rhs() { return rhs_.get(); }

  static BinaryExpressionType TypeFromString(const std::string &type);

  virtual const ExpressionType type() const override {
    return ExpressionType::Binary;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<BinaryExpression>(location_, lhs_->Clone(),
                                              op_type_, rhs_->Clone());
  }

private:
  std::unique_ptr<Expression> lhs_, rhs_;
  BinaryExpressionType op_type_;

  friend class TypeInferenceVisitor;
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
  UnaryExpression(SourceLocation location, UnaryExpressionType op_type,
                  std::unique_ptr<Expression> &&expr)
      : Expression(location), op_type_(op_type), expr_(std::move(expr)) {}

  const UnaryExpressionType op_type() const { return op_type_; }
  const Expression *expression() const { return expr_.get(); }
  Expression *mutable_expression() { return expr_.get(); }

  static UnaryExpressionType TypeFromPrefixString(const std::string &type);

  virtual const ExpressionType type() const override {
    return ExpressionType::Unary;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<UnaryExpression>(location_, op_type_,
                                             expr_->Clone());
  }

private:
  UnaryExpressionType op_type_;
  std::unique_ptr<Expression> expr_;
};

class CallExpression : public Expression {
public:
  CallExpression(SourceLocation location, std::string identifier,
                 std::vector<std::unique_ptr<Expression>> &&args)
      : Expression(location), identifier_(std::move(identifier)),
        args_(std::move(args)) {}

  const std::string &identifier() const { return identifier_; }
  const std::vector<std::unique_ptr<Expression>> &args() const { return args_; }
  virtual const ExpressionType type() const override {
    return ExpressionType::Call;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<CallExpression>(location_, identifier_,
                                            CloneVector(args_));
  }

private:
  std::string identifier_;
  std::vector<std::unique_ptr<Expression>> args_;
};

class RangeExpression : public Expression {
public:
  RangeExpression(SourceLocation location, std::unique_ptr<Expression> &&lhs,
                  std::unique_ptr<Expression> &&rhs)
      : Expression(location), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

  const bool left_bounded() const {
    return lhs() != nullptr && rhs() == nullptr;
  }
  const bool right_bounded() const {
    return lhs() == nullptr && rhs() != nullptr;
  }
  const bool bounded() const { return lhs() != nullptr && rhs() != nullptr; }
  const bool unbounded() const { return lhs() == nullptr && rhs() == nullptr; }

  const Expression *lhs() const { return lhs_.get(); }
  Expression *mutable_lhs() { return lhs_.get(); }
  const Expression *rhs() const { return rhs_.get(); }
  Expression *mutable_rhs() { return rhs_.get(); }

  virtual const ExpressionType type() const override {
    return ExpressionType::Range;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<RangeExpression>(location_,
                                             lhs_ ? lhs_->Clone() : nullptr,
                                             rhs_ ? rhs_->Clone() : nullptr);
  }

private:
  std::unique_ptr<Expression> lhs_, rhs_;
};

class ArrayExpression : public Expression {
public:
  ArrayExpression(SourceLocation location,
                  std::vector<std::unique_ptr<Expression>> &&elements)
      : Expression(location), elements_(std::move(elements)) {}

  const std::vector<std::unique_ptr<Expression>> &elements() const {
    return elements_;
  }
  std::vector<std::unique_ptr<Expression>> &mutable_elements() {
    return elements_;
  }
  virtual const ExpressionType type() const override {
    return ExpressionType::Array;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<ArrayExpression>(location_, CloneVector(elements_));
  }

private:
  std::vector<std::unique_ptr<Expression>> elements_;
};

class CastExpression : public Expression {
public:
  CastExpression(SourceLocation location, const Type *cast_type,
                 std::unique_ptr<Expression> &&expr)
      : Expression(location), cast_type_(cast_type), expr_(std::move(expr)) {}

  const Type *cast_type() const { return cast_type_; }
  const Expression *expression() const { return expr_.get(); }
  Expression *mutable_expression() { return expr_.get(); }
  virtual const ExpressionType type() const override {
    return ExpressionType::Cast;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<CastExpression>(location_, cast_type_,
                                            expr_->Clone());
  }

private:
  const Type *cast_type_;
  std::unique_ptr<Expression> expr_;
};

struct DashTypeTag {};

class ConstantExpression : public Expression {
public:
  using data_type =
      std::variant<DashTypeTag, bool, int64_t, double, std::string, char>;

  ConstantExpression(SourceLocation location, data_type data)
      : Expression(location), data_(data) {}
  const data_type &data() const { return data_; }

  static std::unique_ptr<ConstantExpression> True(SourceLocation location) {
    return std::make_unique<ConstantExpression>(location, true);
  }

  static std::unique_ptr<ConstantExpression> DashInit(SourceLocation location) {
    return std::make_unique<ConstantExpression>(location, DashTypeTag{});
  }

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  Bool(SourceLocation location, const std::string &value);

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  Char(SourceLocation location, const std::string &value);

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  String(SourceLocation location, const std::string &value);

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  Integer(SourceLocation location, const std::string &value);

  static absl::StatusOr<std::unique_ptr<ConstantExpression>>
  Floating(SourceLocation location, const std::string &value);

  virtual const ExpressionType type() const override {
    return ExpressionType::Constant;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<ConstantExpression>(location_, data_);
  }

private:
  data_type data_;
};

class IdentifierExpression : public Expression {
public:
  IdentifierExpression(SourceLocation location, const std::string &identifier)
      : Expression(location), identifier_(identifier) {}

  const std::string identifier() const { return identifier_; }
  virtual const ExpressionType type() const override {
    return ExpressionType::Identifier;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<IdentifierExpression>(location_, identifier_);
  }

private:
  std::string identifier_;
};

class MemberAccessExpression : public Expression {
public:
  MemberAccessExpression(SourceLocation location,
                         std::unique_ptr<Expression> &&expr, bool direct,
                         const std::string &identifier)
      : Expression(location), expr_(std::move(expr)), direct_(direct),
        identifier_(identifier) {}

  const Expression *expression() const { return expr_.get(); }
  const bool direct() const { return direct_; }
  const std::string &identifier() const { return identifier_; }

  virtual const ExpressionType type() const override {
    return ExpressionType::MemberAccess;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<MemberAccessExpression>(location_, expr_->Clone(),
                                                    direct_, identifier_);
  }

private:
  std::unique_ptr<Expression> expr_;
  bool direct_; //  "->" -> !direct_, "." -> direct
  std::string identifier_;
};

class ArrayAccessExpression : public Expression {
public:
  ArrayAccessExpression(SourceLocation location,
                        std::unique_ptr<Expression> &&expr,
                        std::unique_ptr<Expression> &&index)
      : Expression(location), expr_(std::move(expr)), index_(std::move(index)) {
  }

  const Expression *expression() const { return expr_.get(); }
  Expression *mutable_expression() { return expr_.get(); }
  const Expression *index() const { return index_.get(); }
  Expression *mutable_index() { return index_.get(); }

  virtual const ExpressionType type() const override {
    return ExpressionType::ArrayAccess;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<ArrayAccessExpression>(location_, expr_->Clone(),
                                                   index_->Clone());
  }

private:
  std::unique_ptr<Expression> expr_, index_;
};

class CallOpExpression : public Expression {
public:
  CallOpExpression(SourceLocation location, std::unique_ptr<Expression> &&expr,
                   std::vector<std::unique_ptr<Expression>> &&args)
      : Expression(location), expr_(std::move(expr)), args_(std::move(args)) {}

  const Expression *expression() const { return expr_.get(); }
  const std::vector<std::unique_ptr<Expression>> &args() const { return args_; }
  std::vector<std::unique_ptr<Expression>> &mutable_args() { return args_; }

  virtual const ExpressionType type() const override {
    return ExpressionType::CallOp;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<CallOpExpression>(location_, expr_->Clone(),
                                              CloneVector(args_));
  }

private:
  std::unique_ptr<Expression> expr_;
  std::vector<std::unique_ptr<Expression>> args_;
};

class MallocExpression : public Expression {
public:
  MallocExpression(SourceLocation location, const Type *type,
                   std::unique_ptr<Expression> &&expr)
      : Expression(location), decl_type_(type), expr_(std::move(expr)) {}

  const Type *decl_type() const { return decl_type_; }
  const Expression *expression() const { return expr_.get(); }
  Expression *mutable_expression() { return expr_.get(); }

  virtual const ExpressionType type() const override {
    return ExpressionType::Malloc;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<MallocExpression>(location_, decl_type_,
                                              expr_->Clone());
  }

private:
  const Type *decl_type_;
  std::unique_ptr<Expression> expr_;

  friend class TypeInferenceVisitor;
};

class SizeofExpression : public Expression {
public:
  SizeofExpression(SourceLocation location, const Type *type)
      : Expression(location), decl_type_(type) {}

  const Type *decl_type() const { return decl_type_; }
  virtual const ExpressionType type() const override {
    return ExpressionType::Sizeof;
  }
  std::unique_ptr<Expression> Clone() const override {
    return std::make_unique<SizeofExpression>(location_, decl_type_);
  }

private:
  const Type *decl_type_;
};
} // namespace Cobold

#endif /* COBOLD_CORE_EXPRESSION */
