#pragma once

#include "SDL+.h"

class Piece;
class PokeItem;
enum class item_id : short;

enum item_placeholder_type {
	normal_item,
	newline_item,
	space_item,
	terminator_item,
};

struct ItemClass {
	ItemClass();

	ItemClass(PokeItem* (*)(Piece*, ItemClass&), bool (*would_be_useful)(Piece*), void (*draw)(Surface, SDL_Rect*, anchor), void (*)(char*&), bool RNG);

	PokeItem* (*constructor)(Piece*, ItemClass&);
	void (*_draw)(Surface, SDL_Rect*, anchor);
	void (*_update_description)(char*&);

	friend void* init_item_table(void);
public:
	item_placeholder_type type;
	bool is_avaible;
	bool is_RNG_dependant;
	char* description;
	
	bool (*would_be_useful)(Piece*);
	inline void draw(Surface s, SDL_Rect* r = NULL, anchor a = top_left) {
		_draw(s, r, a);
	}

	inline void update_description() {
		if (type == normal_item)
			_update_description(description);
	}

	PokeItem* operator()(Piece*);
};

#include "poketyping.h"
#include "piece.h"

enum class item_id : short {
	no_item = 0,
	basic,
	evolution_stone,
	resistance_berry,
	imunity,

	// - +1/16 crit chance for each consecutive move with this piece
	metronome,
	// - if a bug type can attack you, they cannot move elsewhere
	honey, 

	// - each rng roll is done with dnd advantage
	loaded_dice, 

	// - ignore items while attacking
	protective_pads,
	// - ignore items while attacked
	safety_googles,
	// - the user cant promote
	everstone,
	// - removes potential ground immunity
	// - the user cannot castle
	// - the user cannot perform a pawn double step
	iron_ball,
};


typing operator-(const item_id x, const item_id y);

extern const char* name_table[];

// the property of an item can be both determined by virtual methods and with id for more specific effects
class PokeItem {
public:
	ItemClass& cls;
	static const bool RNG = true;

	const item_id id;
	const char* name;
	const int priority;
	Piece* holder;

	PokeItem(Piece* piece, item_id id_, int prio, ItemClass& IC);
	PokeItem(Piece* piece, ItemClass& IC);
	
	void consume(const char* txt = NULL);
	
	static void draw(Surface dest, SDL_Rect* rect = NULL, anchor a = top_left) { ; }
	static void update_description(char*& txt) { ; }

	static Surface get_sprite() { return NULL; }

	// occurs when given items at random
	static bool would_be_useful(Piece* piece) { return true; }

	// called when attacking a piece
	virtual void attack_modifier(effectiveness& e, Piece* defenser) {}

	// called when a piece tries to kill you
	virtual void defense_modifier(effectiveness& e, Piece* attacker) {}

	// called after attack_modifier is called and defense_modifier of the opponent is called
	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness matchup) {}
	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness matchup) {}

	// called after defense_modifier is called and defense_modifier of the opponent is called
	virtual void evasion_modifier(double& miss_rate, Piece* attacker, effectiveness matchup) {}

	// called when a piece cannot move through base rules to target to know whether the item allow the move
	virtual bool is_move_allowed(Square& target) { return false; }
	// called when a piece can move through base rules to target whether the item ban the move
	virtual bool is_move_disallowed(Square& target) { return false; }
	// called when a piece is trying to move to a space it cannot reach trough normal rules
	virtual move_data move_to(Square& target);

	// called after a move is done
	virtual void after_move_effects(Piece* defenser, move_data& data) {}

	// called when killed
	virtual void revenge(Piece* attacker, move_data& data) {}

	// called when getting the sprites for the promotion selection (on dummy pieces) and after promoting
	virtual void promote(bool not_dummy = true) {}
	// returns true to indicate the item will handle part of the promotion process (such as the creation of buttons to promote any piece)
	// do whatever you want to the promotion process in this method
	virtual bool prepare_promotion() { return false; }

};



#define NB_OF_ITEMS 64
extern ItemClass item_table[NB_OF_ITEMS];

extern void* _table_initialize;