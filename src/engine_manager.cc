#include "engine_manager.h"

#include <chrono>

namespace chess_engine {

EngineManager::EngineManager(AbstractProtocol* protocol, Engine* engine):
protocol_(protocol), engine_(engine){
  protocol_->SetNewGameCallback([this](){NewGame();});
  protocol_->SetMoveRecievedCallback([this](Move move){MakeMove(move);});
  protocol_->SetUndoRecievedCallback([this](){UndoMove();});
  protocol_->SetSetColorCallback([this](Player value){SetEngineColor(value);});
  protocol_->SetSetModeCallback([this](EngineMode mode){SetMode(mode);});
  protocol_->SetSetBoardCallback([this](const Position& position){SetPosition(position);});

  engine_->SetProceedWithBatchCallback([this](){return ProceedWithBatch();});
  engine_->SetReportProgressCallback([this](
    int16_t ply,
    int32_t eval,
    int64_t nodes,
    const std::list<chess_engine::Move>& pv
  ){
    ReportProgress(ply, eval, nodes, pv);
  });

  protocol_->StartInputLoop();
}

void EngineManager::StartMainLoop() {
  // TODO(Andrey): loop condition
  while(true) {
    protocol_-> ProcessCommands();
  }
}

void EngineManager::SetEngineColor(Player value) {
  engine_color_ = value;
  if (engine_->GetPosition().PlayerToMove() == engine_color_) {
    Think();
  }
}

void EngineManager::NewGame() {
  SetPosition(starting_position_);
  SetMode(EngineMode::kPlay);
  SetEngineColor(Opponent(starting_position_.PlayerToMove()));
}

void EngineManager::SetPosition(const Position& position) {
  engine_->SetPosition(position);
  game_ = Game(position);
  if (engine_->GetPosition().PlayerToMove() == engine_color_) {
    Think();
  }
}

void EngineManager::SetMode(EngineMode mode) {
  engine_mode_ = mode;
}

void EngineManager::MakeMove(Move move) {
  engine_->MakeMove(move);
  game_.MakeMove(move);
  if (engine_->GetPosition().PlayerToMove() == engine_color_) {
    Think();
  }
}

void EngineManager::UndoMove() {
  game_.UndoMove();
  SetPosition(game_.GetPosition());
}

void EngineManager::Think() {
  if (engine_mode_ == EngineMode::kForce) {
    return;
  }
  last_engine_start_ = std::chrono::steady_clock::now();
  engine_->StartSearch();
  if (engine_mode_ == EngineMode::kPlay) {
    Move best_move = engine_->GetBestMove();
    engine_->MakeMove(best_move);
    protocol_->MakeMove(best_move);
    game_.MakeMove(best_move);
  }
}

void EngineManager::ReportProgress(
  int16_t ply,
  int32_t eval,
  int64_t nodes,
  const std::list<chess_engine::Move>& pv
) {
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = now-last_engine_start_;
  if (eval < engine_->GetLowestEval() + engine_->GetLongestCheckmate()) {
    int32_t mated_in = eval - engine_->GetLowestEval();
    eval = -100000 - mated_in;
  } else if (eval > engine_->GetHighestEval() - engine_->GetLongestCheckmate()) {
    int32_t mate_in = engine_->GetHighestEval() - eval;
    eval = 100000 + mate_in;
  } else {
    eval /= 10;
  }
  protocol_->DisplayInfo(ply, eval, static_cast<int>(elapsed.count()*100), nodes, pv);
}

bool EngineManager::ProceedWithBatch() {
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = now-last_engine_start_;
  switch (engine_mode_) {
  case EngineMode::kForce:
    return false;
    break;
  case EngineMode::kPlay:
    return elapsed.count() < 5.0;
    break;
  case EngineMode::kAnalyse:
    return true;
  default:
    return false;
    break;
  }
}

}  // namespace chess_engine