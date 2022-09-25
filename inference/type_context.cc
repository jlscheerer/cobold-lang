#include "inference/type_context.h"
#include <optional>
#include <utility>

namespace Cobold {
// `TypeContext` ========================================================
TypeContext::TypeContext(const SourceFile &file) {
  for (const auto &fn : file.functions()) {
    assert(!functions_.contains(fn->name()));
    std::vector<const Type *> arg_types;
    arg_types.reserve(fn->arguments().size());
    for (const auto &arg : fn->arguments()) {
      arg_types.push_back(arg.type);
    }
    functions_[fn->name()] = {fn->return_type(), arg_types};
  }
}

std::optional<const Type *>
TypeContext::LookupVar(const std::string &identifier) {
  for (int i = definitions_.size() - 1; i >= 0; --i) {
    if (definitions_[i].vars.contains(identifier))
      return definitions_[i].vars[identifier];
  }
  return std::nullopt;
}

std::optional<std::pair<const Type *, std::vector<const Type *>>>
TypeContext::LookupFn(const std::string &identifier) {
  if (!functions_.contains(identifier))
    return std::nullopt;
  return functions_[identifier];
}

bool TypeContext::StoreVar(const std::string &identifier, const Type *type) {
  assert(definitions_.size() > 0);
  if (definitions_.back().vars.contains(identifier))
    return false;
  definitions_.back().vars[identifier] = type;
  return true;
}
// `TypeContext` ========================================================
} // namespace Cobold