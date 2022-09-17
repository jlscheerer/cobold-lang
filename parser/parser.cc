#include "parser/parser.h"

#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/status/status.h"
#include "core/function.h"

namespace Cobold {
namespace {
  template<typename T>
  absl::Status InvalidArgument(std::string type, T *t) {
    return absl::InvalidArgumentError(
      absl::StrCat("Invalid ", type, ": ", t != nullptr ? "\"" + t->getText() + "\"" : "null")
    );
  }
}

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

absl::StatusOr<SourceFile> Parser::InternalParse(const std::string &filename, CoboldParser::FileContext *ctx) {
  Parser parser;
  return parser.ParseFile(filename, ctx);
}

absl::StatusOr<SourceFile> Parser::ParseFile(const std::string &filename, CoboldParser::FileContext *ctx) {
  SourceFile file(filename);
  file.imports_.reserve(ctx->importDeclaration().size());
  for (const auto& import: ctx->importDeclaration()) {
    absl::StatusOr<std::string> parsed_import =  ParseImport(import);
    if (!parsed_import.ok()) return parsed_import.status();
    file.imports_.push_back(*parsed_import);
  }
  file.functions_.reserve(ctx->functionDeclaration().size());
  for (const auto &function: ctx->functionDeclaration()) {
    absl::StatusOr<std::unique_ptr<Function>> parsed_fn = ParseFunction(function);
    if (!parsed_fn.ok()) return parsed_fn.status();
    file.functions_.push_back(*std::move(parsed_fn));
  }
  return file;
}

absl::StatusOr<std::string> Parser::ParseImport(CoboldParser::ImportDeclarationContext *ctx) {
  if (ctx == nullptr || ctx->StringConstant() == nullptr) return InvalidArgument("Import", ctx);
  std::string import = ctx->StringConstant()->getText();
  // Empty Import ("") is not allowed!
  if (import.size() < 3 || import[0] != '"' || import[import.size() - 1] != '"') {
    return InvalidArgument("Import", ctx->StringConstant());
  }
  return import.substr(1, import.size() - 2);
}

absl::StatusOr<std::unique_ptr<Function>> Parser::ParseFunction(CoboldParser::FunctionDeclarationContext *ctx) {
  std::string name = ctx->Identifier()->getText();
  const Type *return_type;
  {
    absl::StatusOr<const Type *> status_or_type = ParseType(ctx->typeSpecifier(), /*allow_void*/true);
    if (!status_or_type.ok()) return status_or_type.status();
    return_type = *status_or_type;
  }
  std::vector<FunctionArgument> arguments;
  for (auto *args = ctx->argumentList(); args != nullptr; args = args->argumentList()) {
    absl::StatusOr<const Type *> status_or_type = ParseType(args->typeSpecifier(), /*allow_void*/false);
    if (!status_or_type.ok()) return status_or_type.status();
    arguments.push_back({args->Identifier()->toString(), *status_or_type});
  }
  if (ctx->externSpecifier()) {
    absl::StatusOr<std::string> status_or_specifier = ParseExternSpecifier(ctx->externSpecifier());
    if (!status_or_specifier.ok()) return status_or_specifier.status();
    return std::make_unique<ExternFunction>(std::move(name), std::move(arguments), return_type, std::move(*status_or_specifier));
  }
  CompoundStatement body;
  return std::make_unique<DefinedFunction>(std::move(name), std::move(arguments), return_type, std::move(body));
}

absl::StatusOr<const Type *> Parser::ParseType(CoboldParser::TypeSpecifierContext *ctx, bool allow_void) {
  if (ctx == nullptr) {
    if (allow_void) return NilType::Get();
    return InvalidArgument("Type", ctx);
  }
  if (ctx->I32()) return IntegralType::OfSize(32);
  if (ctx->I64()) return IntegralType::OfSize(64);
  if (ctx->STRING()) return StringType::Get();
  absl::StatusOr<const Type *> status_or_type = ParseType(ctx->typeSpecifier(), /*allow_void*/false);
  if (!status_or_type.ok()) return status_or_type.value();
  if (ctx->LBRACKET()) {
    assert(ctx->RBRACKET());
    return Type::ArrayOf(*status_or_type);
  }
  assert(ctx->POINTER());
  return Type::PointerTo(*status_or_type);
}

absl::StatusOr<std::string> Parser::ParseExternSpecifier(CoboldParser::ExternSpecifierContext *ctx) {
  if (ctx == nullptr || ctx->StringConstant() == nullptr) return InvalidArgument("ExternSpecifier", ctx);
  std::string specifier = ctx->StringConstant()->getText();
  // Empty Specifier ("") is not allowed!
  if (specifier.size() < 3 || specifier[0] != '"' || specifier[specifier.size() - 1] != '"') {
    return InvalidArgument("ExternSpecifier", ctx->StringConstant());
  }
  return specifier.substr(1, specifier.size() - 2);
}
}  // namespace Cobold
