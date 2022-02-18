#include <cstdint>

#include "chess_coordinates.h"
#include "chess_move.h"

namespace chess_engine {

struct Move {
    Coordinates from; 
    Coordinates to;
    Piece piece;  // Piece that moves or piece to promote to.
}

}  // namespace chess_engine