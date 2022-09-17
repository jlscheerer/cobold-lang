#ifndef COBOLD_PARSER_PARSER
#define COBOLD_PARSER_PARSER

#include <string>

#include "absl/status/statusor.h"
#include "parser/internal/CoboldBaseVisitor.h"
#include "parser/internal/CoboldLexer.h"
#include "parser/internal/CoboldParser.h"
#include "parser/internal/options.h"
#include "parser/source_file.h"
#include "core/function.h"

namespace Cobold {
class Parser {
public:
    static absl::StatusOr<SourceFile> Parse(const std::string &filename);

private:
    static absl::StatusOr<SourceFile> InternalParse(const std::string &filename, CoboldParser::FileContext *ctxt);
    
    absl::StatusOr<SourceFile> ParseFile(const std::string &filename, CoboldParser::FileContext *ctx);
    absl::StatusOr<std::string> ParseImport(CoboldParser::ImportDeclarationContext *ctx);
    absl::StatusOr<std::unique_ptr<Function>> ParseFunction(CoboldParser::FunctionDeclarationContext *ctx);
    absl::StatusOr<const Type *> ParseType(CoboldParser::TypeSpecifierContext *ctx, bool allow_void = false);

    absl::StatusOr<std::string> ParseExternSpecifier(CoboldParser::ExternSpecifierContext *ctx);
};
}

#endif /* COBOLD_PARSER_PARSER */
