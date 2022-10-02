#include "core/type.h"

namespace Cobold {
// `Type` ===============================================================
const Type *Type::ArrayOf(const Type *underlying_type) {
  if (underlying_type == nullptr)
    return nullptr;
  if (underlying_type->array_type_ == nullptr) {
    auto array_type = absl::WrapUnique(new ArrayType(underlying_type));
    underlying_type->array_type_ = std::move(array_type);
  }
  return underlying_type->array_type_.get();
}

const Type *Type::PointerTo(const Type *underlying_type) {
  if (underlying_type == nullptr)
    return nullptr;
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

// `DashType` ===========================================================
const DashType *DashType::Get() {
  if (type_ == nullptr) {
    type_ = absl::WrapUnique(new DashType());
  }
  return type_.get();
}
// `DashType` ============================================================

// `IntegralType` =======================================================
const IntegralType *IntegralType::OfSize(int size) {
  if (!type_cache_.contains(size)) {
    type_cache_[size] = absl::WrapUnique(new IntegralType(size));
  }
  return type_cache_[size].get();
}
// `IntegralType` =======================================================

// `RangeType` ==========================================================
const RangeType *RangeType::Of(const Type *underlying_type) {
  if (!type_cache_.contains(underlying_type)) {
    type_cache_[underlying_type] =
        absl::WrapUnique(new RangeType(underlying_type));
  }
  return type_cache_[underlying_type].get();
}
// `RangeType` ==========================================================

// `FloatingType` =======================================================
const FloatingType *FloatingType::OfSize(int size) {
  if (!type_cache_.contains(size)) {
    type_cache_[size] = absl::WrapUnique(new FloatingType(size));
  }
  return type_cache_[size].get();
}
// `FloatingType` =======================================================

// `BoolType` ===========================================================
const CharType *CharType::Get() {
  if (type_ == nullptr) {
    type_ = absl::WrapUnique(new CharType());
  }
  return type_.get();
}
// `BoolType` ===========================================================

// `BoolType` ===========================================================
const BoolType *BoolType::Get() {
  if (type_ == nullptr) {
    type_ = absl::WrapUnique(new BoolType());
  }
  return type_.get();
}
// `BoolType` ===========================================================

// `StringType` =========================================================
const StringType *StringType::Get() {
  if (type_ == nullptr) {
    type_ = absl::WrapUnique(new StringType());
  }
  return type_.get();
}
// `StringType` =========================================================
} // namespace Cobold