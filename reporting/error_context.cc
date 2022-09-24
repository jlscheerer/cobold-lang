#include "reporting/error_context.h"

#include "absl/strings/ascii.h"
#include <iostream>

namespace Cobold {
ReportedError MakeError(SourceLocation location, std::string message,
                        bool addl_context) {
  return ReportedError{
      .location = location, .message = message, .addl_context = addl_context};
}
// `ErrorContext` =======================================================
ErrorContext &ErrorContext::operator<<(ReportedError error) {
  errors_.push_back(error);
  return *this;
}

void ErrorContext::operator*() const {
  if (!ok()) {
    for (const auto &error : errors_) {
      const auto &location = error.location;
      bool context = error.addl_context;
      std::cout << "\x1B[1;37m" << location.filename() << ":" << location.line()
                << ":" << location.column() << ": "
                << "\x1B[1;31merror: \x1B[1;37m" << error.message << "\033[0m"
                << std::endl;
      if (context && location.line() >= 2) {
        std::string context = location.buffer()[location.line() - 2];
        std::string_view stripped_context = absl::StripTrailingAsciiWhitespace(
            absl::StripLeadingAsciiWhitespace(context));
        if (stripped_context.size() > 0) {
          std::cout << location.buffer()[location.line() - 2] << std::endl;
        }
      }
      std::cout << location.buffer()[location.line() - 1] << std::endl;
      std::cout << std::string(location.column(), ' ') << "\x1B[32m^\033[0m"
                << std::endl;
    }
    std::cout << errors_.size() << " error(s) generated." << std::endl;
    std::exit(-1);
  }
}
// `ErrorContext` =======================================================
} // namespace Cobold