#pragma once

#include "SDL+.h"
#include "poketyping.h"
#include "assets.h"

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
private:
	ItemClass(PokeItem* (*)(Piece*, ItemClass&), int (*usefulness_tier)(Piece*), void (*draw)(Surface, SDL_Rect*, size, anchor), const char* name_[], const char** description, bool RNG);

	PokeItem* (*constructor)(Piece*, ItemClass&);
	void (*_draw)(Surface, SDL_Rect*, size, anchor);

	friend void* init_item_table(void);
public:
	ItemClass();

	item_placeholder_type type;
	bool is_avaible;
	bool is_RNG_dependant;
	const char** description;
	const char** name;

	int (*usefulness_tier)(Piece*);
	inline void draw(Surface surface, SDL_Rect* rect = NULL, size s=regular, anchor a = top_left) {
		_draw(surface, rect, s, a);
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

// the property of an item can be both determined by virtual methods and with id for more specific effects
class PokeItem {
protected:
	bool used;
public:
	ItemClass& cls;
	static const bool RNG = true;

	const item_id id;
	const int priority;
	Piece* holder;

	PokeItem(Piece* piece, item_id id_, int prio, ItemClass& IC);
	PokeItem(Piece* piece, ItemClass& IC);
	
	void consume();
	
	static void draw(Surface dest, SDL_Rect* rect = NULL, anchor a = top_left) { ; }

	static Surface get_sprite() { return NULL; }

	// occurs when given items at random
	// negative : the item is detrimental
	// zero : the item would have no effect on piece
	// positive : the item can be useful
	static int usefulness_tier(Piece* piece) { return 0; }

	// called when attacking a piece
	virtual void attack_modifier(move_data& data) {}

	// called when a piece tries to kill you
	virtual void defense_modifier(move_data& data) {}

	// called after attack_modifier is called and defense_modifier of the opponent is called
	virtual void accuracy_modifier(move_data& data) {}
	virtual void crit_modifier(move_data& data) {}

	// called after defense_modifier is called and defense_modifier of the opponent is called
	virtual void evasion_modifier(move_data& data) {}

	// called when a piece cannot move through base rules to target to know whether the item allow the move
	virtual bool is_move_allowed(Square& target) { return false; }
	// called when a piece can move through base rules to target whether the item ban the move
	virtual bool is_move_disallowed(Square& target) { return false; }
	// called when a piece is trying to move to a space it cannot reach trough normal rules
	virtual move_data move_to(Square& target);

	// called after a move is done
	virtual void after_move_effects(move_data& data) {}

	// called when killed or attempted to be killed
	virtual void revenge(move_data& data) {}

	// called when getting the sprites for the promotion selection (on dummy pieces) and after promoting
	virtual void promote(bool not_dummy = true) {}
	// returns true to indicate the item will handle part of the promotion process (such as the creation of buttons to promote any piece)
	// do whatever you want to the promotion process in this method
	virtual bool prepare_promotion() { return false; }

	virtual void add_cosmetic(move_data& data) { ; }
};

#define HOLDS_SAFETY_GOOGLES(piece) (piece != NULL and piece->item != NULL and piece->item->id == item_id::safety_googles)
#define HOLDS_PROTECTIVE_PADS(piece) (piece != NULL and piece->item != NULL and piece->item->id == item_id::protective_pads)
#define HOLDS_HONEY(piece) (piece != NULL and piece->item != NULL and piece->item->id == item_id::honey)

#define IS_SAFETY_GOOGLES(item) ((item) != NULL and (item)->id == item_id::safety_googles)
#define IS_PROTECTIVE_PADS(item) ((item) != NULL and (item)->id == item_id::protective_pads)
#define IS_HONEY(item) ((item) != NULL and (item)->id == item_id::honey)



#define NB_OF_ITEMS 64
extern ItemClass item_table[NB_OF_ITEMS];

extern void* _table_initialize;