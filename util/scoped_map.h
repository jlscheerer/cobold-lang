#ifndef COBOLD_UTIL_SCOPE
#define COBOLD_UTIL_SCOPE

#include <optional>
#include <vector>

#include "absl/container/flat_hash_map.h"

namespace Cobold {
template <typename KeyType, typename ValueType> struct Scope {
public:
  bool contains(const KeyType &key) { return insert_order_.contains(key); }
  void put(const KeyType &key, const ValueType &value) {
    insert_order_[key] = data_.size();
    data_.push_back(value);
  }
  ValueType &operator[](const KeyType &key) {
    return data_[insert_order_[key]];
  }
  auto begin() const { return data_.rbegin(); }
  auto end() const { return data_.rend(); }

private:
  absl::flat_hash_map<KeyType, int> insert_order_;
  std::vector<ValueType> data_;
};

template <typename KeyType, typename ValueType> class ScopedMap {
public:
  void PushScope() { scopes_.push_back({}); }
  Scope<KeyType, ValueType> PopScope() {
    Scope<KeyType, ValueType> scope = std::move(scopes_.back());
    scopes_.pop_back();
    return scope;
  }
  bool put(const KeyType &key, const ValueType &value) {
    if (!Defines(key)) {
      scopes_.back().put(key, value);
      return true;
    }
    return false;
  }
  bool contains(const KeyType &key) {
    for (int i = scopes_.size() - 1; i >= 0; --i) {
      if (scopes_[i].contains(key))
        return true;
    }
    return false;
  }
  bool defines(const KeyType &key) {
    assert(scopes_.size() > 0); // this is a strong logical error!
    return scopes_.back().contains(key);
  }
  std::optional<ValueType> lookup(const KeyType &key) {
    for (int i = scopes_.size() - 1; i >= 0; --i) {
      if (scopes_[i].contains(key))
        return scopes_[i][key];
    }
    return std::nullopt;
  }
  bool store(const KeyType &key, const ValueType &value) {
    if (!defines(key)) {
      scopes_.back().put(key, value);
      return true;
    }
    return false;
  }

private:
  std::vector<Scope<KeyType, ValueType>> scopes_;
};
} // namespace Cobold

#endif /* COBOLD_UTIL_SCOPE */
