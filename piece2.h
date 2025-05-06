#pragma once

#include <array>
#include <cstdint>
typedef uint8_t Uint8;

enum class piece_id : Uint8 {
	none = 0b1111'1111,
	king = 0b0000'0000,
	queen = 0b0000'0001,
	bishop = 0b0000'0010,
	knight = 0b0000'0011,
	rook = 0b0000'0100,
	pawn = 0b0000'0101,
	duck = 0b0000'0110,

	// support for fairy chess pieces

	// queen + knight
	amazon = 0b0000'0111,
	// bishop + knight
	princess = 0b0000'1000,
	// rook + knight
	empress = 0b0000'1001,

	__nb_of_id__,
};

#include "bit_manipulation.h"

#include "assets.h"
#include "poketyping.h"



enum piece_color : Sint8 {
	white = 1,
	no_color = 0,
	black = -1,
};





// if compact is set to true no space is reserved for no_color
template<typename T, bool compact=false>
struct color_array {
private:
	std::conditional_t < compact, std::array< T,2>, std::array<T,3> > data;
public:
	inline constexpr T& operator[](piece_color color) {
		if constexpr (compact) {
			return data[(color+1) >> 1];
		}
		else {
			return data[color + 1];
		}
	}

	inline constexpr T const& operator[](piece_color color) const {
		if constexpr (compact) {
			return data[(color + 1) >> 1];
		}
		else {
			return data[color + 1];
		}
	}

	color_array() {
		;
	}

	color_array(std::conditional_t<compact, std::array<T, 2>, std::array<T, 3>> _arr) : data(_arr) {
		;
	}
};

extern color_array<Uint64[64], true> const pawn_step_patern;
extern color_array<Uint64[64], true> const pawn_capture_patern;

extern Uint64 const king_attack_patern[64];
extern Uint64 const knight_attack_patern[64];

inline piece_color operator -(piece_color c) {
	return (piece_color)-(Uint8)c;
}

inline piece_color operator~(piece_color c) {
	return -c;
}

inline piece_color operator!(piece_color c) {
	return -c;
}



inline Uint64 get_square_mask(bool limbo, Sint8 pos) {
	return limbo ? 0 : (1ull << pos);
}

inline Uint64 get_square_mask(Sint8 pos) {
	return 1ull << pos;
}

class Piece;
struct move_data;
struct Square;
class Board;
class PokeItem;


// does not mask friendly fire
inline Uint64 get_rook_movement(Sint8 const pos, Uint64 const presence_mask) {
	Uint64 reachable_mask = 0;
	int x = pos & 0b111;
	int y = pos >> 3;
	short X = x - 1, Y = y;
	Uint64 pos_mask = 1ull << (X | (Y << 3));
	while (X >= 0 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		X--;
		pos_mask >>= 1;
	}
	
	if (X >= 0) {
		reachable_mask |= pos_mask;
	}

	X = x + 1, Y = y;
	pos_mask = 1ull << (X | (Y << 3));
	while (X < 8 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		X++;
		pos_mask <<= 1;
	}

	if (X < 8) {
		reachable_mask |= pos_mask;
	}

	X = x, Y = y - 1;
	pos_mask = 1ull << (X + (Y << 3));

	while (Y >= 0 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		Y--;
		pos_mask >>= 8;
	}
	if (Y >= 0)
		reachable_mask |= pos_mask;

	X = x, Y = y + 1;
	pos_mask = 1ull << (X + (Y << 3));
	while (Y < 8 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		Y++;
		pos_mask <<= 8;
	}
	if (Y < 8)
		reachable_mask |= pos_mask;

	return reachable_mask;
}

// does not mask friendly fire
inline Uint64 get_bishop_movement(Sint8 pos, Uint64 presence_mask) {
	Uint64 reachable_mask = 0;
	int x = pos & 0b111;
	int y = pos >> 3;
	short X = x - 1, Y = y - 1;

	Uint64 pos_mask = 1ull << (X | (Y << 3));
	while (X >= 0 and Y >= 0 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		X--;
		Y--;
		pos_mask >>= 9;
	}

	if (X >= 0 and Y >= 0) {
		reachable_mask |= pos_mask;
	}

	X = x + 1, Y = y + 1;
	pos_mask = 1ull << (X | (Y << 3));

	while (X < 8 and Y < 8 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		X++;
		Y++;
		pos_mask <<= 9;
	}

	if (X < 8 and Y < 8) {
		reachable_mask |= pos_mask;
	}

	X = x + 1, Y = y - 1;
	pos_mask = 1ull << (X | (Y << 3));
	while (X < 8 and Y >= 0 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		X++;
		Y--;
		pos_mask >>= 7;
	}

	if (X < 8 and Y >= 0) {
		reachable_mask |= pos_mask;
	}

	X = x - 1, Y = y + 1;
	pos_mask = 1ull << (X | (Y << 3));

	while (X >= 0 and Y < 8 and (presence_mask & pos_mask) == 0) {
		reachable_mask |= pos_mask;
		X--;
		Y++;
		pos_mask <<= 7;
	}

	if (X >= 0 and Y < 8) {
		reachable_mask |= pos_mask;
	}

	return reachable_mask;
}






struct PokemonIcon {
private:
	Sint8 x, y;
public:
	inline void draw(Surface surface, int x_dest, int y_dest) const {
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

	inline Uint64 get_ability() const {
		if (x <= 0 or y <= 0 or y >= 19 or x >= 41)
			return 0;
		
		return ability_array[y - 1][x - 1];
	}

	// returns whether it holds a valid sprite or not
	inline operator bool() const {
		return x >= 0 and y >= 0;
	}

	void promote(Piece* promoted_piece);
};

extern color_array<int> promotion_rank;


#include "item2.h"


class Piece {
public:
	Sint8 pos;

	typing base_type;
	typing type;

	bool has_already_move : 1;
	bool evolved : 1;
	bool is_tera : 1;
	bool is_dead : 1;
	bool in_limbo : 1;
	bool is_piece : 1;

	piece_id id;

	char const* get_name(LANGUAGE) const;

	piece_color color;

	PokemonIcon pokeicon;

	PokeItem item;
	union {
		Uint32 ability;
		struct {
			// the bit fields are stored backwards
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

	Piece();

	Piece(piece_id id, piece_color color, typing type, PokeItem item);

	~Piece();

	void draw(Board const&, Surface dest, SDL_Rect* dest_rect = NULL) const;

	Uint64 get_reachable(Board const& board) const;

	void tera(Board&, typing new_type);

	inline void set_pokeicon(PokemonIcon icon=PokemonIcon(-1, -1)) {
		pokeicon = icon;
		ability = icon.get_ability();
	}

	void un_tera(Board&);

	void set_square(Board& board, bool limbo, Sint8 new_pos);

	// assumes the piece is not already in limbo
	void to_limbo(Board& board);

	// assume the piece was dead
	void revive(Board& board);

	void to_graveyard(Board&);

	move_data move_to(Board&, Sint8 new_pos);

	move_data base_move_to(Board&, Sint8 new_pos);

	void set_type(Board&, typing new_type);

	void set_item(Board&, PokeItem new_item);

	inline operator bool() const {
		return is_piece;
	}


	inline bool can_move_to(Board&, Square&) { return false; }

	inline bool do_control(Board&, Square&) { return false; }

	inline bool base_can_move_to(Board&, Square&) { return false; }

	inline bool base_do_control(Board&, Square&) { return false; }

	inline void promote(Board& board, piece_id new_id) {
		bool was_in_limbo = in_limbo;
		to_limbo(board);
		id = new_id;
		item.promote(board, this);
		set_square(board, was_in_limbo, pos);
		pokeicon.promote(this);
	}

	bool can_castle(Board const&, Sint8 target_pos, Sint8* rook_pos) const;
	bool light_can_castle(Board const&, Sint8 target_pos, Sint8* rook_pos) const;

	/*
	bool can_en_passant(Board const&, Sint8 target_pos) const;
	bool light_can_en_passant(Board const&, Sint8 target_pos) const;
	*/

	friend class PokeItem;
	friend struct PokemonIcon;
};

inline Piece* Duck(Piece* piece) {
	*piece = Piece();
	piece->id = piece_id::duck;
	piece->pos = -1;
	piece->is_piece = true;
	return piece;
}


struct move_data {
	inline move_data() {
		;
	}

	void set_type_matchup_data(Board&, Piece* attacker, Piece* defender, Sint8 target_pos);

	Piece* attacker = NULL;
	Piece* defender = NULL;

	Piece attacker_copy, defender_copy;

	Sint8 begin_pos = -1;
	Sint8 target_pos = -1;
	Sint8 end_pos = -1;

	float miss_rate = 0.f, crit_rate = 0.f;

	effectiveness matchup = neutral;

	// flags for regular chess
	bool init_promotion : 1 = false; // stores whether the move resulted in a promotion
	bool end_promotion : 1 = false; // stores whether the move finallized a promotion
	bool en_passant : 1 = false; // stores whether the move is en passant
	bool castling : 1 = false; // stores whether the move is castling
	
	bool interrupt_move : 1 = false; // stores whether the move (such as a promotion) needs to interrupt the normal process of a move
	bool was_piece_first_move : 1 = false;

	/*
	bool was_in_check : 1 = false; // stores whether the attacker was in check before moving
	bool escaped_check : 1 = false; // stores whether the attacker managed to escape a check
	*/

	// flags for pokemon chess

	// general flags in order for the game to work properly
	bool move_again : 1 = false; // stores whether the moving piece will be able to move again next turn
	bool suicide : 1 = false; // stores whether the piece kills itself (example: not very effective)
	// bool cancel : 1 = false; // stores whether the move was canceled (example: miss)

	// more detailed flags for cosmetics
	bool do_crit : 1 = false; // stores whether the move resulted in a critical hit
	bool is_super_effective : 1 = false;
	bool is_not_very_effective : 1 = false;
	bool is_immune : 1 = false;
	bool do_miss : 1 = false; // stores wether the move did miss

	bool tera : 1 = false;
	bool skip_bonus_turn : 1 = false;
	// note that most of the flags are used to display pokemon textbox after the move.

	Uint16 avaible_promotion = 0;
};

std::ostream& operator<<(std::ostream& os, move_data const& data);


extern Piece const NO_PIECE;

extern Uint64 sliding_attack_lookup[56320];

struct MagicBitboardData {
	Uint64 pre_mask;
	Uint64 post_mask;
	Uint64 magic;
	int lookup_offset;
	int shift;
};

extern MagicBitboardData const rook_magic[64];
extern MagicBitboardData const bishop_magic[64];

void init_magic_attack();

inline Uint64 get_magic_rook_attack(int pos, Uint64 presence_mask) {
	MagicBitboardData const data = rook_magic[pos];
	int const key = ((presence_mask & data.pre_mask) * data.magic) >> data.shift;
	return (sliding_attack_lookup + data.lookup_offset)[key] & data.post_mask;
}

inline Uint64 get_magic_bishop_attack(int pos, Uint64 presence_mask) {
	MagicBitboardData const data = bishop_magic[pos];
	int const key = ((presence_mask & data.pre_mask) * data.magic) >> data.shift;
	return (sliding_attack_lookup + data.lookup_offset)[key] & data.post_mask;
}


inline Uint64 get_rook_attack(int pos, Uint64 presence_mask) {
	// return get_rook_movement(pos, presence_mask);
	return get_magic_rook_attack(pos, presence_mask);
}

inline Uint64 get_bishop_attack(int pos, Uint64 presence_mask) {
	// return get_bishop_movement(pos, presence_mask);
	return get_magic_bishop_attack(pos, presence_mask);
}

#include "board.h"