#ifndef COBOLD_CORE_EXPRESSION
#define COBOLD_CORE_EXPRESSION

#include <type_traits>

namespace Cobold {
enum class ExpressionType {

};
class Expression {
public:
    virtual ExpressionType type() const = 0;

    template<typename T>
    const T *As() const {
        static_assert(std::is_base_of_v<Expression, T>, "Attempting to cast to non-derived class.");
        // TODO(jlscheerer) check for `nullptr`
        return dynamic_cast<const T*>(this);
    }

    virtual ~Expression() = default;
};
}

#endif /* COBOLD_CORE_EXPRESSION */
