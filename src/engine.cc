#include "engine.h"

#include <algorithm>
#include <cassert>

namespace chess_engine {
  Engine::Engine(const Position& position, const ZobristHashFunction hash_func):
    root_(position, hash_func)
  {}

  int32_t Engine::GetEvaluation(int16_t min_depth) {
    if (root_info_.depth < min_depth) {
      root_info_ = RunIncrementalSearch(min_depth);
    }
    return root_info_.eval;
  }

  Move Engine::GetBestMove(int16_t min_depth) {
    if (root_info_.depth < min_depth) {
      root_info_ = RunIncrementalSearch(min_depth);
    }
    return root_info_.best_move;
  }

  void Engine::MakeMove(Move move) {
    no_return_table_.Set(root_.GetHash(), true);
    root_.MakeMove(move);
    // root_info_ = transposition_table_.Get(root_.GetHash());
    root_info_ = NodeInfo();
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
        int8_t king_distance = DistanceSquared(king, {file, rank});
        if (king_distance<=2) {
          if (!node.GetAttacksByPlayer({file, rank}, opponent)) {
            king_freedom += 100;
          }
          ++king_denominator;
        }
        int8_t opponents_king_distance = DistanceSquared(opponents_king, {file, rank});
        if (opponents_king_distance<=2) {
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

  std::list<Move> Engine::GetPrincipalVariation() const {
    return principal_variation_;
  }

  int64_t Engine::GetNodesVisited() const {
    return nodes_visited_;
  }

  void Engine::UseTranspositionTable(bool value) {
    use_transposition_table_ = value;
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

  void Engine::SortMoves(std::vector<Move>& moves, const Node& node, int16_t depth) {
    int insert_index = 0;
    NodeInfo old_info = transposition_table_.Get(node.GetHash());
    for (int read_index = insert_index; read_index < static_cast<int>(moves.size()); ++read_index) {
      if (moves[read_index]==old_info.best_move) {
        std::swap(moves[insert_index], moves[read_index]);
        ++insert_index;
      }
    }
    for (int read_index = insert_index; read_index < static_cast<int>(moves.size()); ++read_index) {
      if (moves[read_index]==cut_moves[depth-1].first || moves[read_index]==cut_moves[depth-1].second) {
        std::swap(moves[insert_index], moves[read_index]);
        ++insert_index;
      }
    }
    for (int read_index = insert_index; read_index < static_cast<int>(moves.size()); ++read_index) {
      if (node.MoveIsCheckFast(moves[read_index])) {
        std::swap(moves[insert_index], moves[read_index]);
        ++insert_index;
      }
    }
    for (int read_index = insert_index; read_index < static_cast<int>(moves.size()); ++read_index) {
      if (node.GetSquare(moves[read_index].to) != pieces::kNone) {
        std::swap(moves[insert_index], moves[read_index]);
        ++insert_index;
      }
    }
  }

  Engine::NodeInfo Engine::RunSearch(
    int16_t depth,
    const Node& node,
    std::list<Move>& parent_variation,
    int32_t alpha,
    int32_t beta,
    int16_t ply
  ) {
    if (ply == 0) {
      nodes_visited_ = 0;
    }
    ++nodes_visited_;
    NodeInfo ret;
    if (node.IsCheckmate()) {
      ret = {depth, NodeType::kPV, lowest_eval_, {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (node.IsStalemate()) {
      ret = {depth, NodeType::kPV, 0, {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (!depth) {
      ret =  {0, NodeType::kPV, SimpleEvaluate(node), {{-1,-1}, {-1,-1}, pieces::kNone}};
    }
    if (ret.depth >= 0) {
      if (use_transposition_table_) {
        transposition_table_.Set(node.GetHash(), ret);
      }
      return ret;
    }

    no_return_table_.Set(node.GetHash(), true);

    std::vector<Move> legal_moves = node.GetLegalMoves();
    SortMoves(legal_moves, node, depth);
    Move best_move = legal_moves[0];
    int32_t eval = lowest_eval_;
    NodeType type = NodeType::kFailLow;
    std::list<Move> principal_variation;  // best line in the currently analysed child

    for (Move move:legal_moves) {
      // Search tables
      ZobristHash new_hash = node.HashAfterMove(move);
      NodeInfo child;
      if (no_return_table_.Get(new_hash.Get())) {
        child = {depth, NodeType::kPV, 0, {{-1,-1},{-1,-1}, pieces::kNone}};
      } else {
        child = transposition_table_.Get(new_hash.Get());
        if (child.depth < depth-1) {
          Node new_node = node;
          new_node.MakeMove(move);
          child = RunSearch(depth-1, new_node, principal_variation, -beta, -alpha, ply+1);
        }
      }

      // Deal with wrong bounds
      switch (child.type) {
      case NodeType::kFailLow:
        if (-child.eval < beta) {
          Node new_node = node;
          new_node.MakeMove(move);
          child = RunSearch(depth-1, new_node, principal_variation, -beta, -alpha, ply+1);
        }
        break;
      case NodeType::kFailHigh:
          if (alpha < -child.eval) {
          Node new_node = node;
          new_node.MakeMove(move);
          child = RunSearch(depth-1, new_node, principal_variation, -beta, -alpha, ply+1);
        }
        break;
      }

      // Update evaluation
      if (-child.eval > eval) {
        eval = -child.eval;
        best_move = move;
      }
      if (-child.eval > alpha) {
        type = NodeType::kPV;
        alpha = -child.eval;
        parent_variation = principal_variation;
        parent_variation.push_front(move);
      }
      if (alpha >= beta) {
        // Node is a cut node
        type = NodeType::kFailHigh;
        if (!node.MoveIsCheckFast(move) &&
          node.GetSquare(move.to) == pieces::kNone &&
          cut_moves[depth-1].first != move)
        {
          cut_moves[depth-1].second = cut_moves[depth-1].first;
          cut_moves[depth-1].first = move;
        }
        break;
      }
    }

    // Write to transposition table and return
    if (eval > highest_eval_ - longest_checkmate_) {
      --eval;
    }
    no_return_table_.Set(node.GetHash(), false);
    ret = {depth, type, eval, best_move};
    if (use_transposition_table_) {
      transposition_table_.Set(node.GetHash(), ret);
    }
    return ret;
  }

  Engine::NodeInfo Engine::RunSearch(int16_t depth) {
    cut_moves = std::vector<std::pair<Move,Move>>(
      depth, {{{-1,-1},{-1,-1},pieces::kNone}, {{-1,-1},{-1,-1},pieces::kNone}}
    );
    return RunSearch(depth, root_, principal_variation_);
  }

  Engine::NodeInfo Engine::RunIncrementalSearch(int16_t depth) {
    for(int i = 1; i < depth; ++i) {
      RunSearch(i);
    }
    return RunSearch(depth);
  }
}  // namespace chess_engine