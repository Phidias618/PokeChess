#pragma once

#include "SDL+.h"
#include "assets.h"
#include <array>
#include <set>


enum class item_ID : Uint8;
class PokeItem;
class Piece;
struct move_data;

struct Square;
class Board;
class Game;

#include "poketyping.h"

enum class item_ID : Uint8 {
	none = 0,
	evolution_stone,
	king_promotion,
	everstone,

	resistance_berry,
	immunity,
	HDB,
	eviolite,
	normal_gem,
	baguette,

	tera_shard,
	
	honey,
	
	rocky_helmet,
	red_card,
	sticky_barbs,

	life_orb,
	loaded_dice,
	scope_lens,
	metronome,
	quick_claw,

	blunder_policy,
	wide_lens,
	expert_belt,
	bright_powder,
	
	safety_googles,
	protective_pads,

	shed_shell,
	leppa_berry,
	
	ring_target,
	assault_vest,
	iron_ball,
	black_sludge,
	flame_orb,
	
	light_ball,
	deep_sea_scale,
	deep_sea_tooth,
	rare_candy,

	__nb_of_item__,

	__space_placeholder__,
	__newline_placeholder__,
	__terminator_placeholder__,
};
#define NB_OF_ITEM ((int)item_ID::__nb_of_item__)


#define PokeItem_MemoryLayout \
union { \
	Uint8 __slot1; \
	typing t1; \
	typing type; \
	typing new_type; \
	Uint8 consecutive_counter; \
}; \
\
union { \
	Uint8 __slot2; \
	typing t2; \
	typing old_type; \
}; \
\
union { \
	Uint8 __slot3; \
	bool used; \
}

class PokeItem {
private:
	union {
		Uint32 __block__;
		struct {
			item_ID id;
			PokeItem_MemoryLayout;
		};
	};
	
public:
	inline bool operator<(PokeItem const& other) const {
		return __block__ < other.__block__;
	}

	inline PokeItem() {
		__block__ = 0;
	}

	inline PokeItem(item_ID _id, Uint8 s1=0, Uint8 s2=0, Uint8 s3=0) {
		id = _id;
		__slot1 = s1;
		__slot2 = s2;
		__slot3 = s3;
	}

	inline bool is_placeholder() const {
		return id >= item_ID::__nb_of_item__;
	}

	inline bool is_safety_google() const {
		return id == item_ID::safety_googles;
	}

	inline bool is_protective_pads() const {
		return id == item_ID::protective_pads;
	}

	inline bool is_honey() const {
		return id == item_ID::honey;
	}

	// occurs when given items at random
	// negative : the item is detrimental
	// zero : the item would have no effect on piece
	// positive : the item can be useful
	int usefulness_tier(Board const& board, Piece const* piece) const;

	int priority() const;

	void draw(Surface dest, SDL_Rect* rect = NULL, size s = regular, anchor a = top_left) const;


	void select_holder(Game&, Board&, Piece* holder);

	// called when attacking a piece
	void attack_modifier(Board const&, move_data& data);

	// called when a piece tries to kill you
	void defense_modifier(Board const&, move_data& data);

	// called after attack_modifier is called and defense_modifier of the opponent is called
	void accuracy_modifier(Board const&, move_data& data);
	void crit_modifier(Board const&, move_data& data);

	// called after defense_modifier is called and defense_modifier of the opponent is called
	void evasion_modifier(Board const&, move_data& data);

	std::pair<Uint64, Uint64> get_reachable_and_banned(Board const&, Piece const*, Uint64 const avaible_moves);

	// called when a piece is trying to move to a space marked with item_reachable_mask
	move_data move_to(Board&, Piece* holder, Sint8 target_pos);

	// called every time the piece square gets changed
	void move_with(Board&, Piece* holder, bool was_in_limbo, Sint8 old_pos, bool to_limbo, Sint8 new_pos);

	// called after a move is done
	void after_move_effects(Board&, move_data& data);

	// called when killed or attempted to be killed
	void revenge(Board&, Piece* holder, move_data& data);

	// called when getting the sprites for the promotion selection (on dummy pieces) and after promoting
	void promote(Board&, Piece* new_holder);

	// returns true if avaible_promotion was tampered with
	bool prepare_promotion(Board const&, Piece const* holder, Uint16* avaible_promotion);

	// called after the move is completly done
	// should not have any side effect on the state of the board
	void add_cosmetic(Game&, Board const&, Piece const*, move_data const& data);

	// called after every other function during the move are called
	// all the side effect that would be put in add_cosmetic should be put here
	void update_status(Board&, Piece*, move_data& data);

	inline item_ID get_id() {
		return id;
	}

	inline Uint8 get_slot1() {
		return __slot1;
	}

	inline Uint8 get_slot2() {
		return __slot2;
	}

	inline Uint8 get_slot3() {
		return __slot3;
	}

	inline operator bool() {
		return id != item_ID::none and not is_placeholder();
	}

	inline bool operator == (PokeItem const& other) const {
		return __block__ == other.__block__;
	}

	inline bool operator != (PokeItem const& other) const {
		return __block__ != other.__block__;
	}

	// called whenever there is a change of ownership of the item
	// previous holder may be NULL
	// new_holder may be NULL
	// void change_holder(Board&, Piece* previous_holder, Piece* new_holder);

	

	// called whenever the holder dies
	void to_graveyard(Board&, Piece* holder);

	// called whenever this item is put onto a piece
	void put_on(Board&, Piece*);

	// called whenever the item is removed from the holder during a game
	void remove_from(Board&, Piece*);

	// called whenever the holder status gets updated during the selection phase
	// this include when the holder receive the item during selection too.
	void sync_with_holder(Board&, Piece* holder);

	// remove_from but for the selection phase
	void remove_from_during_selection(Board&, Piece*);

	dstr_t get_name(LANGUAGE);

	dstr_t get_desc(LANGUAGE);
};
static_assert(sizeof(PokeItem) == 4);

// #undef PokeItem_MemoryLayout

#define ITEM_TABLE_LEN 128
extern PokeItem item_table[ITEM_TABLE_LEN];

#ifndef __NOT_DEFINE_CONST
extern std::set<PokeItem> rng_dependant_items;
#endif

extern int number_of_drawed_terashard;
extern int number_of_drawed_resistance_berry;

PokeItem const static NO_ITEM;

