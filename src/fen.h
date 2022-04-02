// FEN (Forsyth–Edwards Notation) is a popular chess notation for describing
// chess positions

#ifndef CHESS_ENGINE_SRC_FEN
#define CHESS_ENGINE_SRC_FEN

#include <string>

#include "chess_defines.h"
#include "position.h"
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

} //  namespace chess_engine

#endif  // CHESS_ENGINE_SRC_FEN