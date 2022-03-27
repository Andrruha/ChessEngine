#ifndef CHESS_ENGINE_SRC_COUNT_MOVES_
#define CHESS_ENGINE_SRC_COUNT_MOVES_

#include "node.h"
#include "position.h"
#include "position_table.h"
#include "zobrist_hash.h"

namespace chess_engine {

int64_t CountMoves(const Position& position, int depth);

struct HashEntry {
  int depth = -1;
  int64_t count = -1;
};

int64_t CountMovesWithHash(const Node& node, int depth, PositionTable<HashEntry, 24> &table);
int64_t CountMovesWithHash(const Position& pos, int depth, const ZobristHashFunction& func);


}

#endif  // CHESS_ENGINE_SRC_COUNT_MOVES_