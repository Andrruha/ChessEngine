#ifndef SRC_ENGINE_MANAGER_H_
#define SRC_ENGINE_MANAGER_H_

#include <chrono>
#include <list>

#include "src/abstract_protocol.h"
#include "src/game.h"
#include "src/engine.h"
#include "src/fen.h"
#include "src/position.h"
#include "src/time_control.h"

namespace chess_engine {

class EngineManager {
 public:
  EngineManager(AbstractProtocol* protocol, Engine* engine);
  void StartMainLoop();

  void SetEngineColor(Player value);
  void NewGame();
  void SetPosition(const Position& Position);
  void SetMode(EngineMode mode);
  void SetTime(TimeControl tc);

  void MakeMove(Move move);
  void UndoMove();
  void Think();

  void MakeBestMove();

  void ReportProgress(
    int16_t ply,
    int32_t eval,
    int64_t nodes,
    const std::list<chess_engine::Move>& pv
  );

 private:
  bool ProceedWithBatch();

  AbstractProtocol* protocol_;
  Engine* engine_;

  Position starting_position_ = chess_engine::FenToPosition(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  );

  Game game_ = Game(starting_position_);
  EngineMode engine_mode_ = EngineMode::kForce;
  Player engine_color_ = Player::kBlack;
  TimeControl time_control_ = {40, 600.0, 0};

  bool thought_ = false;
  bool abort_thinking_ = false;

  std::chrono::steady_clock::time_point last_engine_start_;
};

}  // namespace chess_engine

#endif  // SRC_ENGINE_MANAGER_H_
