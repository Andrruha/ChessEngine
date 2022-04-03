#include "winboard_protocol.h"

#include <iostream>
#include <fstream>
#include <string>

#include "fen.h"

namespace chess_engine {

void WinboardProtocol::WaitForCommands() {
  // TODO(Andrey): do I need a loop here?
  std::string command;
  std::getline(std::cin, command);
  if (!command.empty()) {
    std::ofstream log;
    log.open("log_protocol.txt", std::ios_base::app);
    log << "recieved: " << command << "\n"; 
    log.close();
    std::vector<std::string> parts;
    int pos = -1;
    int next_pos;
    while ((next_pos = command.find(' ', pos+1)) != std::string::npos) {
      std::string part = command.substr(pos+1, next_pos - pos - 1);
      parts.push_back(part);
      pos = next_pos;
    }
    std::string part = command.substr(pos+1, next_pos);
    parts.push_back(part);
    if (parts[0] == "protover") {
      SendFeatures();
    } else if (parts[0] == "new") {
      new_game_callback_();
    } else if (parts[0] == "usermove") {
      move_recieved_callback_(XBoardToMove(parts[1]));
    }
  }
}

void WinboardProtocol::MakeMove(Move move) {
  std::cout << "move " << MoveToXBoard(move) << std::endl;
}

void WinboardProtocol::DisplayInfo(
  int16_t ply,
  int32_t centipawns,
  int32_t centiseconds,
  int64_t nodes,
  const std::list<Move>& pv
) const {
  std::cout << ply << " " << centipawns << " " << centiseconds << " " << nodes << " ";
  for (chess_engine::Move move: pv) {
    std::cout << chess_engine::MoveToXBoard(move) << " ";
  }
  std::cout << std::endl;
}

void WinboardProtocol::SendFeatures() {
  std::cout << "feature setboard=1 usermove=1 done=1" << std::endl;
}

}  // namespace chess_engine