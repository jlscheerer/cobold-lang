#ifndef COBOLD_CORE_STATEMENT
#define COBOLD_CORE_STATEMENT

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "core/expression.h"
#include "core/type.h"

namespace Cobold {
enum class StatementType {
  Return,
  Assignment,
  Compound,
  Expression,
  If,
  For,
  While,
  Declaration,
  Break,
  Continue
};

class Statement {
public:
  virtual StatementType type() const = 0;

  template <typename T> const T *As() const {
    static_assert(std::is_base_of_v<Statement, T>,
                  "Attempting to cast to non-derived class.");
    // TODO(jlscheerer) check for `nullptr`
    return dynamic_cast<const T *>(this);
  }

  template <typename T> T *As() {
    static_assert(std::is_base_of_v<Statement, T>,
                  "Attempting to cast to non-derived class.");
    // TODO(jlscheerer) check for `nullptr`
    return dynamic_cast<T *>(this);
  }

  virtual ~Statement() = default;
};

enum class AssignmentType {
  EQ,     // =
  MUL_EQ, // *=
  DIV_EQ, // /=
  MOD_EQ, // %=
  ADD_EQ, // +=
  SUB_EQ, // -=
  SHL_EQ, // <<=
  SHR_EQ, // >>=
  AND_EQ, // &=
  XOR_EQ, // ^=
  OR_EQ,  // |=
};

class AssignmentStatement : public Statement {
public:
  AssignmentStatement(std::unique_ptr<Expression> &&lhs,
                      const std::string &assign_type,
                      std::unique_ptr<Expression> &&rhs)
      : AssignmentStatement(std::move(lhs), TypeFromString(assign_type),
                            std::move(rhs)) {}

  AssignmentStatement(std::unique_ptr<Expression> &&lhs,
                      AssignmentType assign_type,
                      std::unique_ptr<Expression> &&rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)), assgn_type_(assign_type) {}

  const Expression *lhs() const { return lhs_.get(); }
  Expression *mutable_lhs() { return lhs_.get(); }

  const AssignmentType assgn_type() const { return assgn_type_; }

  const Expression *rhs() const { return rhs_.get(); }
  Expression *mutable_rhs() { return rhs_.get(); }

  StatementType type() const { return StatementType::Assignment; }

  static std::string TypeToString(const AssignmentType assgn_type);

private:
  static AssignmentType TypeFromString(const std::string &assign_type);

  std::unique_ptr<Expression> lhs_, rhs_;
  AssignmentType assgn_type_;

  friend class TypeInferenceVisitor;
};

class CompoundStatement : public Statement {
public:
  CompoundStatement()
      : CompoundStatement(std::vector<std::unique_ptr<Statement>>{}) {}
  CompoundStatement(std::vector<std::unique_ptr<Statement>> &&statements)
      : statements_(std::move(statements)) {}

  const std::vector<std::unique_ptr<Statement>> &statements() const {
    return statements_;
  }
  StatementType type() const { return StatementType::Compound; }

private:
  std::vector<std::unique_ptr<Statement>> statements_;
};

class ExpressionStatement : public Statement {
public:
  ExpressionStatement(std::unique_ptr<Expression> &&expression)
      : expression_(std::move(expression)) {}

  const Expression *expression() const { return expression_.get(); }
  Expression *mutable_expression() { return expression_.get(); }

  StatementType type() const { return StatementType::Expression; }

private:
  std::unique_ptr<Expression> expression_;

  friend class TypeInferenceVisitor;
};

class ReturnStatement : public ExpressionStatement {
public:
  using ExpressionStatement::ExpressionStatement;

  StatementType type() const { return StatementType::Return; }
};

struct IfBranch {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<CompoundStatement> body;
};

class IfStatement : public Statement {
public:
  IfStatement() : IfStatement(std::vector<IfBranch>{}) {}
  IfStatement(std::vector<IfBranch> &&branches)
      : branches_(std::move(branches)) {}

  const std::vector<IfBranch> &branches() const { return branches_; }
  StatementType type() const { return StatementType::If; }

private:
  std::vector<IfBranch> branches_;
};

class WhileStatement : public Statement {
public:
  WhileStatement(std::unique_ptr<Expression> &&condition,
                 std::unique_ptr<CompoundStatement> &&body)
      : condition_(std::move(condition)), body_(std::move(body)) {}

  const Expression *condition() const { return condition_.get(); }
  Expression *mutable_condition() { return condition_.get(); }

  const std::unique_ptr<CompoundStatement> &body() const { return body_; }
  StatementType type() const { return StatementType::While; }

private:
  std::unique_ptr<Expression> condition_;
  std::unique_ptr<CompoundStatement> body_;

  friend class TypeInferenceVisitor;
};

class DeclarationStatement : public Statement {
public:
  DeclarationStatement(bool is_const, std::string identifier,
                       const Type *decl_type,
                       std::unique_ptr<Expression> &&expression)
      : is_const_(is_const), identifier_(identifier), decl_type_(decl_type),
        expression_(std::move(expression)) {}

  const bool is_const() const { return is_const_; }
  const std::string &identifier() const { return identifier_; }
  const Type *decl_type() const { return decl_type_; }

  const Expression *expression() const { return expression_.get(); }
  Expression *mutable_expression() { return expression_.get(); }

  StatementType type() const { return StatementType::Declaration; }

private:
  void infer_type(const Type *decl_type) { decl_type_ = decl_type; }

  bool is_const_; // "var" -> !is_const, "let" -> is_const
  std::string identifier_;
  const Type *decl_type_;
  std::unique_ptr<Expression> expression_;

  friend class TypeInferenceVisitor;
};

class ForStatement : public DeclarationStatement {
public:
  ForStatement(std::string identifier, const Type *decl_type,
               std::unique_ptr<Expression> &&expression,
               std::unique_ptr<CompoundStatement> &&body)
      : DeclarationStatement(/*is_const=*/false, identifier, decl_type,
                             std::move(expression)),
        body_(std::move(body)) {}

  const std::unique_ptr<CompoundStatement> &body() const { return body_; }

  StatementType type() const { return StatementType::For; }

private:
  std::unique_ptr<CompoundStatement> body_;
};

class BreakStatement : public Statement {
  StatementType type() const { return StatementType::Break; }
};

class ContinueStatement : public Statement {
  StatementType type() const { return StatementType::Continue; }
};
} // namespace Cobold

#endif /* COBOLD_CORE_STATEMENT */
