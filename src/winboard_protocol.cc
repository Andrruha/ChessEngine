#include "src/winboard_protocol.h"

#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <vector>

#include "src/fen.h"

namespace chess_engine {

void WinboardProtocol::ProcessCommands() {
  std::queue<std::string> to_process;
  {
    std::unique_lock lock(mutex_);
    if (commands_received_.wait_for(
      lock, std::chrono::seconds(0), [&]{return !command_queue_.empty();}
    )) {
      std::swap(command_queue_, to_process);
      command_queue_ = {};
    }
  }
  while (!to_process.empty()) {
    std::string command = to_process.front();
    to_process.pop();
    std::ofstream log;
    log.open("log_protocol.txt", std::ios_base::app);
    log << "received: " << command << "\n";
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
      set_mode_callback_(EngineMode::kAnalyze);
    } else if (parts[0] == "new") {
      new_game_callback_();
    } else if (parts[0] == "setboard") {
      Position position = FenToPosition(
        parts[1] + " " + parts[2] + " " + parts[3] + " " +
        parts[4] + " " + parts[5] + " " + parts[6]
      );
      set_board_callback_(position);
    } else if (parts[0] == "usermove") {
      move_received_callback_(XBoardToMove(parts[1]));
    } else if (parts[0] == "undo") {
      undo_received_callback_();
    } else if (parts[0] == "level") {
      TimeControl tc;
      tc.period = std::stoi(parts[1]);
      tc.seconds_per_period = StringToSeconds(parts[2]);
      tc.increment = std::stoi(parts[3]);
      set_time_callback_(tc);
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
      commands_received_.notify_one();
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

double WinboardProtocol::StringToSeconds(const std::string& str) {
  size_t delimeter_pos = str.find(':');
  if (delimeter_pos == std::string::npos) {
    return 60 * std::stoi(str);
  } else {
    std::string minutes_str = str.substr(0, delimeter_pos);
    std::string seconds_str = str.substr(delimeter_pos + 1);
    return 60 * std::stoi(minutes_str) + std::stoi(seconds_str);
  }
}

}  // namespace chess_engine
