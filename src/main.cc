#include "src/chess_defines.h"
#include "src/fen.h"
#include "src/zobrist_hash.h"
#include "src/engine.h"
#include "src/engine_manager.h"
#include "src/winboard_protocol.h"

int main() {
  // Create hash-function for chess positions
  chess_engine::ZobristHashFunction func(14159265358979323846ull);

  // Set up the starting position
  chess_engine::Position starting_position = chess_engine::FenToPosition(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  );

  // Create instances of the engine and protocol
  chess_engine::Engine engine(starting_position, func);
  chess_engine::WinboardProtocol protocol;
  chess_engine::EngineManager manager(&protocol, &engine);

  manager.StartMainLoop();
}
