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
  return InternalParse(file);
}

absl::StatusOr<SourceFile> Parser::InternalParse(CoboldParser::FileContext *ctx) {
  Parser parser;
  return parser.ParseFile(ctx);
}

absl::StatusOr<SourceFile> Parser::ParseFile(CoboldParser::FileContext *ctx) {
  SourceFile file;
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
  Type *return_type;
  {
    absl::StatusOr<Type *> status_or_type = ParseType(ctx->typeSpecifier(), /*allow_void*/true);
    if (!status_or_type.ok()) return status_or_type.status();
    return_type = *status_or_type;
  }
  std::vector<Type *> argument_types;
  for (auto *args = ctx->argumentList(); args != nullptr; args = args->argumentList()) {
    absl::StatusOr<Type *> status_or_type = ParseType(ctx->typeSpecifier(), /*allow_void*/false);
    if (!status_or_type.ok()) return status_or_type.status();
    argument_types.push_back(*status_or_type);
  }
  if (ctx->externSpecifier()) {
    std::string specifier = ctx->externSpecifier()->StringConstant()->getText();
    std::cout << specifier << std::endl;
    return std::make_unique<ExternFunction>(std::move(name), std::move(argument_types), return_type);
  }
  return std::make_unique<Function>(std::move(name), std::move(argument_types), return_type);
}

absl::StatusOr<Type *> Parser::ParseType(CoboldParser::TypeSpecifierContext *ctx, bool allow_void) {
  return nullptr;
}
}  // namespace Cobold
