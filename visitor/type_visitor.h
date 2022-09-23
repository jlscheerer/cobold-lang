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
    case TypeClass::Integral:
      return DispatchIntegral(type->As<IntegralType>());
    case TypeClass::String:
      return DispatchString(type->As<StringType>());
    case TypeClass::Array:
      return DispatchArray(type->As<ArrayType>());
    case TypeClass::Pointer:
      return DispatchPointer(type->As<PointerType>());
      break;
    }
  }

protected:
  virtual RetType DispatchEmpty() { assert(false); }
  virtual RetType DispatchNil(const NilType *type) = 0;
  virtual RetType DispatchIntegral(const IntegralType *type) = 0;
  virtual RetType DispatchString(const StringType *type) = 0;
  virtual RetType DispatchArray(const ArrayType *type) = 0;
  virtual RetType DispatchPointer(const PointerType *type) = 0;

  virtual ~TypeVisitor() = default;
};
} // namespace Cobold

#endif /* COBOLD_VISITOR_TYPE_VISITOR */
