#include "src/engine_manager.h"

#include <chrono>
#include <list>

namespace chess_engine {

EngineManager::EngineManager(AbstractProtocol* protocol, Engine* engine):
protocol_(protocol), engine_(engine) {
  protocol_->SetNewGameCallback([this](){NewGame();});
  protocol_->SetMoveReceivedCallback([this](Move move){MakeMove(move);});
  protocol_->SetUndoReceivedCallback([this](){UndoMove();});
  protocol_->SetSetColorCallback([this](Player value){SetEngineColor(value);});
  protocol_->SetSetModeCallback([this](EngineMode mode){SetMode(mode);});
  protocol_->SetSetBoardCallback(
    [this](const Position& position){SetPosition(position);}
  );
  protocol_->SetSetTimeCallback([this](TimeControl tc){SetTime(tc);});

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
  // TODO(Andrey): Loop condition.
  while (true) {
    protocol_->ProcessCommands();
    abort_thinking_ = false;
    thought_ = false;
    if (engine_mode_ == EngineMode::kAnalyze) {
      Think();  // Don't set thought to true, because depth might be too low.
    } else if (
      engine_->GetPosition().PlayerToMove() == engine_color_ &&
      engine_mode_ == EngineMode::kPlay
    ) {
      Think();
      thought_ = true;
    }
    protocol_->ProcessCommands();  // Might've received commands while thinking.
    if (
      engine_->GetPosition().PlayerToMove() == engine_color_ &&
      engine_mode_ == EngineMode::kPlay &&
      !abort_thinking_ &&
      thought_
    ) {
      MakeBestMove();
    }
  }
}

void EngineManager::SetEngineColor(Player value) {
  engine_color_ = value;
}

void EngineManager::NewGame() {
  SetPosition(starting_position_);
  SetMode(EngineMode::kPlay);
  SetEngineColor(Opponent(starting_position_.PlayerToMove()));
  abort_thinking_ = true;
}

void EngineManager::SetPosition(const Position& position) {
  engine_->SetPosition(position);
  game_ = Game(position);
  abort_thinking_ = true;
}

void EngineManager::SetMode(EngineMode mode) {
  engine_mode_ = mode;
  if (mode == EngineMode::kPlay) {
    engine_color_ = engine_->GetPosition().PlayerToMove();
  }
  abort_thinking_ = true;
}


void EngineManager::SetTime(TimeControl tc) {
  time_control_ = tc;
}

void EngineManager::MakeMove(Move move) {
  engine_->MakeMove(move);
  game_.MakeMove(move);
  abort_thinking_ = true;
}

void EngineManager::UndoMove() {
  game_.UndoMove();
  engine_->SetPosition(game_.GetPosition());
  abort_thinking_ = true;
}

void EngineManager::Think() {
  last_engine_start_ = std::chrono::steady_clock::now();
  engine_->StartSearch();
}

void EngineManager::MakeBestMove() {
  Move best_move = engine_->GetBestMove();
  engine_->MakeMove(best_move);
  protocol_->MakeMove(best_move);
  game_.MakeMove(best_move);
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
  } else if (
    eval > engine_->GetHighestEval() - engine_->GetLongestCheckmate()
  ) {
    int32_t mate_in = engine_->GetHighestEval() - eval;
    eval = 100000 + mate_in;
  } else {
    eval /= 10;
  }
  protocol_->DisplayInfo(
    ply, eval, static_cast<int>(elapsed.count()*100), nodes, pv
  );
}

bool EngineManager::ProceedWithBatch() {
  protocol_->ProcessCommands();
  if (abort_thinking_) {
    return false;
  }
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = now-last_engine_start_;
  switch (engine_mode_) {
  case EngineMode::kForce:
    return false;
    break;
  case EngineMode::kPlay:
    return elapsed.count() < time_control_.GuaranteedTimePerMove() * 0.95;
    break;
  case EngineMode::kAnalyze:
    return true;
  default:
    return false;
    break;
  }
}

}  // namespace chess_engine
