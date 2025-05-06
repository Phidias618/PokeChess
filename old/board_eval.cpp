#include <chrono>


#include "board_eval.h"


EvalFailureException::EvalFailureException() noexcept
	: std::runtime_error("")
{
	;
}



static float const king_value = 10;
static float const pawn_value = 1;
float const queen_value = 9;
float const rook_value = 5;
float const bishop_value = 3;
float const knight_value = 3;

float const __immune_effectiveness_mul = -0.1;
static float const effectiveness_mul[3] = {-0.05, 0, 0.1};
float const pawn_advancement_bonus = 1.f;



std::chrono::nanoseconds depth0_duration;

// positive for white
// negative for black
float eval_depth0(Board& board) {

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	if (board.end_of_game) {
		if (board.winner == no_color) {
			return 0;
		}
		else {
			return INFINITY * board.winner;
		}
	}

	float score = 0.f;

	// piece base score
	Uint64 const black_mask = *(board.color_mask + black);
	score += king_value * popcount64(board.royal_mask & board.color_mask[white]);
	score -= king_value * popcount64(board.royal_mask & black_mask);

	score += pawn_value * popcount64(board.pawn_mask & board.color_mask[white]);
	score -= pawn_value * popcount64(board.pawn_mask & black_mask);

	Uint64 const knight_mask = board.knight_movement_mask & (~board.rook_movement_mask) & (~board.bishop_movement_mask);

	score += knight_value * popcount64(knight_mask & board.color_mask[white]);
	score -= knight_value * popcount64(knight_mask & black_mask);

	Uint64 const rook_mask = (~board.knight_movement_mask) & board.rook_movement_mask & (~board.bishop_movement_mask);

	score += rook_value * popcount64(rook_mask & board.color_mask[white]);
	score -= rook_value * popcount64(rook_mask & black_mask);

	Uint64 const bishop_mask = (~board.knight_movement_mask) & (~board.rook_movement_mask) & board.bishop_movement_mask;

	score += bishop_value * popcount64(bishop_mask & board.color_mask[white]);
	score -= bishop_value * popcount64(bishop_mask & black_mask);

	Uint64 const queen_mask = (~board.knight_movement_mask) & board.rook_movement_mask & board.bishop_movement_mask;

	score += queen_value * popcount64(queen_mask & board.color_mask[white]);
	score -= queen_value * popcount64(queen_mask & black_mask);

	// white pawn advancement score
	float bonus = pawn_advancement_bonus;
	for (int i = 2; i < 7; i++) {
		score += bonus * popcount8((Uint8)((board.pawn_mask & board.color_mask[white]) >> (i << 3))); // only selects the pawn from the i-th row
		bonus += pawn_advancement_bonus;
	}
	// black pawn advancement score
	bonus = pawn_advancement_bonus;
	for (int i = 5; i > 0; i--) {
		score -= bonus * popcount8((Uint8)((board.pawn_mask & black_mask) >> (i << 3))); // only selects the pawn from the i-th row
		bonus += pawn_advancement_bonus;
	}

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	depth0_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);

	return score;

	for (Square& square : board) {
		if (square.piece != NULL) {
			iter_typing(t) {
				// bonus for offensive typechart advantage
				score += square.piece->color * effectiveness_mul[get_effectiveness(square.piece->type, t, board.with_antichess)] * popcount64(board.type_mask[t] & board.color_mask[not square.piece->color]);

				// bonus for defensive typechart advantage
				score -= square.piece->color * effectiveness_mul[get_effectiveness(t, square.piece->type, board.with_antichess)] * popcount64(board.type_mask[t] & board.color_mask[not square.piece->color]);
			}
		}
	}

	

	return score;
}


void PRINT_BOARD(Board& board) {
	for (int y = 7; y >= 0; y--) {
		for (int x = 0; x <= 7; x++) {
			Square& s = board[x][y];
			if (s.piece) {
				if (s.piece->is_duck) {
					putchar('D');
				}
				if (s.piece->is_royal) {
					putchar('K');
				}
				else if (s.piece->is_pawn) {
					putchar('p');
				}
				else if (s.piece->knight_movement) {
					putchar('N');
				}
				else if (s.piece->rook_movement) {
					if (s.piece->bishop_movement)
						putchar('Q');
					else
						putchar('R');
				}
				else if (s.piece->bishop_movement) {
					putchar('B');
				}
				else
					putchar('u');
			}
			else {
				putchar(' ');
			}
		}
		putchar('\n');
	}
	putchar('\n');
}

bool constexpr restore = false;

inline constexpr float max_float(float x, float y) {
	return (x > y) ? x : y;
}

#define DISABLE_ALPHA_BETA_PRUNING false
//
static void eval_piece_movement(Board& board, Sint8 piece_x, Sint8 piece_y, int depth, float* alpha, float const beta, float* best_score, bot_move_data* best_move) {
	
	if (game.interupt_eval)
		return;

	piece_color const color = board.active_player;
	Uint64 iterated_mask = board.reachable_mask_array[piece_x | (piece_y << 3)];

	Piece* const piece = board[piece_x][piece_y].piece;

	while (iterated_mask) {
		int pos = bitScanForward(iterated_mask);
		iterated_mask &= ~(1ull << pos);
		Sint8 const target_x = pos & 0b111;
		Sint8 const target_y = pos >> 3;

		// Board board_copy = board;
		
		// std::cout << (int)piece_x << "," << (int)piece_y << "->" << (int)target_x << "," << (int)target_y << ';';
		board.move_piece_to(piece, target_x, target_y);

		float score;
		
		if (board.promoting_piece != NULL) {
			Uint16 avaible = board.avaible_promotion;
			for (Uint8 id = 0; id < 16; id++) {
				if ((1 << id) & board.avaible_promotion) {
					avaible &= ~(1 << id);

					board.promote(id);

					score = color * eval_board(board, depth, -beta, -*alpha, NULL);

					board.cancel_last_move(restore);

					if (score >= *best_score) {
						*best_score = score;
						if (best_move != NULL) {
							best_move->begin_x = piece_x;
							best_move->begin_y = piece_y;

							best_move->target_x = target_x;
							best_move->target_y = target_y;

							best_move->promotion = true;
							best_move->promotion_characteristics = id;
						}
					}

#if not DISABLE_ALPHA_BETA_PRUNING
					*alpha = max_float(*alpha, score);

					if (beta <= *alpha or *best_score == +INFINITY) {
						return;
					}
#endif
					if (avaible)
						board.move_piece_to(piece, target_x, target_y);
				}
			}
			
		}
		else {
			score = color * eval_board(board, depth, -beta, -*alpha, NULL);

			board.cancel_last_move(restore);

			if (score >= *best_score) {
				*best_score = score;

				if (best_move != NULL) {
					best_move->begin_x = piece_x;
					best_move->begin_y = piece_y;

					best_move->target_x = target_x;
					best_move->target_y = target_y;
					best_move->tera = best_move->promotion = false;
				}
			}
#if not DISABLE_ALPHA_BETA_PRUNING
			*alpha = max_float(*alpha, score);

			if (beta <= *alpha or *best_score == +INFINITY) {
				return;
			}
#endif
		}
	}

	if (board.is_tera_avaible[piece->color] and piece->item.get_id() == item_ID::tera_shard) {
		board.tera_piece(piece);

		float score = color * eval_board(board, depth, -beta, -*alpha, NULL);

		board.cancel_last_move(restore);

		if (score >= *best_score) {
			*best_score = score;
			if (best_move != NULL) {
				best_move->begin_x = piece_x;
				best_move->begin_y = piece_y;

				best_move->target_x = piece_x;
				best_move->target_y = piece_y;
				best_move->tera = true;
				best_move->promotion = false;
			}
		}

		*alpha = max_float(*alpha, score);
	}
}

float eval_board(Board& board, int depth, float alpha, float beta, bot_move_data* best_move) {
	piece_color const color = board.active_player;

	if (game.interupt_eval)
		return 0;
	if (depth == 0)
		return eval_depth0(board);

	if (board.end_of_game) {
		if (board.winner == no_color) {
			return 0;
		}
		else {
			return INFINITY * board.winner;
		}
	}

	static bool is_first = true;
	bool was_first = is_first;
	is_first = false;

	float best_score = -INFINITY;
	
	if (board.in_bonus_move) {
		// alpha and beta were wrongfully swapped and negated
		alpha *= -1;
		beta *= -1;
		eval_piece_movement(board, board.last_move_data.end_x, board.last_move_data.end_y, depth, &beta, alpha, &best_score, best_move);

#if not DISABLE_ALPHA_BETA_PRUNING
		// remember, alpha and beta are swapped, as a consequence this check correspond to the usual beta <= alpha
		if (alpha <= beta) {
			return color * best_score;
		}
#endif
		Piece* piece = board[board.last_move_data.end_x][board.last_move_data.end_y].piece;
		// std::cout << "skip\n";

		alpha *= -1;
		beta *= -1;

		board.skip_bonus_turn();

		float score = color * eval_board(board, depth - 1, alpha, beta, NULL);
		// std::cout << "cancel\n";
		board.cancel_last_move(restore);

		if (score > best_score) {
			best_score = score;
			if (best_move != NULL) {
				best_move->begin_x = board.last_move_data.end_x;
				best_move->begin_y = board.last_move_data.end_y;

				best_move->target_x = board.last_move_data.end_x;
				best_move->target_y = board.last_move_data.end_y;

				best_move->skip_bonus_turn = true;
				best_move->promotion = false;
			}
		}
	}
	else {
		// check if there is any instant victory
		Uint64 const other_king_mask = board.royal_mask & board.color_mask[not board.active_player];
		if (popcount64(other_king_mask) == 1) {
			Uint64 iterated_mask = board.color_mask[board.active_player];
			while (iterated_mask) {
				int const pos = bitScanForward(iterated_mask);
				iterated_mask &= ~(1ull << pos);
				Sint8 const begin_x = pos & 0b111;
				Sint8 const begin_y = pos >> 3;

				if (board.reachable_mask_array[pos] & other_king_mask) {
					if (best_move) {
						best_move->begin_x = begin_x;
						best_move->begin_y = begin_y;
						int king_pos = bitScanForward(other_king_mask);
						best_move->target_x = king_pos & 0b111;
						best_move->target_y = pos >> 3;
					}
					return color * INFINITY;
				}
			}
		}

		auto t0 = std::chrono::high_resolution_clock::now();

		// copy the avaible move, as such they dont need to be computed again
		Uint64 reachable_mask_array_copy[64];
		memcpy(reachable_mask_array_copy, board.reachable_mask_array, 64 * sizeof(Uint64));
		Uint64 item_reachable_mask_array_copy[64];
		memcpy(item_reachable_mask_array_copy, board.item_reachable_mask_array, 64 * sizeof(Uint64));

		auto t1 = std::chrono::high_resolution_clock::now();
		copy_duration += t1 - t0;

		Uint64 iterated_mask = board.color_mask[board.active_player];
		while (iterated_mask) {
			int const pos = bitScanForward(iterated_mask);
			iterated_mask &= ~(1ull << pos);
			Sint8 const begin_x = pos & 0b111;
			Sint8 const begin_y = pos >> 3;

			board.reachable_mask_array[pos] = reachable_mask_array_copy[pos];
			board.item_reachable_mask_array[pos] = item_reachable_mask_array_copy[pos];

			eval_piece_movement(board, begin_x, begin_y, depth - 1, &alpha, beta, &best_score, best_move);
#if not DISABLE_ALPHA_BETA_PRUNING
			if (beta <= alpha or best_score == +INFINITY) {
				break;
			}
#endif
		}
	}

	return color * best_score;
}

void start_eval(Game& game, Board& board) {

	game.in_board_eval = true;

	std::cout << "start eval\n";

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	movement_duration = movement_duration.zero();
	depth0_duration = depth0_duration.zero();
	copy_duration = copy_duration.zero();
	reachable_duration = reachable_duration.zero();


	game.best_move.promotion = false;
	game.best_move.begin_x = game.best_move.begin_y = game.best_move.target_x = game.best_move.target_y = -1;
	Board copy = board;
	try {
		eval_board(copy, game.search_depth, -INFINITY, INFINITY, &game.best_move);
	}
	catch (EvalFailureException) {
		game.eval_failure = true;
	}

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	auto total_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
	
	auto other_duration = total_duration - depth0_duration - copy_duration - reachable_duration - movement_duration;

	std::cout << "total_duration: " << std::chrono::duration_cast<std::chrono::seconds>(total_duration) << ":\n";
	std::cout << " - depth0 duration: " << std::chrono::duration_cast<std::chrono::seconds>(depth0_duration) << " -> " << 10000 * depth0_duration / total_duration << "%\n";
	std::cout << " - copy duration: " << std::chrono::duration_cast<std::chrono::seconds>(copy_duration) << " -> " << 10000 * copy_duration / total_duration << "%\n";
	std::cout << " - reachable duration: " << std::chrono::duration_cast<std::chrono::seconds>(reachable_duration) << " -> " << 10000 * reachable_duration / total_duration << "%\n";
	std::cout << " - movement duration: " << std::chrono::duration_cast<std::chrono::seconds>(movement_duration) << " -> " << 10000 * movement_duration / total_duration << "%\n";
	std::cout << " - other duration: " << std::chrono::duration_cast<std::chrono::seconds>(other_duration) << " -> " << 10000 * other_duration / total_duration << "%\n";
	
	std::cout << "\nend eval\n";

	game.in_board_eval = false;
}