#ifndef CHESS_ENGINE_SRC_COUNT_MOVES_
#define CHESS_ENGINE_SRC_COUNT_MOVES_

#include "position.h"

namespace chess_engine {

int64_t CountMoves(const Position& position, int depth);

}

#endif  // CHESS_ENGINE_SRC_COUNT_MOVES_