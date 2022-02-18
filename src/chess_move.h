#ifndef CHESS_ENGINE_SRC_CHESS_MOVE_
#define CHESS_ENGINE_SRC_CHESS_MOVE_

#include <cstdint>

#include "chess_coordinates.h"
#include "chess_piece.h"

namespace chess_engine {

struct Move {
    Coordinates from; 
    Coordinates to;
    Piece piece;  // Piece that moves or piece to promote to.
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_CHESS_MOVE_