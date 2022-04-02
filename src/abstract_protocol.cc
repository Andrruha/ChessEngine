#include "abstract_protocol.h"

namespace chess_engine {

void AbstractProtocol::SetNewGameCallback(std::function<void()> callback) {
  new_game_callback_ = callback;
}

void AbstractProtocol::SetMoveRecievedCallback(std::function<void(Move)> callback) {
  move_recieved_callback_ = callback;
}

}  // namespace chess_engine