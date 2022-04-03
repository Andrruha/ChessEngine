#ifndef CHESS_ENGINE_SRC_ENGINE_MANAGER_
#define CHESS_ENGINE_SRC_ENGINE_MANAGER_

#include <chrono>

#include "abstract_protocol.h"
#include "engine.h"
#include "fen.h"
#include "position.h"

namespace chess_engine {

class EngineManager {
 public:
  EngineManager(AbstractProtocol* protocol, Engine* engine);
  void StartMainLoop();

  void SetEngineColor(Player value);

  void NewGame();
  void MakeMove(Move move);
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

  Position starting_position_ = chess_engine::FenToPosition (
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  );

  Player engine_color_ = Player::kBlack;

  std::chrono::steady_clock::time_point last_engine_start_;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_ENGINE_MANAGER_