#ifndef COBOLD_PARSER_SOURCE_FILE
#define COBOLD_PARSER_SOURCE_FILE

#include <vector>
#include <string>

#include "core/function.h"

namespace Cobold {
class SourceFile {
public:
    const std::vector<std::string> imports() const {
        return imports_;
    }
    std::string DebugString() const;
private:
    std::vector<std::string> imports_;
    std::vector<std::unique_ptr<Function>> functions_;

    friend class Parser;
};
}

#endif /* COBOLD_PARSER_SOURCE_FILE */
