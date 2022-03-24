#include "position.h"

#include <array>
#include <cassert>
#include <cstdint>

#include "chess_defines.h"

namespace chess_engine {

bool Position::IsCheck() const {
  return GetChecks(to_move_) > 0;
}

bool Position::IsCheckmate() const {
  return IsCheck() && GetLegalMoves().empty();
}

bool Position::IsStalemate() const {
  return !IsCheck() && GetLegalMoves().empty();
}

Player Position::PlayerToMove() const {
  return to_move_;
}

void Position::SetPlayerToMove(Player player) {
  to_move_ = player;
}

 void Position::PassTheTurn() {
   if (to_move_ == Player::kWhite) {
     to_move_ = Player::kBlack;
   } else {
     ++move_number_;
     to_move_ = Player::kWhite;
   }
 }

void Position::MakeMove(Move move) {
  check_segment_ = {{-1,-1},{-1,-1}};  // If move is legal it deals with all checks

  // En pessant
  int8_t dir = PawnDirection(to_move_);
  if (move.to == en_pessant_ && move.piece.type == PieceType::kPawn) {
    Coordinates taken = move.to;
    taken.rank -= dir;
    SetSquare(taken, pieces::kNone);
    SetSquare(move.from, pieces::kNone);
    SetSquare(move.to, move.piece);
    halfmove_clock_ = 0;
  }
  
  if (move.piece.type == PieceType::kKing) {
    if (move.to == move.from + Coordinates{2,0}) {
      SetSquare(move.from, pieces::kNone);
      SetSquare(move.from + Coordinates{3,0}, pieces::kNone);
      SetSquare(move.to, {PieceType::kKing, to_move_});
      SetSquare(move.to + Coordinates{-1,0}, {PieceType::kRook, to_move_});
    }
    if (move.to == move.from + Coordinates{-2,0}) {
      SetSquare(move.from, pieces::kNone);
      SetSquare(move.from + Coordinates{-4,0}, pieces::kNone);
      SetSquare(move.to, {PieceType::kKing, to_move_});
      SetSquare(move.to + Coordinates{1,0}, {PieceType::kRook, to_move_});
    }
  }

  // Checks and pins are updated inside the SetSquare function
  SetSquare(move.from, pieces::kNone);
  SetSquare(move.to, move.piece);

  // Update en-pessant
  if (
    move.piece.type == PieceType::kPawn && 
    move.to.rank - move.from.rank == dir*2
  ) {
    en_pessant_ = move.from;
    en_pessant_.rank += dir;
  } else {
    en_pessant_ = {-1,-1};
  }

  // Update castling rights
  if (move.from == Coordinates{0,0} || move.to == Coordinates{0,0}) {
    white_castle_queenside_ = false;
  }
  if (move.from == Coordinates{7,0} || move.to == Coordinates{7,0}) {
    white_castle_kingside_ = false;
  }
  if (move.from == Coordinates{0,7} || move.to == Coordinates{0,7}) {
    black_castle_queenside_ = false;
  }
  if (move.from == Coordinates{7,7} || move.to == Coordinates{7,7}) {
    black_castle_kingside_ = false;
  }


  if (move.piece == pieces::kWhiteKing) {
    white_castle_kingside_ = false;
    white_castle_queenside_ = false;
  }
  if (move.piece == pieces::kBlackKing) {
    black_castle_kingside_ = false;
    black_castle_queenside_ = false;
  }

  if (GetSquare(move.from).type == PieceType::kPawn) {
    halfmove_clock_ = 0;
  } else {
    ++halfmove_clock_;
  }

  PassTheTurn();
}

std::vector<Move> Position::GetLegalMoves() const {
  if (!moves_generated_) {
    legal_moves_.clear();
    GenerateMoves();
  }
  return legal_moves_;
}

bool Position::MoveIsLegal(Move move) const {
  // TODO(Andrey): Implement
}

Piece Position::GetSquare(Coordinates square) const {
  return board_[square.file][square.rank];
}

void Position::SetSquare(Coordinates square, Piece piece) {
  // Nothing -> Nothing - do nothing
  // Nothing -> Piece - add attacks, block
  // Piece -> Nothing - remove attacks, add discoveries
  // Piece -> Piece - remove attackes, add attacks
  
  Piece old_piece = GetSquare(square);

  if (old_piece.type == PieceType::kKing) {
    DeletePins(old_piece.player);
  }

  if (piece.type == PieceType::kKing) {
    if (piece.player == Player::kWhite) {
      white_king_ = square;
      RecalculatePins(Player::kWhite);
    } else {
      black_king_ = square;
      RecalculatePins(Player::kBlack);
    }
  }

  Attacks basic_white_attacks = {1,0};
  Attacks basic_blacks_attacks = {0,1};

  if (old_piece == pieces::kNone && piece == pieces::kNone) {
    return;
  }

  AttackInfo delayed_attacks = {};
  
  if (old_piece != pieces::kNone) {
    delayed_attacks += UpdateAttacks(square, old_piece, -1);
  }
  if (piece != pieces::kNone) {
    delayed_attacks += UpdateAttacks(square, piece, 1);
  }

  int8_t block_delta = 0;
  if (old_piece != pieces::kNone && piece == pieces::kNone) {
    block_delta = 1;
  }
  if (old_piece == pieces::kNone && piece != pieces::kNone) {
    block_delta = -1;
  }
  UpdateStraightAttacks(square, old_piece, delayed_attacks, block_delta);

  board_[square.file][square.rank] = piece;
}


bool Position::GetCastlingRights(Player player, Castle castle) const {
  if (player == Player::kWhite) {
    if (castle == Castle::kKingside) {
      return white_castle_kingside_;
    } else if (castle == Castle::kQueenside){
      return white_castle_queenside_;
    }
  } else if (player == Player::kBlack) {
    if (castle == Castle::kKingside) {
      return black_castle_kingside_;
    } else if (castle == Castle::kQueenside){
      return black_castle_queenside_;
    }
  }
  assert(false);  // Invalid player or castling side
  return false;
}

void Position::SetCastlingRights(Player player, Castle castle, bool value) {
  if (player == Player::kWhite) {
    if (castle == Castle::kKingside) {
      white_castle_kingside_ = value;
    } else if (castle == Castle::kQueenside){
      white_castle_queenside_ = value;
    }
  } else if (player == Player::kBlack) {
    if (castle == Castle::kKingside) {
      black_castle_kingside_ = value;
    } else if (castle == Castle::kQueenside){
      black_castle_queenside_ = value;
    }
  } else {
    assert(false);  // Invalid player or castling side
  }
}

  int16_t Position::GetMoveNumber() const {
    return move_number_;
  }

  void Position::SetMoveNumber(int16_t value) {
    move_number_ = value;
  }

  int16_t Position::GetHalfmoveClock() const {
    return halfmove_clock_;
  }
  void Position::SetHalfmoveClock(int16_t value) {
    halfmove_clock_ = value;
  }

Coordinates Position::GetEnPessant() const {
  return en_pessant_;
}

void Position::SetEnPessant(Coordinates square) {
  en_pessant_ = square;
}

Coordinates Position::GetKing(Player player) const {
  if (player == Player::kWhite) {
    return white_king_;
  } else if (player == Player::kBlack) {
    return black_king_;
  }
  assert(false);  // Invalid player
  return {-1,-1};
}

Position::AttackInfo Position::UpdateAttacks (
  Coordinates square,
  Piece piece,
  int8_t delta
) {
  Attacks basic_attacks;
  if (piece.player == Player::kWhite) {
    basic_attacks = {1, 0};
  } else if (piece.player == Player::kBlack) {
    basic_attacks = {0, 1};
  } else {
    assert(false);  // Invalid player
  }
  basic_attacks *= delta;
  AttackInfo straight_attacks = {};
  switch (piece.type) {
    case (PieceType::kPawn):
      UpdatePawnAttacks(square, piece.player, 1, basic_attacks);
      UpdatePawnAttacks(square, piece.player, -1, basic_attacks);
    break;
    case (PieceType::kRook):
      straight_attacks.up = basic_attacks;
      straight_attacks.right = basic_attacks;
      straight_attacks.down = basic_attacks;
      straight_attacks.left = basic_attacks;
    break;
    case (PieceType::kKnight):
      UpdateKnightAttacks(square, basic_attacks);
    break;
    case (PieceType::kBishop):
      straight_attacks.up_right = basic_attacks;
      straight_attacks.down_right = basic_attacks;
      straight_attacks.down_left = basic_attacks;
      straight_attacks.up_left = basic_attacks;
    break;
    case (PieceType::kQueen):
      straight_attacks.up = basic_attacks;
      straight_attacks.right = basic_attacks;
      straight_attacks.down = basic_attacks;
      straight_attacks.left = basic_attacks;
      straight_attacks.up_right = basic_attacks;
      straight_attacks.down_right = basic_attacks;
      straight_attacks.down_left = basic_attacks;
      straight_attacks.up_left = basic_attacks;
    break;
    case (PieceType::kKing):
      UpdateKingAttacks(square, basic_attacks);
    break;
    default:
      assert(false);  // Invalid piece
    break;
  }
  return straight_attacks;
}

void Position::UpdateKnightAttacks(Coordinates square, Attacks delta) {
  Piece piece = GetSquare(square);
  std::array<Coordinates, 8> jumps = {
    Coordinates{2,1}, {2,-1}, {-2,1}, {-2,-1}, {1,2}, {1,-2}, {-1,2}, {-1,-2}
  };
  for (Coordinates jump : jumps) {
    Coordinates destination = square;
    destination += jump;
    if (!WithinTheBoard(destination)) {
      continue;
    }
    attacks_[destination.file][destination.rank] += delta;
    if (destination == GetKing(Opponent(to_move_))) {
      check_segment_ = {square, square};
    }
  }
}

void Position::UpdateKingAttacks (Coordinates square, Attacks delta) {
  for (int8_t file = square.file-1; file <= square.file+1; ++file) {
    for (int8_t rank = square.rank-1; rank <= square.rank+1; ++rank) {
      if (file == square.file && rank == square.rank) {
        continue;
      }
      Coordinates destination = {file, rank};
      if (!WithinTheBoard(destination)) {
        continue;
      }
      attacks_[file][rank] += delta;
    }
  }
}

void Position::UpdatePawnAttacks(Coordinates square, Player player, int8_t file_delta, Attacks delta) {
Coordinates destination = square;
  int8_t dir = PawnDirection(player);
  destination.rank += dir;
  destination.file += file_delta;
  if (!WithinTheBoard(destination)) {
    return;
  }
  attacks_[destination.file][destination.rank] += delta;

  if (destination == GetKing(Opponent(to_move_))) {
    check_segment_ = {square, square};
  }
}

void Position::UpdateStraightAttacks (
  Coordinates square,
  Piece moving_piece,
  AttackInfo attack_delta,
  int8_t block_delta
) {
  AttackDirection(square, moving_piece, square, {0,1}, attack_delta.up, PieceType::kRook, block_delta, false);
  AttackDirection(square, moving_piece, square, {1,0}, attack_delta.right, PieceType::kRook, block_delta, false);
  AttackDirection(square, moving_piece, square, {0,-1}, attack_delta.down, PieceType::kRook, block_delta, false);
  AttackDirection(square, moving_piece, square, {-1,0}, attack_delta.left, PieceType::kRook, block_delta, false);
moving_piece, 
  AttackDirection(square, moving_piece, square, {1,1}, attack_delta.up_right, PieceType::kBishop, block_delta, false);
  AttackDirection(square, moving_piece, square, {1,-1}, attack_delta.down_right, PieceType::kBishop, block_delta, false);
  AttackDirection(square, moving_piece, square, {-1,-1}, attack_delta.down_left, PieceType::kBishop, block_delta, false);
  AttackDirection(square, moving_piece, square, {-1,1}, attack_delta.up_left, PieceType::kBishop, block_delta, false);
}

void Position::DeletePins(Player player) {
  if (player == Player::kWhite) {
    pins_on_white_ = {};
  } else if (player == Player::kBlack) {
    pins_on_black_ = {};
  } else {
    assert(false);  // Invalid player
  }
}

void Position::RecalculatePins(Player player) {
  Coordinates square = ((player == Player::kWhite)?white_king_:black_king_);
  PinDirection(square, player, {0,1}, PieceType::kRook);
  PinDirection(square, player, {1,0}, PieceType::kRook);
  PinDirection(square, player, {0,-1}, PieceType::kRook);
  PinDirection(square, player, {-1,0}, PieceType::kRook);
 
  PinDirection(square, player, {1,1}, PieceType::kBishop);
  PinDirection(square, player, {1,-1}, PieceType::kBishop);
  PinDirection(square, player, {-1,-1}, PieceType::kBishop);
  PinDirection(square, player, {-1,1}, PieceType::kBishop);
}

Position::Attacks Position::GetAttacks(Coordinates square) const {
  return attacks_[square.file][square.rank];
}

int8_t Position::GetAttacksByPlayer(Coordinates square, Player player) const {
  if (player == Player::kWhite) {
    return GetAttacks(square).by_white;
  } else if (player == Player::kBlack) {
    return GetAttacks(square).by_black;
  }
  assert(false);  // Invalid player
  return 0;
}

int8_t Position::GetChecks(Player player) const {
  if (player == Player::kWhite) {
    return GetAttacks(white_king_).by_black;
  } else if (player == Player::kBlack) {
    return GetAttacks(black_king_).by_white;
  }
  assert(false);  // Invalid player
  return 0;
}

void Position::GenerateMoves() const {
  if (halfmove_clock_ == 100) {
    return;
  }

  GenerateCastles();

  for (int8_t file = 0; file < 8; ++file) {
    for (int8_t rank = 0; rank < 8; ++rank) {
      Piece piece = board_[file][rank];
      if (piece.player != to_move_) {
        continue;
      }

      Pins pins = GetPins({file, rank}, to_move_);
      bool pin = pins.horisontal || pins.vertical || pins.upward || pins.downward;

      // directions the piece can move in
      bool vertical_free = !pins.horisontal && !pins.upward && !pins.downward;
      bool horisontal_free = !pins.vertical && !pins.upward && !pins.downward;
      bool upward_free = !pins.horisontal && !pins.vertical && !pins.downward;
      bool downward_free = !pins.horisontal && !pins.vertical && !pins.upward;

      int8_t dir = PawnDirection(to_move_);

      // if we are not in check we should be carefull to not check
      // our king by moving pinned piece

      int8_t checks = GetChecks(to_move_);

      if (checks && pin) {
        continue;
      }
      if (checks >= 2 && piece.type != PieceType::kKing) {
        continue;
      }

      switch (piece.type) {
        case (PieceType::kPawn): 
          if (vertical_free) {
            GeneratePawnPushes({file,rank});
          }
          if (upward_free) {
            GeneratePawnCaptures({file, rank}, dir);
          }
          if (downward_free) {
            GeneratePawnCaptures({file, rank}, -dir);
          }
        break;
        case (PieceType::kRook):
          if (vertical_free) {
            GenerateStraightMoves({file, rank}, {0,1});
            GenerateStraightMoves({file, rank}, {0,-1});
          }
          if (horisontal_free) {
            GenerateStraightMoves({file, rank}, {1,0});
            GenerateStraightMoves({file, rank}, {-1,0});
          }
        break;
        case (PieceType::kKnight):
          if (!pin) {
            GenerateKnightMoves({file, rank});
          }
        break;
        case (PieceType::kBishop):
          if (upward_free) {
            GenerateStraightMoves({file, rank}, {1,1});
            GenerateStraightMoves({file, rank}, {-1,-1});
          }
          if (downward_free) {
            GenerateStraightMoves({file, rank}, {1,-1});
            GenerateStraightMoves({file, rank}, {-1,1});
          }
        break;
        case (PieceType::kQueen):
          if (vertical_free) {
            GenerateStraightMoves({file, rank}, {0,1});
            GenerateStraightMoves({file, rank}, {0,-1});
          }
          if (horisontal_free) {
            GenerateStraightMoves({file, rank}, {1,0});
            GenerateStraightMoves({file, rank}, {-1,0});
          }
          if (upward_free) {
            GenerateStraightMoves({file, rank}, {1,1});
            GenerateStraightMoves({file, rank}, {-1,-1});
          }
          if (downward_free) {
            GenerateStraightMoves({file, rank}, {1,-1});
            GenerateStraightMoves({file, rank}, {-1,1});
          }
        break;
        case (PieceType::kKing):
          GenerateKingMoves({file, rank});
        break;
        default:
          assert(false);  // Invalid piece
        break;
      }
    }
  }
}

void Position::GenerateKnightMoves(Coordinates original_square) const {
  Piece piece = GetSquare(original_square);
  std::array<Coordinates, 8> jumps = {
    Coordinates{2,1}, {2,-1}, {-2,1}, {-2,-1}, {1,2}, {1,-2}, {-1,2}, {-1,-2}
  };
  for (Coordinates jump : jumps) {
    Coordinates destination = original_square;
    destination.file += jump.file;
    destination.rank += jump.rank;
    if (!WithinTheBoard(destination)) {
      continue;
    }
    if (GetSquare(destination).player != to_move_) {
      PushLegalMove({original_square, destination, piece});
    }
  }
}

void Position::GenerateKingMoves(Coordinates original_square) const {
  for (int8_t file = original_square.file-1; file <= original_square.file+1; ++file) {
    for (int8_t rank = original_square.rank-1; rank <= original_square.rank+1; ++rank) {
      if (file == original_square.file && rank == original_square.rank) {
        continue;
      }
      Coordinates destination = {file, rank};
      if (!WithinTheBoard(destination)) {
        continue;
      }
      if (GetSquare(destination).player != to_move_) {
        PushLegalMove({original_square, destination, {PieceType::kKing, to_move_}});
      }
    }
  }
  // TODO(Andrey): castling
}

void Position::GenerateStraightMoves(Coordinates original_square,
                                     Coordinates delta) const {
  Piece piece = GetSquare(original_square);
  Coordinates destination = original_square;
  destination += delta;
  while (WithinTheBoard(destination)) {
    if (GetSquare(destination) != pieces::kNone) {
      if (GetSquare(destination).player != to_move_) {
        PushLegalMove({original_square, destination, piece});
      }
      return;
    }
    PushLegalMove({original_square, destination, piece});
    destination += delta;
  }
}

void Position::GeneratePawnPushes(Coordinates original_square) const {
  int8_t dir = PawnDirection(to_move_);
  Coordinates destination = original_square;
  destination.rank += dir;
  if (!WithinTheBoard(destination)) {
    return;  // 1-square push outside the board -> 2-square push is too 
  }
  if (GetSquare(destination) == pieces::kNone) {
    if (destination.rank != PromotionRank(to_move_)) {
      PushLegalMove({original_square, destination, {PieceType::kPawn, to_move_}});
    } else {
      for (PieceType promote_to : kPromotionOptions) {
        PushLegalMove({original_square, destination, {promote_to, to_move_}});
      }
    }
    if (original_square.rank == DoubleJumpRank(to_move_)) {
      destination.rank += dir;
      if (!WithinTheBoard(destination)) {
        return;
      }
      if (GetSquare(destination) == pieces::kNone) {
        // implicitly using the size of the board
        PushLegalMove({original_square, destination, {PieceType::kPawn, to_move_}});
      }
    }
  }
}

void Position::GeneratePawnCaptures(Coordinates original_square, int8_t file_delta) const {
  Coordinates destination = original_square;
  int dir = PawnDirection(to_move_);
  destination.rank += dir;
  destination.file += file_delta;
  if (!WithinTheBoard(destination)) {
    return;
  }
  if (GetSquare(destination).player != Opponent(to_move_) && 
      en_pessant_ != destination) {
    return;  // nothing to capture
  }

  // En pessant is annoying, because it removes 2 pawn from 1 rank
  if (en_pessant_ == destination) {
    Piece first_piece = pieces::kNone;
    Coordinates current = original_square;
    current.file += file_delta*2;
    for (;0 <= current.file && current.file < 8; current.file += file_delta) {
      if (GetSquare(current) != pieces::kNone) {
        first_piece = GetSquare(current);
      }
    }
    Piece second_piece;
    current = original_square;
    current.file -= file_delta;
    for (;0 <= current.file && current.file < 8; current.file += file_delta) {
      if (GetSquare(current) != pieces::kNone) {
        second_piece = GetSquare(current);
      }
    }
    if (
      first_piece == Piece{PieceType::kKing, to_move_} && 
      (
        second_piece == Piece{PieceType::kKing, Opponent(to_move_)} || 
        second_piece == Piece{PieceType::kQueen, Opponent(to_move_)}
      )
    ) {
      return;
    }
    if (
      second_piece == Piece{PieceType::kKing, to_move_} && 
      (
        first_piece == Piece{PieceType::kKing, Opponent(to_move_)} || 
        first_piece == Piece{PieceType::kQueen, Opponent(to_move_)}
      )
    ) {
      return;
    }
  }

  if (destination.rank != PromotionRank(to_move_)) {
    PushLegalMove({
      original_square, destination, {PieceType::kPawn, to_move_}
    });
  } else {
    for (PieceType promote_to : kPromotionOptions) {
      PushLegalMove({original_square, destination, {promote_to, to_move_}});
    }
  }
}

void Position::GenerateCastles() const {
  if (IsCheck()) {
    return;
  }
  if (GetCastlingRights(to_move_, Castle::kKingside)) {
    Coordinates king = GetKing(to_move_);
    Coordinates current = king;
    bool possible = true;
    for (int8_t i = 0; i < 2; ++i) {
      current += {1,0};
      if (GetAttacksByPlayer(current, Opponent(to_move_))) {
        possible = false;
      }
      if (GetSquare(current) != pieces::kNone) {
        possible = false;
      }
    }
    if (possible) {
      PushLegalMove({king, current, {PieceType::kKing, to_move_}});
    }
  }
  if (GetCastlingRights(to_move_, Castle::kQueenside)) {
    Coordinates king = GetKing(to_move_);
    Coordinates current = king;
    bool possible = true;
    for (int8_t i = 0; i < 2; ++i) {
      current += {-1,0};
      if (GetAttacksByPlayer(current, Opponent(to_move_))) {
        possible = false;
      }
      if (GetSquare(current) != pieces::kNone) {
        possible = false;
      }
    }
    Coordinates extra = current;
    extra += {-1, 0};
    if (GetSquare(extra) != pieces::kNone) {
      possible = false;
    }
    if (possible) {
      PushLegalMove({king, current, {PieceType::kKing, to_move_}});
    }
  }
}

// TODO(Andrey): Reduce code duplication!
void Position::AttackDirection(
    Coordinates square,
    Piece moving_piece,
    Coordinates origin, 
    Coordinates delta,
    Attacks attack_delta,
    PieceType attacker,
    int8_t block_delta,
    bool second_wave,
    Attacks pin_delta
) {
  Attacks basic_white_attacks = {1,0};
  Attacks basic_black_attacks = {0,1};
  if (moving_piece == pieces::kWhiteKing) {
    basic_black_attacks = {0,0};
  }
  if (moving_piece == pieces::kBlackKing) {
    basic_white_attacks = {0,0};
  }

  Coordinates current = square;
  current += delta;
  Attacks second_wave_attacks = {0, 0};
  Attacks ghost_delta = {0, 0};
  Piece blocking;
  while (WithinTheBoard(current)) {
    attacks_[current.file][current.rank] += attack_delta;
    blocking = GetSquare(current);
    if (blocking != pieces::kNone) {
      if (blocking.type == PieceType::kQueen || blocking.type == attacker) {
        if (blocking.player == Player::kWhite) {
          second_wave_attacks = basic_white_attacks;
        } else if (blocking.player == Player::kBlack) {
          second_wave_attacks = basic_black_attacks;
        } else {
          assert(false);  // Invalid player
        }
      }

      // Update check segment if needed
      if (blocking.type == PieceType::kKing) {
        if (blocking.player == Player::kWhite) {
          if (attack_delta.by_black == 1) {
            check_segment_ = {origin, current};
            ghost_delta = basic_black_attacks;
          }
          if (attack_delta.by_black == -1) {
            if (second_wave) {
              SetPin(square, Player::kWhite, delta, true);
            }
            ghost_delta = -basic_black_attacks;
          }
        } else if (blocking.player == Player::kBlack){
          if (attack_delta.by_white == 1) {
            check_segment_ = {origin, current};
            ghost_delta = basic_white_attacks;
          }
          if (attack_delta.by_white == -1) {
            if (second_wave) {
              SetPin(square, Player::kBlack, delta, true);
            }
            ghost_delta = -basic_white_attacks;
          }
        } else {
          assert(false);  // Invalid player
        }
        if (blocking.player == Player::kWhite) {
          if (pin_delta.by_black == 1) {
            SetPin(origin, Player::kWhite, delta, true);
          }
          if (pin_delta.by_black == -1) {
            SetPin(origin, Player::kWhite, delta, false);
          }
        } else if (blocking.player == Player::kBlack) {
          if (pin_delta.by_white == 1) {
            SetPin(origin, Player::kBlack, delta, true);
          }
          if (pin_delta.by_white == -1) {
            SetPin(origin, Player::kBlack, delta, false);
          }
        } else {
          assert(false);  // Invalid player
        }
      }
      break;
    }
    current += delta;
  }
  Coordinates block = current;
  current += delta;
  while (WithinTheBoard(current)) {
    attacks_[current.file][current.rank] += ghost_delta;
    Piece current_piece = GetSquare(current);
    if (current_piece != pieces::kNone) {
      if (current_piece.type == PieceType::kKing) {
        if (current_piece.player == Player::kWhite) {
          if (attack_delta.by_black == 1 && blocking.player == Player::kWhite) {
            SetPin(block, Player::kWhite, delta, true);
          }
          if (attack_delta.by_black == -1 && blocking.player == Player::kWhite) {
            SetPin(block, Player::kWhite, delta, false);
          }
        } else if (current_piece.player == Player::kBlack) {
          if (attack_delta.by_white == 1 && blocking.player == Player::kBlack) {
            SetPin(block, Player::kBlack, delta, true);
          }
          if (attack_delta.by_white == -1 && blocking.player == Player::kBlack) {
            SetPin(block, Player::kBlack, delta, false);
          }
        } else {
          assert(false);  // Invalid player
        }
      }
      if (current_piece.type == PieceType::kQueen || current_piece.type == attacker) {
        if (current_piece.player == Player::kWhite) {
          pin_delta = basic_white_attacks;
        } else if (current_piece.player == Player::kBlack) {
          pin_delta = basic_black_attacks;
        } else {
          assert(false);  // Invalid player
        }
      }
      break;
    }
    current += delta;
  }
  second_wave_attacks *= block_delta;
  pin_delta *= block_delta;
  if (second_wave_attacks != Attacks{0,0} || pin_delta != Attacks{0,0}) {
    AttackDirection(square, moving_piece, block, -delta, second_wave_attacks, attacker, 0, true, pin_delta);
  }
}

void Position::PinDirection(Coordinates square, Player player, Coordinates delta, PieceType attacker) {
  Coordinates current = square;
  current += delta;
  Attacks second_wave_attacks = {0, 0};
  Piece blocking;
  while (WithinTheBoard(current)) {
    blocking = GetSquare(current);
    if (blocking != pieces::kNone) {
      if (blocking.type == PieceType::kQueen || blocking.type == attacker) {
        if (blocking.player != player) {
          check_segment_ = {square, current};
        }
      }
      break;
    }
    current += delta;
  }
  Coordinates block = current;
  current += delta;
  while (WithinTheBoard(current)) {
    Piece current_piece = GetSquare(current);
    if (current_piece != pieces::kNone) {
      if (current_piece.player != player && (current_piece.type == PieceType::kQueen || current_piece.type == attacker) ) {
        SetPin(block, player, delta, true);
      }
      break;
    }
    current += delta;
  }
}

Position::Pins Position::GetPins(Coordinates square, Player player) const {
  if (player == Player::kWhite) {
    return pins_on_white_[square.file][square.rank];
  } else if (player == Player::kBlack) {
    return pins_on_black_[square.file][square.rank];
  } else {
    assert(false);  // Invalid player
  }
  return {};
}

void Position::SetPin(Coordinates square, Player player, Coordinates delta, bool value) {
  Pins& pins = (player == Player::kWhite)? 
               pins_on_white_[square.file][square.rank]:
               pins_on_black_[square.file][square.rank];
  if (delta == Coordinates{0,1} || delta == Coordinates{0,-1}) {
    pins.vertical += (value ? 1 : -1);
  } else if (delta == Coordinates{1,0} || delta == Coordinates{-1,0}) {
    pins.horisontal += (value ? 1 : -1);
  } else if (delta == Coordinates{1,1} || delta == Coordinates{-1,-1}) {
    pins.upward += (value ? 1 : -1);
  } else if (delta == Coordinates{1,-1} || delta == Coordinates{-1,1}) {
    pins.downward += (value ? 1 : -1);
  }
};

void Position::PushLegalMove(Move move) const {
  if (move.piece.type == PieceType::kKing) {
    if (move.piece.player == Player::kWhite) {
      if (!GetAttacks(move.to).by_black) {
        legal_moves_.push_back(move);
      }
    } else if (move.piece.player == Player::kBlack) {
      if (!GetAttacks(move.to).by_white) {
        legal_moves_.push_back(move);
      }
    } else {
      assert(false);  // Ivalid player
    }
  } else {
    if (!GetChecks(to_move_) || BelongsToSegment(check_segment_, move.to)) {
      legal_moves_.push_back(move);
    }
  }
}

Position::Attacks& Position::Attacks::operator+=(Attacks other) {
  by_white += other.by_white;
  by_black += other.by_black;
  return *this;
}

Position::Attacks& Position::Attacks::operator*=(int8_t mult) {
  by_white *= mult;
  by_black *= mult;
  return *this;
}

Position::Attacks Position::Attacks::operator-(){
  return {static_cast<int8_t>(-by_white), static_cast<int8_t>(-by_black)};
}

bool Position::Attacks::operator==(Attacks other) const {
  return by_white == other.by_white && by_black == other.by_black;
}

bool Position::Attacks::operator!=(Attacks other) const {
  return !(*this==other);
}

Position::AttackInfo& Position::AttackInfo::operator+=(AttackInfo other) {
  up += other.up;
  up_right += other.up_right;
  right += other.right;
  down_right += other.down_right;
  down += other.down;
  down_left += other.down_left;
  left += other.left;
  up_left += other.up_left;
  return *this;
}

Position::AttackInfo Position::AttackInfo::operator-() {
  return {
    -up, -up_right, -right, -down_right, -down, -down_left, -left, -up_left
  };
}

}  // namespace chess_engine