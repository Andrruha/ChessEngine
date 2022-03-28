#ifndef CHEES_ENGINE_SRC_ENGINE_
#define CHESS_ENGINE_SRC_ENGINE_

#include <limits>

#include "chess_defines.h"
#include "node.h"
#include "position_table.h"
#include "zobrist_hash.h"

namespace chess_engine {

class Engine {
 public:
  Engine(const Position& position, const ZobristHashFunction hash_func);
  int32_t GetEvaluation(int16_t min_depth);
  Move GetBestMove(int16_t min_depth);
  void MakeMove(Move move);

  const Position& GetPosition() const;

  static int32_t SimpleEvaluate(const Node& node);

  static int32_t GetHighestEval();
  static int32_t GetLowestEval();
  static int32_t GetLongestCheckmate();
 private:
   struct NodeInfo {
    int16_t depth = -1;
    int32_t eval = 0;
    Move best_move = {{-1,-1},{-1,-1}, pieces::kNone};
  };
  NodeInfo RunSearch(
    int16_t depth,
    const Node& root,
    int32_t alpha = lowest_eval_,
    int32_t beta = highest_eval_
  );
  void SortMoves(std::vector<Move>& moves, const Position& position);

  Node root_;
  NodeInfo root_info_;

  PositionTable<NodeInfo, 24> transposition_table;

  static const int32_t lowest_eval_ = -2000000000;
  static const int32_t highest_eval_ = 2000000000;
  static const int32_t longest_checkmate_ = 1000;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_ENGINE_