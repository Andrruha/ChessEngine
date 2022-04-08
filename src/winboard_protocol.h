#ifndef CHESS_ENGINE_SRC_WINBOARD_PROTOCOL_
#define CHESS_ENGINE_SRC_WNIBOARD_PROTOCOL_

#include <condition_variable>
#include <queue>
#include <string>
#include <thread>

#include "abstract_protocol.h"
#include "chess_defines.h"

namespace chess_engine {

class WinboardProtocol : public AbstractProtocol {
 public:
  void ProcessCommands() override;
  void StartInputLoop() override;

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

  std::condition_variable commands_recieved_;
  std::mutex mutex_;
  std::queue<std::string> command_queue_;
  std::thread input_thread_;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_WINBOARD_PROTOCOL_