#ifndef COBOLD_CORE_TYPE
#define COBOLD_CORE_TYPE

#include <string>
#include <type_traits>

#include "absl/strings/str_cat.h"
#include "absl/container/flat_hash_map.h"

namespace Cobold {
enum class TypeClass {
    Nil, Integral, String, Array, Pointer
};
class Type {
public:
    static const Type *ArrayOf(const Type *underlying_type);
    static const Type *PointerTo(const Type *underlying_type);

    virtual const TypeClass type_class() const { return class_; }
    virtual const std::string DebugString() const = 0;
    template<typename T>
    const T *As() const {
        static_assert(std::is_base_of_v<Type, T>, "Attempting to cast to non-derived class.");
        // TODO(jlscheerer) check for `nullptr`
        return dynamic_cast<const T*>(this);
    }
    virtual ~Type() = default;
private:
    TypeClass class_;

    mutable std::unique_ptr<Type> array_type_;
    mutable std::unique_ptr<Type> pointer_type_;
};

class NilType: public Type {
public:
    static const NilType *Get();
    const TypeClass type_class() const override { return TypeClass::Nil; }
    const std::string DebugString() const override { return "nil"; }

private:
    NilType() {}

    static inline std::unique_ptr<NilType> type_;
};

class IntegralType: public Type {
public:
    static const IntegralType *OfSize(int size);

    const int size() const { return size_; }
    const TypeClass type_class() const override { return TypeClass::Integral; }
    const std::string DebugString() const override {
        return absl::StrCat("i", size());
    }
private:
    IntegralType(int size): size_(size) {}

    static inline absl::flat_hash_map<int, std::unique_ptr<IntegralType>> type_cache_;
    int size_;
};

class StringType : public Type {
public:
    static const StringType *Get();
    const TypeClass type_class() const override { return TypeClass::String; }
    const std::string DebugString() const override { return "string"; }

private:
    StringType() {}

    static inline std::unique_ptr<StringType> type_;
};

class ArrayType: public Type {
public:
    const TypeClass type_class() const override { return TypeClass::Array; }
    const std::string DebugString() const override {
        return absl::StrCat("[", underlying_type_->DebugString(), "]");
    }

private:
    ArrayType(const Type *underlying_type) : underlying_type_(underlying_type) {}

    const Type *underlying_type_;
    friend class Type;
};

class PointerType: public Type {
public:
    const TypeClass type_class() const override { return TypeClass::Pointer; }
    const std::string DebugString() const override {
        return absl::StrCat(underlying_type_->DebugString(), "*");
    }

private:
    PointerType(const Type *underlying_type) : underlying_type_(underlying_type) {}

    const Type *underlying_type_;
    friend class Type;
};
}

#endif /* COBOLD_CORE_TYPE */
