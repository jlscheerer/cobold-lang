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

namespace Cobold {
namespace {
template <typename T> absl::Status InvalidArgument(std::string type, T *t) {
  return absl::InvalidArgumentError(
      absl::StrCat("Invalid ", type, ": ",
                   t != nullptr ? "\"" + t->getText() + "\"" : "null"));
}
} // namespace

absl::StatusOr<SourceFile> Parser::Parse(const std::string &filename) {
  std::ifstream source_file(filename);
  if (!source_file.is_open()) {
    return absl::InvalidArgumentError("Could not open source file.");
  }

  antlr4::ANTLRInputStream input(source_file);
  CoboldLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  CoboldParser _parser(&tokens);
  CoboldParser::FileContext *file = _parser.file();

  source_file.close();
  tokens.fill();
  return InternalParse(filename, file);
}

absl::StatusOr<SourceFile>
Parser::InternalParse(const std::string &filename,
                      CoboldParser::FileContext *ctx) {
  Parser parser;
  return parser.ParseFile(filename, ctx);
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
  return nullptr;
}
} // namespace Cobold
