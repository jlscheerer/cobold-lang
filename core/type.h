#ifndef COBOLD_CORE_TYPE
#define COBOLD_CORE_TYPE

#include <string>
#include <type_traits>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"

namespace Cobold {
enum class TypeClass {
  Nil,
  Bool,
  Char,
  Integral,
  Floating,
  String,
  Array,
  Range,
  Pointer
};
class Type {
public:
  static const Type *ArrayOf(const Type *underlying_type);
  static const Type *PointerTo(const Type *underlying_type);

  virtual const TypeClass type_class() const { return class_; }
  virtual const std::string DebugString() const = 0;
  template <typename T> const T *As() const {
    static_assert(std::is_base_of_v<Type, T>,
                  "Attempting to cast to non-derived class.");
    // TODO(jlscheerer) check for `nullptr`
    return dynamic_cast<const T *>(this);
  }
  virtual ~Type() = default;

private:
  TypeClass class_;

  mutable std::unique_ptr<Type> array_type_;
  mutable std::unique_ptr<Type> pointer_type_;
};

class NilType : public Type {
public:
  static const NilType *Get();
  const TypeClass type_class() const override { return TypeClass::Nil; }
  const std::string DebugString() const override { return "nil"; }

private:
  NilType() {}

  static inline std::unique_ptr<NilType> type_;
};

class IntegralType : public Type {
public:
  static const IntegralType *OfSize(int size);

  const int size() const { return size_; }
  const TypeClass type_class() const override { return TypeClass::Integral; }
  const std::string DebugString() const override {
    return absl::StrCat("i", size());
  }

private:
  IntegralType(int size) : size_(size) {}

  static inline absl::flat_hash_map<int, std::unique_ptr<IntegralType>>
      type_cache_;
  int size_;
};

class FloatingType : public Type {
public:
  static const FloatingType *OfSize(int size);

  const int size() const { return size_; }
  const TypeClass type_class() const override { return TypeClass::Floating; }
  const std::string DebugString() const override {
    return absl::StrCat("f", size());
  }

private:
  FloatingType(int size) : size_(size) {}

  static inline absl::flat_hash_map<int, std::unique_ptr<FloatingType>>
      type_cache_;
  int size_;
};

class BoolType : public Type {
public:
  static const BoolType *Get();
  const TypeClass type_class() const override { return TypeClass::Bool; }
  const std::string DebugString() const override { return "bool"; }

private:
  BoolType() {}

  static inline std::unique_ptr<BoolType> type_;
};

class CharType : public Type {
public:
  static const CharType *Get();
  const TypeClass type_class() const override { return TypeClass::Char; }
  const std::string DebugString() const override { return "char"; }

private:
  CharType() {}

  static inline std::unique_ptr<CharType> type_;
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

class ArrayType : public Type {
public:
  const TypeClass type_class() const override { return TypeClass::Array; }
  const std::string DebugString() const override {
    return absl::StrCat("[", underlying_type_->DebugString(), "]");
  }
  const Type *underlying_type() const { return underlying_type_; }

private:
  ArrayType(const Type *underlying_type) : underlying_type_(underlying_type) {}

  const Type *underlying_type_;
  friend class Type;
};

class RangeType : public Type {
public:
  static const RangeType *Of(const Type *underlying_type);
  const TypeClass type_class() const override { return TypeClass::Range; }
  const std::string DebugString() const override {
    return absl::StrCat("[|", underlying_type_->DebugString(), "|]");
  }
  const Type *underlying_type() const { return underlying_type_; }

private:
  RangeType(const Type *underlying_type) : underlying_type_(underlying_type) {}

  const Type *underlying_type_;
  static inline absl::flat_hash_map<const Type *, std::unique_ptr<RangeType>>
      type_cache_;
};

class PointerType : public Type {
public:
  const TypeClass type_class() const override { return TypeClass::Pointer; }
  const std::string DebugString() const override {
    return absl::StrCat(underlying_type_->DebugString(), "*");
  }
  const Type *underlying_type() const { return underlying_type_; }

private:
  PointerType(const Type *underlying_type)
      : underlying_type_(underlying_type) {}

  const Type *underlying_type_;
  friend class Type;
};
} // namespace Cobold

#endif /* COBOLD_CORE_TYPE */
