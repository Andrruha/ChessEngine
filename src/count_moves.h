#ifndef SRC_COUNT_MOVES_H_
#define SRC_COUNT_MOVES_H_

#include "src/node.h"
#include "src/position.h"
#include "src/position_table.h"
#include "src/zobrist_hash.h"

namespace chess_engine {

int64_t CountMoves(const Position& position, int depth);

struct HashEntry {
  int depth = -1;
  int64_t count = -1;
};

int64_t CountMovesWithHash(
  const Node& node, int depth, PositionTable<HashEntry, 24> *table
);
int64_t CountMovesWithHash(
  const Position& pos, int depth, const ZobristHashFunction& func
);


}  // namespace chess_engine

#endif  // SRC_COUNT_MOVES_H_
