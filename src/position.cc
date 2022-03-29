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
  moves_generated_ = false;
}

 void Position::PassTheTurn() {
   if (to_move_ == Player::kWhite) {
     to_move_ = Player::kBlack;
   } else {
     ++move_number_;
     to_move_ = Player::kWhite;
   }
   UpdateCheckSegment();
 }

void Position::MakeMove(Move move) {
  check_segment_ = {{-1,-1},{-1,-1}};  // If move is legal it deals with all checks
  if (GetSquare(move.from).type == PieceType::kPawn || GetSquare(move.to) != pieces::kNone) {
    halfmove_clock_ = 0;
  } else {
    ++halfmove_clock_;
  }

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
  
  // Castling
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
    Coordinates new_en_pessant = move.from;
    new_en_pessant.rank += dir;
    SetEnPessant(new_en_pessant);
  } else {
    SetEnPessant({-1,-1});
  }

  // Update castling rights
  if (move.from == Coordinates{0,0} || move.to == Coordinates{0,0}) {
    SetCastlingRights(Player::kWhite, Castle::kQueenside, false);
  }
  if (move.from == Coordinates{7,0} || move.to == Coordinates{7,0}) {
    SetCastlingRights(Player::kWhite, Castle::kKingside, false);
  }
  if (move.from == Coordinates{0,7} || move.to == Coordinates{0,7}) {
    SetCastlingRights(Player::kBlack, Castle::kQueenside, false);
  }
  if (move.from == Coordinates{7,7} || move.to == Coordinates{7,7}) {
    SetCastlingRights(Player::kBlack, Castle::kKingside, false);
  }


  if (move.piece == pieces::kWhiteKing) {
    SetCastlingRights(Player::kWhite, Castle::kQueenside, false);
    SetCastlingRights(Player::kWhite, Castle::kKingside, false);
  }
  if (move.piece == pieces::kBlackKing) {
    SetCastlingRights(Player::kBlack, Castle::kQueenside, false);
    SetCastlingRights(Player::kBlack, Castle::kKingside, false);
  }

  PassTheTurn();
}

std::vector<Move> Position::GetLegalMoves() const {
  if (!moves_generated_) {
    legal_moves_.clear();
    GenerateMoves();
    moves_generated_ = true;
  }
  return legal_moves_;
}

bool Position::MoveIsLegal(Move move) const {
  // TODO(Andrey): Implement
}

bool Position::MoveIsCheckFast(Move move) const {
  Pins pins = GetPins(move.from, Opponent(to_move_));
  bool pin = pins.vertical || pins.upward || pins.horisontal || pins.downward;
  AttackInfo check_info = checking_squares_[move.to.file][move.to.rank];

  int8_t Attacks::* by_king = to_move_==Player::kWhite ? &Attacks::by_black : & Attacks::by_white; 
  bool rook_check = 
  check_info.up.*by_king || check_info.right.*by_king ||
  check_info.down.*by_king || check_info.left.*by_king;
  bool bishop_check = 
  check_info.up_right.*by_king || check_info.down_right.*by_king ||
  check_info.down_left.*by_king || check_info.up_left.*by_king;
  Coordinates king = GetKing(Opponent(to_move_));

  // TODO(Andrey): King discoveries!
  int8_t dir = PawnDirection(to_move_);
  switch (move.piece.type) {
    case (PieceType::kPawn): 
      if (move.to + Coordinates{1,dir} == king || move.to + Coordinates{-1,dir} == king) {
        return true;
      }
      if (move.to == move.from + Coordinates{0, dir} && (pins.upward || pins.horisontal || pins.downward)) {
        return true;
      }
      return false;
      // We assume promotions always happen on the back rank
    break;
    case (PieceType::kRook):
      return pin || rook_check;
    break;
    case (PieceType::kKnight):
      return pin || KnightMoveAway(move.to, king);
    break;
    case (PieceType::kBishop):
      return pin || bishop_check;
    break;
    case (PieceType::kQueen):
      return pin || rook_check || bishop_check;
    break;
  }
  return false;
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

  int8_t blocked = 0;
  int8_t blocked_for_white = 0;
  int8_t blocked_for_black = 0;
  int8_t white_king_factor = 0;
  int8_t black_king_factor = 0;

  if (old_piece != pieces::kNone) {
    ++blocked;
    if (old_piece.type == PieceType::kKing) {
      if (old_piece.player == Player::kWhite) {
        ++blocked_for_white;
        --white_king_factor;
      } else if (old_piece.player == Player::kBlack) {
        ++blocked_for_black;
        --black_king_factor;
      } else {
        assert(false);  // Invalid player
      }
    } else {
      ++blocked_for_white;
      ++blocked_for_black;
    }
  }

  if (piece != pieces::kNone) {
    --blocked;
    if (piece.type == PieceType::kKing) {
      if (piece.player == Player::kWhite) {
        --blocked_for_white;
        ++white_king_factor;
        white_king_ = square;
      } else if (piece.player == Player::kBlack) {
        --blocked_for_black;
        ++black_king_factor;
        black_king_ = square;
      } else {
        assert(false);  // Invalid player
      }
    } else {
      --blocked_for_white;
      --blocked_for_black;
    }
  }
  
  AttackInfo king_attacks = {{1,1},{1,1},{1,1},{1,1},{1,1},{1,1},{1,1},{1,1}};
  AttackInfo directed_attacks = directed_attacks_[square.file][square.rank];
  AttackInfo checking_squares = checking_squares_[square.file][square.rank]; 
  
  directed_attacks.MultiplyPlayerAttacks(Player::kWhite, blocked_for_white);
  directed_attacks.MultiplyPlayerAttacks(Player::kBlack, blocked_for_black);
  directed_attacks += delayed_attacks;

  king_attacks.MultiplyPlayerAttacks(Player::kWhite, white_king_factor);
  king_attacks.MultiplyPlayerAttacks(Player::kBlack, black_king_factor);
  checking_squares *= blocked;
  checking_squares += king_attacks;

  UpdateStraightAttacks(square, directed_attacks, checking_squares);

  board_[square.file][square.rank] = piece;

  moves_generated_ = false;
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
  moves_generated_ = false;
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
    moves_generated_ = false;
  }

Coordinates Position::GetEnPessant() const {
  return en_pessant_;
}

void Position::SetEnPessant(Coordinates square) {
  en_pessant_ = square;
  moves_generated_ = false;
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
  AttackInfo attack_delta,
  AttackInfo checking_square_delta
) {
  AttackDirection(square, {0,1}, attack_delta.up, checking_square_delta.up);
  AttackDirection(square, {1,0}, attack_delta.right, checking_square_delta.right);
  AttackDirection(square, {0,-1}, attack_delta.down, checking_square_delta.down);
  AttackDirection(square, {-1,0}, attack_delta.left, checking_square_delta.left);

  AttackDirection(square, {1,1}, attack_delta.up_right, checking_square_delta.up_right);
  AttackDirection(square, {1,-1}, attack_delta.down_right, checking_square_delta.down_right);
  AttackDirection(square, {-1,-1}, attack_delta.down_left, checking_square_delta.down_left);
  AttackDirection(square, {-1,1}, attack_delta.up_left, checking_square_delta.up_left);
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

void Position::AttackDirection(
  Coordinates square,
  Coordinates delta,
  Attacks attack_delta,
  Attacks checking_square_delta
) {
  AttackInfo directed_delta = {};
  directed_delta.SetByDelta(delta, attack_delta);
  AttackInfo directed_king_delta = {};
  directed_king_delta.SetByDelta(delta, checking_square_delta);

  Coordinates current = square;
  current += delta;
  while (WithinTheBoard(current)) {
    attacks_[current.file][current.rank] += attack_delta;
    directed_attacks_[current.file][current.rank] += directed_delta;
    checking_squares_[current.file][current.rank] += directed_king_delta;
    Piece current_piece = GetSquare(current);
    if (current_piece != pieces::kNone) {
      if (current_piece != pieces::kBlackKing) {
        attack_delta.by_white = 0;
        directed_delta.MultiplyPlayerAttacks(Player::kWhite, 0);
      }
      if (current_piece != pieces::kWhiteKing) {
        attack_delta.by_black = 0;
        directed_delta.MultiplyPlayerAttacks(Player::kBlack, 0);
      }
      directed_king_delta = {};  // set all members to 0
    }
    current += delta;
  }
}

void Position::UpdateCheckSegment(){
  int8_t checks = GetChecks(to_move_);
  if (checks != 1) {
    return;  // no checks - no need to worry, double check - run
  }
  Coordinates king = GetKing(to_move_);
  Coordinates current = king;
  Coordinates delta;
  AttackInfo attacks_on_king = directed_attacks_[current.file][current.rank];
  // Member pointer to a member, idndicating opponnts atacks
  int8_t Attacks::*by_opponent = to_move_ == Player::kWhite ? &Attacks::by_black : &Attacks::by_white;

  if (attacks_on_king.up.*by_opponent > 0) {
    delta = {0, -1};
  } else if (attacks_on_king.up_right.*by_opponent > 0) {
    delta = {-1, -1};
  } else if (attacks_on_king.right.*by_opponent > 0) {
    delta = {-1, 0};
  } else if (attacks_on_king.down_right.*by_opponent > 0) {
    delta = {-1, 1};
  } else if (attacks_on_king.down.*by_opponent > 0) {
    delta = {0, 1};
  } else if (attacks_on_king.down_left.*by_opponent > 0) {
    delta = {1, 1};
  } else if (attacks_on_king.left.*by_opponent > 0) {
    delta = {1, 0};
  } else if (attacks_on_king.up_left.*by_opponent > 0) {
    delta = {1, -1};
  } else {
    return;  // Must've been checked by a knight or a pawn
  }
  current += delta;
  Piece current_piece = GetSquare(current);
  while (current_piece == pieces::kNone) {
    current += delta;
    current_piece = GetSquare(current);
  }
  check_segment_ = {king, current};
}

Position::Pins Position::GetPins(Coordinates square, Player player) const {
  Pins ret;
  AttackInfo attacks = directed_attacks_[square.file][square.rank];
  AttackInfo king_attacks = checking_squares_[square.file][square.rank];

  // Member pointers to avoid ifs and code duplication
  int8_t Attacks::*by_player = (player == Player::kWhite ? &Attacks::by_white : &Attacks::by_black);
  int8_t Attacks::*by_opponent = (player == Player::kWhite ? &Attacks::by_black : &Attacks::by_white);

  ret.vertical = (attacks.up.*by_opponent > 0 && king_attacks.down.*by_player > 0) ||
                 (attacks.down.*by_opponent > 0 && king_attacks.up.*by_player > 0);
  ret.upward = (attacks.up_right.*by_opponent > 0 && king_attacks.down_left.*by_player > 0) ||
               (attacks.down_left.*by_opponent > 0 && king_attacks.up_right.*by_player > 0);
  ret.horisontal = (attacks.right.*by_opponent > 0 && king_attacks.left.*by_player > 0) ||
                   (attacks.left.*by_opponent > 0 && king_attacks.right.*by_player > 0);
  ret.downward = (attacks.down_right.*by_opponent > 0 && king_attacks.up_left.*by_player > 0) ||
                 (attacks.up_left.*by_opponent > 0 && king_attacks.down_right.*by_player > 0);
  return ret; 
}

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

void Position::AttackInfo::SetByDelta(Coordinates delta, Attacks value) {
  if (delta == Coordinates{0,1}) {
    up = value;
  } else if (delta == Coordinates{1,1}) {
    up_right = value;
  } else if (delta == Coordinates{1,0}) {
    right = value;
  } else if (delta == Coordinates{1,-1}) {
    down_right = value;
  } else if (delta == Coordinates{0,-1}) {
    down = value;
  } else if (delta == Coordinates{-1,-1}) {
    down_left = value;
  } else if (delta == Coordinates{-1, 0}) {
    left = value;
  } else if (delta == Coordinates{-1,1}) {
    up_left = value;
  }
}

void Position::AttackInfo::MultiplyPlayerAttacks(Player player, int8_t factor) {
  // Memeber pointer to avoid ifs and code duplication
  int8_t Attacks::*by_player = player == Player::kWhite ? &Attacks::by_white : &Attacks::by_black;
  up.*by_player *= factor;
  up_right.*by_player *= factor;
  right.*by_player *= factor;
  down_right.*by_player *= factor;
  down.*by_player *= factor;
  down_left.*by_player *= factor;
  left.*by_player *= factor;
  up_left.*by_player *= factor;
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

Position::AttackInfo& Position::AttackInfo::operator*=(int8_t other) {
  up *= other;
  up_right *= other;
  right *= other;
  down_right *= other;
  down *= other;
  down_left *= other;
  left *= other;
  up_left *= other;
  return *this;
}

Position::AttackInfo Position::AttackInfo::operator-() {
  return {
    -up, -up_right, -right, -down_right, -down, -down_left, -left, -up_left
  };
}

}  // namespace chess_engine