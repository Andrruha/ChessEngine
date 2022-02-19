#include "fen.h"

#include "chess_defines.h"

#include <cassert>
#include <cctype>

namespace chess_engine {

  unsigned char PieceToFen(Piece piece) {
    unsigned char ret;
    Piece type = GetPieceType(piece);
    switch (type) {
      case Piece::kNone:
        return ' ';  // Return, because color doesn't make sense here
        break;
      case Piece::kPawn:
        ret = 'p';        
        break;
      case Piece::kRook:
        ret = 'r';
        break;
      case Piece::kKnight:
        ret = 'n';
        break;
      case Piece::kBishop:
        ret = 'b';
        break;
      case Piece::kQueen:
        ret = 'q';
        break;
      case Piece::kKing:
        ret = 'k';
        break;
      default:
        assert(false);  // Piece type is invalid
        return '?';  // Return, because color is nonsensical
    }

    Piece color = GetPieceColor(piece);
    assert(color != Piece::kNone);  // Piece doesn't have a color
    if (color == Piece::kWhite) {
      ret = std::toupper(ret);
    }

    return ret;
  }

  Piece FenToPiece(unsigned char fen) {
    Piece color = Piece::kNone;
    if (std::islower(fen)) {
      color = Piece::kBlack;
    } else if (std::isupper(fen)) {
      color = Piece::kWhite;
      fen = std::tolower(fen);
    }

    switch (fen) {
      case 'p':
        return SetPieceType(color, Piece::kPawn);        
        break;
      case 'r':
        return SetPieceType(color, Piece::kRook);
        break;
      case 'n':
        return SetPieceType(color, Piece::kKnight);
        break;
      case 'b':
        return SetPieceType(color, Piece::kBishop);
        break;
      case 'q':
        return SetPieceType(color, Piece::kQueen);
        break;
      case 'k':
        return SetPieceType(color, Piece::kKing);
        break;
      default:
        return Piece::kNone;  // No piece type, but also no color.
    }
  }

}  // namespace chess_engine