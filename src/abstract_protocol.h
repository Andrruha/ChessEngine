#ifndef SRC_ABSTRACT_PROTOCOL_H_
#define SRC_ABSTRACT_PROTOCOL_H_

#include <cstdint>
#include <functional>
#include <list>

#include "src/chess_defines.h"
#include "src/position.h"

namespace chess_engine {

enum struct EngineMode {
  kForce = 0,
  kPlay = 1,
  kAnalyse = 2
};

// Handles interaction with protocols, such as XBoard, UCI
// or possibly my own UI.
class AbstractProtocol {
 public:
  // Process recieved commands, call corresponding callbacks.
  virtual void ProcessCommands() = 0;
  // Start recieving commands.
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
  void SetMoveRecievedCallback(std::function<void(Move)> callback);
  void SetUndoRecievedCallback(std::function<void()>);
  void SetSetColorCallback(std::function<void(Player)> callback);
  void SetSetModeCallback(std::function<void(EngineMode)> callback);
  void SetSetBoardCallback(std::function<void(const Position&)> callback);

 protected:
  std::function<void()> new_game_callback_;
  std::function<void(Move)> move_recieved_callback_;
  std::function<void()> undo_recieved_callback_;
  std::function<void(Player)> set_color_callback_;
  std::function<void(EngineMode)> set_mode_callback_;
  std::function<void(Position)> set_board_callback_;
};

}  // namespace chess_engine

#endif  // SRC_ABSTRACT_PROTOCOL_H_
