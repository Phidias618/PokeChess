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

extern bool is_cls_init;
bool init_all_cls();

class Square;

class Piece;
class Rook;
class Bishop;
class Queen;
class Knight;
class WhitePawn;
class BlackPawn;
class King;



struct move_data {
	move_data() {
		matchup = neutral;
		promotion = en_passant = castling = interrupt_move = was_in_check = escaped_check = move_again = suicide = cancel = do_crit = is_super_effective = is_not_very_effective = is_immune = do_miss = false;
		attacker = NULL;
		defender = NULL;
		begin_square = target_square = NULL;
	}

	void set_type_matchup_data(Piece* attacker, Piece* defender, Square* target_square);

	Piece* attacker;
	Piece* defender;

	Square* begin_square;
	Square* target_square;

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
	PieceClass();

	PieceClass(std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> constructor, int id, const char* const name);

	bool operator==(void*) const;

	PieceClass& operator=(const PieceClass& other);

	Piece* operator()(Board& board, piece_color color, Square* sq, typing type, PokeItem* item);
	int id;
	const char* const name;
};

#include "assets.h"
#include "item.h"
#include "board.h"




bool operator==(void* other, const PieceClass Class);


class Piece {
	static PieceClass __cls;
public:
	static PieceClass* const cls;
	PieceClass* const Class;

	typing type;
	const piece_color color;
	
	bool has_already_move : 1;
	bool is_in_graveyard : 1;
	
	PokeItem* item;

	Square* square;
	
	Board& board;

	
	Surface sprite;

	Piece(Board& board_, PieceClass* const Class, piece_color color_, Square* sq, typing type_=typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& square) -> bool;
	
	bool can_move_to(Square& target);

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

	void set_type(typing new_type);

	void set_item(PokeItem* new_item);

	void resize_sprite();
	void update_sprite();

	virtual ~Piece();

	friend class PokeItem;
	friend bool init_all_cls();
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
	friend bool init_all_cls();

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
	friend bool init_all_cls();

};

class Knight : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Knight(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item=NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;
	friend bool init_all_cls();

};

class Rook : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Rook(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;
	friend bool init_all_cls();

};

class Bishop : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Bishop(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;
	friend bool init_all_cls();

};

class Queen : public virtual Rook, public virtual Bishop {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Queen(Board& board_, piece_color color, Square* sq, typing type_ = typeless, PokeItem* item = NULL);

	auto virtual base_can_move_to(Square& target_square) -> bool;

	auto virtual base_do_control(Square& target_square) -> bool;

	friend class PokeItem;
	friend bool init_all_cls();

};


