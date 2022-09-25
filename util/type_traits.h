#ifndef COBOLD_UTIL_TYPE_TRAITS
#define COBOLD_UTIL_TYPE_TRAITS

#include <type_traits>

template <bool AddConst, typename T>
using enable_const_if = std::conditional<AddConst, std::add_const_t<T>, T>;

template <bool AddConst, typename T>
using enable_const_if_t = typename enable_const_if<AddConst, T>::type;

#endif /* COBOLD_UTIL_TYPE_TRAITS */
