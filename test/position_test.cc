#include <catch2/catch_all.hpp>

#include "chess_defines.h"
#include "position.h"

chess_engine::Position StartingPosition() {
  chess_engine::Position ret;

  ret.SetSquare({0,0}, chess_engine::pieces::kWhiteRook);
  ret.SetSquare({1,0}, chess_engine::pieces::kWhiteKnight);
  ret.SetSquare({2,0}, chess_engine::pieces::kWhiteBishop);
  ret.SetSquare({3,0}, chess_engine::pieces::kWhiteQueen);
  ret.SetSquare({4,0}, chess_engine::pieces::kWhiteKing);
  ret.SetSquare({5,0}, chess_engine::pieces::kWhiteBishop);
  ret.SetSquare({6,0}, chess_engine::pieces::kWhiteKnight);
  ret.SetSquare({7,0}, chess_engine::pieces::kWhiteRook);

  for (int i = 0;  i < 8; ++i) {
    ret.SetSquare({i,1}, chess_engine::pieces::kWhitePawn);
  }

  for (int i = 0;  i < 8; ++i) {
    ret.SetSquare({i,6}, chess_engine::pieces::kBlackPawn);
  }

  ret.SetSquare({0,7}, chess_engine::pieces::kBlackRook);
  ret.SetSquare({1,7}, chess_engine::pieces::kBlackKnight);
  ret.SetSquare({2,7}, chess_engine::pieces::kBlackBishop);
  ret.SetSquare({3,7}, chess_engine::pieces::kBlackQueen);
  ret.SetSquare({4,7}, chess_engine::pieces::kBlackKing);
  ret.SetSquare({5,7}, chess_engine::pieces::kBlackBishop);
  ret.SetSquare({6,7}, chess_engine::pieces::kBlackKnight);
  ret.SetSquare({7,7}, chess_engine::pieces::kBlackRook);

  return ret;
}
  

TEST_CASE("Make some moves", "[position]") {
  chess_engine::Position pos = StartingPosition();

  REQUIRE(pos.GetSquare({4,1}) == chess_engine::pieces::kWhitePawn});
  REQUIRE(pos.GetSquare(4,3) == chess_engine::pieces::kNone);
  pos.MakeMove({{4,1}, {4,3}, chess_engine::pieces::kWhitePawn});
  REQUIRE(pos.GetSquare(4,1) == chess_engine::pieces::kNone);
  REQUIRE(pos.GetSquare(4,3) == chess_engine::pieces::kWhitePawn);

  REQUIRE(pos.GetSquare(6,7) == chess_engine::pieces::kBlackKnight);
  REQUIRE(pos.GetSquare(5,5) == chess_engine::pieces::kNone);
  pos.MakeMove({{6,7}, {5,5}, chess_engine::pieces::kBlackKnight});
  REQUIRE(pos.GetSquare(6,7) == chess_engine::pieces::kNone);
  REQUIRE(pos.GetSquare(5,5) == chess_engine::pieces::kBlackKnight);
}

TEST_CASE("Count moves", "[position]") {
  chess_engine::Position pos = StartingPosition();
  REQUIRE(pos.GetLegalMoves.size() == 40u);
}