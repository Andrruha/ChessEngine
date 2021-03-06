#ifndef SRC_POSITION_TABLE_H_
#define SRC_POSITION_TABLE_H_

#include <cstdint>
#include <vector>

namespace chess_engine {

// Simple hash table, that deals with collisions, by simply
// ovewrriting entries with the same indecies.
template<class T, int index_size>
class PositionTable {
 public:
  T Get(uint64_t key) const {
    const Entry& entry = elements_[key & mask_];
    if (entry.key != key) {
      return T();
    } else {
      return entry.value;
    }
  }
  void Set(uint64_t key, T value) {
    elements_[key & mask_] = {key, value};  // Always replace for now.
  }
  void Clear() {
    elements_ = std::vector<Entry>(1ull << index_size, {0ull, T()});
  }
 private:
  struct Entry {
    uint64_t key;
    T value;
  };
  static const uint64_t mask_ = (1ull << index_size)-1;
  std::vector<Entry> elements_ =
    std::vector<Entry>(1ull << index_size, {0ull, T()});
};

}  // namespace chess_engine

#endif  // SRC_POSITION_TABLE_H_
