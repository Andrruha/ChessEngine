#include <iostream>
#include <string>

#include "chess_defines.h"
#include "count_moves.h"
#include "fen.h"
#include "position.h"

int main() {
	std::cout << "Enter starting position fen:\n";
	std::string fen;
	std::getline(std::cin, fen);
	chess_engine::Position pos = chess_engine::FenToPosition(fen);

	std::cout << "Enter the list of moves. Enter start to start debugging:\n";
	std::string move_str;
	std::cin >> move_str;
	while (move_str != "start") {
		pos.MakeMove(chess_engine::UciToMove(move_str, pos));
		std::cin >> move_str;
	}

	std::cout << "Enter depth:\n";
	int depth;
	std::cin >> depth;

	while (depth) {
		int64_t total = 0;
		std::vector<chess_engine::Move> moves = pos.GetLegalMoves();
		for (auto move : moves) {
			chess_engine::Position new_pos = pos;
			new_pos.MakeMove(move);
			int64_t move_count = CountMoves(new_pos, depth-1);
			std::cout << chess_engine::MoveToUci(move, pos) << " " << move_count << "\n";
			total += move_count;
		}
		std::cout << "total: " << total << "\n\n";

		std::cout << "Enter next move:\n";
		std::string move_str;
		std::cin >> move_str;

		chess_engine::Move move = chess_engine::UciToMove(move_str, pos);
		pos.MakeMove(move);
		--depth;
	}
}