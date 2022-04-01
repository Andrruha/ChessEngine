#ifndef CHEES_ENGINE_SRC_ENGINE_
#define CHESS_ENGINE_SRC_ENGINE_

#include <array>
#include <limits>
#include <list>
#include <functional>

#include "chess_defines.h"
#include "node.h"
#include "position_table.h"
#include "zobrist_hash.h"

namespace chess_engine {

class Engine {
 public:
  Engine(const Position& position, const ZobristHashFunction hash_func);

  int32_t GetEvaluation(int16_t min_depth = 0);
  Move GetBestMove(int16_t min_depth = 0);

  // 'proceed' callback says when to stop evaluating
  void StartSearch(std::function<bool(int)> proceed);

  void MakeMove(Move move);
  int32_t SimpleEvaluate(const Node& node);

  const Position& GetPosition() const;
  
  std::list<Move> GetPrincipalVariation() const;
  int64_t GetNodesVisited() const;

  void UseTranspositionTable(bool value);

  static int32_t GetHighestEval();
  static int32_t GetLowestEval();
  static int32_t GetLongestCheckmate();
 private:
  enum struct NodeType {
    kFailLow = 0,
    kPV = 1,
    kFailHigh = 2
  };
   struct NodeInfo {
    int16_t depth = -1;
    NodeType type;
    int32_t eval = 0;
    Move best_move = {{-1,-1},{-1,-1}, pieces::kNone};
  };
  NodeInfo RunSearch(
    int16_t depth,
    const Node& node,
    std::list<Move>& parent_variation,
    int32_t alpha = lowest_eval_,
    int32_t beta = highest_eval_,
    int16_t ply = 0
  );

  NodeInfo RunSearch(int16_t depth);
  NodeInfo RunIncrementalSearch(int16_t depth);
  NodeInfo RunInfiniteSearch(std::function<bool(int16_t)> proceed);

  void SortMoves(std::vector<Move>& moves, const Node& node, int16_t depth);

  Node root_;
  NodeInfo root_info_;

  std::array<int32_t, 6> piece_values = {1000, 5000, 3000, 3000, 9000, 0};

  std::list<Move> current_variation_;
  std::list<Move> principal_variation_;
  int64_t nodes_visited_;

  PositionTable<NodeInfo, 25> transposition_table_;
  bool use_transposition_table_ = true;
  PositionTable<bool, 16> no_return_table_;
  std::vector<std::pair<Move, Move>> cut_moves;

  std::function<bool(int)> proceed_with_batch_;
  std::function<void(int, int64_t, std::list<Move>)> report_progress_;

  static const int32_t lowest_eval_ = -2000000000;
  static const int32_t highest_eval_ = 2000000000;
  static const int32_t longest_checkmate_ = 1000;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_ENGINE_