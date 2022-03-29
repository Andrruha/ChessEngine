#include "engine.h"

#include <algorithm>

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
    no_return_table_.Set(root_.GetHash().Get(), true);
    root_.MakeMove(move);
    root_info_ = transposition_table_.Get(root_.GetHash().Get());
  }

  const Position& Engine::GetPosition() const {
    return root_.GetPosition();
  }

  int32_t Engine::SimpleEvaluate(const Node& node) {
    int32_t ret = 0;
    Player to_move = node.PlayerToMove();
    Player opponent = Opponent(to_move);
    Coordinates king = node.GetKing(to_move);
    Coordinates opponents_king = node.GetKing(Opponent(to_move));
    int32_t king_freedom = 0;
    int32_t king_denominator = 0;
    int32_t opponents_king_freedom = 0;
    int32_t opponents_king_denominator = 0;
    for (int8_t file = 0; file < 8; ++file) {
      for (int8_t rank = 0; rank < 8; ++rank) {
        if (DistanceSquared(king, {file, rank})<=2) {
          if (!node.GetAttacksByPlayer({file, rank}, opponent)) {
            king_freedom += 100;
          }
          ++king_denominator;
        }
        if (DistanceSquared(opponents_king, {file, rank})<=2) {
          if (!node.GetAttacksByPlayer({file, rank}, to_move)) {
            opponents_king_freedom += 100;
          }
          ++opponents_king_denominator;
        }
        ret += node.GetAttacksByPlayer({file,rank}, to_move);
        ret -= node.GetAttacksByPlayer({file, rank}, opponent);
        Piece piece = node.GetSquare({file, rank});
        if (piece != pieces::kNone) {
          ret += piece.player==to_move? 
            piece_values[static_cast<int>(piece.type)-1]: 
            -piece_values[static_cast<int>(piece.type)-1];
        }
      }
    }
    king_freedom /= king_denominator;
    ret += king_freedom;
    opponents_king_freedom /= opponents_king_denominator;
    ret -= opponents_king_freedom;
    return ret;
  }

  int32_t Engine::GetLowestEval() {
    return lowest_eval_;
  }
  int32_t Engine::GetHighestEval() {
    return highest_eval_;
  }
  int32_t Engine::GetLongestCheckmate() {
    return longest_checkmate_;
  }

  void Engine::SortMoves(std::vector<Move>& moves, const Position& position) {
    int insert_index = 0;
    for (int read_index = 0; read_index < static_cast<int>(moves.size()); ++read_index) {
      if (position.MoveIsCheckFast(moves[read_index])) {
        std::swap(moves[insert_index], moves[read_index]);
        ++insert_index;
      }
    }
    for (int read_index = insert_index; read_index < static_cast<int>(moves.size()); ++read_index) {
      if (position.GetSquare(moves[read_index].to) != pieces::kNone) {
        std::swap(moves[insert_index], moves[read_index]);
        ++insert_index;
      }
    }
  }

  // TODO(Andrey): proper hashing
  // TODO(Andrey): rewrite depth
  Engine::NodeInfo Engine::RunSearch(int16_t depth, const Node& node, int32_t alpha, int32_t beta) {
    if (node.IsCheckmate()) {
      return {depth, lowest_eval_, {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (node.IsStalemate()) {
      return {depth, 0, {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (!depth) {
      return {0, SimpleEvaluate(node), {{-1,-1}, {-1,-1}, pieces::kNone}};
    }

    no_return_table_.Set(node.GetHash().Get(), true);

    std::vector<Move> legal_moves = node.GetLegalMoves();
    SortMoves(legal_moves, node.GetPosition());
    int32_t eval = lowest_eval_;
    Move best_move = legal_moves[0];
    for (Move move:legal_moves) {
      ZobristHash new_hash = node.HashAfterMove(move);
      if (no_return_table_.Get(new_hash.Get())) {
        if (0 > eval) {
          eval = 0;
          best_move = move;
          if (eval > beta) {
            no_return_table_.Set(node.GetHash().Get(), false);
            return {depth, eval, best_move};
          }
          if (eval > alpha) {
            alpha = eval;
          }
        }
        continue;
      }
      NodeInfo child = transposition_table_.Get(new_hash.Get());
      if (child.depth < depth-1) {
        Node new_node = node;
        new_node.MakeMove(move);

        // TODO(Andrey): Simple evaluation after move?
        child = RunSearch(depth-1, new_node, -beta, -alpha);
      }
      if (-child.eval > eval) {
        eval = -child.eval;
        best_move = move;
        if (eval >= beta) {
          // Node is a cut node
          no_return_table_.Set(node.GetHash().Get(), false);
          if (eval > highest_eval_ - longest_checkmate_) {
            --eval;
          }
          return {depth, eval, best_move};
        }
        if (eval > alpha) {
          // Node is a PV node
          alpha = eval;
        }
      }
    }
    if (eval > highest_eval_ - longest_checkmate_) {
      --eval;
    }
    no_return_table_.Set(node.GetHash().Get(), false);
    return {depth, eval, best_move};
  }
}  // namespace chess_engine