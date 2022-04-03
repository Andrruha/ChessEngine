#include "chess_defines.h"
#include "fen.h"
#include "zobrist_hash.h"
#include "engine.h"
#include "engine_manager.h"
#include "winboard_protocol.h"

int main() {
  chess_engine::ZobristHashFunction func(14159265358979323846ull);
  chess_engine::Position starting_position = chess_engine::FenToPosition (
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  );
  chess_engine::Engine engine(starting_position, func);
  chess_engine::WinboardProtocol protocol;
  chess_engine::EngineManager manager(&protocol, &engine);

  manager.StartMainLoop();
}