#ifndef CHESS_ENGINE_SRC_WINBOARD_PROTOCOL_
#define CHESS_ENGINE_SRC_WNIBOARD_PROTOCOL_

#include "abstract_protocol.h"
#include "chess_defines.h"

namespace chess_engine {

class WinboardProtocol : public AbstractProtocol {
 public:
  void WaitForCommands() override;

  void MakeMove(Move move) override;
  void DisplayInfo(
    int16_t ply,
    int32_t centipawns,
    int32_t centiseconds,
    int64_t nodes,
    const std::list<Move>& pv
  ) const override;
 private:
  void SendFeatures();
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_WINBOARD_PROTOCOL_