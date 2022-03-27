#include "count_moves.h"

#include <vector>

#include "node.h"
#include "position.h"
#include "position_table.h"
#include "zobrist_hash.h"

#include <iostream>

namespace chess_engine {

int64_t CountMoves(const Position& position, int depth) {
  if (!depth) {
    return 1;
  }
  int64_t ret = 0;
  std::vector<Move> legal_moves = position.GetLegalMoves();
  if (depth == 1) {
    return legal_moves.size();
  }
  for (Move move:legal_moves) {
    Position new_position = position;
    new_position.MakeMove(move);
    ret += CountMoves(new_position, depth-1);
  }
  return ret;
}

int64_t CountMovesWithHash(const Node& node, int depth, PositionTable<HashEntry, 24> &table) {
  if (!depth) {
    return 1;
  }
  int64_t ret = 0;
  std::vector<Move> legal_moves = node.GetLegalMoves();
  if (depth == 1) {
    table.Set(node.GetHash().Get(), {depth, static_cast<int64_t>(legal_moves.size())});
    return legal_moves.size();
  }
  for (Move move:legal_moves) {
    Node new_node = node;
    HashEntry hashed = table.Get(node.HashAfterMove(move).Get());
    if (hashed.depth == depth-1) {
      ret += hashed.count;
    } else {
      new_node.MakeMove(move);
      ret += CountMovesWithHash(new_node, depth-1, table);
    }
  }
  table.Set(node.GetHash().Get(), {depth, ret});
  return ret;
}

int64_t CountMovesWithHash(const Position& position, int depth, const ZobristHashFunction& func) {
  Node root(position, func);
  PositionTable<HashEntry, 24> table;
  int64_t ret = CountMovesWithHash(root, depth, table);
  return ret;
}

}