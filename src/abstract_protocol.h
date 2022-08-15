#ifndef SRC_ABSTRACT_PROTOCOL_H_
#define SRC_ABSTRACT_PROTOCOL_H_

#include <cstdint>
#include <functional>
#include <list>

#include "src/chess_defines.h"
#include "src/position.h"
#include "src/time_control.h"

namespace chess_engine {

enum struct EngineMode {
  kForce = 0,
  kPlay = 1,
  kAnalyze = 2
};

// Handles interaction with protocols, such as XBoard, UCI
// or possibly my own UI.
class AbstractProtocol {
 public:
  // Process received commands, call corresponding callbacks.
  virtual void ProcessCommands() = 0;
  // Start receiving commands.
  virtual void StartInputLoop() = 0;

  // Send an engine move via protocol.
  virtual void MakeMove(Move move) = 0;
  virtual void DisplayInfo(
    int16_t ply,
    int32_t centipawns,
    int32_t centiseconds,
    int64_t nodes,
    const std::list<Move>& pv
  ) const = 0;

  void SetNewGameCallback(std::function<void()> callback);
  void SetMoveReceivedCallback(std::function<void(Move)> callback);
  void SetUndoReceivedCallback(std::function<void()>);
  void SetSetColorCallback(std::function<void(Player)> callback);
  void SetSetModeCallback(std::function<void(EngineMode)> callback);
  void SetSetBoardCallback(std::function<void(const Position&)> callback);
  void SetSetTimeCallback(std::function<void(TimeControl)> callback);

 protected:
  std::function<void()> new_game_callback_;
  std::function<void(Move)> move_received_callback_;
  std::function<void()> undo_received_callback_;
  std::function<void(Player)> set_color_callback_;
  std::function<void(EngineMode)> set_mode_callback_;
  std::function<void(Position)> set_board_callback_;
  std::function<void(TimeControl)> set_time_callback_;
};

}  // namespace chess_engine

#endif  // SRC_ABSTRACT_PROTOCOL_H_
