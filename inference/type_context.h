#ifndef COBOLD_INFERENCE_TYPE_CONTEXT
#define COBOLD_INFERENCE_TYPE_CONTEXT

#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "core/type.h"
#include "parser/source_file.h"

namespace Cobold {
struct VarDefinitionScope {
  // TODO(jlscheerer) Change this to the definition to check for let/var
  absl::flat_hash_map<std::string, const Type *> vars;
};

class TypeContext {
public:
  TypeContext(const SourceFile &file);
  std::optional<const Type *> LookupVar(const std::string &identifier);

  // TODO(jlscheerer) Improve the method signature.
  std::optional<std::pair<const Type *, std::vector<const Type *>>>
  LookupFn(const std::string &identifier);

  bool StoreVar(const std::string &identifier, const Type *type);

  void PushScope() { definitions_.push_back({}); }
  void PopScope() { definitions_.pop_back(); }

  void PushFunctionReturn(const Type *type) {
    assert(function_return_ == nullptr);
    function_return_ = type;
  }
  const Type *FunctionReturnType() const { return function_return_; }
  void PopFunctionReturn() {
    assert(function_return_ != nullptr);
    function_return_ = nullptr;
  }

private:
  const Type *function_return_ = nullptr; // return type of the current function

  absl::flat_hash_map<std::string,
                      std::pair<const Type *, std::vector<const Type *>>>
      functions_;
  std::vector<VarDefinitionScope> definitions_;
};
} // namespace Cobold

#endif /* COBOLD_INFERENCE_TYPE_CONTEXT */
