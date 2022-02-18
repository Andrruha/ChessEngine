#ifndef CHESS_ENGINE_SRC_CHESS_COORDINATES_
#define CHESS_ENGINE_SRC_CHESS_COORDINATES_

#include <cstdint>

namespace chess_engine {

struct Coordinates {
  int8_t file;  // aka "x-coordinate" (0 corresponds to the a-file)
  int8_t rank;  // aka "y-coordinate" (0 corresponds to the first rank)
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_CHESS_COORDINATES_