#include "src/engine.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

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

void Engine::StartSearch() {
  NodeInfo last;
  proceed_with_batch_value_ = true;
  nodes_visited_ = 0;
  for (int16_t i = 1; i < max_depth_; ++i) {
    last = RunSearch(i, 0);
    if (last.depth != -1) {
      root_info_ = last;
      report_progress_(
        i, root_info_.eval, nodes_visited_, principal_variation_
      );
    } else {
      break;
    }
  }
}

void Engine::SetBatchSize(int64_t value) {
  batch_size_ = value;
}

void Engine::SetProceedWithBatchCallback(
  std::function<bool()> value
) {
  proceed_with_batch_ = value;
}

void Engine::SetReportProgressCallback(
  std::function<void(int16_t, int32_t, int64_t, std::list<Move>)> value
) {
  report_progress_ = value;
}

void Engine::MakeMove(Move move) {
  no_return_table_.Set(root_.GetHash(), true);
  root_.MakeMove(move);
  root_info_ = NodeInfo();
}

void Engine::SetPosition(const Position& position) {
  root_.SetPosition(position);
  transposition_table_.Clear();
  no_return_table_.Clear();
}

const Position& Engine::GetPosition() const {
  return root_.GetPosition();
}

int32_t Engine::SimpleEvaluate(const Node& node) {
  // TODO(Andrey): Better evaluation function.
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
      // King safety.
      int8_t king_distance = DistanceSquared(king, {file, rank});
      if (king_distance <= 2) {
        if (!node.GetAttacksByPlayer({file, rank}, opponent)) {
          king_freedom += 100;
        }
        ++king_denominator;
      }
      int8_t opponents_king_distance =
        DistanceSquared(opponents_king, {file, rank});

      if (opponents_king_distance <= 2) {
        if (!node.GetAttacksByPlayer({file, rank}, to_move)) {
          opponents_king_freedom += 100;
        }
        ++opponents_king_denominator;
      }

      // Board control.
      ret += node.GetAttacksByPlayer({file, rank}, to_move);
      ret -= node.GetAttacksByPlayer({file, rank}, opponent);

      // Material.
      Piece piece = node.GetSquare({file, rank});
      if (piece != pieces::kNone) {
        ret += piece.player == to_move?
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

void Engine::SortMoves(
  std::vector<Move>* moves, const Node& node, int16_t ply
) {
  // TODO(Andrey): Might be faster to use std::sort at this point.
  int insert_index = 0;

  // Old best move.
  NodeInfo old_info = transposition_table_.Get(node.GetHash());
  for (
    int read_index = insert_index;
    read_index < static_cast<int>(moves->size());
    ++read_index
  ) {
    if ((*moves)[read_index] == old_info.best_move) {
      std::swap((*moves)[insert_index], (*moves)[read_index]);
      ++insert_index;
    }
  }

  // PV Move.
  if (static_cast<int>(principal_variation_.size()) > ply) {
    auto pv_iterator = principal_variation_.begin();
    std::advance(pv_iterator, ply);
    Move pv_move = *pv_iterator;
    for (
      int read_index = insert_index;
      read_index < static_cast<int>(moves->size());
      ++read_index
    ) {
      if ((*moves)[read_index] == pv_move) {
        std::swap((*moves)[insert_index], (*moves)[read_index]);
        ++insert_index;
      }
    }
  }

  // Cut moves.
  for (
    int read_index = insert_index;
    read_index < static_cast<int>(moves->size());
    ++read_index
  ) {
    if (
      (*moves)[read_index] == cut_moves[ply].first ||
      (*moves)[read_index] == cut_moves[ply].second
    ) {
      std::swap((*moves)[insert_index], (*moves)[read_index]);
      ++insert_index;
    }
  }

  // Some of the checks.
  for (
    int read_index = insert_index;
    read_index < static_cast<int>(moves->size());
    ++read_index
  ) {
    if (node.MoveIsCheckFast((*moves)[read_index])) {
      std::swap((*moves)[insert_index], (*moves)[read_index]);
      ++insert_index;
    }
  }

  // Captures.
  for (
    int read_index = insert_index;
    read_index < static_cast<int>((*moves).size());
    ++read_index
  ) {
    if (node.GetSquare((*moves)[read_index].to) != pieces::kNone) {
      std::swap((*moves)[insert_index], (*moves)[read_index]);
      ++insert_index;
    }
  }
}

Engine::NodeInfo Engine::RunSearch(
  int16_t depth,
  int16_t check_extra_depth,
  const Node& node,
  std::list<Move>* parent_variation,
  int32_t alpha,
  int32_t beta,
  int16_t ply
) {
  ++processed_in_the_batch_;
  if (processed_in_the_batch_ >= batch_size_) {
    proceed_with_batch_value_ = proceed_with_batch_();
    processed_in_the_batch_ = 0;
  }
  if (!proceed_with_batch_) {
    return NodeInfo();
  }
  ++nodes_visited_;
  NodeInfo ret;
  if (node.IsCheckmate()) {
    ret = {depth, NodeType::kPV, lowest_eval_, kNullMove};
  }
  if (node.IsStalemate()) {
    ret = {depth, NodeType::kPV, 0, kNullMove};
  }
  if (ret.depth >= 0) {
    if (use_transposition_table_) {
      transposition_table_.Set(node.GetHash(), ret);
    }
    return ret;
  }

  std::vector<Move> legal_moves;
  if (depth > 0) {
    legal_moves = node.GetLegalMoves();
    SortMoves(&legal_moves, node, ply);
  } else {
    if (node.GetLastCapture() != Coordinates{-1, -1}) {
      legal_moves = node.GetCapturesOnSquare(
        node.GetLastCapture(), node.PlayerToMove()
      );
      legal_moves.push_back(kNullMove);  // Hack for now.
    } else {
      return {0, NodeType::kPV, SimpleEvaluate(node), kNullMove};
    }
  }

  Move best_move = legal_moves[0];
  int32_t eval = lowest_eval_;
  NodeType type = NodeType::kFailLow;
  std::list<Move> principal_variation;  // Best line from a subcall.

  for (Move move : legal_moves) {
    int16_t child_depth = depth;
    if (child_depth <= 0) {
      child_depth = 1;  // We are already doing a quiescense search.
    } else {
      if (check_extra_depth && (node.IsCheck() || node.MoveIsCheckFast(move))) {
        ++child_depth;
        --check_extra_depth;
      }
    }
    
    // Search tables.
    ZobristHash new_hash = node.HashAfterMove(move);
    NodeInfo child;
    if (no_return_table_.Get(new_hash.Get())) {
      child = {
        max_depth_, NodeType::kPV, 0, {{-1, -1}, {-1, -1}, pieces::kNone}
      };
    } else {
      child = transposition_table_.Get(new_hash.Get());
      if (child.depth < child_depth-1) {
        Node new_node = node;
        new_node.MakeMove(move);
        child = RunSearch(
          child_depth-1,
          check_extra_depth,
          new_node,
          &principal_variation,
          -beta, -alpha, ply+1
        );
      }
    }

    // Deal with wrong bounds.
    switch (child.type) {
    case NodeType::kFailLow:
      if (-child.eval < beta) {
        Node new_node = node;
        new_node.MakeMove(move);
        int32_t new_alpha = std::max(alpha, -child.eval);
        child = RunSearch(
          child_depth-1,
          check_extra_depth,
          new_node,
          &principal_variation,
          -beta, -new_alpha, ply+1
        );
      }
      break;
    case NodeType::kFailHigh:
        if (alpha < -child.eval) {
        Node new_node = node;
        new_node.MakeMove(move);
        int32_t new_beta = std::min(beta, -child.eval);
        child = RunSearch(
          child_depth-1,
          check_extra_depth,
          new_node,
          &principal_variation,
          -new_beta, -alpha, ply+1
        );
      }
      break;
    }

    // Could've changed after subcalls.
    if (!proceed_with_batch_value_) {
      return NodeInfo();
    }

    // Update evaluation.
    if (-child.eval > eval) {
      eval = -child.eval;
      best_move = move;
    }
    if (-child.eval > alpha) {
      type = NodeType::kPV;
      alpha = -child.eval;
      *parent_variation = principal_variation;
      parent_variation->push_front(move);
      if (ply == 0) {
        int32_t eval_to_report = eval;
        if (eval_to_report > highest_eval_ - longest_checkmate_) {
          --eval_to_report;
        }
        report_progress_(
          depth, eval_to_report, nodes_visited_, principal_variation_
        );
      }
    }
    if (alpha >= beta) {
      // Node is a cut node.
      type = NodeType::kFailHigh;
      if (!node.MoveIsCheckFast(move) &&
        node.GetSquare(move.to) == pieces::kNone &&
        cut_moves[ply].first != move
      ) {
        cut_moves[ply].second = cut_moves[ply].first;
        cut_moves[ply].first = move;
      }
      break;
    }
  }

  // Write to transposition table and return.
  if (eval > highest_eval_ - longest_checkmate_) {
    --eval;
  }
  ret = {depth, type, eval, best_move};
  if (use_transposition_table_) {
    transposition_table_.Set(node.GetHash(), ret);
  }
  return ret;
}

Engine::NodeInfo Engine::RunSearch(int16_t depth, int16_t check_extra_depth) {
  return RunSearch(depth, check_extra_depth, root_, &principal_variation_);
}

Engine::NodeInfo Engine::RunIncrementalSearch(int16_t depth) {
  for (int i = 1; i < depth; ++i) {
    RunSearch(i);
  }
  return RunSearch(depth);
}

Engine::NodeInfo Engine::RunInfiniteSearch(
  std::function<bool(int16_t)> proceed
) {
  NodeInfo ret;
  for (int i = 1; proceed(i); ++i) {
    ret = RunSearch(i);
  }
  return ret;
}

}  // namespace chess_engine
