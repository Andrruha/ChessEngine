#include "fen.h"

#include "chess_defines.h"

#include <cassert>
#include <cctype>
#include <cstdint>

namespace chess_engine {

unsigned char PieceToFen(Piece piece) {
  unsigned char ret;
  switch (piece.type) {
    case PieceType::kNone:
      return ' ';  // Return, because color doesn't make sense here
      break;
    case PieceType::kPawn:
      ret = 'p';        
      break;
    case PieceType::kRook:
      ret = 'r';
      break;
    case PieceType::kKnight:
      ret = 'n';
      break;
    case PieceType::kBishop:
      ret = 'b';
      break;
    case PieceType::kQueen:
      ret = 'q';
      break;
    case PieceType::kKing:
      ret = 'k';
      break;
    default:
      assert(false);  // Piece type is invalid
      return '?';  // Return, because color is nonsensical
  }
  // Check if player is valid
  assert(piece.player == Player::kWhite || piece.player == Player::kBlack);
  if (piece.player == Player::kWhite) {
    ret = std::toupper(ret);
  }
  return ret;
}

Piece FenToPiece(unsigned char fen) {
  Piece ret;
  if (std::islower(fen)) {
    ret.player = Player::kBlack;
  } else if (std::isupper(fen)) {
    ret.player = Player::kWhite;
    fen = std::tolower(fen);
  }

  switch (fen) {
    case 'p':
      ret.type = PieceType::kPawn;      
      break;
    case 'r':
      ret.type = PieceType::kRook;
      break;
    case 'n':
      ret.type = PieceType::kKnight;
      break;
    case 'b':
      ret.type = PieceType::kBishop;
      break;
    case 'q':
      ret.type = PieceType::kQueen;
      break;
    case 'k':
      ret.type = PieceType::kKing;
      break;
    default:
      ret.type = PieceType::kNone;  // No piece type, but also no color.
      ret.player = Player::kNone;
  }
  return ret;
}

std::string CoordinatesToString(Coordinates coordinates) {
  if (coordinates == Coordinates{-1,-1}) {
    return "-";
  }
  std::string ret = "??";
  ret[0] = 'a' + coordinates.file;
  ret[1] = '1' + coordinates.rank;
  return ret;
}

Coordinates StringToCoordinates(const std::string& str) {
  if (str == "-") {
    return {-1,-1};
  }
  Coordinates ret;
  ret.file = str[0] - 'a';
  ret.rank = str[1] - '1';
  return ret;
}

Position FenToPosition(const std::string& fen) {
  int pos = -1;
  int next_pos;
  std::vector<std::string> parts;
  while ((next_pos = fen.find(' ', pos+1)) != std::string::npos) {
    std::string part = fen.substr(pos+1, next_pos - pos - 1);
    parts.push_back(part);
    pos = next_pos;
  }
  std::string part = fen.substr(pos, next_pos);
  parts.push_back(part);

  Position ret;

  std::string position_str = parts[0];
  int position_index = 0;
  int to_skip = 0;
  for (int8_t rank = 7; rank >= 0; --rank) {
    for (int8_t file = 0; file < 8; ++file) {
      if(to_skip) {
        --to_skip;
        continue; 
      }
      char current = parts[0][position_index];
      if ('1' <= current && current <= '8') {
        to_skip = current - '1';
      } else {
        ret.SetSquare({file, rank}, FenToPiece(current));
      }
      ++position_index;
    }
    ++position_index;
  }

  if (parts[1] == "w") {
    ret.SetPlayerToMove(Player::kWhite);
  } else if (parts[1] == "b") {
    ret.SetPlayerToMove(Player::kBlack);
  } else {
    assert(false);  // Invalid player
  }

  if (parts[2].find('K') == std::string::npos) {
    ret.SetCastlingRights(Player::kWhite, Castle::kKingside, false);
  }
  if (parts[2].find('Q') == std::string::npos) {
    ret.SetCastlingRights(Player::kWhite, Castle::kQueenside, false);
  }
  if (parts[2].find('k') == std::string::npos) {
    ret.SetCastlingRights(Player::kBlack, Castle::kKingside, false);
  }
  if (parts[2].find('q') == std::string::npos) {
    ret.SetCastlingRights(Player::kBlack, Castle::kQueenside, false);
  }
  if (parts[3] == "-") {
    ret.SetEnPessant({-1,-1});
  } else {
    ret.SetEnPessant(StringToCoordinates(parts[3]));
  }
  // TODO(Andrey): 50 moves rule and turn number
  return ret;
}

std::string PositionToFen(const Position& position) {
  std::string ret;
  int counter = 0;
  for (int8_t rank = 7; rank >= 0; --rank) {
    for (int8_t file = 0; file < 8; ++file) {
      if (position.GetSquare({file, rank}) == pieces::kNone) {
        ++counter;
      } else {
        if (counter) {
          ret += '0' + counter;
          counter = 0;
        }
        ret += PieceToFen(position.GetSquare({file, rank}));
      } 
    }
    if (counter) {
      ret += '0' + counter;
      counter = 0;
    }
    if (rank != 0) {
      ret += '/';
    }
  }
  ret += ' ';

  if (position.PlayerToMove() == Player::kWhite) {
    ret += 'w';
  } else {
    ret += 'b';
  }
  ret += ' ';

  bool no_castle = true;
  if (position.GetCastlingRights(Player::kWhite, Castle::kKingside)) {
    ret += 'K';
    no_castle = false;
  }
  if (position.GetCastlingRights(Player::kWhite, Castle::kQueenside)) {
    ret += 'Q';
    no_castle = false;
  }
  if (position.GetCastlingRights(Player::kBlack, Castle::kKingside)) {
    ret += 'k';
    no_castle = false;
  }
  if (position.GetCastlingRights(Player::kBlack, Castle::kQueenside)) {
    ret += 'q';
    no_castle = false;
  }
  if (no_castle) {
    ret += '-';
  }
  ret += ' ';

  ret += CoordinatesToString(position.GetEnPessant());

  ret += " 0 0";  // TODO(Andrey): Finish!

  return ret;
}

}  // namespace chess_engine
