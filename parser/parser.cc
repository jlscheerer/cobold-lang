#include "parser/parser.h"

#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "core/expression.h"
#include "core/function.h"
#include "source_location.h"

namespace Cobold {
namespace {
template <typename T> absl::Status InvalidArgument(std::string type, T *t) {
  return absl::InvalidArgumentError(
      absl::StrCat("Invalid ", type, ": ",
                   t != nullptr ? "\"" + t->getText() + "\"" : "null"));
}

std::vector<std::string> ReadFileToBuffer(const std::string &filename) {
  std::string str;
  std::vector<std::string> file_contents;

  std::fstream file;
  file.open(filename, std::ios::in);

  while (getline(file, str)) {
    file_contents.push_back(str);
  }
  return file_contents;
}
} // namespace
  // `ParserErrorListener` ================================================
void ParserErrorListener::syntaxError(antlr4::Recognizer *recognizer,
                                      antlr4::Token *offendingSymbol,
                                      size_t line, size_t charPositionInLine,
                                      const std::string &msg,
                                      std::exception_ptr e) {
  parser_->error_context_ << MakeError(SourceLocation(parser_->filename_, line,
                                                      charPositionInLine,
                                                      parser_->buffer_),
                                       msg, true);
}

void ParserErrorListener::reportAmbiguity(antlr4::Parser *recognizer,
                                          const antlr4::dfa::DFA &dfa,
                                          size_t startIndex, size_t stopIndex,
                                          bool exact,
                                          const antlrcpp::BitSet &ambigAlts,
                                          antlr4::atn::ATNConfigSet *configs) {
  // TODO(jlscheerer) Switch to SourceSpan once this is supported.
  assert(false);
}

void ParserErrorListener::reportAttemptingFullContext(
    antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
    size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
    antlr4::atn::ATNConfigSet *configs) {
  assert(false);
}

void ParserErrorListener::reportContextSensitivity(
    antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
    size_t stopIndex, size_t prediction, antlr4::atn::ATNConfigSet *configs) {
  assert(false);
}
// `ParserErrorListener` ================================================

// `Parser` =============================================================
absl::StatusOr<SourceFile> Parser::Parse(const std::string &filename) {
  std::ifstream source_file(filename);
  if (!source_file.is_open()) {
    return absl::InvalidArgumentError("Could not open source file.");
  }

  Parser parser(filename, ReadFileToBuffer(filename));

  antlr4::ANTLRInputStream input(source_file);
  CoboldLexer lexer(&input);

  lexer.removeErrorListeners();
  lexer.addErrorListener(&parser.listener_);
  *parser.error_context_;

  antlr4::CommonTokenStream tokens(&lexer);
  CoboldParser _parser(&tokens);

  _parser.removeErrorListeners();
  _parser.addErrorListener(&parser.listener_);

  CoboldParser::FileContext *file = _parser.file();
  *parser.error_context_;

  source_file.close();
  tokens.fill();
  return parser.ParseFile(filename, file);
}

absl::StatusOr<SourceFile> Parser::ParseFile(const std::string &filename,
                                             CoboldParser::FileContext *ctx) {
  SourceFile file(filename);
  file.imports_.reserve(ctx->importDeclaration().size());
  for (const auto &import : ctx->importDeclaration()) {
    absl::StatusOr<std::string> parsed_import = ParseImport(import);
    if (!parsed_import.ok())
      return parsed_import.status();
    file.imports_.push_back(*parsed_import);
  }
  file.functions_.reserve(ctx->functionDeclaration().size());
  for (const auto &function : ctx->functionDeclaration()) {
    absl::StatusOr<std::unique_ptr<Function>> parsed_fn =
        ParseFunction(function);
    if (!parsed_fn.ok())
      return parsed_fn.status();
    file.functions_.push_back(*std::move(parsed_fn));
  }
  *error_context_;
  return file;
}

absl::StatusOr<std::string>
Parser::ParseImport(CoboldParser::ImportDeclarationContext *ctx) {
  if (ctx == nullptr || ctx->StringConstant() == nullptr)
    return InvalidArgument("Import", ctx);
  std::string import = ctx->StringConstant()->getText();
  // Empty Import ("") is not allowed!
  if (import.size() < 3 || import[0] != '"' ||
      import[import.size() - 1] != '"') {
    return InvalidArgument("Import", ctx->StringConstant());
  }
  return import.substr(1, import.size() - 2);
}

absl::StatusOr<std::unique_ptr<Function>>
Parser::ParseFunction(CoboldParser::FunctionDeclarationContext *ctx) {
  std::string name = ctx->Identifier()->getText();
  const Type *return_type;
  {
    absl::StatusOr<const Type *> status_or_type =
        ParseType(ctx->typeSpecifier(), /*allow_void*/ true);
    if (!status_or_type.ok())
      return status_or_type.status();
    return_type = *status_or_type;
  }
  std::vector<FunctionArgument> arguments;
  for (auto *args = ctx->argumentList(); args != nullptr;
       args = args->argumentList()) {
    absl::StatusOr<const Type *> status_or_type =
        ParseType(args->typeSpecifier(), /*allow_void*/ false);
    if (!status_or_type.ok())
      return status_or_type.status();
    arguments.push_back({args->Identifier()->toString(), *status_or_type});
  }
  if (ctx->externSpecifier()) {
    absl::StatusOr<std::string> status_or_specifier =
        ParseExternSpecifier(ctx->externSpecifier());
    if (!status_or_specifier.ok())
      return status_or_specifier.status();
    return std::make_unique<ExternFunction>(std::move(name),
                                            std::move(arguments), return_type,
                                            std::move(*status_or_specifier));
  }
  absl::StatusOr<CompoundStatement> status_or_body =
      ParseFunctionBody(ctx->compoundStatement());
  if (!status_or_body.ok())
    return status_or_body.status();
  return std::make_unique<DefinedFunction>(std::move(name),
                                           std::move(arguments), return_type,
                                           std::move(*status_or_body));
}

absl::StatusOr<CompoundStatement>
Parser::ParseFunctionBody(CoboldParser::CompoundStatementContext *ctx) {
  absl::StatusOr<std::unique_ptr<CompoundStatement>> status_or_stmt =
      ParseCompoundStatement(ctx);
  if (!status_or_stmt.ok())
    return status_or_stmt.status();
  return std::move(*(status_or_stmt->release()));
}

absl::StatusOr<std::string>
Parser::ParseExternSpecifier(CoboldParser::ExternSpecifierContext *ctx) {
  if (ctx == nullptr || ctx->StringConstant() == nullptr)
    return InvalidArgument("ExternSpecifier", ctx);
  std::string specifier = ctx->StringConstant()->getText();
  // Empty Specifier ("") is not allowed!
  if (specifier.size() < 3 || specifier[0] != '"' ||
      specifier[specifier.size() - 1] != '"') {
    return InvalidArgument("ExternSpecifier", ctx->StringConstant());
  }
  return specifier.substr(1, specifier.size() - 2);
}

absl::StatusOr<const Type *>
Parser::ParseType(CoboldParser::TypeSpecifierContext *ctx, bool allow_void) {
  if (ctx == nullptr) {
    if (allow_void)
      return NilType::Get();
    return InvalidArgument("Type", ctx);
  }
  if (ctx->I32())
    return IntegralType::OfSize(32);
  if (ctx->I64())
    return IntegralType::OfSize(64);
  if (ctx->STRING())
    return StringType::Get();
  absl::StatusOr<const Type *> status_or_type =
      ParseType(ctx->typeSpecifier(), /*allow_void*/ false);
  if (!status_or_type.ok())
    return status_or_type.value();
  if (ctx->LBRACKET()) {
    assert(ctx->RBRACKET());
    return Type::ArrayOf(*status_or_type);
  }
  assert(ctx->POINTER());
  return Type::PointerTo(*status_or_type);
}

absl::StatusOr<std::unique_ptr<Statement>>
Parser::ParseBlockListItem(CoboldParser::BlockItemContext *ctx) {
  if (ctx->statement())
    return ParseStatement(ctx->statement());
  if (ctx->declaration())
    return ParseDeclaration(ctx->declaration());
  return InvalidArgument("Statement", ctx);
}

absl::StatusOr<std::unique_ptr<Statement>>
Parser::ParseStatement(CoboldParser::StatementContext *ctx) {
  if (ctx->returnStatement())
    return ParseReturnStatement(ctx->returnStatement());
  if (ctx->assignmentStatement())
    return ParseAssignmentStatement(ctx->assignmentStatement());
  if (ctx->compoundStatement())
    return ParseCompoundStatement(ctx->compoundStatement());
  if (ctx->expressionStatement())
    return ParseExpressionStatement(ctx->expressionStatement());
  if (ctx->ifStatement())
    return ParseIfStatementStatement(ctx->ifStatement());
  if (ctx->iterationStatement()) {
    if (ctx->iterationStatement()->forStatement())
      return ParseForStatement(ctx->iterationStatement()->forStatement());
    if (ctx->iterationStatement()->whileStatement())
      return ParseWhileStatement(ctx->iterationStatement()->whileStatement());
  }
  return InvalidArgument("Statement", ctx);
}

absl::StatusOr<std::unique_ptr<ReturnStatement>>
Parser::ParseReturnStatement(CoboldParser::ReturnStatementContext *ctx) {
  absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
      ParseExpression(ctx->expression());
  if (!status_or_expr.ok())
    return status_or_expr.status();
  return std::make_unique<ReturnStatement>(std::move(*status_or_expr));
}

absl::StatusOr<std::unique_ptr<AssignmentStatement>>
Parser::ParseAssignmentStatement(
    CoboldParser::AssignmentStatementContext *ctx) {
  absl::StatusOr<std::unique_ptr<Expression>> status_or_lhs =
      ParseExpression(ctx->expression(0));
  if (!status_or_lhs.ok())
    return status_or_lhs.status();
  absl::StatusOr<std::unique_ptr<Expression>> status_or_rhs =
      ParseExpression(ctx->expression(1));
  if (!status_or_rhs.ok())
    return status_or_rhs.status();
  return std::make_unique<AssignmentStatement>(
      std::move(*status_or_lhs), ctx->assignmentOperator()->getText(),
      std::move(*status_or_rhs));
}

absl::StatusOr<std::unique_ptr<CompoundStatement>>
Parser::ParseCompoundStatement(CoboldParser::CompoundStatementContext *ctx) {
  if (ctx == nullptr || ctx->blockItemList() == nullptr) {
    return std::make_unique<CompoundStatement>();
  }
  std::vector<std::unique_ptr<Statement>> statements;
  const auto &items = ctx->blockItemList();
  statements.reserve(items->blockItem().size());
  for (const auto &item : items->blockItem()) {
    absl::StatusOr<std::unique_ptr<Statement>> status_or_stmt =
        ParseBlockListItem(item);
    statements.push_back(std::move(*status_or_stmt));
  }
  return std::make_unique<CompoundStatement>(std::move(statements));
}

absl::StatusOr<std::unique_ptr<ExpressionStatement>>
Parser::ParseExpressionStatement(
    CoboldParser::ExpressionStatementContext *ctx) {
  absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
      ParseExpression(ctx->expression());
  if (!status_or_expr.ok())
    return status_or_expr.status();
  return std::make_unique<ExpressionStatement>(std::move(*status_or_expr));
}

absl::StatusOr<std::unique_ptr<IfStatement>>
Parser::ParseIfStatementStatement(CoboldParser::IfStatementContext *ctx) {
  assert(ctx->expression().size() == ctx->compoundStatement().size() ||
         ctx->expression().size() + 1 == ctx->compoundStatement().size());
  int cond_branches = ctx->expression().size();
  bool has_else =
      ctx->expression().size() + 1 == ctx->compoundStatement().size();
  std::vector<IfBranch> branches;
  for (int i = 0; i < cond_branches; ++i) {
    absl::StatusOr<std::unique_ptr<Expression>> status_or_cond =
        ParseExpression(ctx->expression(i));
    if (!status_or_cond.ok())
      return status_or_cond.status();
    absl::StatusOr<std::unique_ptr<CompoundStatement>> status_or_body =
        ParseCompoundStatement(ctx->compoundStatement(i));
    if (!status_or_body.ok())
      return status_or_body.status();
    branches.push_back(
        {std::move(*status_or_cond), std::move(*status_or_body)});
  }
  branches.push_back(
      {ConstantExpression::True(),
       std::make_unique<CompoundStatement>()}); // always have an "else" branch
  if (has_else) {
    absl::StatusOr<std::unique_ptr<CompoundStatement>> status_or_body =
        ParseCompoundStatement(ctx->compoundStatement(cond_branches));
    if (!status_or_body.ok())
      return status_or_body.status();
    branches.back().body = std::move(*status_or_body);
  }
  return std::make_unique<IfStatement>(std::move(branches));
}

absl::StatusOr<std::unique_ptr<ForStatement>>
Parser::ParseForStatement(CoboldParser::ForStatementContext *ctx) {
  std::string identifier = ctx->Identifier()->toString();
  const Type *decl_type = nullptr;
  if (ctx->typeSpecifier()) {
    absl::StatusOr<const Type *> status_or_type =
        ParseType(ctx->typeSpecifier());
    if (!status_or_type.ok())
      return status_or_type.status();
    decl_type = *status_or_type;
  }
  absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
      ParseExpression(ctx->expression());
  if (!status_or_expr.ok())
    return status_or_expr.status();
  absl::StatusOr<std::unique_ptr<CompoundStatement>> status_or_body =
      ParseCompoundStatement(ctx->compoundStatement());
  if (!status_or_body.ok())
    return status_or_body.status();
  return std::make_unique<ForStatement>(identifier, decl_type,
                                        std::move(*status_or_expr),
                                        std::move(*status_or_body));
}

absl::StatusOr<std::unique_ptr<WhileStatement>>
Parser::ParseWhileStatement(CoboldParser::WhileStatementContext *ctx) {
  absl::StatusOr<std::unique_ptr<Expression>> status_or_cond =
      ParseExpression(ctx->expression());
  if (!status_or_cond.ok())
    return status_or_cond.status();
  absl::StatusOr<std::unique_ptr<CompoundStatement>> status_or_body =
      ParseCompoundStatement(ctx->compoundStatement());
  if (!status_or_body.ok())
    return status_or_body.status();
  return std::make_unique<WhileStatement>(std::move(*status_or_cond),
                                          std::move(*status_or_body));
}

absl::StatusOr<std::unique_ptr<DeclarationStatement>>
Parser::ParseDeclaration(CoboldParser::DeclarationContext *ctx) {
  assert(ctx->LET() != nullptr || ctx->VAR() != nullptr);
  bool is_const = ctx->LET() != nullptr;
  std::string identifier = ctx->Identifier()->toString();
  const Type *decl_type = nullptr;
  if (ctx->typeSpecifier()) {
    absl::StatusOr<const Type *> status_or_type =
        ParseType(ctx->typeSpecifier());
    if (!status_or_type.ok())
      return status_or_type.status();
    decl_type = *status_or_type;
  }
  std::unique_ptr<Expression> expression;
  if (ctx->expression()) {
    assert(ctx->DASH_INIT() == nullptr);
    absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
        ParseExpression(ctx->expression());
    if (!status_or_expr.ok())
      return status_or_expr.status();
    expression = std::move(*status_or_expr);
  } else {
    // var example: i32; <=> var example: i32 = --;
    expression = ConstantExpression::DashInit();
  }
  return std::make_unique<DeclarationStatement>(
      is_const, std::move(identifier), decl_type, std::move(expression));
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParseExpression(CoboldParser::ExpressionContext *ctx) {
  if (ctx->conditionalExpression()) {
    return ParseConditionalExpression(ctx->conditionalExpression());
  }
  if (ctx->callExpression()) {
    return ParseCallExpression(ctx->callExpression());
  }
  if (ctx->rangeExpression()) {
    return ParseRangeExpression(ctx->rangeExpression());
  }
  if (ctx->arrayExpression()) {
    return ParseArrayExpression(ctx->arrayExpression());
  }
  assert(false);
}

absl::StatusOr<std::unique_ptr<Expression>> Parser::ParseConditionalExpression(
    CoboldParser::ConditionalExpressionContext *ctx) {
  absl::StatusOr<std::unique_ptr<Expression>> status_or_cond =
      ParseLogicalOrExpression(ctx->logicalOrExpression());
  if (!ctx->expression())
    return status_or_cond;
  assert(ctx->conditionalExpression());
  absl::StatusOr<std::unique_ptr<Expression>> status_or_true =
      ParseExpression(ctx->expression());
  if (!status_or_true.ok())
    return status_or_true.status();
  absl::StatusOr<std::unique_ptr<Expression>> status_or_false =
      ParseConditionalExpression(ctx->conditionalExpression());
  if (!status_or_false.ok())
    return status_or_false.status();
  return std::make_unique<TernaryExpression>(std::move(*status_or_cond),
                                             std::move(*status_or_true),
                                             std::move(*status_or_false));
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParseCallExpression(CoboldParser::CallExpressionContext *ctx) {
  std::string identifier = ctx->Identifier()->getText();
  std::vector<std::unique_ptr<Expression>> args;
  args.reserve(ctx->expression().size());
  for (const auto &expr : ctx->expression()) {
    absl::StatusOr<std::unique_ptr<Expression>> status_or_arg =
        ParseExpression(expr);
    if (!status_or_arg.ok())
      return status_or_arg.status();
    args.push_back(std::move(*status_or_arg));
  }
  return std::make_unique<CallExpression>(std::move(identifier),
                                          std::move(args));
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParseRangeExpression(CoboldParser::RangeExpressionContext *ctx) {
  std::unique_ptr<Expression> left;
  if (ctx->leftExpression()) {
    assert(ctx->leftExpression()->expression());
    absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
        ParseExpression(ctx->leftExpression()->expression());
    if (!status_or_expr.ok())
      return status_or_expr.status();
    left = std::move(*status_or_expr);
  }
  std::unique_ptr<Expression> right;
  if (ctx->rightExpression()) {
    assert(ctx->rightExpression()->expression());
    absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
        ParseExpression(ctx->rightExpression()->expression());
    if (!status_or_expr.ok())
      return status_or_expr.status();
    right = std::move(*status_or_expr);
  }
  return std::make_unique<RangeExpression>(std::move(left), std::move(right));
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParseArrayExpression(CoboldParser::ArrayExpressionContext *ctx) {
  std::vector<std::unique_ptr<Expression>> elements;
  elements.reserve(ctx->expression().size());
  for (const auto &expr : ctx->expression()) {
    absl::StatusOr<std::unique_ptr<Expression>> status_or_elem =
        ParseExpression(expr);
    if (!status_or_elem.ok())
      return status_or_elem.status();
    elements.push_back(std::move(*status_or_elem));
  }
  return std::make_unique<ArrayExpression>(std::move(elements));
}

#define EXPAND_EXPRESSION(FN_NAME)

#define EXPAND_BINARY_EXPRESSION(FN_NAME, FN_CTX, UNDERLYING_EXPR_PARSER,      \
                                 UNDERLYING_EXPR, SYMBOL)                      \
  absl::StatusOr<std::unique_ptr<Expression>> Parser::FN_NAME(                 \
      CoboldParser::FN_CTX *ctx) {                                             \
    const int n = ctx->UNDERLYING_EXPR().size();                               \
    assert(n >= 1);                                                            \
    absl::StatusOr<std::unique_ptr<Expression>> status_or_left =               \
        UNDERLYING_EXPR_PARSER(ctx->UNDERLYING_EXPR(0));                       \
    if (!status_or_left.ok())                                                  \
      return status_or_left.status();                                          \
    std::unique_ptr<Expression> expr = std::move(*status_or_left);             \
    for (int i = 1; i < n; ++i) {                                              \
      absl::StatusOr<std::unique_ptr<Expression>> status_or_right =            \
          UNDERLYING_EXPR_PARSER(ctx->UNDERLYING_EXPR(i));                     \
      if (!status_or_right.ok())                                               \
        return status_or_right.status();                                       \
      expr = std::make_unique<BinaryExpression>(                               \
          std::move(expr), BinaryExpression::TypeFromString(SYMBOL),           \
          std::move(*status_or_right));                                        \
    }                                                                          \
    return expr;                                                               \
  }

EXPAND_BINARY_EXPRESSION(ParseLogicalOrExpression, LogicalOrExpressionContext,
                         ParseLogicalAndExpression, logicalAndExpression, "||")

EXPAND_BINARY_EXPRESSION(ParseLogicalAndExpression, LogicalAndExpressionContext,
                         ParseInclusiveOrExpression, inclusiveOrExpression,
                         "&&")

EXPAND_BINARY_EXPRESSION(ParseInclusiveOrExpression,
                         InclusiveOrExpressionContext,
                         ParseExclusiveOrExpression, exclusiveOrExpression, "|")

EXPAND_BINARY_EXPRESSION(ParseExclusiveOrExpression,
                         ExclusiveOrExpressionContext, ParseAndExpression,
                         andExpression, "^")

EXPAND_BINARY_EXPRESSION(ParseAndExpression, AndExpressionContext,
                         ParseEqualityExpression, equalityExpression, "&")

EXPAND_BINARY_EXPRESSION(ParseEqualityExpression, EqualityExpressionContext,
                         ParseRelationalExpression, relationalExpression,
                         (ctx->equalityOperator(i - 1)->getText()))

EXPAND_BINARY_EXPRESSION(ParseRelationalExpression, RelationalExpressionContext,
                         ParseShiftExpression, shiftExpression,
                         (ctx->relationalOperator(i - 1)->getText()))

EXPAND_BINARY_EXPRESSION(ParseShiftExpression, ShiftExpressionContext,
                         ParseAddtiveExpression, additiveExpression,
                         (ctx->shiftOperator(i - 1)->getText()))

EXPAND_BINARY_EXPRESSION(ParseAddtiveExpression, AdditiveExpressionContext,
                         ParseMultiplicativeExpression,
                         multiplicativeExpression,
                         (ctx->additiveOperator(i - 1)->getText()))

EXPAND_BINARY_EXPRESSION(ParseMultiplicativeExpression,
                         MultiplicativeExpressionContext, ParseCastExpression,
                         castExpression,
                         (ctx->multiplicativeOperator(i - 1)->getText()))

#undef EXPAND_BINARY_EXPRESSION

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParseCastExpression(CoboldParser::CastExpressionContext *ctx) {
  if (ctx->unaryExpression())
    return ParseUnaryExpression(ctx->unaryExpression());
  assert(ctx->typeSpecifier() && ctx->castExpression());
  absl::StatusOr<const Type *> status_or_type = ParseType(ctx->typeSpecifier());
  absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
      ParseCastExpression(ctx->castExpression());
  if (!status_or_expr.ok())
    return status_or_expr.status();
  return std::make_unique<CastExpression>(std::move(*status_or_type),
                                          std::move(*status_or_expr));
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParseUnaryExpression(CoboldParser::UnaryExpressionContext *ctx) {
  std::unique_ptr<Expression> expr;
  if (ctx->postfixExpression()) {
    absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
        ParsePostfixExpression(ctx->postfixExpression());
    if (!status_or_expr.ok())
      return status_or_expr.status();
    expr = std::move(*status_or_expr);
  } else {
    assert(ctx->unaryOperator() && ctx->castExpression());
    absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
        ParseCastExpression(ctx->castExpression());
    if (!status_or_expr.ok())
      return status_or_expr.status();
    expr = std::make_unique<UnaryExpression>(
        UnaryExpression::TypeFromPrefixString(ctx->unaryOperator()->getText()),
        std::move(*status_or_expr));
  }
  for (const auto &op : ctx->prefixOperator()) {
    expr = std::make_unique<UnaryExpression>(
        UnaryExpression::TypeFromPrefixString(op->getText()), std::move(expr));
  }
  return expr;
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParsePostfixExpression(CoboldParser::PostfixExpressionContext *ctx) {
  absl::StatusOr<std::unique_ptr<Expression>> status_or_expr =
      ParsePrimaryExpression(ctx->primaryExpression());
  if (!status_or_expr.ok())
    return status_or_expr.status();
  std::unique_ptr<Expression> expr = std::move(*status_or_expr);
  for (auto op = ctx->postfixOperations(); op != nullptr;
       op = op->postfixOperations()) {
    // TODO(jlscheerer) Clean up this code.
    // For now we can determine the type of operation based on the first
    // character of the `postfixOperations` object.
    char op_type = op->getText()[0];
    if (op_type == '[') {
      // array access operation a[..]
      assert(op->expression());
      absl::StatusOr<std::unique_ptr<Expression>> status_or_index =
          ParseExpression(op->expression());
      if (!status_or_index.ok())
        return status_or_index.status();
      expr = std::make_unique<ArrayAccessExpression>(
          std::move(expr), std::move(*status_or_index));
    } else if (op_type == '(') {
      // call operation a(...)
      std::vector<std::unique_ptr<Expression>> args;
      if (op->argumentExpressionList()) {
        for (const auto &cond_expr :
             op->argumentExpressionList()->conditionalExpression()) {
          absl::StatusOr<std::unique_ptr<Expression>> status_or_arg =
              ParseConditionalExpression(cond_expr);
          if (!status_or_arg.ok())
            return status_or_arg.status();
          args.push_back(std::move(*status_or_arg));
        }
      }
      expr =
          std::make_unique<CallOpExpression>(std::move(expr), std::move(args));
    } else if (op_type == '.' || (op_type == '-' && op->Identifier())) {
      // member access a.identifier or a->identifier
      assert(op->Identifier());
      expr = std::make_unique<MemberAccessExpression>(
          std::move(expr), op_type == '.', op->Identifier()->getText());
    } else if (op_type == '+' || (op_type == '-' && !op->Identifier())) {
      // post increment/decrement
      UnaryExpressionType post_type = (op_type == '+')
                                          ? UnaryExpressionType::POST_INCREMENT
                                          : UnaryExpressionType::POST_DECREMENT;
      expr = std::make_unique<UnaryExpression>(post_type, std::move(expr));
    } else {
      assert(false);
    }
  }
  return expr;
}

absl::StatusOr<std::unique_ptr<Expression>>
Parser::ParsePrimaryExpression(CoboldParser::PrimaryExpressionContext *ctx) {
  if (ctx->StringConstant()) {
    return ConstantExpression::String(ctx->StringConstant()->getText());
  } else if (ctx->IntegerConstant()) {
    return ConstantExpression::Integer(ctx->IntegerConstant()->getText());
  } else if (ctx->FloatingConstant()) {
    return ConstantExpression::Floating(ctx->FloatingConstant()->getText());
  }
  return std::make_unique<IdentifierExpression>(ctx->Identifier()->getText());
}

SourceLocation Parser::LocationOf(antlr4::tree::TerminalNode *node) {
  return SourceLocation(filename_, node->getSymbol()->getLine(),
                        node->getSymbol()->getCharPositionInLine(), buffer_);
}
// `Parser` =============================================================
} // namespace Cobold