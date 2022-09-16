#include "parser/source_file.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace Cobold {
    std::string SourceFile::DebugString() const {
        return absl::StrCat(
            "SourceFile {\n",
            " imports {\n  ",
            absl::StrJoin(imports_, ",\n  "),
            "\n },\n",
            "}\n"
        );
    }
}