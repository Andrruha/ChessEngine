#include "count_moves.h"

#include <vector>

namespace chess_engine {

int64_t CountMoves(const Position& position, int depth) {
  if (!depth) {
    return 1;
  }
  int64_t ret = 0;
  std::vector<Move> legal_moves = position.GetLegalMoves();
  if (depth == 1) {
    return legal_moves.size();
  }
  for (Move move:legal_moves) {
    Position new_position = position;
    new_position.MakeMove(move);
    ret += CountMoves(new_position, depth-1);
  }
  return ret;
}

}