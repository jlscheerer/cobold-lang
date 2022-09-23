#ifndef COBOLD_PARSER_SOURCE_FILE
#define COBOLD_PARSER_SOURCE_FILE

#include <memory>
#include <string>
#include <vector>

#include "core/function.h"

namespace Cobold {
class SourceFile {
public:
  SourceFile(const std::string &filename) : filename_(filename) {}
  const std::string &filename() { return filename_; }
  const std::vector<std::string> &imports() const { return imports_; }
  const std::vector<std::unique_ptr<Function>> &functions() const {
    return functions_;
  }
  std::string DebugString() const;

private:
  std::string filename_;
  std::vector<std::string> imports_;
  std::vector<std::unique_ptr<Function>> functions_;

  friend class Parser;
};
} // namespace Cobold

#endif /* COBOLD_PARSER_SOURCE_FILE */
