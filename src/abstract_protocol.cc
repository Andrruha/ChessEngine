#include "src/abstract_protocol.h"

namespace chess_engine {

void AbstractProtocol::SetNewGameCallback(std::function<void()> callback) {
  new_game_callback_ = callback;
}

void AbstractProtocol::SetMoveReceivedCallback(
  std::function<void(Move)> callback
) {
  move_received_callback_ = callback;
}

void AbstractProtocol::SetUndoReceivedCallback(
  std::function<void()> callback
) {
  undo_received_callback_ = callback;
}

void AbstractProtocol::SetSetColorCallback(
  std::function<void(Player)> callback
) {
  set_color_callback_ = callback;
}

void AbstractProtocol::SetSetModeCallback(
  std::function<void(EngineMode)> callback
) {
  set_mode_callback_ = callback;
}

void AbstractProtocol::SetSetBoardCallback(
  std::function<void(const Position&)> callback
) {
  set_board_callback_ = callback;
}

void AbstractProtocol::SetSetTimeCallback(
  std::function<void(TimeControl)> callback
) {
  set_time_callback_ = callback;
}

}  // namespace chess_engine
