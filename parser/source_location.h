#ifndef COBOLD_PARSER_SOURCE_LOCATION
#define COBOLD_PARSER_SOURCE_LOCATION

#include <string>
#include <vector>
namespace Cobold {
class SourceLocation {
public:
  SourceLocation(const std::string &filename, int line, int column,
                 const std::vector<std::string> &buffer)
      : filename_(&filename), line_(line), column_(column), buffer_(&buffer) {}

  const std::string &filename() const { return *filename_; }
  const int line() const { return line_; }
  const int column() const { return column_; }
  const std::vector<std::string> &buffer() const { return *buffer_; }

private:
  const std::string *filename_;
  int line_, column_;
  const std::vector<std::string> *buffer_;
};
} // namespace Cobold

#endif /* COBOLD_PARSER_SOURCE_LOCATION */
