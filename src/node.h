#ifndef SRC_NODE_H_
#define SRC_NODE_H_

#include <vector>

#include "src/chess_defines.h"
#include "src/position.h"
#include "src/zobrist_hash.h"

namespace chess_engine {

// Represents a node in the search tree.
// Has position functionality, but also handles hash.
class Node {
 public:
  explicit Node(const ZobristHashFunction& hash_func);
  Node(const Position& position, const ZobristHashFunction& func);

  bool IsCheck() const;
  bool IsCheckmate() const;
  bool IsStalemate() const;

  // Whose turn it is.
  Player PlayerToMove() const;
  void SetPlayerToMove(Player player);
  void PassTheTurn();

  bool MoveIsCheckFast(Move move) const;
  std::vector<Move> GetLegalMoves() const;
  std::vector<Move> GetCapturesOnSquare(
    Coordinates square, Player player
  ) const;

  void MakeMove(Move move);

  // Incrementaly update hash after a move in a current position.
  void HashMove(ZobristHash* hash, Move move) const;
  // Somewhat similar to MakeMove, but only for hash.
  ZobristHash HashAfterMove(Move move) const;

  Piece GetSquare(int file, int rank) const;
  Piece GetSquare(Coordinates square) const;
  void SetSquare(Coordinates square, Piece piece);

  bool GetCastlingRights(Player player, Castle castle) const;
  void SetCastlingRights(Player player, Castle castle, bool value);

  Coordinates GetEnPessant() const;
  void SetEnPessant(Coordinates square);

  Coordinates GetKing(Player player) const;
  int8_t GetChecks(Player player) const;
  int8_t GetAttacksByPlayer(Coordinates square, Player player) const;

  int16_t GetMoveNumber() const;
  void SetMoveNumber(int16_t value);

  int16_t GetHalfmoveClock() const;
  void SetHalfmoveClock(int16_t value);

  Coordinates GetLastCapture() const;

  ZobristHash GetHash() const;
  void SetPosition(const Position& position);
  const Position& GetPosition() const;

 private:
  ZobristHash hash_;
  Position position_;
  Coordinates last_capture_ = {-1, -1};
};

}  // namespace chess_engine

#endif  // SRC_NODE_H_
