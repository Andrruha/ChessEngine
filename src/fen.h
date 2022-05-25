// FEN (Forsyth–Edwards Notation) is a popular chess notation for describing
// chess positions.

#ifndef SRC_FEN_H_
#define SRC_FEN_H_

#include <string>

#include "src/chess_defines.h"
#include "src/position.h"
namespace chess_engine {

// In FEN the case of a letter indicates piece color.
unsigned char PieceToFen(Piece piece);
// In FEN the case of a letter indicates piece color.
Piece FenToPiece(unsigned char fen);

std::string CoordinatesToString(Coordinates coordinates);
Coordinates StringToCoordinates(const std::string& str);

Position FenToPosition(const std::string& fen);
std::string PositionToFen(const Position& position);

std::string MoveToUci(Move move);
Move UciToMove(const std::string& str);

std::string MoveToXBoard(Move move);
Move XBoardToMove(const std::string& str);

}  // namespace chess_engine

#endif  // SRC_FEN_H_
