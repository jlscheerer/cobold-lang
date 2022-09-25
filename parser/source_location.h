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

  static const SourceLocation Generated() {
    return SourceLocation(SOURCE_LOCATION_GENERATED);
  }

  static const SourceLocation Complex() {
    return SourceLocation(SOURCE_LOCATION_COMPLEX);
  }

private:
  // Generated code by the parser/type_checker
  static constexpr int SOURCE_LOCATION_GENERATED = -1;

  // Represents a complex object without a direct location
  // TODO(jlscheerer) Use a SourceSpan here instead
  static constexpr int SOURCE_LOCATION_COMPLEX = -2;

  SourceLocation(int code)
      : filename_(nullptr), line_(0), column_(code), buffer_(nullptr) {}

  const std::string *filename_;
  int line_, column_;
  const std::vector<std::string> *buffer_;
};
} // namespace Cobold

#endif /* COBOLD_PARSER_SOURCE_LOCATION */
