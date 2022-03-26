#include "node.h"

#include "chess_defines.h"
#include "position.h"
#include "zobrist_hash.h"

namespace chess_engine {

bool Node::IsCheck() const {
  return position_.IsCheck();
}

bool Node::IsCheckmate() const {
  return position_.IsCheckmate();
}

bool Node::IsStalemate() const {
  return position_.IsStalemate();
}

Player Node::PlayerToMove() const {
  return position_.PlayerToMove();
}

void Node::SetPlayerToMove(Player player) {
  if (player != position_.PlayerToMove()) {
    hash_.PassTheTurn();
  }
  position_.SetPlayerToMove(player);
}

void Node::PassTheTurn() {
  position_.PassTheTurn();
  hash_.PassTheTurn();
}

std::vector<Move> Node::GetLegalMoves() const {
  return position_.GetLegalMoves();
}

void Node::MakeMove(Move move) {
  HashMove(hash_, move);
  position_.MakeMove(move);
}

void Node::HashMove(ZobristHash& hash, Move move) const {
  Piece old_piece = position_.GetSquare(move.from);

  int8_t dir = PawnDirection(position_.PlayerToMove());
  Coordinates en_pessant = position_.GetEnPessant();
  Player to_move = position_.PlayerToMove();

  bool normal_move = true;

  // En pessant
  if (move.to == en_pessant && move.piece.type == PieceType::kPawn) {
    Coordinates taken = move.to;
    taken.rank -= dir;
    hash.ToggleSquare(taken, {PieceType::kPawn, Opponent(to_move)});
    hash.ToggleSquare(move.from, {PieceType::kPawn, to_move});
    hash.ToggleSquare(move.to, {PieceType::kPawn, to_move});
    normal_move = false;
  }
  
  // Castling
  if (move.piece.type == PieceType::kKing) {
    if (move.to == move.from + Coordinates{2,0}) {
      hash.ToggleSquare(move.from, {PieceType::kKing, to_move});
      hash.ToggleSquare(move.from + Coordinates{3,0}, {PieceType::kRook, to_move});
      hash.ToggleSquare(move.to, {PieceType::kKing, to_move});
      hash.ToggleSquare(move.to + Coordinates{-1,0}, {PieceType::kRook, to_move});
    }
    if (move.to == move.from + Coordinates{-2,0}) {
      hash.ToggleSquare(move.from, {PieceType::kKing, to_move});
      hash.ToggleSquare(move.from + Coordinates{-4,0}, {PieceType::kRook, to_move});
      hash.ToggleSquare(move.to, {PieceType::kKing, to_move});
      hash.ToggleSquare(move.to + Coordinates{1,0}, {PieceType::kRook, to_move});
    }
    normal_move = false;
  }

  Piece taken = position_.GetSquare(move.to);

  hash.ToggleSquare(move.from, move.piece);
  hash.ToggleSquare(move.to, taken);
  hash.ToggleSquare(move.to, move.piece);

  // Update en-pessant
  hash.ToggleEnPessant(en_pessant);

  if (
    move.piece.type == PieceType::kPawn && 
    move.to.rank - move.from.rank == dir*2
  ) {
    Coordinates new_en_pessant = move.from;
    new_en_pessant.rank += dir;
    hash.ToggleEnPessant(new_en_pessant);
  }

  // Update castling rights
  if (move.from == Coordinates{0,0} || move.to == Coordinates{0,0}) {
    if (position_.GetCastlingRights(Player::kWhite, Castle::kQueenside)) {
      hash.ToggleCastlingRights(Player::kWhite, Castle::kQueenside);
    }
  }
  if (move.from == Coordinates{7,0} || move.to == Coordinates{7,0}) {
    if (position_.GetCastlingRights(Player::kWhite, Castle::kKingside)) {
      hash.ToggleCastlingRights(Player::kWhite, Castle::kKingside);
    }
  }
  if (move.from == Coordinates{0,7} || move.to == Coordinates{0,7}) {
    if (position_.GetCastlingRights(Player::kBlack, Castle::kQueenside)) {
      hash.ToggleCastlingRights(Player::kBlack, Castle::kQueenside);
    }
  }
  if (move.from == Coordinates{7,7} || move.to == Coordinates{7,7}) {
    if (position_.GetCastlingRights(Player::kBlack, Castle::kKingside)) {
      hash.ToggleCastlingRights(Player::kBlack, Castle::kKingside);
    }
  }

  if (move.piece == pieces::kWhiteKing) {
    if (position_.GetCastlingRights(Player::kWhite, Castle::kQueenside)) {
      hash.ToggleCastlingRights(Player::kWhite, Castle::kQueenside);
    }
    if (position_.GetCastlingRights(Player::kWhite, Castle::kKingside)) {
      hash.ToggleCastlingRights(Player::kWhite, Castle::kKingside);
    }
  }

  if (move.piece == pieces::kBlackKing) {
    if (position_.GetCastlingRights(Player::kBlack, Castle::kQueenside)) {
      hash.ToggleCastlingRights(Player::kBlack, Castle::kQueenside);
    }
    if (position_.GetCastlingRights(Player::kBlack, Castle::kKingside)) {
      hash.ToggleCastlingRights(Player::kBlack, Castle::kKingside);
    }
  }

  hash.PassTheTurn();
}

ZobristHash Node::HashAfterMove(Move move) const {
  ZobristHash ret = hash_;
  HashMove(ret, move);
  return ret;
}

Piece Node::GetSquare(Coordinates square) const {
  return position_.GetSquare(square);
}

void Node::SetSquare(Coordinates square, Piece piece) {
  Piece old_piece = position_.GetSquare(square);
  hash_.ToggleSquare(square, old_piece);
  hash_.ToggleSquare(square, piece);

  position_.SetSquare(square, piece);
}

bool Node::GetCastlingRights(Player player, Castle castle) const {
  return position_.GetCastlingRights(player, castle);
}

void Node::SetCastlingRights(Player player, Castle castle, bool value) {
  if (position_.GetCastlingRights(player, castle) == value) {
    return;
  }
  hash_.ToggleCastlingRights(player, castle);  // we now know that the rights have changed
  position_.SetCastlingRights(player, castle, value);
}

Coordinates Node::GetEnPessant() const {
  return position_.GetEnPessant();
}

void Node::SetEnPessant(Coordinates square) {
  Coordinates old_square = position_.GetEnPessant();
  hash_.ToggleEnPessant(old_square);
  hash_.ToggleEnPessant(square);
  position_.SetEnPessant(square);
}

Coordinates Node::GetKing(Player player) const {
  return position_.GetKing(player);
}

int8_t Node::GetChecks(Player player) const {
  return position_.GetChecks(player);
}

int16_t Node::GetMoveNumber() const {
  return position_.GetMoveNumber();
}

void Node::SetMoveNumber(int16_t value) {
  position_.SetMoveNumber(value);
}

int16_t Node::GetHalfmoveClock() const {
  return position_.GetHalfmoveClock();
}

void Node::SetHalfmoveClock(int16_t value) {
  position_.SetHalfmoveClock(value);
}

ZobristHash Node::GetHash() const {
  return hash_;
}

}