/*This files contains all the code related to chess piece*/
#pragma once
#include <functional>
#include "poketyping.h"
#include "SDL+.h"

enum piece_color : char {
	white = 0,
	black = 1,
	no_color = 2
};

inline piece_color operator not(piece_color c) {
	return (piece_color) ((c < 2) ? (1 - c) : c);
}

class PokeItem;

class Square;

class Piece;
class Rook;
class Bishop;
class Queen;
class Knight;
class WhitePawn;
class BlackPawn;
class King;
class Duck;





struct move_data {
	move_data() {
		matchup = neutral;
		promotion = en_passant = castling = interrupt_move = was_in_check = escaped_check = move_again = suicide = cancel = do_crit = is_super_effective = is_not_very_effective = is_immune = do_miss = false;
		attacker = NULL;
		defender = NULL;
		crit_rate = miss_rate = 0;
		begin_square = target_square = NULL;
		attacker_item_slot = defenser_item_slot = NULL;
	}

	void set_type_matchup_data(Piece* attacker, Piece* defender, Square* target_square);

	Piece* attacker;
	Piece* defender;

	Square* begin_square;
	Square* target_square;

	double miss_rate, crit_rate;

	PokeItem* attacker_item_slot;
	PokeItem* defenser_item_slot;

	effectiveness matchup;

	// flags for regular chess
	bool promotion : 1; // stores whether the move resulted in a promotion
	bool en_passant : 1; // stores whether the move is en passant
	bool castling : 1; // stores whether the move is castling
	bool interrupt_move : 1; // stores whether the move (such as a promotion) needs to interrupt the normal process of a move

	bool was_in_check : 1; // stores whether the attacker was in check before moving
	bool escaped_check : 1; // stores whether the attacker managed to escape a check

	// flags for pokemon chess

	// general flags in order for the game to work properly
	bool move_again : 1; // stores whether the moving piece will be able to move again next turn
	bool suicide : 1; // stores whether the piece kills itself (example: not very effective)
	bool cancel : 1; // stores whether the move was canceled (example: miss)

	// more detailed flags for cosmetics
	bool do_crit : 1; // stores whether the move resulted in a critical hit
	bool is_super_effective : 1;
	bool is_not_very_effective : 1;
	bool is_immune : 1;
	bool do_miss : 1; // stores wether the move did miss

	// note that most of the flags are used to display pokemon textbox after the move.
};

class Board;
class Square;
class PokeItem;

class PieceClass {
private:
	std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> constructor;
public:
	std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> base_promotion_constructor;


	PieceClass();
	PieceClass(int _id, char const* names...);
	PieceClass(std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> constructor, int id, const char* const names...);

	bool operator==(void*) const;

	PieceClass& operator=(const PieceClass& other);

	Piece* operator()(Board& board, piece_color color, Square* sq, typing type, PokeItem* item);
	int id;
	const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];
};

#include "assets.h"
#include "item.h"
#include "board.h"

struct PokemonIcon {
private:
	short x, y;
public:
	inline void draw(Surface surface, int x_dest, int y_dest) {
		if (x >= 0 and y >= 0) {
			SDL_Rect dest(x_dest, y_dest);
			SDL_Rect area(x * TILE_SIZE * 3 / 4, y * TILE_SIZE * 3 / 4, TILE_SIZE * 3 / 4, TILE_SIZE * 3 / 4);
			surface.blit(pokemon_icons, &dest, &area, top_left);
		}
	}

	inline PokemonIcon(short _x, short _y) {
		x = _x;
		y = _y;
	}

	PokemonIcon(Piece const* const piece);

	inline PokemonIcon() {
		x = y = -1;
	}

	// returns whether it holds a valid sprite or not
	inline operator bool() {
		return x >= 0 and y >= 0;
	}
};

bool operator==(void* other, const PieceClass Class);

enum move_flags_mask {
	ignore_item = 0b1,
	ignore_movement_restriction = 0b10,
	ignore_movement_bonus = 0b100,
	ignore_honey = 0b1000,
};

class Piece {
	static PieceClass __cls;
	PokemonIcon pokeicon;
public:
	static PieceClass* const cls;
	PieceClass* const Class;

	typing type;
	const piece_color color;
	
	bool has_already_move : 1 = false;
	bool is_in_graveyard : 1 = false;
	bool evolved : 1 = false;
	
	PokeItem* item;

	Square* square;
	
	Board& board;
	
	Surface sprite;

	Piece(Board& board_, PieceClass* const Class, piece_color color_, Square* sq, typing type_=typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& square) -> bool;
	
	bool can_move_to(Square& target, Uint64 flags = 0);

	auto virtual base_do_control(Square& square) -> bool;

	bool do_control(Square& target);

	move_data move_to(Square& square);

	virtual move_data base_move_to(Square& square); // move to itself to the desired square, while returning some informations about the move

	inline int get_x() {
		return ((short*)square)[4];
		//return square->x;
	}
	inline int get_y() {
		return ((short*)square)[5];
		//return square->y;
	}
	__declspec(property(get = get_x)) int x;
	__declspec(property(get = get_y)) int y;

	inline bool has_icon() {
		return static_cast<bool>(pokeicon);
	}

	void set_type(typing new_type);

	void set_item(PokeItem* new_item);

	void set_pokeicon(PokemonIcon pokeicon=PokemonIcon(-1, -1));

	virtual void resize_sprite();
	void update_sprite();

	virtual ~Piece();

	friend class PokeItem;
};


class King : public virtual Piece {
	static PieceClass __cls;

	auto can_castle(Square& target_square, bool base_rule=false) -> bool;

	auto castle(Square& target_square) -> void;
public:
	static PieceClass* const cls;

	King(Board& board_, piece_color color, Square* sq, typing type_=typeless, PokeItem* item=NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	auto virtual base_move_to(Square& target_square) ->move_data;

	bool is_in_check(bool base_rule=false);

	virtual ~King();

	friend class LeppaBerry;
	friend class PokeItem;
	friend class IronBall;
};

class Pawn : public virtual Piece {
	static PieceClass __cls;

	auto can_en_passant(Square& target_square, bool base_rule=false) -> bool;

	bool can_double_step(Square& traget, bool base_rule = false);
public:
	static PieceClass* const cls;

	Pawn(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item=NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	auto virtual base_move_to(Square& square) ->move_data;

	friend class LeppaBerry;
	friend class PokeItem;
	friend class Piece;
	friend class IronBall;
};

class Knight : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Knight(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item=NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;
};

class Rook : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Rook(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;

};

class Bishop : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Bishop(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;
};

class Queen : public virtual Rook, public virtual Bishop {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Queen(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;

};

class Duck : public virtual Piece {
	static PieceClass __cls;
public:
	static PieceClass* const cls;

	Duck(Board& board, Square* square);

	virtual bool base_can_move_to(Square& target);

	virtual void resize_sprite();
};
