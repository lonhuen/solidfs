#pragma once

#include <ostream>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace solid {
  template<typename S, typename D>
  S &operator<<(S &out, const std::vector<D> &vec) {
    out << "[";
    if (!vec.empty()) {
        auto iter = vec.cbegin(), end = vec.cend();
        out << *iter;
        while (++iter != end) {
        out << ", " << *iter;
        }
    }
    out << "]";
    return out;
  }

  template<typename S, typename K, typename V>
  S &operator<<(S &out, const std::unordered_map<K, V> &map) {
    out << "{";
    if (!map.empty()) {
        auto iter = map.cbegin(), end = map.cend();
        out << *iter;
        while (++iter != end) {
        out << ", " << *iter;
        }
    }
    out << "}";
    return out;
  }

}
