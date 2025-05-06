#pragma once

#include <cstdint>
#include <atomic>
#include "bit_manipulation.h"


struct Lock {
private:
	std::atomic_flag is_locked;
public:
	inline Lock() { ; }

	inline void lock() {
		while (std::atomic_flag_test_and_set(&is_locked)) {
			;
		}
	}

	inline void unlock() {
		std::atomic_flag_clear(&is_locked);
	}

	inline bool try_lock() {
		return !(std::atomic_flag_test_and_set(&is_locked));
	}
};




int get_number_of_CPU_core();


#include <exception>

class EvalFailureException : public std::runtime_error {
public:
	EvalFailureException() noexcept;
};

enum class piece_id : Uint8;

struct bot_move_data {
	Sint8 begin_pos = -1;
	Sint8 target_pos = -1;
	bool promotion = false;
	bool tera = false;
	bool skip_bonus_turn = false;
	piece_id promotion_id = std::bit_cast<piece_id>((Uint8)-1);
};

template<typename T, size_t capacity>
struct MoveContainer {
private:
	std::array<T, capacity> data;
	size_t len;
public:
	inline MoveContainer() {
		len = 0;
	}

	inline void push(T const& value) {
#if ENABLE_SAFETY_CHECKS
		if (length == capacity)
			throw;
#endif
		data[len++] = value;
	}

	inline T* begin() {
		return data._Unchecked_begin();
	}

	inline T* end() {
		return begin() + len;
	}

	inline bool empty() {
		return len == 0;
	}

	inline T& back() {
		return data[len-1];
	}

	inline void pop() {
#if ENABLE_SAFETY_CHECKS
		if (length == 0)
			throw;
#endif
		len--;
	}

	inline constexpr size_t length() {
		return len;
	}

	inline constexpr T& operator[](size_t index) {
#if ENABLE_SAFETY_CHECKS
		if (index >= length)
			throw;
#endif
		return data[index];
	}

	inline constexpr T const& operator[](size_t index) const {
#if ENABLE_SAFETY_CHECKS
		if (index >= length)
			throw;
#endif
		return data[index];
	}
};


#include "board.h"
#include "piece2.h"


float eval_depth0(Board& board);

float eval_board(Board& board, int depth, float alpha, float beta, bot_move_data* best_move);

void start_eval(Game&, Board& board);

void start_threaded_eval(Game& game, Board& board);

Uint64 perft(Board& board, int depth);

void print_perft(Board board, int depth);

