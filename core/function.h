#ifndef COBOLD_CORE_FUNCTION
#define COBOLD_CORE_FUNCTION

#include <string>
#include <type_traits>
#include <vector>

#include "core/type.h"
#include "core/statement.h"

namespace Cobold {
struct FunctionArgument {
    std::string name;
    const Type *type;

    std::string DebugString() const { return absl::StrCat(name, ": ", type->DebugString()); }
};

class Function {
public:
    Function(std::string name,
             std::vector<FunctionArgument> arguments, const Type *return_type)
             : name_(name), arguments_(arguments), return_type_(return_type) {}
    virtual ~Function() = default;

    const std::string name() const { return name_; };
    const std::vector<FunctionArgument> &arguments() const { return arguments_; }
    const Type *return_type() const { return return_type_; }
    virtual const bool external() const = 0;
    template<typename T>
    const T *As() const {
        static_assert(std::is_base_of_v<Function, T>, "Attempting to cast to non-derived class.");
        // TODO(jlscheerer) check for `nullptr`
        return dynamic_cast<const T*>(this);
    }
    virtual std::string DebugString() const {
        return GetSignature();
    }

protected:
    std::string GetSignature() const;

private:
    std::string name_;
    std::vector<FunctionArgument> arguments_;
    const Type *return_type_;
};

class DefinedFunction : public Function {
public:
    DefinedFunction(std::string name, std::vector<FunctionArgument> arguments,
                   const Type *return_type, CompoundStatement &&body)
             : Function(name, arguments, return_type), body_(std::move(body)) {}
    const bool external() const override { return false; }

    std::string DebugString() const override {
        return absl::StrCat(GetSignature(), " {...}");
    }

private:
    CompoundStatement body_;
};

class ExternFunction : public Function {
public:
    ExternFunction(std::string name, std::vector<FunctionArgument> arguments,
                   const Type *return_type, std::string specifier)
             : Function(name, arguments, return_type), specifier_(specifier) {}
    const bool external() const override { return true; }
    const std::string &specifier() const { return specifier_; }

    std::string DebugString() const override {
        return absl::StrCat(GetSignature(), " #extern(", specifier_, ");");
    }
private:
    std::string specifier_;
};
}

#endif /* COBOLD_CORE_FUNCTION */
