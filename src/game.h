#ifndef CHESS_ENGINE_SRC_GAME_
#define CHESS_ENGINE_SRC_GAME_

#include <vector>

#include "chess_defines.h"
#include "position.h"

namespace chess_engine {

class Game {
 public:
  explicit Game(const Position& starting_position);
  Position GetPosition();
  void MakeMove(Move move);
  void UndoMove();
 private:
  Position starting_position_;
  std::vector<Move> moves_;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_GAME_s