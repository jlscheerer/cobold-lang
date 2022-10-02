#ifndef COBOLD_VISITOR_TYPE_VISITOR
#define COBOLD_VISITOR_TYPE_VISITOR

#include "core/type.h"

namespace Cobold {
template <typename RetType> class TypeVisitor {
public:
  RetType Visit(const Type *type) {
    if (type == nullptr)
      DispatchEmpty();
    TypeClass type_class = type->type_class();
    switch (type_class) {
    case TypeClass::Nil:
      return DispatchNil(type->As<NilType>());
    case TypeClass::Dash:
      return DispatchDash(type->As<DashType>());
    case TypeClass::Bool:
      return DispatchBool(type->As<BoolType>());
    case TypeClass::Char:
      return DispatchChar(type->As<CharType>());
    case TypeClass::Integral:
      return DispatchIntegral(type->As<IntegralType>());
    case TypeClass::Floating:
      return DispatchFloating(type->As<FloatingType>());
    case TypeClass::String:
      return DispatchString(type->As<StringType>());
    case TypeClass::Array:
      return DispatchArray(type->As<ArrayType>());
    case TypeClass::Range:
      return DispatchRange(type->As<RangeType>());
    case TypeClass::Pointer:
      return DispatchPointer(type->As<PointerType>());
      break;
    }
  }

protected:
  virtual RetType DispatchEmpty() { assert(false); }
  virtual RetType DispatchNil(const NilType *type) = 0;
  virtual RetType DispatchDash(const DashType *type) = 0;
  virtual RetType DispatchBool(const BoolType *type) = 0;
  virtual RetType DispatchChar(const CharType *type) = 0;
  virtual RetType DispatchIntegral(const IntegralType *type) = 0;
  virtual RetType DispatchFloating(const FloatingType *type) = 0;
  virtual RetType DispatchString(const StringType *type) = 0;
  virtual RetType DispatchArray(const ArrayType *type) = 0;
  virtual RetType DispatchRange(const RangeType *type) = 0;
  virtual RetType DispatchPointer(const PointerType *type) = 0;

  virtual ~TypeVisitor() = default;
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_TYPE_VISITOR */
