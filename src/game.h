#ifndef SRC_GAME_H_
#define SRC_GAME_H_

#include <vector>

#include "src/chess_defines.h"
#include "src/position.h"

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

#endif  // SRC_GAME_H_
