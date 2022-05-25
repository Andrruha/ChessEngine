#include "src/winboard_protocol.h"

#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <vector>

#include "src/fen.h"

namespace chess_engine {

void WinboardProtocol::ProcessCommands() {
  std::unique_lock lock(mutex_);
  if (commands_recieved_.wait_for(
    lock, std::chrono::seconds(0), [&]{return !command_queue_.empty();}
  )) {
    while (!command_queue_.empty()) {
      std::string command = command_queue_.front();
      command_queue_.pop();
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
      } else if (parts[0] == "white") {
        set_color_callback_(Player::kBlack);
      } else if (parts[0] == "black") {
        set_color_callback_(Player::kWhite);
      } else if (parts[0] == "force") {
        set_mode_callback_(EngineMode::kForce);
      } else if (parts[0] == "go") {
        set_mode_callback_(EngineMode::kPlay);
      } else if (parts[0] == "analyze") {
        set_mode_callback_(EngineMode::kAnalyse);
      } else if (parts[0] == "new") {
        new_game_callback_();
      } else if (parts[0] == "setboard") {
        Position position = FenToPosition(
          parts[1] + " " + parts[2] + " " + parts[3] + " " +
          parts[4] + " " + parts[5] + " " + parts[6]
        );
        set_board_callback_(position);
      } else if (parts[0] == "usermove") {
        move_recieved_callback_(XBoardToMove(parts[1]));
      } else if (parts[0] == "undo") {
        undo_recieved_callback_();
      }
    }
  }
}

void WinboardProtocol::StartInputLoop() {
  input_thread_ = std::thread([this]{
    // TODO(Andrey): Better loop condition.
    while (true) {
      std::string command;
      std::getline(std::cin, command);
      std::lock_guard lock(mutex_);
      command_queue_.push(command);
      commands_recieved_.notify_one();
    }
  });
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
  std::cout << ply << " " << centipawns << " "
            << centiseconds << " " << nodes << " ";
  for (Move move : pv) {
    std::cout << MoveToXBoard(move) << " ";
  }
  std::cout << std::endl;
}

void WinboardProtocol::SendFeatures() {
  std::cout << "feature colors=0 playother=1 setboard=1 usermove=1 done=1"
            << std::endl;
}

}  // namespace chess_engine
