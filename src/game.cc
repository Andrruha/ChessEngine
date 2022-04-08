#include "game.h"

#include "chess_defines.h"

namespace chess_engine {

  Game::Game(const Position& starting_position):
    starting_position_(starting_position)
  {}

  Position Game::GetPosition() {
    Position position = starting_position_;
    for (Move move : moves_) {
      position.MakeMove(move);
    }
    return position;
  }

  void Game::MakeMove(Move move) {
    moves_.push_back(move);
  }

  void Game::UndoMove() {
    moves_.pop_back();
  }
}