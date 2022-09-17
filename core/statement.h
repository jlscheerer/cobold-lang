#ifndef COBOLD_CORE_STATEMENT
#define COBOLD_CORE_STATEMENT

#include <type_traits>
#include <vector>

namespace Cobold {
enum class StatementType {
    Compound
};
class Statement {
public:
    virtual StatementType type() const = 0;

    template<typename T>
    const T *As() const {
        static_assert(std::is_base_of_v<Statement, T>, "Attempting to cast to non-derived class.");
        // TODO(jlscheerer) check for `nullptr`
        return dynamic_cast<const T*>(this);
    }

    virtual ~Statement() = default;
};
class CompoundStatement: public Statement {
public:
    StatementType type() const { return StatementType::Compound; }

private:
    std::vector<std::unique_ptr<Statement>> statements_;
};
}

#endif /* COBOLD_CORE_STATEMENT */
