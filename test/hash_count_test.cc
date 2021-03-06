#include <catch2/catch_all.hpp>

#include "src/chess_defines.h"
#include "src/count_moves.h"
#include "src/fen.h"
#include "src/position.h"
#include "src/zobrist_hash.h"

TEST_CASE(
  "Move count with hash is correct in tricky positions",
  "[position][hash]"
) {
  chess_engine::ZobristHashFunction func(14159265358979323846ull);
  {
    chess_engine::Position pos = chess_engine::FenToPosition(
      "rnbqkb1r/pppp1ppp/8/4p3/8/5N2/PPPPBnPP/RNBQ1RK1 w kq - 8 5"
    );
    pos.MakeMove(chess_engine::UciToMove("g1f2"));
    pos.MakeMove(chess_engine::UciToMove("d8h4"));
    pos.MakeMove(chess_engine::UciToMove("f3h4"));
    pos.MakeMove(chess_engine::UciToMove("a7a5"));
    REQUIRE(chess_engine::CountMovesWithHash(pos, 5, func) == 23946398);
  }
  {
    chess_engine::Position pos = chess_engine::FenToPosition(
      "rnbqkb1r/pppp1ppp/8/4p3/8/5N2/PPPPBnPP/RNBQ1RK1 w kq - 8 5"
    );
    pos.MakeMove(chess_engine::UciToMove("g1f2"));
    pos.MakeMove(chess_engine::UciToMove("d8h4"));
    pos.MakeMove(chess_engine::UciToMove("f3h4"));
    pos.MakeMove(chess_engine::UciToMove("a7a5"));
    pos.MakeMove(chess_engine::UciToMove("h4g6"));
    pos.MakeMove(chess_engine::UciToMove("f8a3"));
    pos.MakeMove(chess_engine::UciToMove("g6h8"));
    REQUIRE(chess_engine::CountMovesWithHash(pos, 5, func) == 17675852);
  }
  {
    chess_engine::Position pos = chess_engine::FenToPosition(
      "5r1k/p5pp/8/2pQ4/3p1p2/3P3q/PrPN1PR1/4RK2 b - - 1 27"
    );
    REQUIRE(chess_engine::CountMovesWithHash(pos, 5, func) == 87680580);
  }
  {
    chess_engine::Position pos = chess_engine::FenToPosition(
      "8/5p1p/5P2/1K1p2p1/1Ppk2P1/P7/2P3P1/8 w - - 1 32"
    );
    REQUIRE(chess_engine::CountMovesWithHash(pos, 5, func) == 29991);
  }
  {
    chess_engine::Position pos = chess_engine::FenToPosition(
      "5rk1/5p1p/5R2/p2pp3/q7/1rP5/1P4PP/1R1Q3K w - - 0 25"
    );
    REQUIRE(chess_engine::CountMovesWithHash(pos, 5, func) == 43033472);
  }
  {
    chess_engine::Position pos = chess_engine::FenToPosition(
      "r4rk1/1ppb1pp1/1p1p2np/4p3/1PR5/P1qB4/2P2PPQ/2B2RK1 b - - 5 21"
    );
    REQUIRE(chess_engine::CountMovesWithHash(pos, 5, func) == 120413132);
  }
}
