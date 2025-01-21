#include "item.h"
#include "game.h"
#include "board.h"

#define DESCRIPTION_MAX_SIZE 256

typing operator-(const item_id x, const item_id y) {
	return (typing)((short)x - (short)y);
}

move_data PokeItem::move_to(Square& target) {
	return holder->move_to(target);
}

const char* name_table[] = {
	"",
	"",
	"",
	"fire stone",
	"water stone",
	"leaf stone",
	"volt stone",
	"ice stone",
	"",
	"",
	"",
	"",
	"",
	"shiny stone",
	"",
	"",
	"",
	"",
	"dusk stone",
	"",
	"soothe bell",

	"leppa berry",
	""
	"air balloon",
	"cell battery",
	"snowball",
	"absorb bulb",
	"",
	"bright powder",

	"expert belt",
	"wide lens",
	"blunder policy",

	"life orb",
	"metronome",
	"light ball",
	"lucky punch",
	"scope lens",

	"honey",

	"loaded_dice",

	"protective pads",
	"safety googles",

	"shed_shell",

	"normal gem",

	"heavy duty boots",

	"eviolite",
	"",
	"rocky_helmet",
	"red_card",
	"sticky_barb",
	"",
	"ring target",
	"everstone",
	"iron ball",
	"assault vest",
	"baguette",

};

PokeItem::PokeItem(Piece* piece, item_id id_, int prio, ItemClass& IC) : id(id_), holder(piece), priority(prio), cls(IC) {
	name = NULL;
}

PokeItem::PokeItem(Piece* piece, ItemClass& IC) : id(item_id::basic), holder(piece), priority(0), cls(IC) {
	;
}


void PokeItem::consume(const char* txt) {
	if (txt != NULL) {
		game.add_textbox(txt);
	}
	holder->item = NULL;
	delete this;
}

#define desc_add(str) \
{ \
	const char* __add_str__ = (str); \
	if (__add_str__ != NULL) \
		for (int __add_counter__ = 0; __add_str__[__add_counter__]; __add_counter__++) { \
			description[end++] = __add_str__[__add_counter__]; \
		} \
	description[end] = '\0'; \
	if (end >= 256) \
		PRINT_DEBUG("BANDE DE KON C'EST PAS POSSIBLE D'ETRE AUSSI CON, MEME TRUMP C MIEUX"); \
}

const char* evolution_stone_names[(long)LANGUAGE::NB_OF_LANGUAGE][18] = {
	{ // FRENCH
		"Pierre Stase",
		"Pierre Feu",
		"Pierre Eau",
		"Pierre Plante",
		"Pierre Foudre",
		"Pierre Glace",
		"",
		"",
		"",
		"",
		"Pierre Eclat",
		"",
		"Obsidienne",
		"",
		"Ecaille Draco",
		"Pierre Nuit",
		"Peau Métal",
		"Grelot Zen"
	},
	{ // ENGLISH
		"Everstone",
		"Fire Stone",
		"Water Stone",
		"Leaf Stone",
		"Thunder Stone",
		"Ice Stone",
		"",
		"",
		"",
		"",
		"Shiny Stone",
		"",
		"Black Augurite",
		"",
		"Dragon Scale",
		"Dusk Stone",
		"Metal Coat",
		"Soothe Bell"
	},
	{ // GERMAN
		"Ewigstein",
		"Feuerstein",
		"Wasserstein",
		"Blattstein",
		"Donnerstein",
		"Eisstein",
		"",
		"",
		"",
		"",
		"Leuchtstein",
		"",
		"Schwarzaugit",
		"",
		"Drachenschuppe",
		"Finsterstein",
		"Metallmantel",
		"Sanftglocke"
	},
	{ // SPANISH
		"Piedra Eterna",
		"Piedra Fuego",
		"Piedra Agua",
		"Piedra Hoja",
		"Piedra Trueno",
		"Piedra Hielo",
		"",
		"",
		"",
		"",
		"Piedra Dia",
		"",
		"Mineral Negro",
		"",
		"Escama Dragón",
		"Piedra Noche",
		"Revestimiento Metálico",
		"Cascabel Alivio"
	},
};

template<typing old_type, typing new_type>
class EvolutionStone : public PokeItem {
public:
	static const bool RNG = false;
	EvolutionStone(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::evolution_stone, 0, IC) {
		;
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(0, new_type * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	static bool would_be_useful(Piece* piece) {
		return piece->id == Pawn::cls.id && piece->type == old_type;
	}

	virtual bool prepare_promotion() {
		if (holder->type == old_type) {
#define ctor(Class) [](Board& b, piece_color color, Square* sq, typing type, PokeItem* item) -> Piece* { return new Class(b, color, sq, new_type, NULL); }

			game.buttons->add(new PromotionButton(ctor(Queen), 7.0, 5.5));
			game.buttons->add(new PromotionButton(ctor(Rook), 8.0, 5.5));
			game.buttons->add(new PromotionButton(ctor(Bishop), 9.0, 5.5));
			game.buttons->add(new PromotionButton(ctor(Knight), 10.0, 5.5));
#undef ctor
			return true;
		}
		return false;

	}

	virtual void promote(bool not_dummy=true) {
		if (holder->type == new_type) {
			consume();
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];
		int end = 0;

		desc_add(evolution_stone_names[(int)game.language][new_type]);
		desc_add(" :\n");

		switch (game.language) {
		case LANGUAGE::FRENCH:
			desc_add(" - Uniquement pour les pions.\n")
			desc_add(" - Si l'utilisateur est de type ");
			desc_add(type_str[(int)game.language][old_type]);
			desc_add(",\n   il deviendra de type ");
			desc_add(type_str[(int)game.language][new_type]);
			desc_add(" en evoluant.\n");

			break;
		case LANGUAGE::ENGLISH:
			desc_add(" - Only for pawns.\n");
			desc_add(" - If the user is a ");
			desc_add(type_str[(int)game.language][old_type]);
			desc_add(" type,\n   he will become a ");
			desc_add(type_str[(int)game.language][new_type]);
			desc_add(" type when evolving.\n");
			break;
		}
		
	}
};

const char* resistance_berry_names[(int)LANGUAGE::NB_OF_LANGUAGE][18] = {
	{ // FRENCH
		"Baie Zalis",
		"Baie Chocco",
		"Baie Pocpoc",
		"Baie Ratam",
		"Baie Parma",
		"Baie Nanone",
		"Baie Pomroz",
		"Baie Kébia",
		"Baie Jouca",
		"Baie Cobaba",
		"Baie Yapap",
		"Baie Charti",
		"Baie Sédra",
		"Baie Fraigo",
		"Baie Lampou",
		"Baie Lampou",
		"Baie Babiri",
		"Baie Selro",
	},
	{ // ENGLISH
		"Chilan Berry",
		"Occa Berry",
		"Passho Berry",
		"Rindo Berry",
		"Wacan Berry",
		"Yache Berry",
		"Chople Berry",
		"Kebia Berry",
		"Shuca Berry",
		"Coba Berry",
		"Papaya Berry",
		"Tanga Berry",
		"Charti Berry",
		"Kasib Berry",
		"Haban Berry",
		"Colbur Berry",
		"Babiri Berry",
		"Roseli Berry"
	},
};



template <typing type>
class ResistanceBerry : public PokeItem {
public:
	static const bool RNG = false;
	ResistanceBerry(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::resistance_berry, -1, IC) {
		static_assert(type != typeless);
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(ITEM_SIZE, type * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	static bool would_be_useful(Piece* piece) {
		const effectiveness e = typechart[type][piece->type];
		return e == super_effective or (type == normal and e == neutral);
	}

	virtual void defense_modifier(effectiveness& e, Piece* attacker) {
		if (attacker->type == type) {
			if (e == super_effective or type == normal)
				--e;
			else {
				--e.intensity;
			}
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		desc_add(resistance_berry_names[(int)game.language][type]);
		desc_add(" :\n");
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			if (type == normal) {
				desc_add(" - Its user gains a normal type resistance.\n");
			}
			else {
				desc_add(" - Its user loses a weakness to ")
				desc_add(type_str[(int)game.language][type]);
				desc_add(".\n");
			}
			break;
		case LANGUAGE::FRENCH:
			if (type == normal) {
				desc_add(" - Son utilisateur gagne une resistance contre le type normal.\n");
			}
			else {
				desc_add(" - Son utilisateur perd une faiblesse contre le type ");
				desc_add(type_str[(int)game.language][type]);
				desc_add(".\n");
			}
			break;
		}
	}
};

class RingTarget : public PokeItem {
	const char* _name = "Ring target";
public:
	static const bool RNG = false;
	RingTarget(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, 0x7FFFFFFF, IC) {
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(4 * ITEM_SIZE, 0, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	static bool would_be_useful(Piece* piece) {
		for (typing t = normal; t <= fairy; t++) {
			if (typechart[t][piece->type] == immune)
				return true;
		}
		return false;
	}

	virtual void defense_modifier(effectiveness& e, Piece* attacker) {
		if (e.main == immune) {
			e.main = neutral;
			if (e.intensity > 0) {
				e.intensity--;
				e.main = super_effective;
			}
			else if (e.intensity < 0) {
				e.intensity++;
				e.main = not_very_effective;
			}
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Ring Target :\n");
			desc_add(" - Removes all the immunity of its user.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Point de Mire :\n");
			desc_add(" - Retire toutes les immunités de son utilisateur.\n");
			break;
		}
	}
};


static const char* const immunity_item_names[(int)LANGUAGE::NB_OF_LANGUAGE][18] = {
		{ // FRENCH
			"",
			"Boule de neige",
			"Bulbe",
			"",
			"Pile",
			"",
			"",
			"",
			"Ballon",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{ // ENGLISH
			"",
			"Snowball",
			"Water Bulb",
			"",
			"Cell Battery",
			"",
			"",
			"",
			"Air Balloon",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		}
};

template <typing type>
class ImmunityItem : public PokeItem {
	
public:
	static const bool RNG = false;
	ImmunityItem(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::imunity, 1, IC) {
		;
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(2 * ITEM_SIZE, (type) * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	static bool would_be_useful(Piece* piece) {
		return typechart[type][piece->type] != immune;
	}

	virtual void defense_modifier(effectiveness& e, Piece* attacker) {
		if (attacker->type == type) {
			e = immune;
			char buffer[256];
			strcat_s(buffer, holder->name);
			switch (type) {
			case ground:
				strcat_s(buffer, "\'s Balloon\nPoped.");
				break;
			case electric:
				strcat_s(buffer, "\'s Battery\nSurchaged");
				break;
			case ice:
				strcat_s(buffer, "\'s Snowball\nMelted");
				break;
			case water:
				strcat_s(buffer, "\'s Bulb\ndied");
				break;
			}
			game.add_textbox(buffer);
			consume();
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		desc_add(immunity_item_names[(int)game.language][type]);
		desc_add(" :\n");
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - One time use.\n");
			desc_add(" - Its user is immune to ");
			desc_add(type_str[(int)game.language][type]);
			desc_add(".\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Utilisable une unique fois.\n");
			desc_add(" - Son utilisateur est immunisé contre le type ");
			desc_add(type_str[(int)game.language][type]);
			desc_add(".\n");
			break;
		}
	}
};

class ExpertBelt : public PokeItem {
public:
	ExpertBelt(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, -2, IC) {
		;
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 13 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	static bool would_be_useful(Piece* piece) {
		for (typing type = normal; type <= fairy; type++) {
			if (typechart[piece->type][type] == super_effective)
				return true;
		}
		return false;
	}

	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness e) {
		if (e.main == super_effective)
			miss_rate = -INFINITY;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Expert Belt :\n");
			desc_add(" - Its user cannot miss super effective moves.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Ceinture Pro :\n");
			desc_add(" - Son utilisateur ne peux pas echoué d'attaque super efficace.\n");
			break;
		}
	}
};


class BlunderPolicy : public PokeItem {
public:
	BlunderPolicy(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, 0, IC) {
		;
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 9 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness e) {
		miss_rate = -INFINITY; // cannot miss
		consume();
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Blunder Policy :\n");
			desc_add(" - One time use.\n");
			desc_add(" - Its user cannot miss his first attack.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Assurance Echec :\n");
			desc_add(" - Utilisable une unique fois.\n")
			desc_add(" - Son utilisateur ne peut pas echoué sa premiere attaque.\n");
			break;
		}
	}
};


#define defctor(name) name(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {}
class BrightPowder : public PokeItem {
public:
	defctor(BrightPowder);

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 15 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void evasion_modifier(double& miss_rate, Piece* attacker, effectiveness e) {
		miss_rate = 1 - ((1 - miss_rate) * 1.1);
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Bright powder :\n");
			desc_add(" - Increase slightly the evasion of its user.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Poudre Claire :\n");
			desc_add(" - Augmente légerement le taux d'esquive de son utilisateur.\n");
			break;
		}
	}
};


class LifeOrb : public PokeItem {
public:
	defctor(LifeOrb);

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 5 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness e) {
		crit_rate *= 2;
	}

	virtual void after_move_effects(Piece* defenser, move_data& data) {
		if (defenser != NULL and not data.suicide && game.RNG() <= 0.1) {
			holder->square->to_graveyard();
			char buffer[256];
			strcat_s(buffer, holder->name);
			strcat_s(buffer, "\nDied to life orb");
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Life Orb :\n");
			desc_add(" - Its user has a 1/10 chance to die each time he attacks.\n");
			desc_add(" - Greatly increase the critical hit rate of its user.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Orbe vie :\n");
			desc_add(" - Son Utilisateur a 1/10 chance de mourir a chaque fois qu'il attaque.\n");
			desc_add(" - Augmente énormément le taux de coup critique de son utilisateur.\n");
			break;
		}
	}
};


class LoadedDice : public PokeItem {
public:
	LoadedDice(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, -1, IC) {
		;
	}

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 6 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness e) {
		if (crit_rate < 0)
			crit_rate = -INFINITY;
		else
			crit_rate = sqrt(crit_rate); // the equivallent of a roll with dnd advantage
	}

	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness e) {
		if (miss_rate < 0) {
			miss_rate = -INFINITY;
		}
		else {
			miss_rate = 1 - sqrt(1 - miss_rate); // the equivallent of a roll with dnd advantage
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Loaded dice :\n");
			desc_add(" - Each rng roll affecting its user is perform with \"advantage\".\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Dé pipé :\n");
			desc_add(" - Chaque utilisation d'aléatoire concernant son utilisateur est effectué avec \"avantage\".\n");
			break;
		}
	}
};

class RockyHelmet : public PokeItem {
public:
	defctor(RockyHelmet);
	
	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 0 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void revenge(Piece* attacker, move_data& data) {
		if (not data.suicide && game.RNG() < 1.0 / 6.0) {
			attacker->square->to_graveyard();
			char buffer[256];
			strcat_s(buffer, attacker->name);
			strcat_s(buffer, "\nDied to Rcky Hlmt");
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Rocky helmet :\n");
			desc_add(" - If its user die to an enemy piece,\n");
			desc_add("     this piece has a 1/6 chance to die.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add("Casque brut :\n");
			desc_add(" - Si son utilisateur meurt d'une piece enemie,\n");
			desc_add("     cette piece a 1/6 chance de mourrir.\n");
			break;
		}
	}
};


class RedCard : public PokeItem {
public:
	static const bool RNG = false;

	defctor(RedCard);

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 1 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void revenge(Piece* attacker, move_data& data) {
		if (data.move_again and not data.suicide) {
			data.move_again = false;
			attacker->move_to(*data.begin_square);
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Red card :\n");
			desc_add(" - If its user die to an attack that results in a bonus move,\n");
			desc_add("     the attacker gets send back to its starting square and the bonus move is canceled.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add("Carton rouge :\n");
			desc_add(" - Si son utilisateur meurt d'une attaque qui résulte en un coup bonus,\n");
			desc_add("     l'attaquant revient sur sa case de départ, et le coup bonus est annulé.\n");
			break;
		}
	}
};

#define defdraw(x, y) static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) { \
	SDL_Rect r((x) * ITEM_SIZE, (y) * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE); \
	dest.blit(item_sheet, pos, &r, a); \
}
class StickyBarbs : public PokeItem {
public:
	defctor(StickyBarbs);

	static void draw(Surface dest, SDL_Rect* pos = NULL, anchor a = top_left) {
		SDL_Rect r(3 * ITEM_SIZE, 2 * ITEM_SIZE, ITEM_SIZE, ITEM_SIZE);
		dest.blit(item_sheet, pos, &r, a);
	}

	virtual void revenge(Piece* attacker, move_data& data) {
		attacker->item->consume();
		attacker->item = this;
		holder->resize_sprite();
		holder = attacker;
	}

	virtual void after_move_effects(move_data& data, Piece* defenser) {
		if (not data.suicide && game.RNG() < 0.125) {
			holder->square->to_graveyard();
			char buffer[256];
			strcat_s(buffer, holder->name);
			strcat_s(buffer, "\nDied to StckyBrbs");
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Sticky barbs :\n");
			desc_add(" - Its user has 1/10 chance to die each time it moves\n");
			desc_add(" - If its user is killed by an enemy piece,\n");
			desc_add("     the piece gets its item replaced by sticky barbs.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Piquants :\n");
			desc_add(" - Son utilisateur a 1/10 chance de mourrir quand il se déplace\n");
			desc_add(" - Si son utilisateur est tué par une piece enemie,\n");
			desc_add("     cette piece voit son objet remplacer par des piquants.\n");
			break;
		}
	}
};

class AssaultVest : public PokeItem {
public:
	static const bool RNG = false;

	defctor(AssaultVest);

	defdraw(4, 1);
	virtual bool is_move_disallowed(Square& target) {
		return target.piece != NULL;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Assault vest :\n");
			desc_add(" - Its user cannot move unless it results in a capture.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Veste de combat:\n");
			desc_add(" - Son utilisateur ne peux pas se deplacer, sauf pour une capture.\n");
			break;
		}
	}
};


class ShedShell : public PokeItem {
public:
	static const bool RNG = false;

	defctor(ShedShell);

	defdraw(3, 16);

	static bool would_be_useful(Piece* piece) {
		return piece->id != Knight::cls.id;
	}
	virtual bool is_move_allowed(Square& target) {
		{	
			Knight K = Knight(game.board, holder->color, holder->square, holder->type, NULL);
			if (not K.base_do_control(target))
				return false;
		}
		for (Square& square : game.board) {
			if (holder->base_can_move_to(square)) {
				return false;
			}
		}

		return holder->square->is_controlled_by(not holder->color);
	}

	virtual move_data move_to(Square& target) {
		move_data data = holder->move_to(target);
		char buffer[256];
		strcat_s(buffer, holder->name);
		strcat_s(buffer, "\nfled successfuly");
		consume(buffer);
		return data;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Shed shell :\n");
			desc_add(" - If each move of its user puts him in danger,\n");
			desc_add("   he can perform a knight move.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add("Carapace mue :\n");
			desc_add(" - Si tous les déplacements de l'utilisateur le mette en danger,\n");
			desc_add("   il peut effectuer un deplacement de cavalier.\n")
			break;
		}
	}
};


class ScopeLens : public PokeItem {
public:
	defctor(ScopeLens);

	defdraw(3, 12);
	
	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness e) {
		crit_rate *= 1.3;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Scope lens :\n");
			desc_add(" - Increase slightly the critical hit rate of its user.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Lentilscope :\n");
			desc_add(" - Augmente légerement le taux de coup critique de son utilisateur.\n");
			break;
		}
	}
};


class WideLens : public PokeItem {
public:
	defctor(WideLens);

	defdraw(3, 11);

	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness e) {
		miss_rate /= 2;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("WideLens :\n");
			desc_add(" - Increase the accuracy of its user.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Loupe :\n");
			desc_add(" - Augmente la precision de son utilisateur.\n");
			break;
		}
	}
};


class Honey : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(2, bug);

	Honey(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::honey, 0, IC) {
		;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Honey :\n");
			desc_add(" - Opposing bug types cannot resist the attraction of honey,\n");
			desc_add("   If they can reach some, they wont be able to move elsewhere.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Miel :\n");
			desc_add(" - Les types insectes adverse ne peuvent resister a l'envie de miel,\n");
			desc_add("   S'ils peuvent en atteindre ils ne pourront bouger ailleurs.\n");
			break;
		}
	}
};

class ProtectivePads : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(3, 8);

	ProtectivePads(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::protective_pads, 0, IC) {
		;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Protective pads :\n");
			desc_add(" - Ignore opposing items when its user is attacking.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Parre-Effet :\n");
			desc_add(" - Ignore l'objet du defenseur quand son porteur attaque.\n");
			break;
		}
	}
};


class SafetyGoogles : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(3, 7);

	SafetyGoogles(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::safety_googles, 0, IC) {
		;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Safety googles :\n");
			desc_add(" - Ignores opposing items when its user is attacked.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Lunettes filtre :\n");
			desc_add(" - Ignore les objets adverse quand son utilisateur est attaqué.\n");
			break;
		}
	}
};


class NormalGem : public PokeItem {
public:
	static const bool RNG = false;

	defctor(NormalGem);

	defdraw(2, normal);

	static bool would_be_useful(Piece* piece) {
		return piece->type == normal;
	}

	virtual void attack_modifier(effectiveness& e, Piece* defenser) {
		if (holder->type == normal)
			e++;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Normal gem :\n");
			desc_add(" - Only for normal type pieces.\n");
			desc_add(" - One time use.\n");
			desc_add(" - The first attack of the user will be super effective\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Gemme normale :\n");
			desc_add(" - Uniquement pour les pieces de type normal\n");
			desc_add(" - Utilisable une seule fois.\n");
			desc_add(" - La première attaque de son porteur sera super efficace.\n")
			break;
		}
	}
};


class LeppaBerry : public PokeItem {
public:
	static const bool RNG = false;

	defctor(LeppaBerry);

	defdraw(3, 10);

	static bool would_be_useful(Piece* piece) {
		return piece->id == King::cls.id or piece->id == Pawn::cls.id;
	}

	virtual bool is_move_allowed(Square& target) {
		if (not holder->has_already_move)
			return false;
		holder->has_already_move = false;
		bool res = false;
		if (holder->id == Pawn::cls.id) {
			res = (dynamic_cast<Pawn*>(holder))->can_double_step(target);
		}
		else if (holder->id == King::cls.id) {
			res = (dynamic_cast<King*>(holder)->can_castle(target));
		}
		holder->has_already_move = true;
		return res;
	}

	virtual move_data move_to(Square& target) {
		move_data data = holder->move_to(target);
		consume();
		return data;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Leppa berry :\n");
			desc_add(" - One time use.\n");
			desc_add(" - Allow its user to perform special move a second time.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Baie mepo :\n");
			desc_add(" - Utilisable une fois.\n");
			desc_add(" - Permet a son utilisateur de refaire un deplacement special.\n");
			break;
		}
	}
};


class BlackSludge : public PokeItem {
public:
	defctor(BlackSludge);

	defdraw(2, poison);

	static bool would_be_useful(Piece* piece) {
		return piece->type != poison;
	}

	virtual void after_move_effects(Piece* defenser, move_data& data) {
		if (holder->type != poison and game.RNG() < 0.125) {
			holder->square->to_graveyard();
			char buffer[256];
			strcat_s(buffer, holder->name);
			strcat_s(buffer, "\nDied to BlckSldge");
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Black sludge :\n");
			desc_add(" - If not poison type,\n")
			desc_add("   1/8 chance to kill its user each time he moves.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Boue noire :\n");
			desc_add(" - S'il n'est pas de type poison,\n");
			desc_add("   1/8 chance de tuer son utilisateur quand il se déplace\n");
			break;
		}
	}
};


class HeavyDutyBoots : public PokeItem {
public:
	static const bool RNG = false;
	defctor(HeavyDutyBoots);

	defdraw(2, rock);

	static bool would_be_useful(Piece* piece) {
		return typechart[piece->type][rock] == not_very_effective;
	}

	virtual void attack_modifier(effectiveness& e, Piece* defenser) {
		if (defenser != NULL and defenser->type == rock) {
			if (e.main == not_very_effective) {
				e++;
			}
			else {
				e.intensity++;
			}
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Heavy duty boots :\n");
			desc_add(" - The holder wont be resited by rock type anymore.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Grosses bottes :\n");
			desc_add(" - L'utilisateur ne sera plus resité par le type roche.\n");
			break;
		}
	}
};


class LightBall : public PokeItem {
public:
	defctor(LightBall);

	defdraw(3, electric);

	static bool would_be_useful(Piece* piece) {
		return piece->type == electric and piece->id == Pawn::cls.id;
	}
	
	virtual void crit_modifier(double& crit_rate, Piece* defenser) {
		if (would_be_useful(holder))
			crit_rate *= 3;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Light Ball :\n");
			desc_add(" - Only for electric type pawns.\n");
			desc_add(" - Increases critical hit chance.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add("Balle lumiere :\n")
			desc_add(" - Uniquement pour les pions de type électrique.\n");
			desc_add(" - Augmente le taux de coup critique.\n")
			break;
		}
	}

};


class Eviolite : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Eviolite);

	defdraw(3, 3);

	static bool would_be_useful(Piece* piece) {
		return piece->id == Pawn::cls.id;
	}

	virtual void defense_modifier(effectiveness& e, Piece* attacker) {
		if (would_be_useful(holder)) {
			if (e == super_effective) {
				e--;
			}
			else {
				e.intensity--;
			}
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Eviolite :\n")
			desc_add(" - Only for pawns.\n")
			desc_add(" - +1 resistance to all weaknesses.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Evoluroc :\n");
			desc_add(" - Uniquement pour les pions.\n");
			desc_add(" - +1 de resistance a chaques faiblesses.\n");
			break;
		}
	}
};


class Baguette : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Baguette);

	defdraw(4, 3);

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[64];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Baguette : \n");
			desc_add(" - A baguette.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Baguette : \n");
			desc_add(" - Une baguette.\n");
			break;
		case LANGUAGE::GERMAN:
			desc_add("Baguette : \n");
			desc_add(" - Ein Baguette.\n");
			break;
		case LANGUAGE::SPANISH:
			desc_add("Baguette : \n");
			desc_add(" - Una baguette.\n");
			break;
		}
	}
};


class LeadersCrest : public PokeItem {
public:
	static const bool RNG = false;
	defctor(LeadersCrest);

	defdraw(2, steel);

	static bool would_be_useful(Piece* piece) {
		return (piece->id == Pawn::cls.id) and piece->type == dark or piece->type == steel;
	}

	virtual bool prepare_promotion() {
		if (holder->type == dark or holder->type == steel) {
#define ctor(Class) [](Board& b, piece_color color, Square* sq, typing type, PokeItem* item) -> Piece* { return new Class(b, color, sq, type, NULL); }
			game.buttons->add(new PromotionButton(ctor(King), 6.5, 5.5));
			game.buttons->add(new PromotionButton(Queen::cls, 7.5, 5.5));
			game.buttons->add(new PromotionButton(Rook::cls, 8.5, 5.5));
			game.buttons->add(new PromotionButton(Bishop::cls, 9.5, 5.5));
			game.buttons->add(new PromotionButton(Knight::cls, 10.5, 5.5));
#undef ctor
			return true;
		}
		return false;
	}

	virtual void promote(bool) {
		if (holder->id == King::cls.id) {
			consume();
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];
		
		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add("Leader's Crest :\n");
			desc_add(" - Only for steel/dark type pawns.\n")
			desc_add(" - Allow promotion to King.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add("Embleme du General :\n")
			desc_add(" - Uniquement pour les pions de type acier/tenebres.\n");
			desc_add(" - Permet la promotion en Roi.\n");
			break;
		}
	}
};


void* init_item_table() {
	int counter = 0;

#define Itemize(class) item_table[counter++] = ItemClass([](Piece* piece, ItemClass& IC) -> PokeItem* { return new class(piece, IC); }, class::would_be_useful, class::draw, class::update_description, class::RNG);
#define AddPlaceHolder(n) counter += n
#define EndLine() item_table[counter++].type = newline_item
#define End() item_table[counter].type = terminator_item
#define _ ,
	Itemize(EvolutionStone<normal _ fire>);
	

	Itemize(EvolutionStone<normal _ water>);
	Itemize(EvolutionStone<normal _ electric>);
	Itemize(EvolutionStone<normal _ grass>);
	Itemize(EvolutionStone<normal _ ice>);
	Itemize(EvolutionStone<normal _ psychic>);
	Itemize(EvolutionStone<normal _ dark>);
	Itemize(EvolutionStone<normal _ fairy>);
	Itemize(EvolutionStone<water _ dragon>);
	Itemize(EvolutionStone<bug _ steel>);
	Itemize(EvolutionStone<bug _ rock>);
	Itemize(LeadersCrest);

	EndLine();

	Itemize(ResistanceBerry<normal>);
	Itemize(ResistanceBerry<fire>);
	Itemize(ResistanceBerry<water>);
	Itemize(ResistanceBerry<grass>);
	Itemize(ResistanceBerry<electric>);
	Itemize(ResistanceBerry<ice>);
	Itemize(ResistanceBerry<fighting>);
	Itemize(ResistanceBerry<poison>);
	Itemize(ResistanceBerry<ground>);
	Itemize(ResistanceBerry<flying>);
	Itemize(ResistanceBerry<psychic>);
	Itemize(ResistanceBerry<bug>);
	Itemize(ResistanceBerry<rock>);
	Itemize(ResistanceBerry<ghost>);
	Itemize(ResistanceBerry<dragon>);
	Itemize(ResistanceBerry<dark>);
	Itemize(ResistanceBerry<steel>);
	Itemize(ResistanceBerry<fairy>);
	Itemize(HeavyDutyBoots);
	Itemize(NormalGem);
	
	EndLine();

	// return NULL;

	Itemize(RingTarget);
	Itemize(Eviolite);
	Itemize(ImmunityItem<ground>);
	Itemize(ImmunityItem<electric>);
	
	EndLine();

	Itemize(ExpertBelt);
	Itemize(BlunderPolicy);
	Itemize(BrightPowder);
	Itemize(WideLens);
	Itemize(ScopeLens);
	Itemize(LifeOrb);
	Itemize(LoadedDice);
	
	//return NULL;

	EndLine();

	Itemize(ShedShell);
	Itemize(LeppaBerry);
	Itemize(AssaultVest);
	Itemize(Honey);

	EndLine();

	Itemize(ProtectivePads);
	Itemize(SafetyGoogles);

	Itemize(RockyHelmet);
	Itemize(RedCard);
	Itemize(StickyBarbs);

	Itemize(BlackSludge);
	Itemize(Baguette);

	End();

	return NULL;
#undef Itemize
#undef _
}

ItemClass::ItemClass() {
	constructor = [](Piece* piece, ItemClass& IC) -> PokeItem* { return NULL; };
	would_be_useful = NULL;
	is_avaible = false;
	type = space_item;
	is_RNG_dependant = false;
	description = NULL;
}

ItemClass::ItemClass(PokeItem* (*ctor)(Piece*, ItemClass&), bool (*w)(Piece*), void (*d)(Surface, SDL_Rect*, anchor), void (*update_desc)(char*&), bool RNG) {
	constructor = ctor;
	would_be_useful = w;
	is_avaible = true;
	type = normal_item;
	_update_description = update_desc;
	_draw = d;
	is_RNG_dependant = RNG;
	description = NULL;

	update_description();
}

PokeItem* ItemClass::operator()(Piece* piece) {
	return constructor(piece, self);
}

ItemClass item_table[NB_OF_ITEMS];

void* _table_initialize = init_item_table();