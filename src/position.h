#ifndef CHESS_ENGINE_SRC_POSITION_
#define CHESS_ENGINE_SRC_POSITION_

#include <array>
#include <cstdint>
#include <vector>

#include "chess_defines.h"

namespace chess_engine {

// One of the central classes.
// Responsible for keeping track of the board state, quickly generating legal moves,
// checking if the games has ended (via checkate or stalemate).  
class Position {
 public:
  bool IsCheck() const;
  bool IsCheckmate() const;
  bool IsStalemate() const;

  // whose turn it is
  Player PlayerToMove() const;
  void SetPlayerToMove(Player player);
  void PassTheTurn();

  std::vector<Move> GetLegalMoves() const;
  bool MoveIsLegal(Move move) const;

  // doesn't check for the move legality
  void MakeMove(Move move);
  
  Piece GetSquare(int file, int rank) const;
  Piece GetSquare(Coordinates square) const;
  void SetSquare(Coordinates square, Piece piece);

  // Castling works correctly, if rooks and kings are on the normal squares
  bool GetCastlingRights(Player player, Castle castle) const; 
  void SetCastlingRights(Player player, Castle castle, bool value);

  int16_t GetMoveNumber() const;
  void SetMoveNumber(int16_t value);

  int16_t GetHalfmoveClock() const;
  void SetHalfmoveClock(int16_t value);

  Coordinates GetEnPessant() const;
  void SetEnPessant(Coordinates square);
  
  Coordinates GetKing(Player player) const;

  // The amount of checks on player is in (only 0, 1 or 2 are possible)
  int8_t GetChecks(Player player) const;

 private:
  struct Pins {
    int8_t horisontal = 0;
    int8_t vertical = 0;
    int8_t upward = 0;
    int8_t downward = 0;
  };

  Pins GetPins(Coordinates square, Player player) const;
  void SetPin(Coordinates square, Player player, Coordinates delta, bool value);

  void GenerateMoves() const;
  void GeneratePawnMoves(Coordinates original_square) const;
  void GenerateKnightMoves(Coordinates original_square) const;
  void GenerateKingMoves(Coordinates original_square) const;

  void GenerateStraightMoves(Coordinates original_square, Coordinates delta) const;
  void GeneratePawnPushes(Coordinates original_square) const;
  void GeneratePawnCaptures(Coordinates original_square, int8_t file_delta) const;
  
  void GenerateCastles() const;

  struct Attacks {
    int8_t by_white;
    int8_t by_black;
    Attacks& operator+=(Attacks other);
    Attacks& operator*=(int8_t mult);
    Attacks operator-();
    bool operator==(Attacks other) const;
    bool operator!=(Attacks other) const;
  };

  struct AttackInfo {
    Attacks up = {0,0};
    Attacks up_right = {0,0};
    Attacks right = {0,0};
    Attacks down_right = {0,0};
    Attacks down = {0,0};
    Attacks down_left = {0,0};
    Attacks left = {0,0};
    Attacks up_left = {0,0};
    AttackInfo& operator+=(AttackInfo other);
    AttackInfo operator-();
  };

  Attacks GetAttacks(Coordinates square) const;
  int8_t GetAttacksByPlayer(Coordinates square, Player player) const;

  // Returns AttackInfo for the delayed update
  AttackInfo UpdateAttacks (
    Coordinates square,
    Piece piece,
    int8_t delta
  );

  void UpdateKnightAttacks (Coordinates square, Attacks delta);
  void UpdateKingAttacks (Coordinates square, Attacks delta);
  void UpdatePawnAttacks (Coordinates square, Player player, int8_t file_delta, Attacks delta);

  // Returns AttackInfo for the second wave
  void UpdateStraightAttacks (
    Coordinates square,
    Piece moving_piece,
    AttackInfo attack_delta,
    int8_t block_delta
  );

  void DeletePins(Player player);
  void RecalculatePins(Player player);

  // Add/remove attacks and pins in a given direction 
  // 'attack_delta' says if we should add or remove attacks.
  // 'attacker' is a piece that attacks in a give directions besides the queen.
  void AttackDirection(
    Coordinates square,
    Piece moveig_piece,
    Coordinates origin, 
    Coordinates delta,
    Attacks attack_delta,
    PieceType attacker,
    int8_t block_delta,
    bool second_wave,
    Attacks pin_delta = {0,0}
  );

  void PinDirection(
    Coordinates square,
    Player player,
    Coordinates delta,
    PieceType attacker
  );
  // Do some (not all) legality checks and push the move to leagal_moves_
  void PushLegalMove(Move move) const;

  Player to_move_ = Player::kWhite;
  bool white_castle_kingside_ = true;
  bool white_castle_queenside_ = true;
  bool black_castle_kingside_ = true;
  bool black_castle_queenside_ = true;

  Coordinates en_pessant_ = {-1,-1};
  int16_t halfmove_clock_ = 0;
  int16_t move_number_ = 0;

  Coordinates white_king_ = {-1,-1};
  Coordinates black_king_ = {-1,-1};
  Segment check_segment_ = {{-1,-1},{-1,-1}};

  std::array<std::array<Piece,8>, 8> board_ = {};
  std::array<std::array<Pins, 8>, 8> pins_on_white_ = {};
  std::array<std::array<Pins, 8>, 8> pins_on_black_ = {};
  std::array<std::array<Attacks, 8>, 8> attacks_ = {};

  mutable bool moves_generated_ = false;
  mutable std::vector<Move> legal_moves_;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_POSITION_