#include "engine.h"

#include <iostream>

#include "fen.h"

namespace chess_engine {
  Engine::Engine(const Position& position, const ZobristHashFunction hash_func):
    root_(position, hash_func)
  {}

  int32_t Engine::GetEvaluation(int16_t min_depth) {
    if (root_info_.depth < min_depth) {
      root_info_ = RunSearch(min_depth, root_);
    }
    return root_info_.eval;
  }

  Move Engine::GetBestMove(int16_t min_depth) {
    if (root_info_.depth < min_depth) {
      root_info_ = RunSearch(min_depth, root_);
    }
    return root_info_.best_move;
  }

  void Engine::MakeMove(Move move) {
    root_.MakeMove(move);
    root_info_ = transposition_table.Get(root_.GetHash().Get());
  }

  const Position& Engine::GetPosition() const {
    return root_.GetPosition();
  }

  int32_t Engine::SimpleEvaluate(const Node& node) {
    int32_t ret = 0;
    Player to_move = node.PlayerToMove();
    Coordinates king = node.GetKing(to_move);
    Coordinates opponents_king = node.GetKing(Opponent(to_move));
    for (int8_t file = 0; file < 8; ++file) {
      for (int8_t rank = 0; rank < 8; ++rank) {
        ret += node.GetAttacksByPlayer({file, rank}, to_move);
        ret -= node.GetAttacksByPlayer({file, rank}, Opponent(to_move));
      }
    }
    return ret;
  }

  Engine::NodeInfo Engine::RunSearch(int16_t depth, const Node& node, int32_t alpha, int32_t beta) {
    if (node.IsCheckmate()) {
      return {depth, lowest_eval, {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (node.IsStalemate()) {
      return {depth, 0, {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (depth < 10) {
      transposition_table.Set(
        node.GetHash().Get(),
        {0, SimpleEvaluate(node), {{-1,-1}, {-1,-1}, pieces::kNone}}
      );
      return {0, SimpleEvaluate(node), {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    std::vector<Move> legal_moves = node.GetLegalMoves();
    int32_t eval = lowest_eval;
    Move best_move = legal_moves[0];
    for (Move move:legal_moves) {
      NodeInfo child = transposition_table.Get(node.HashAfterMove(move).Get());
      if (child.depth < depth-10) {
        Node new_node = node;
        new_node.MakeMove(move);
        int16_t cost = 10;
        if (node.IsCheck()) {
          cost = 2;
        } else if (new_node.IsCheck()) {
          cost = 2;
        } else if (node.GetSquare(move.to) != pieces::kNone) {
          cost = 5;
        }
        child = RunSearch(depth-cost, new_node, -beta, -alpha);
      }
      if (-child.eval > eval) {
        eval = -child.eval;
        best_move = move;
        if (eval > beta) {
          // TODO(Andrey): Transposition table?
          return {depth, eval, best_move};
        }
        if (eval > alpha) {
          alpha = eval;
        }
      }
    }
    transposition_table.Set(node.GetHash().Get(), {depth, eval, best_move});
    if (eval < lowest_eval + longest_checkmate) {
      ++eval;
    }
    return {depth, eval, best_move};
  }
}  // namespace chess_engine