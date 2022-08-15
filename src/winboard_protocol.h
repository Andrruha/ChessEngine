#ifndef SRC_WINBOARD_PROTOCOL_H_
#define SRC_WINBOARD_PROTOCOL_H_

#include <condition_variable>
#include <list>
#include <queue>
#include <string>
#include <thread>

#include "src/abstract_protocol.h"
#include "src/chess_defines.h"

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
  static double StringToSeconds(const std::string& str);

  std::condition_variable commands_received_;
  std::mutex mutex_;
  std::queue<std::string> command_queue_;
  std::thread input_thread_;
};

}  // namespace chess_engine

#endif  // SRC_WINBOARD_PROTOCOL_H_
