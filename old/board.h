#pragma once

class Board;
class File;
struct Square;
class BoardIterator;

#define self (*this)

#include <forward_list>
#include <stack>
#include <chrono>

#include "Debugger.h"

#include "SDL+.h"

#include "game.h"
#include "item2.h"
#include "piece.h"

struct Square {
private:
	Board* board;
public:
	short const volatile x;
	short const volatile y;

	bool is_accessible : 1;

	Piece* piece;

	Square();

	Square(Board* b);

	Square(Board* b, int x_, int y_);
	Square(Board* b, Piece* p, int x_, int y_);

	// return wether a piece from the <color> side can attack this square
	bool is_controlled_by(piece_color color) const; 

	// move the piece within this square to the graveyard
	void to_graveyard();

	void draw() const;

	// clear the space while freeing memory
	void clear(); 

	// clear the space without freeing memory
	void remove(); 

	inline Uint64 get_mask() const {
		return 1ull << (x | (y << 3));
	}

	friend class File;
};

class File {
private:
	//static Piece *trash;
	Square data[8];
	File();
	File(Board* b, int y_position);
	auto init(Board* b, int y) -> void;

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
};

#define declare_color_array(type, name) \
type __black_element_ ## name; \
type name[2];

class Board {
private:

	File grid[8];

public:
	PieceClass* layout[8];

	Duck* duck = NULL;
	
	short white_death = 0;
	short black_death = 0;
	
	Piece* white_graveyard[16];
	Piece* black_graveyard[16];
	
	Color light_square_color;
	Color dark_square_color;
	Color selected_piece_color;
	Color possible_move_color;
	Color last_move_color;

	/*Square* last_move_begin_square;
	Square* last_move_end_square;*/
	move_data last_move_data;
	
	piece_color active_player;
	double crit_rate;
	double miss_rate;

	declare_color_array(bool, is_tera_avaible);

	bool in_bonus_move = false;

	bool in_ai_play = false;

	bool end_of_game = false;
	piece_color winner = no_color;

	Piece* promoting_piece;
	Uint8 avaible_promotion;

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
	
	Uint64 presence_mask;

	declare_color_array(Uint64, color_mask);
	
	Uint64 piece_mask[7];
	struct {
		// allows for type_mask[typeless]
		Uint64 __typeless_mask;
		Uint64 type_mask[18];
	};

	Uint64 honey_holder_mask;

	void resize_surface();

	auto begin() -> BoardIterator;

	auto end() -> BoardIterator;

	Surface surface;

	Board();

	void init();

	bool in_stalemate(piece_color color);

	void clear();

	void reset();

	move_data const get_last_nonduck_move();

	void cancel_last_move(bool restore_reachable=true);

	void set_reachable_squares();

	void move_piece_to(Piece*, Square*);

	void check_for_end_of_game();

	void promote(Uint8 promoted_piece_id);

	void resume_move();

	void change_turn();

	void skip_bonus_turn();

	void tera_piece(Piece*);

	inline File& operator[](int i) {
#if ENABLE_SAFETY_CHECKS
		if (i < 0 or i >= 8) {
			PRINT_DEBUG("Out of Bounds board access");
			throw;
		}
#endif
		return grid[i];
	}

	Board copy();

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

	BoardIterator(Board& board, int index_);
};
