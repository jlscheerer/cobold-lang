#include "codegen/build_context.h"

namespace Cobold {
// `CoboldBuildContext` =================================================
// `BuildContext` =======================================================
llvm::Constant *BuildContext::AddStringConstant(std::string &value) {
  llvm::Constant *ret = module_->getOrInsertGlobal(
      /*name=*/"",
      llvm::ArrayType::get(llvm::Type::getIntNTy(*context_, 8), value.size()),
      [this, value]() {
        std::vector<llvm::Constant *> chars;
        chars.reserve(value.size());
        for (const char c : value) {
          chars.push_back(
              llvm::ConstantInt::get(*context_, llvm::APInt(8, c, true)));
        }
        auto init = llvm::ConstantArray::get(
            llvm::ArrayType::get(llvm::Type::getIntNTy(*context_, 8),
                                 chars.size()),
            chars);
        llvm::GlobalVariable *v = new llvm::GlobalVariable(
            *module_, init->getType(), true,
            llvm::GlobalVariable::InternalLinkage, init, value);
        return v;
      });
  return ret;
}
// `BuildContext` =======================================================
} // namespace Cobold