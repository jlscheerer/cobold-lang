#ifndef COBOLD_PARSER_PARSER
#define COBOLD_PARSER_PARSER

#include <string>

#include "absl/status/statusor.h"
#include "core/expression.h"
#include "core/function.h"
#include "parser/internal/CoboldBaseVisitor.h"
#include "parser/internal/CoboldLexer.h"
#include "parser/internal/CoboldParser.h"
#include "parser/internal/options.h"
#include "parser/source_file.h"

namespace Cobold {
class Parser {
public:
  static absl::StatusOr<SourceFile> Parse(const std::string &filename);

private:
  static absl::StatusOr<SourceFile>
  InternalParse(const std::string &filename, CoboldParser::FileContext *ctxt);

  absl::StatusOr<SourceFile> ParseFile(const std::string &filename,
                                       CoboldParser::FileContext *ctx);

  absl::StatusOr<std::string>
  ParseImport(CoboldParser::ImportDeclarationContext *ctx);
  absl::StatusOr<std::unique_ptr<Function>>
  ParseFunction(CoboldParser::FunctionDeclarationContext *ctx);

  absl::StatusOr<CompoundStatement>
  ParseFunctionBody(CoboldParser::CompoundStatementContext *ctx);
  absl::StatusOr<std::string>
  ParseExternSpecifier(CoboldParser::ExternSpecifierContext *ctx);

  absl::StatusOr<const Type *>
  ParseType(CoboldParser::TypeSpecifierContext *ctx, bool allow_void = false);

  // Statements
  absl::StatusOr<std::unique_ptr<Statement>>
  ParseBlockListItem(CoboldParser::BlockItemContext *ctx);
  absl::StatusOr<std::unique_ptr<Statement>>
  ParseStatement(CoboldParser::StatementContext *ctx);
  absl::StatusOr<std::unique_ptr<ReturnStatement>>
  ParseReturnStatement(CoboldParser::ReturnStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<AssignmentStatement>>
  ParseAssignmentStatement(CoboldParser::AssignmentStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<CompoundStatement>>
  ParseCompoundStatement(CoboldParser::CompoundStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<ExpressionStatement>>
  ParseExpressionStatement(CoboldParser::ExpressionStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<IfStatement>>
  ParseIfStatementStatement(CoboldParser::IfStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<ForStatement>>
  ParseForStatement(CoboldParser::ForStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<WhileStatement>>
  ParseWhileStatement(CoboldParser::WhileStatementContext *ctx);
  absl::StatusOr<std::unique_ptr<DeclarationStatement>>
  ParseDeclaration(CoboldParser::DeclarationContext *ctx);

  // Expressions
  absl::StatusOr<std::unique_ptr<Expression>>
  ParseExpression(CoboldParser::ExpressionContext *ctx);
};
} // namespace Cobold

#endif /* COBOLD_PARSER_PARSER */
