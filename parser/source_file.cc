#include "parser/source_file.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace Cobold {
    std::string SourceFile::DebugString() const {
        std::vector<std::string> functions;
        functions.reserve(functions_.size());
        for (const auto &function: functions_) {
            functions.push_back(function->DebugString());
        }
        return absl::StrCat(
            "SourceFile {\n",
            " imports {\n  ",
            absl::StrJoin(imports_, ",\n  "),
            "\n },\n",
            " functions {\n  ",
            absl::StrJoin(functions, ", \n  "),
            "\n },\n",
            "}\n"
        );
    }
}