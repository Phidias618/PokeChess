#include <chrono>
#include <mutex>
#include <atomic>

#include "board_eval.h"

#if defined(_WIN32) or defined(_WIN64)
#include <Windows.h>
#undef small
#undef max
#undef min
#endif

int get_number_of_CPU_core() {
#if defined(_WIN32) or defined(_WIN64)
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#elif defined(__linux)
	cpu_set_t cpuset;
	sched_getaffinity(0, sizeof(cpuset), &cpuset);
	return CPU_COUNT(&cpuset);
#elif defined(__APPLE__)
	return sysconf(_SC_NPROCESSORS_ONLN);
#else
	return 1;
#endif
}


EvalFailureException::EvalFailureException() noexcept
	: std::runtime_error("")
{
	;
}


__forceinline constexpr
float multiply_color_with_float(piece_color color, float x) {
	return static_cast<float>(color) * x;
}


float const piece_value[(int)piece_id::__nb_of_id__] = { 5.f, 9.f, 3.f, 3.f, 5.f, };

float const __immune_effectiveness_mul = -0.1;
static float const effectiveness_mul[3] = {-0.05f, 0.f, 0.1f};
float const pawn_advancement_bonus = .1f;

float const win_bonus = 10000.f;

TimerLock depth0_lock;
std::chrono::nanoseconds depth0_duration;

Uint64 node_counter = 0;


// positive for white
// negative for black
float eval_depth0(Board& board) {
	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	if (board.end_of_game) {
		if (board.winner == no_color) {
			return 0;
		}
		else {
			return multiply_color_with_float(board.winner, win_bonus);
		}
	}

	float score = 0.f;

	// piece base score
	for (int i = 0; i < (int)piece_id::__nb_of_id__; i++) {
		score += piece_value[i] * popcount64(board.piece_mask[i] & board.color_mask[white]);
		score -= piece_value[i] * popcount64(board.piece_mask[i] & board.color_mask[black]);

	}
	// white pawn advancement score
	float bonus = pawn_advancement_bonus;
	for (int i = 2; i < 7; i++) {
		score += bonus * popcount8((Uint8)((board.piece_mask[(int)piece_id::pawn] & board.color_mask[white]) >> (i << 3))); // only selects the pawn from the i-th row
		bonus += pawn_advancement_bonus;
	}
	// black pawn advancement score
	bonus = pawn_advancement_bonus;
	for (int i = 5; i > 0; i--) {
		score -= bonus * popcount8((Uint8)((board.piece_mask[(int)piece_id::pawn] & board.color_mask[black]) >> (i << 3))); // only selects the pawn from the i-th row
		bonus += pawn_advancement_bonus;
	}

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	depth0_lock.lock();
	depth0_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
	depth0_lock.unlock();

	return score;

	for (Square& square : board) {
		if (square.piece != NULL) {
			iter_typing(t) {
				// bonus for offensive typechart advantage
				score += multiply_color_with_float(square.piece->color, effectiveness_mul[get_effectiveness(square.piece->type, t, board.with_antichess)] * popcount64(board.type_mask[t] & board.color_mask[-(square.piece->color)]));

				// bonus for defensive typechart advantage
				score -= multiply_color_with_float(square.piece->color, effectiveness_mul[get_effectiveness(t, square.piece->type, board.with_antichess)] * popcount64(board.type_mask[t] & board.color_mask[-(square.piece->color)]));
			}
		}
	}

	

	return score;
}

#if false
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
#endif

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
			return multiply_color_with_float(board.winner, win_bonus);
		}
	}

	static bool is_first = true;
	bool was_first = is_first;
	is_first = false;

	float best_score = -INFINITY;
	
	/*
	// check if there is any instant victory
	Uint64 const other_king_mask = board.royal_mask & board.color_mask[not board.active_player];
	if (has_one_bit_set(other_king_mask)) {
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
	}*/

	// a huge thanks to ChessProgramming.org that features this algorithm
	for (bot_move_data move : board.get_avaible_move()) {
		node_counter++;
		board.execute_move(move);
		float score;
		if (board.in_bonus_move or board.promoting_piece != NULL) {
			score = multiply_color_with_float(color, eval_board(board, depth, alpha, beta, NULL));
		}
		else {
			score = multiply_color_with_float(color, eval_board(board, depth - 1, -beta, -alpha, NULL));
		}
		board.cancel_last_move(false);

		if (score > best_score) {
			best_score = score;
			if (score > alpha)
				alpha = score;
			
			if (best_move != NULL) [[unlikely]] {
				*best_move = move;
			}
		}
		if (score >= beta)
			break;
	}

	return multiply_color_with_float(color, best_score);
}

void start_eval(Game& game, Board& board) {
	game.eval_failure = false;
	game.in_board_eval = true;

	std::cout << "start eval\n";

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	node_counter = 0;
	movement_duration = movement_duration.zero();
	depth0_duration = depth0_duration.zero();
	copy_duration = copy_duration.zero();
	reachable_duration = reachable_duration.zero();


	game.best_move.promotion = false;
	game.best_move.begin_pos = game.best_move.target_pos = -1;
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
	std::cout << " - node per seconds: " << (1'000'000'000 * node_counter) / total_duration.count() << '\n';
	std::cout << node_counter << " node visited\n";
	std::cout << "\nend eval\n";

	game.in_board_eval = false;
}


#define DISABLE_CROSS_THREAD_PRUNNING false


void thread_starter(Board board, Lock& task_lock, Lock& score_lock, int depth, float& best_score, bot_move_data& best_move, float& alpha, float& beta, MoveContainer<bot_move_data, MOVE_CONTAINER_CAPACITY>& tasks) {
	
	piece_color const color = board.active_player;

	while (true) {
		task_lock.lock();
		if (tasks.empty()) {
			task_lock.unlock();
			break;
		}
		bot_move_data move = tasks.back();
		tasks.pop();
		task_lock.unlock();
		node_counter++;
		board.execute_move(move);
		float score;


		score_lock.lock();
		float alpha_copy = alpha;
		float beta_copy = beta;
		score_lock.unlock();

		if (board.in_bonus_move or board.promoting_piece != NULL) {
			score = multiply_color_with_float(color, eval_board(board, depth - 1, alpha_copy, beta_copy, NULL));
		}
		else {
			score = multiply_color_with_float(color, eval_board(board, depth - 2, -beta_copy, -alpha_copy, NULL));
		}
		
		board.cancel_last_move(false);

		score_lock.lock();
		
		if (score > best_score) {
			best_score = score;
			if (score > alpha)
				alpha = score;

			best_move = move;
		}

#if not DISABLE_CROSS_THREAD_PRUNNING
		if (score >= beta) {
			score_lock.unlock();
			break;
		}
#endif

		score_lock.unlock();
	}
}

static inline constexpr int MAX_INT(int x, int y) {
	return (x > y) ? x : y;
}

void start_threaded_eval(Game& game, Board& board) {


	game.in_board_eval = true;

	std::cout << "start threaded eval\n";

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	node_counter = 0;
	movement_duration = movement_duration.zero();
	depth0_duration = depth0_duration.zero();
	copy_duration = copy_duration.zero();
	reachable_duration = reachable_duration.zero();


	game.best_move.promotion = false;
	game.best_move.begin_pos = game.best_move.target_pos = -1;

	
	float best_score = -INFINITY;
	float alpha = -INFINITY;
	float beta = INFINITY;

	// only uses 1/2 of all avaible cores
	int const NB_CPU = MAX_INT(get_number_of_CPU_core() >> 1, 1);


	MoveContainer<bot_move_data, MOVE_CONTAINER_CAPACITY> tasks = board.get_avaible_move();

	std::vector<std::thread> threads;
	
	Lock task_lock;
	Lock score_lock;
	threads.reserve(NB_CPU);

	for (int i = NB_CPU; i > 0; i--) {
		// threads.emplace_back(thread_starter, board, lock, game.search_depth, best_score, best_move, alpha, beta, tasks);
		threads.emplace_back([&]() -> void { thread_starter(board, task_lock, score_lock, game.search_depth, best_score, game.best_move, alpha, beta, tasks); });
	}

	while (not threads.empty()) {
		threads.back().join();
		threads.pop_back();
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
	std::cout << " - node per seconds: " << (1'000'000'000 * node_counter) / total_duration.count() << '\n';
	std::cout << node_counter << " node visited\n";
	std::cout << "\nend threaded eval\n";

	game.in_board_eval = false;
}

Uint64 perft(Board& board, int depth) {
	if (depth == 0 or board.end_of_game) {
		return 1;
	}

	Uint64 res = 0;

	auto moves = board.get_avaible_move();
	for (bot_move_data move : moves) {
		board.execute_move(move);
		if (board.in_bonus_move)
			res += perft(board, depth);
		else
			res += perft(board, depth - 1);
			
		board.cancel_last_move(false);
	}
	
	return res;
}


void print_perft(Board board, int depth) {

	auto t0 = std::chrono::high_resolution_clock::now();

	Uint64 nodes = perft(board, depth);

	auto t1 = std::chrono::high_resolution_clock::now();
	auto total_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);

	std::cout << "Perft(" << depth << ") : \n";
	std::cout << " - node visited : " << nodes << "\n";
	std::cout << " - time : " << total_duration << "\n";
	std::cout << " - nodes per seconds : " << (1'000'000'000 * nodes) / total_duration.count() << '\n';
}