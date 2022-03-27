#ifndef CHESS_ENGINE_SRC_POSITION_TABLE_
#define CHESS_ENGINE_SRC_POSITION_TABLE_

#include <cstdint>
#include <vector>

namespace chess_engine {

template<class T, int index_size>
class PositionTable {
 public:
  T Get(uint64_t key) const {
    const Entry& entry= elements_[key & mask_];
    if (entry.key != key) {
      return T();
    } else {
      return entry.value;
    }
  }
  void Set(uint64_t key, T value) {
    elements_[key & mask_] = {key, value};  // Always replace for now
  }
 private:
  struct Entry {
    uint64_t key;
    T value;
  };
  const uint64_t mask_ = (1ull << index_size)-1;
  std::vector<Entry> elements_ = std::vector<Entry>(1ull << index_size, {0ull, T()});
};

}

#endif  // CHESS_ENGINE_SRC_POSITION_TABLE_