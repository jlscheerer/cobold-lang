#include "core/type.h"

namespace Cobold {
// `Type` ===============================================================
const Type *Type::ArrayOf(const Type *underlying_type) {
    if (underlying_type == nullptr) return nullptr;
    if (underlying_type->array_type_ == nullptr) {
        auto array_type = absl::WrapUnique(new ArrayType(underlying_type));
        underlying_type->array_type_ = std::move(array_type);
    }
    return underlying_type->array_type_.get();
}

const Type *Type::PointerTo(const Type *underlying_type) {
    if (underlying_type == nullptr) return nullptr;
    if (underlying_type->pointer_type_ == nullptr) {
        auto pointer_type = absl::WrapUnique(new PointerType(underlying_type));
        underlying_type->pointer_type_ = std::move(pointer_type);
    }
    return underlying_type->pointer_type_.get();
}
// `Type` ===============================================================

// `NilType` ============================================================
const NilType *NilType::Get() {
    if (type_ == nullptr) {
        type_ = absl::WrapUnique(new NilType());
    }
    return type_.get();
}
// `NilType` ============================================================

// `IntegralType` =======================================================
const IntegralType *IntegralType::OfSize(int size) {
    if (!type_cache_.contains(size)) {
        type_cache_[size] = absl::WrapUnique(new IntegralType(size));
    }
    return type_cache_[size].get();
}
// `IntegralType` =======================================================

// `StringType` =========================================================
const StringType *StringType::Get() {
    if (type_ == nullptr) {
        type_ = absl::WrapUnique(new StringType());
    }
    return type_.get();
}
// `StringType` =========================================================
}