#include "core/function.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace Cobold {
// `Function` ===========================================================
std::string Function::GetSignature() const {
  std::vector<std::string> arguments;
  arguments.reserve(arguments_.size());
  for (const auto &argument : arguments_) {
    arguments.push_back(argument.DebugString());
  }
  return absl::StrCat(name_, "(", absl::StrJoin(arguments, ", "), ") -> ",
                      return_type_->DebugString());
}
// `Function` ===========================================================
} // namespace Cobold