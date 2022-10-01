#include "codegen/cobold_build_context.h"

namespace Cobold {
// `CoboldBuildContext` =================================================
llvm::Constant *CoboldBuildContext::AddStringConstant(std::string &value) {
  llvm::Constant *ret = module->getOrInsertGlobal(
      /*name=*/"",
      llvm::ArrayType::get(llvm::Type::getIntNTy(*context, 8), value.size()),
      [this, value]() {
        std::vector<llvm::Constant *> chars;
        chars.reserve(value.size());
        for (const char c : value) {
          chars.push_back(
              llvm::ConstantInt::get(*context, llvm::APInt(8, c, true)));
        }
        auto init = llvm::ConstantArray::get(
            llvm::ArrayType::get(llvm::Type::getIntNTy(*context, 8),
                                 chars.size()),
            chars);
        llvm::GlobalVariable *v = new llvm::GlobalVariable(
            *module, init->getType(), true,
            llvm::GlobalVariable::InternalLinkage, init, value);
        return v;
      });
  return ret;
}
// `CoboldBuildContext` =================================================
} // namespace Cobold