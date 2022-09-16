#ifndef COBOLD_CORE_FUNCTION
#define COBOLD_CORE_FUNCTION

#include <string>
#include <vector>

#include "core/type.h"

namespace Cobold {
class Function {
public:
    Function(std::string name,
             std::vector<Type *> argument_types, Type *return_type)
             : name_(name), argument_types_(argument_types), return_type_(return_type) {}
    virtual ~Function() = default;

    const std::string name() const { return name_; };
    const std::vector<Type *> &argument_types() const { return argument_types_; }
    const Type *return_type() const { return return_type_; }
    virtual const bool external() const { return false; }
private:
    std::string name_;
    std::vector<Type *> argument_types_;
    Type *return_type_;
};

class ExternFunction : public Function {
public:
    using Function::Function;
    const bool external() const override { return true; }
};
}

#endif /* COBOLD_CORE_FUNCTION */
