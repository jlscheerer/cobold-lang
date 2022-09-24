#ifndef COBOLD_REPORTING_ERROR_CONTEXT
#define COBOLD_REPORTING_ERROR_CONTEXT

#include <string>
#include <vector>

#include "parser/source_location.h"

namespace Cobold {
struct ReportedError {
  SourceLocation location;
  std::string message;
  bool addl_context;
};
ReportedError MakeError(SourceLocation location, std::string message,
                        bool addl_context = false);

class ErrorContext {
public:
  ErrorContext &operator<<(ReportedError error);
  bool ok() const { return errors_.size() == 0; }
  void operator*() const;

private:
  std::vector<ReportedError> errors_;
};
} // namespace Cobold

#endif /* COBOLD_REPORTING_ERROR_CONTEXT */
