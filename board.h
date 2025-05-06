#pragma once


#include <queue>
#include <forward_list>
#include <stack>
#include <chrono>
#include <mutex>

#define MOVE_CONTAINER_CAPACITY 256

class Board;
class File;
struct Square;
class BoardIterator;
struct bot_move_data;
template<typename T, size_t capacity>
struct MoveContainer;

#define MAKE_TIMER_LOCK_DUMMY true

#if MAKE_TIMER_LOCK_DUMMY
// mutex for non critical code that can be disabled when compiling
struct TimerLock {
	inline void lock() {}
	inline void unlock() {}
	inline bool try_lock() { return true; }
};
#else
using TimerLock = std::mutex;
#endif

#define self (*this)



#include "Debugger.h"

#include "SDL+.h"

#include "game.h"
#include "item2.h"
#include "piece2.h"


extern TimerLock reachable_lock;
extern std::chrono::nanoseconds reachable_duration;

extern TimerLock copy_lock;
extern std::chrono::nanoseconds copy_duration;

extern TimerLock movement_lock;
extern std::chrono::nanoseconds movement_duration;

extern TimerLock cancel_lock;
extern std::chrono::nanoseconds cancel_duration;

struct Square {
public:
	Piece* piece;

	Square();

	Square(Piece* p);

	// return wether a piece from the <color> side can attack this square
	bool is_controlled_by(Board&, piece_color color); 

	// move the piece within this square to the graveyard
	void to_graveyard(Board&);

	void draw(Game& game, Board& board, int pos);


	inline void clear() {
		piece = NULL;
	}

	friend class File;
};
/*
class File {
private:
	//static Piece *trash;
	Square data[8];
	File();
	File(int y_position);
	auto init(int y) -> void;

public:

	inline Square& operator[](int i) {
#if ENABLE_SAFETY_CHECKS
		if (i < 0 or i >= 8) {
			PRINT_DEBUG("Out of Bounds board access");
			throw;
		}
#endif
		return data[i];
	}

	void clear();

	~File();

	friend class Board;
};*/



class Board {
private:

	Square grid[64];

	Piece piece_pool[32];
	Piece __duck__;
	Uint8 nb_of_piece = 0;
public:
	static piece_id layout[8];
	
	Uint64 reachable_mask_array[64];
	// Uint64 item_reachable_mask_array[64];

	Piece* duck;
	
	color_array<Sint8> nb_of_death;
	
	color_array<Piece* [16], true> graveyard;
	
	Color light_square_color;
	Color dark_square_color;
	Color selected_piece_color;
	Color possible_move_color;
	Color last_move_color;

	/*Square* last_move_begin_square;
	Square* last_move_end_square;*/
	move_data last_move_data;
	
	piece_color active_player;
	float crit_rate;
	float miss_rate;

	color_array<bool> is_tera_avaible;

	bool in_bonus_move = false;

	bool end_of_game = false;
	piece_color winner = no_color;

	Piece* promoting_piece;
	Uint16 avaible_promotion;

	// some bool that sets the gamerules
	bool with_typing = true;
	bool with_items = false;
	bool with_check = false;
	bool with_RNG = false;
	bool with_random_battle = false;
	bool with_duck_chess = false;
	bool with_antichess = false;
	bool with_ability = true;
	bool with_AG = false;
	bool with_reversed_typechart = false;

	int turn_number;
	bool first_turn = true;

	std::list<move_data> move_historic;
	
	union {
		Uint64 presence_mask;
		Uint64 __first_mask;
	};
	
	color_array<Uint64> color_mask;
	
	Uint64 piece_mask[(int)piece_id::__nb_of_id__];
	Uint64 avaible_en_passant_mask;
	struct {
		// allows for type_mask[typeless]
		Uint64 __typeless_mask;
		Uint64 type_mask[18];
	};

	union {
		Uint64 honey_holder_mask;
		Uint64 __last_mask;
	};


	Board(Board&);
	Board& operator=(Board&);

	void resize_surface();

	BoardIterator begin();

	BoardIterator end();

	Surface surface;

	Board();

	void init();

	bool in_stalemate(piece_color color);

	void clear();

	void reset();

	move_data const get_last_nonduck_move();

	void cancel_last_move(bool restore_reachable=true);

	void set_reachable_squares();

	void move_piece_to(Piece*, Sint8);

	void first_half_move(Piece*, Sint8);

	void second_half_move(move_data& first_half_move_data);

	void check_for_end_of_game();

	void promote(piece_id new_id);

	void resume_move();

	void change_turn();

	void skip_bonus_turn();

	void tera_piece(Piece*);


	MoveContainer<bot_move_data, MOVE_CONTAINER_CAPACITY> get_avaible_move();

	void execute_move(bot_move_data);

	inline Square& operator[](int i) {
#if ENABLE_SAFETY_CHECKS
		if (i < 0 or i >= 64) {
			PRINT_DEBUG("Out of Bounds board access");
			throw;
		}
#endif
		return grid[i];
	}

	inline Square const& operator[](int i) const {
#if ENABLE_SAFETY_CHECKS
		if (i < 0 or i >= 64) {
			PRINT_DEBUG("Out of Bounds board access");
			throw;
		}
#endif
		return grid[i];
	}

	~Board();

	friend class BoardIterator;
};

class BoardIterator {
public:
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = signed int;
	using value_type = Square&;
	using pointer = Square*;
	using reference = Square&;

	BoardIterator(Board& board, int x, int y);

	BoardIterator(const BoardIterator& other);

	auto operator*()->reference;
	auto operator->()->pointer;
	
	auto operator++()->BoardIterator&;
	auto operator++(int)->BoardIterator;
	
	auto operator--()->BoardIterator&;
	auto operator--(int)->BoardIterator;
	
	auto operator+=(difference_type step)->BoardIterator&;
	auto operator+(difference_type step)->BoardIterator;
	
	auto operator-=(difference_type step)->BoardIterator&;
	auto operator-(difference_type step)->BoardIterator;


	
	friend auto operator==(const BoardIterator& lhs, const BoardIterator& rhs) -> bool;
	friend auto operator !=(const BoardIterator& lhs, const BoardIterator& rhs) -> bool;
	friend class Board;
private:
	Board& iterated_board;
	int index;
public:
	BoardIterator(Board& board, int pos);
};
