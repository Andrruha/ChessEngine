#include "fen.h"

#include "chess_defines.h"

#include <cassert>
#include <cctype>

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
    if (color == Piece::kWhite) {
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
        ret.type = PieceType::kNone;
        break;
      default:
        ret.type = PieceType::kNone;  // No piece type, but also no color.
        ret.player = PieceType::kNone;
    }
    return ret;
  }

}  // namespace chess_engine