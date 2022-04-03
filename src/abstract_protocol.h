#ifndef CHESS_ENGINE_SRC_ABSTRACT_PROTOCOL_
#define CHESS_ENGINE_SRC_ABSTRACT_PROTOCOL_

#include <cstdint>
#include <functional>
#include <list>

#include "chess_defines.h"
#include "position.h"

namespace chess_engine {

class AbstractProtocol {
 public:
  virtual void WaitForCommands() = 0;

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
  void SetSetColorCallback(std::function<void(Player)> callback);
 protected:
  std::function<void()> new_game_callback_;
  std::function<void(Move)> move_recieved_callback_;
  std::function<void(Player)> set_color_callback_;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_ABSTRACT_PROTOCOL_