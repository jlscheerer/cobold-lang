#ifndef COBOLD_INFERENCE_TYPE_INFERENCE_VISITOR
#define COBOLD_INFERENCE_TYPE_INFERENCE_VISITOR

#include "core/function.h"
#include "inference/type_context.h"
#include "parser/source_file.h"
#include "visitor/expression_visitor.h"
#include "visitor/statement_visitor.h"

namespace Cobold {
class TypeInferenceVisitor : private ExpressionVisitor<false, void>,
                             private StatementVisitor<false> {
public:
  static void Annotate(SourceFile &file);

private:
  TypeInferenceVisitor(SourceFile &file) : type_context_(file) {}
  void AnnotateFunction(DefinedFunction *function);

  static bool CanCastExplicitTo(const Type *from, const Type *to);
  static bool CanCastImplicitTo(const Type *from, const Type *to);

  static const Type *UnifyArrayTypes(std::vector<const Type *> &element_types);
  static const Type *IteratorType(const Type *type);

  static bool ArgumentTypesCompatible(std::vector<const Type *> expected_args,
                                      std::vector<const Type *> actual_args);

  // Statements
  void DispatchReturn(ReturnStatement *stmt) override;
  void DispatchAssignment(AssignmentStatement *stmt) override;
  void DispatchCompound(CompoundStatement *stmt) override;
  void DispatchExpression(ExpressionStatement *stmt) override;
  void DispatchIf(IfStatement *stmt) override;
  void DispatchFor(ForStatement *stmt) override;
  void DispatchWhile(WhileStatement *stmt) override;
  void DispatchDeclaration(DeclarationStatement *stmt) override;

  // Expressions
  void DispatchEmpty() override;
  void DispatchTernary(TernaryExpression *expr) override;
  void DispatchBinary(BinaryExpression *expr) override;
  void DispatchUnary(UnaryExpression *expr) override;
  void DispatchCall(CallExpression *expr) override;
  void DispatchRange(RangeExpression *expr) override;
  void DispatchArray(ArrayExpression *expr) override;
  void DispatchCast(CastExpression *expr) override;
  void DispatchConstant(ConstantExpression *expr) override;
  void DispatchIdentifier(IdentifierExpression *expr) override;
  void DispatchMemberAccess(MemberAccessExpression *expr) override;
  void DispatchArrayAccess(ArrayAccessExpression *expr) override;
  void DispatchCallOp(CallOpExpression *expr) override;

  std::unique_ptr<Expression>
  WrapExplicitCast(const Type *type, std::unique_ptr<Expression> &&expr);

  // TODO(jlscheerer) this should be part of the module
  TypeContext type_context_;
};
} // namespace Cobold

#endif /* COBOLD_INFERENCE_TYPE_INFERENCE_VISITOR */
