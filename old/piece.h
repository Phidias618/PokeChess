/*This files contains all the code related to chess piece*/

#if false
#pragma once
#include <functional>

#include "Debugger.h"

#include "poketyping.h"
#include "SDL+.h"



enum piece_color : Sint8 {
	white = 1,
	black = -1,
	no_color = 0
};

inline piece_color operator not(piece_color c) {
	return (piece_color)-c;
}

class PokeItem;

struct Square;

class Piece;
class Rook;
class Bishop;
class Queen;
class Knight;
class WhitePawn;
class BlackPawn;
class King;
class Duck;

#include "item2.h"

template<typename T>
inline T* dynacast(Piece* piece);

template<typename T>
inline T* dynacast_unsafe(Piece* piece);

struct move_data {
	inline move_data() {
		;
	}

	void set_type_matchup_data(Piece* attacker, Piece* defender, Square* target_square);

	Piece* attacker = NULL;
	Piece* defender = NULL;

	Square* begin_square = NULL;
	Square* target_square = NULL;

	double miss_rate = 0, crit_rate = 0;

	PokeItem attacker_item;
	PokeItem defender_item;

	effectiveness matchup = neutral;

	// flags for regular chess
	bool promotion : 1 = false; // stores whether the move resulted in a promotion
	bool en_passant : 1 = false; // stores whether the move is en passant
	bool castling : 1 = false; // stores whether the move is castling
	bool interrupt_move : 1 = false; // stores whether the move (such as a promotion) needs to interrupt the normal process of a move
	bool was_piece_first_move : 1 = false;
	

	bool was_in_check : 1 = false; // stores whether the attacker was in check before moving
	bool escaped_check : 1 = false; // stores whether the attacker managed to escape a check

	// flags for pokemon chess

	// general flags in order for the game to work properly
	bool move_again : 1 = false; // stores whether the moving piece will be able to move again next turn
	bool suicide : 1 = false; // stores whether the piece kills itself (example: not very effective)
	bool cancel : 1 = false; // stores whether the move was canceled (example: miss)

	// more detailed flags for cosmetics
	bool do_crit : 1 = false; // stores whether the move resulted in a critical hit
	bool is_super_effective : 1 = false;
	bool is_not_very_effective : 1 = false;
	bool is_immune : 1 = false;
	bool do_miss : 1 = false; // stores wether the move did miss

	bool tera : 1 = false;
	bool skip_bonus_turn : 1 = false;
	// note that most of the flags are used to display pokemon textbox after the move.

	Uint8 promotion_id = 0;
};

std::ostream& operator<<(std::ostream& os, move_data const data);

class Board;
struct Square;
class PokeItem;

class PieceClass {
private:
	std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem)> constructor;

	Uint64 offset = 0;
public:
	std::function<Piece* (Piece*)> base_promotion_constructor;


	PieceClass();
	PieceClass(int _id, char const* names...);

	template<typename T>
	PieceClass(std::function<T* (Board&, piece_color, Square*, typing, PokeItem)> constructor, int id, const char* const names...);

	bool operator==(void*) const;

	PieceClass& operator=(const PieceClass& other);

	Piece* operator()(Board& board, piece_color  color, Square* sq, typing type, PokeItem item);
	int id;
	const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	template<typename T> friend T* dynacast<T>(Piece* piece);
	template<typename T> friend T* dynacast_unsafe<T>(Piece* piece);
};

#include "assets.h"
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

	inline Uint64 get_ability() {
		if (x <= 0 or y <= 0 or y >= 19 or x >= 41)
			return 0;
		if (ability_array[y - 1][x - 1])
			PRINT_DEBUG("ability", type_str[0][y-1], ",", x, "->", ability_array[y - 1][x - 1]);
		return ability_array[y - 1][x - 1];
	}

	// returns whether it holds a valid sprite or not
	inline operator bool() {
		return x >= 0 and y >= 0;
	}

	void promote(Piece* const promoted_piece);
};

bool operator==(void* other, const PieceClass Class);

enum move_flags_mask {
	ignore_item = 0b1,
	ignore_movement_restriction = 0b10,
	ignore_movement_bonus = 0b100,
	ignore_honey = 0b1000,
};

extern PieceClass* piece_class_tab[7];

class Piece {
	static PieceClass __cls;
	PokemonIcon pokeicon;
public:
	typing base_type;
	typing type;
	const piece_color color;

	bool has_already_move : 1 = false;
	bool is_in_graveyard : 1 = false;
	bool evolved : 1 = false;
	bool is_tera : 1 = false;

	static PieceClass* const cls;
	PieceClass* const Class;

	Uint64 reachable_mask = 0;
	Uint64 item_reachable_mask = 0;


	PokeItem item;

	Square* square;
	
	union {
		Uint64 ability;
		struct {
			// the bit fields are stored backwards
			Uint32 u64;
			Uint8 u32;
			bool u24 : 1;
			bool u23 : 1;
			bool u22 : 1;
			bool u21 : 1;

			bool u20 : 1;

			// -1 effectiveness from fire/ice
			bool ability_thick_fat : 1;

			// gives a ground immunity
			bool ability_levitate : 1;
			// gives an electric immunity
			bool ability_volt_absorb : 1;


			// gives a grass immunity
			bool ability_sap_sipper : 1;
			// gives a water immunity
			bool ability_water_absorb : 1;

			// gives a fire immunity
			bool ability_torch : 1;
			// you can play again if you consume your item (good luck activacting it)
			bool ability_unburden : 1;

			// +crit chance if you are holding flame/toxic orb
			bool ability_guts : 1;
			// +crit chance
			bool ability_sniper : 1;
			// +crit chance per captured ally piece
			bool ability_supreme_overlord : 1;
			// same as rocky helmet
			bool ability_iron_thorns : 1;

			// if you are not holding an item and capture a piece you will receive honey
			bool ability_honey_gathering : 1;
			// you cannot miss
			bool ability_compound_eye : 1;

			// your item cannot be replaced/remove
			bool ability_sticky_hold : 1;
			// copy the ability of the last captured piece
			bool ability_trace : 1;

			// steal opposing item when captured
			bool ability_pickpocket : 1;
			// can steal opposing item on capture if the user is not holding one
			bool ability_magician : 1;

			// the user cannot be crit
			bool ability_shell_armor : 1;
			// the user can't miss not be missed
			bool ability_no_guard : 1;
		};
	};

	Board& board;
	
	// Surface sprite;

	Piece(Board& board_, PieceClass* const Class, piece_color color_, Square* sq, typing type_, PokeItem item);

	bool base_can_move_to(Square const& square) const;
	
	bool can_move_to(Square const& target, Uint64 flags = 0) const;

	virtual bool base_do_control(Square const& square) const;

	virtual void set_reachable() {}

	bool do_control(Square const& target) const;

	void set_square(Square* target);

	move_data move_to(Square& square);

	virtual void highlight_moves() {}

	virtual move_data base_move_to(Square& square); // move to itself to the desired square, while returning some informations about the move

	inline int get_x() const {
		return ((short*)square)[4];
		//return square->x;
	}
	inline int get_y() const {
		return ((short*)square)[5];
		//return square->y;
	}
	__declspec(property(get = get_x)) int x;
	__declspec(property(get = get_y)) int y;

	inline bool has_icon() {
		return static_cast<bool>(pokeicon);
	}

	void set_type(typing new_type);
	
	void tera(typing new_type);

	void un_tera();

	void set_item(PokeItem new_item);

	void set_pokeicon(PokemonIcon pokeicon=PokemonIcon(-1, -1));

	void update_sprite();
	virtual void draw(Surface dest, SDL_Rect* dest_rect = NULL);

	Piece* copy(Board&);

	virtual ~Piece();

	friend class PokeItem;
	friend class PieceClass;
};


class King : public virtual Piece {
	static PieceClass __cls;

	bool can_castle(Square const& target_square, Square** rook_pos) const;

	void castle(Square* rook_square);
public:
	static PieceClass* const cls;

	King(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item);

	auto virtual base_can_move_to(Square const& target_square) -> bool;

	virtual void set_reachable();

	virtual bool base_do_control(Square const& target_square) const;

	virtual move_data base_move_to(Square& target_square);

	bool is_in_check(bool base_rule=false);

	virtual ~King();

	friend class LeppaBerry;
	friend class PokeItem;
	friend class IronBall;
};

class Pawn : public virtual Piece {
	static PieceClass __cls;

	bool can_en_passant(Square const& target_square) const;

	bool can_double_step(Square const& target) const;

	// suppose the move is already diagonal
	bool light_en_passant_check(short X, short Y) const;
public:
	static PieceClass* const cls;

	Pawn(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item);

	virtual bool base_can_move_to(Square const& target_square) const;

	virtual void set_reachable();

	virtual bool base_do_control(Square const& target_square) const;

	virtual move_data base_move_to(Square& square);

	friend class LeppaBerry;
	friend class PokeItem;
	friend class Piece;
	friend class IronBall;
};


class Knight : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Knight(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item);

	virtual bool base_can_move_to(Square const& target_square) const;

	void set_reachable();

	virtual bool base_do_control(Square const& target_square) const;

	friend class PokeItem;
};

class Rook : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Rook(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item);

	virtual bool base_can_move_to(Square const& target_square) const;

	virtual void set_reachable();

	virtual bool base_do_control(Square const& target_square) const;

	friend class PokeItem;

};

class Bishop : public virtual Piece {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Bishop(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item);

	virtual bool base_can_move_to(Square const& target_square) const;

	virtual void set_reachable();

	virtual bool base_do_control(Square const& target_square) const;

	friend class PokeItem;
};

class Queen : public virtual Rook, public virtual Bishop {
	static PieceClass __cls;

public:
	static PieceClass* const cls;

	Queen(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item);

	virtual bool base_can_move_to(Square const& target_square) const;
	
	virtual void set_reachable();

	virtual bool base_do_control(Square const& target_square) const;

	friend class PokeItem;

};

class Duck : public virtual Piece {
	static PieceClass __cls;
public:
	static PieceClass* const cls;

	Duck(Board& board, Square* square);

	virtual void draw(Surface dest, SDL_Rect* dest_rect = NULL);

	virtual bool base_can_move_to(Square& target);

	virtual void set_reachable();
};

template<typename T>
inline T* dynacast(Piece* piece) {
#if ENABLE_SAFETY_CHECKS
	if (piece == NULL or piece->Class != T::cls) {
		PRINT_DEBUG("Error when downcasting a piece subclass");
		throw;
	}
#endif
	return (T*)((Uint8*)piece - T::cls->offset);
}

template<typename T>
inline T* dynacast_unsafe(Piece* piece) {
	return (T*)((Uint8*)piece - T::cls->offset);
}

#define pawn_dynacast ERREUR_CHANGEMENT_DE_NOM

#define king_dynacast ERREUR_CHANGEMENT_DE_NOM

#endif