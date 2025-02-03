#include "item.h"
#include "game.h"
#include "board.h"

#define DESCRIPTION_MAX_SIZE 256

typing operator-(const item_id x, const item_id y) {
	return (typing)((short)x - (short)y);
}

move_data PokeItem::move_to(Square& target) {
	return holder->base_move_to(target);
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
	holder->update_sprite();
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

#define defdraw(x, y) \
static void draw(Surface dest, SDL_Rect* pos = NULL, size s = regular, anchor a = top_left) { \
	Surface sheet = item_sheet; \
	int w = ITEM_SIZE; \
	if (s == mini) { \
		sheet = item_sheet_mini; \
		w = ITEM_MINI_SIZE; \
	} \
	else if (s == mega) { \
		sheet = item_sheet_mega; \
		w = ITEM_MEGA_SIZE; \
	} \
	SDL_Rect r((x) * w, (y) * w, w, w); \
	dest.blit(sheet, pos, &r, a); \
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
	}, // ITALIAN 
	{
		"Pietrastante",
		"Pietrafocaia",
		"Pietraidrica",
		"Pietrafoglia",
		"Pietratuono",
		"Pietragelo",
		"",
		"",
		"",
		"",
		"Pietrabrillo",
		"",
		"Augite Nera",
		"",
		"Squama Drago",
		"Neropietra",
		"Metalcopertura",
		"Calmanella"
	},
};

template<typing old_type, typing new_type>
class EvolutionStone : public PokeItem {
public:
	static const bool RNG = false;
	EvolutionStone(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::evolution_stone, 0, IC) {
		;
	}

	defdraw(0, new_type);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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

		switch (game.language) {
		case LANGUAGE::FRENCH:
			desc_add(" - Uniquement pour les pions.\n")
			desc_add(" - Si l'utilisateur est de type ");
			desc_add(type_str[(int)game.language][old_type]);
			desc_add(", il deviendra de type ");
			desc_add(type_str[(int)game.language][new_type]);
			desc_add(" en evoluant.\n");

			break;
		case LANGUAGE::ENGLISH:
			desc_add(" - Only for pawns.\n");
			desc_add(" - If the user is a ");
			desc_add(type_str[(int)game.language][old_type]);
			desc_add(" type, he will become a ");
			desc_add(type_str[(int)game.language][new_type]);
			desc_add(" type when evolving.\n");
			break;
		}
		
	}
};
template<typing t1, typing t2>
const char* EvolutionStone<t1, t2>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	evolution_stone_names[0][t2],
	evolution_stone_names[1][t2],
	evolution_stone_names[2][t2],
	evolution_stone_names[3][t2],
	evolution_stone_names[4][t2],
};

class Everstone : public PokeItem {
public:
	static const bool RNG = false;
	Everstone(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::everstone, 0, IC) {
		;
	}

	defdraw(0, 0);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->id == Pawn::cls.id) ? -1 : 0;
	}

	virtual bool prepare_promotion() {
		game.to_game(true);
		game.resume_move();
		return true;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;

		switch (game.language) {
		case LANGUAGE::FRENCH:
			desc_add("Pierre Stase :\n");
			desc_add(" - Empêche son porteur d'évoluer.\n");
			break;
		case LANGUAGE::ENGLISH:
			desc_add("Everstone :\n");
			desc_add(" - Prevents its user from evolving.\n");
		}
	}
};

const char* Everstone::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	evolution_stone_names[0][0],
	evolution_stone_names[1][0],
	evolution_stone_names[2][0],
	evolution_stone_names[3][0],
	evolution_stone_names[4][0],
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
		"Baie Panga",
		"Baie Charti",
		"Baie Sédra",
		"Baie Fraigo",
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
		"Payapa Berry",
		"Tanga Berry",
		"Charti Berry",
		"Kasib Berry",
		"Haban Berry",
		"Colbur Berry",
		"Babiri Berry",
		"Roseli Berry"
	},
	{ // GERMAN
		"Latchibeere",
		"Koakobeere",
		"Foepasbeere",
		"Grindobeere",
		"Kerzalbeere",
		"Kiroyabeere",
		"Rospelbeere",
		"Grarzbeere",
		"Schukebeere",
		"Kobabeere",
		"Pyapabeere",
		"Tanigabeere",
		"Chiaribeere",
		"Zitarzbeere",
		"Terirobeere",
		"Burleobeere",
		"Babiribeere",
		"Hibisbeere",
	},
	{ // SPANISH
		"Baya Chilan",
		"Baya Caoca",
		"Baya Pasio",
		"Baya Tamar",
		"Baya Gualot",
		"Baya Rimoya",
		"Baya Pomaro",
		"Baya Kebia",
		"Baya Acardo",
		"Baya Kouba",
		"Baya Payapa",
		"Baya Yecana",
		"Baya Alcho",
		"Baya Drasi",
		"Baya Anjiro",
		"Baya Dillo",
		"Baya Baribá",
		"Baya Hibis",
	},
	{ // ITALIAN
		"Baccacinlan",
		"Baccacao",
		"Baccapasflo",
		"Baccarindo",
		"Baccaparmen",
		"Baccamoya",
		"Baccarosmel",
		"Baccakebia",
		"Baccanaca",
		"Baccababa",
		"Baccapayapa",
		"Baccaitan",
		"Baccaciofo",
		"Baccacitrus",
		"Baccahaban",
		"Baccaxan",
		"Baccababiri",
		"Baccarcadè"
	},
};



template <typing type>
class ResistanceBerry : public PokeItem {
public:
	static const bool RNG = false;
	ResistanceBerry(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::resistance_berry, -1, IC) {
		static_assert(type != typeless);
	}

	defdraw(1, type);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		const effectiveness e = typechart[type][piece->type];
		// the randomize wont draw a super effective berry from the item pool
		return (int) (e == super_effective or (type == normal and e == neutral));
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

	virtual void revenge(move_data& data) {
		if (data.attacker->type == type and not data.do_miss) {
			consume();
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
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

template<typing type>
const char* ResistanceBerry<type>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	type != typeless ? resistance_berry_names[0][type] : "",
	type != typeless ? resistance_berry_names[1][type] : "",
	type != typeless ? resistance_berry_names[2][type] : "",
	type != typeless ? resistance_berry_names[3][type] : "",
	type != typeless ? resistance_berry_names[4][type] : "",
};

class RingTarget : public PokeItem {
public:
	static const bool RNG = false;
	RingTarget(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, 0x7FFFFFFF, IC) {
	}

	defdraw(4, 0);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		int counter = 0;
		iter_typing(t) {
			if (typechart[t][piece->type] == immune) {
				counter--;
			}
		}
		return counter;
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
			desc_add(" - Removes all the immunity of its user.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Retire toutes les immunités de son utilisateur.\n");
			break;
		}
	}
};

const char* RingTarget::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Point de Mire", // FRENCH
	"Ring Target", // ENGLISH
	"Zielscheibe", // GERMAN
	"Blanco", // SPANISH
	"Facilsaglio", // ITALIAN
};

template <typing type>
class ImmunityItem : public PokeItem {
	
public:
	static const bool RNG = false;
	ImmunityItem(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::imunity, 1, IC) {
		;
	}

	defdraw(2, type);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


	static int usefulness_tier(Piece* piece) {
		return (int)typechart[type][piece->type];
	}

	virtual void defense_modifier(effectiveness& e, Piece* attacker) {
		if (attacker->type == type) {
			e = immune;
		}
	}

	virtual void revenge(move_data& data) {
		if ((not data.do_miss) and data.attacker->type == type) {
			char buffer[256] = "\0";
			strcat_s(buffer, holder->name);
			switch (type) {
			case ground:
				strcat_s(buffer, "\'s\nBalloon poped.");
				break;
			case electric:
				strcat_s(buffer, "\'s\nBattery surchaged");
				break;
			case fire:
				strcat_s(buffer, "\'s\nSnowball melted");
				break;
			case water:
				strcat_s(buffer, "\'s\nBulb died");
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

const char* ImmunityItem<ground>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Ballon", // FRENCH
	"Air Balloon", // ENGLISH
	"Luftballon", // GERMAN
	"Globo Helio", // SPANISH
	"Palloncino", // ITALIAN
};

const char* ImmunityItem<fire>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Boule de Neige", // FRENCH
	"Snowball", // ENGLISH
	"Schneeball", // GERMAN
	"Bola de Nieve", // SPANISH
	"Palla di Neve", // ITALIAN
};

const char* ImmunityItem<electric>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Pile", // FRENCH
	"Cell Battery", // ENGLISH
	"Akku", // GERMAN
	"Pila", // SPANISH
	"Ricaripila", // ITALIAN
};

class ExpertBelt : public PokeItem {
public:
	ExpertBelt(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, -2, IC) {
		;
	}

	defdraw(3, 13);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


	static int usefulness_tier(Piece* piece) {
		int counter = 0;
		iter_typing(type) {
			if (typechart[piece->type][type] == super_effective)
				counter++;
		}
		if (counter == 0)
			return 0;
		else if (counter < 2)
			return 1;
		else
			return 2;
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
			desc_add(" - Prevents super effective move from missing.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Empeche d'echouer les attaques super efficace.\n");
			break;
		}
	}
};

const char* ExpertBelt::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Ceinture Pro", // FRENCH
	"Expert Belt", // ENGLISH
	"Expertengurt", // GERMAN
	"Cinta Experto", // SPANISH
	"Abilcintura", // ITALIAN
};

class BlunderPolicy : public PokeItem {
public:
	BlunderPolicy(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, 0, IC) {
		;
	}

	defdraw(3, 9);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness e) {
		miss_rate = -INFINITY; // cannot miss
		consume();
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - One time use.\n");
			desc_add(" - Its user cannot miss his first attack.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Utilisable une unique fois.\n")
			desc_add(" - Son utilisateur ne peut pas echoué sa premiere attaque.\n");
			break;
		}
	}
};

const char* BlunderPolicy::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Assurance Échec", // FRENCH
	"Blunder Policy", // ENGLISH
	"Fehlschlagschutz", // GERMAN
	"Seguro Fallo", // SPANISH
	"Fiascopolizza", // ITALIAN
};

#define defctor(name) name(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {}
class BrightPowder : public PokeItem {
public:
	defctor(BrightPowder);

	defdraw(3, 15);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


	virtual void evasion_modifier(double& miss_rate, Piece* attacker, effectiveness e) {
		miss_rate = 1 - ((1 - miss_rate) * 1.1);
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Increase slightly evasion.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Augmente légerement l'ésquive.\n");
			break;
		}
	}
};

const char* BrightPowder::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Poudre Claire", // FRENCH
	"Bright Powder", // ENGLISH
	"Blendpuder", // GERMAN
	"Polvo Brillo", // SPANISH
	"Luminpolvere", // ITALIAN
};

class LifeOrb : public PokeItem {
public:
	defctor(LifeOrb);

	defdraw(3, 5);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness e) {
		crit_rate *= 2;
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void after_move_effects(Piece* defenser, move_data& data) {
		if (defenser != NULL and not data.suicide && ((double)game.RNG() / (double)game.RNG.max()) <= 0.1) {
			holder->square->to_graveyard();
			char buffer[256] = "\0";
			strcat_s(buffer, holder->name);
			strcat_s(buffer, "\nDied to life orb");
			game.add_textbox(buffer);
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Its user has a 1/10 chance to die each time he attacks.\n");
			desc_add(" - Greatly increase critical hit rate.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Son Utilisateur a 1/10 chance de mourir a chaque fois qu'il attaque.\n");
			desc_add(" - Augmente énormément le taux de coup critique.\n");
			break;
		}
	}
};

const char* LifeOrb::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Orbe Vie", // FRENCH
	"Life Orb", // ENGLISH
	"Leben-Orb", // GERMAN
	"Vidasfera", // SPANISH
	"Assorbisfera", // ITALIAN
};


class LoadedDice : public PokeItem {
public:
	LoadedDice(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, -1, IC) {
		;
	}

	defdraw(3, 6);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
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
			desc_add(" - Each rng roll affecting its user is perform with \"advantage\".\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Chaque utilisation d'aléatoire concernant son utilisateur est effectué avec \"avantage\".\n");
			break;
		}
	}
};

const char* LoadedDice::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Dé Pipé", // FRENCH
	"Loaded Dice", // ENGLISH
	"Gezinkter Würfel", // GERMAN
	"Dado Trucado", // SPANISH
	"Dado Truccato", // ITALIAN
};

class RockyHelmet : public PokeItem {
public:
	defctor(RockyHelmet);
	
	defdraw(3, 0);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		if (not data.suicide && ((double)game.RNG() / (double)game.RNG.max()) < 1.0 / 6.0) {
			data.attacker->square->to_graveyard();
			char buffer[256] = "\0";
			strcat_s(buffer, data.attacker->name);
			strcat_s(buffer, "\nDied to Rcky Hlmt");
			game.add_textbox(buffer);
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - If its user die to an enemy piece, this piece has a chance to die.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Si son utilisateur meurt d'une piece enemie, cette piece a une chance de mourrir.\n");
			break;
		}
	}
};

const char* RockyHelmet::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Casque Brut", // FRENCH
	"Rocky Helmet", // ENGLISH
	"Beulenhelm", // GERMAN
	"Casco dentado", // SPANISH
	"Bitorzolelmo", // ITALIAN
};


class Metronome : public PokeItem {
	short nb_of_consecutives;
public:
	static const bool RNG = true;

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


	Metronome(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {
		nb_of_consecutives = 0;
	};

	defdraw(3, 14);

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness matchup) {
		for (move_data& const data : game.board.move_historic) {
			if (data.attacker->color == holder->color) {
				if (data.attacker == holder && data.defender != NULL) {
					nb_of_consecutives++;
					if (nb_of_consecutives >= 5)
						nb_of_consecutives = 5;
				}
				else {
					nb_of_consecutives = 1;
				}
				break;
			}
		}
		crit_rate *= nb_of_consecutives;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Increase critical hit rate for each consecutive moves.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Augmente le taux de critique pour chaques coups consecutifs.\n");
			break;
		}
	}
};

const char* Metronome::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Métronome", // FRENCH
	"Metronome", // ENGLISH
	"Metronom", // GERMAN
	"Metrónomo", // SPANISH
	"Plessimetro", // ITALIAN
};

class RedCard : public PokeItem {
public:
	static const bool RNG = false;

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	defctor(RedCard);

	defdraw(3, 1);

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		if (data.move_again and not data.suicide) {
			data.move_again = false;
			game.board.in_bonus_move = false;
			data.attacker->base_move_to(*data.begin_square);
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - If its user die to an attack that results in a bonus move, the attacker gets send back to its starting square.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Si son utilisateur meurt d'une attaque qui résulte en un coup bonus, l'attaquant revient sur sa case de départ.\n");
			break;
		}
	}
};

const char* RedCard::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Carton Rouge", // FRENCH
	"Red Card", // ENGLISH
	"Rote Karte", // GERMAN
	"tarjeta roja", // SPANISH
	"Cartelrosso", // ITALIAN
};


class StickyBarbs : public PokeItem {
public:
	defctor(StickyBarbs);

	defdraw(3, 2);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		data.attacker->item->consume();
		data.attacker->item = this;
		holder->update_sprite();
		holder = data.attacker;
		holder->update_sprite();
	}

	virtual void after_move_effects(move_data& data) {
		if ((not data.suicide) && (((double)game.RNG() / (double)game.RNG.max()) < 0.125)) {
			holder->square->to_graveyard();
			char buffer[256] = "\0";
			strcat_s(buffer, holder->name);
			strcat_s(buffer, "\nDied to StckyBrbs");
			game.add_textbox(buffer);
			data.suicide = true;
			data.move_again = false;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Its user has a chance to die each time it moves\n");
			desc_add(" - Stick to enemy piece.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Son utilisateur a une chance de mourrir quand il se déplace\n");
			desc_add(" - Colle aux pièces ennemie.\n");
			break;
		}
	}
};

const char* StickyBarbs::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Piquants", // FRENCH
	"Sticky Barb", // ENGLISH
	"Klettdorn", // GERMAN
	"Toxiestrella", // SPANISH
	"Vischiopunta", // ITALIAN
};

class AssaultVest : public PokeItem {
public:
	static const bool RNG = false;

	defctor(AssaultVest);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return -2;
	}

	defdraw(4, 1);
	virtual bool is_move_disallowed(Square& target) {
		return target.piece == NULL;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Block non capturing moves.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Empêche les coups qui ne sont pas des captures.\n");
			break;
		}
	}
};

const char* AssaultVest::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Veste de Combat", // FRENCH
	"Assault Vest", // ENGLISH
	"Offensivweste", // GERMAN
	"Chaleco Asalto", // SPANISH
	"Corpetto assalto", // ITALIAN
};


class ShedShell : public PokeItem {
public:
	static const bool RNG = false;

	defctor(ShedShell);

	defdraw(3, 16);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->id == Knight::cls.id) ? 0 : 2;
	}

	static bool would_be_useful(Piece* piece) {
		return piece->id != Knight::cls.id;
	}
	virtual bool is_move_allowed(Square& target) {
		if (target.piece != NULL)
			return false;
		{	
			Knight K = Knight(game.board, holder->color, holder->square, holder->type, NULL);
			if (not K.base_do_control(target))
				return false;
		}
		for (Square& square : game.board) {
			if (holder->can_move_to(square)) {
				return false;
			}
		}

		return true;
	}

	virtual move_data move_to(Square& target) {
		move_data data = holder->base_move_to(target);
		char buffer[256] = "\0";
		strcat_s(buffer, holder->name);
		strcat_s(buffer, "\nfled successfuly");
		game.add_textbox(buffer);
		consume(buffer);
		return data;
	}

	virtual void after_move_effect(move_data& data) {
		if (holder->id != Knight::cls.id) {
			Knight K = Knight(game.board, holder->color, data.begin_square, holder->type, NULL);
			if (K.base_do_control(*data.target_square) and not data.cancel) {
				consume();
			}
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - One time use.\n");
			desc_add(" - If trap you can perform a knight move.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Utilisable une seule fois.\n");
			desc_add(" - Si piegé vous pouvez vous deplacez en cavalier.\n");
			break;
		}
	}
};

const char* ShedShell::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Carapace Mue", // FRENCH
	"Shed Shell", // ENGLISH
	"Wechselhülle", // GERMAN
	"Muda Concha", // SPANISH
	"Disfoguscio", // ITALIAN
};

class ScopeLens : public PokeItem {
public:
	defctor(ScopeLens);

	defdraw(3, 12);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void crit_modifier(double& crit_rate, Piece* defenser, effectiveness e) {
		crit_rate *= 1.3;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Increase slightly critical hit rate.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Augmente légerement le taux de critique.\n");
			break;
		}
	}
};

const char* ScopeLens::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Lentilscope", // FRENCH
	"Scope Lens", // ENGLISH
	"Scope-Linse", // GERMAN
	"Periscopio", // SPANISH
	"Mirino", // ITALIAN
};

class WideLens : public PokeItem {
public:
	defctor(WideLens);

	defdraw(3, 11);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void accuracy_modifier(double& miss_rate, Piece* defenser, effectiveness e) {
		miss_rate /= 2;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Increase accuracy.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Augmente la precision.\n");
			break;
		}
	}
};

const char* WideLens::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Loupe", // FRENCH
	"Wide Lens", // ENGLISH
	"Grosslinse", // GERMAN
	"Lupa", // SPANISH
	"Grandelente", // ITALIAN
};

class Honey : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(2, bug);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	Honey(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::honey, 0, IC) {
		;
	}

	static int usefulness_tier(Piece* piece) {
		return (typechart[bug][piece->type] <= not_very_effective) ? 2 : 1;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Opposing bug types cannot resist the attraction of honey.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Les types insectes adverse ne peuvent resister a l'envie de miel.\n");
			break;
		}
	}
};

const char* Honey::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Miel", // FRENCH
	"Honey", // ENGLISH
	"Honig", // GERMAN
	"Miel", // SPANISH
	"Miele", // ITALIAN
};

class ProtectivePads : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(3, 8);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	ProtectivePads(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::protective_pads, 0, IC) {
		;
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Ignore opposing items when attacking.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Ignore les objets ennemi quand vous attaquez.\n");
			break;
		}
	}
};

const char* ProtectivePads::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Parre-Effet", // FRENCH
	"Protective Pads", // ENGLISH
	"Schutzpolster", // GERMAN
	"Paracontacto", // SPANISH
	"Smorzaurti", // ITALIAN
};

class SafetyGoogles : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(3, 7);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	SafetyGoogles(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::safety_googles, 0, IC) {
		;
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Ignores opposing items when attacked.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Ignore les objets adverse vous êtes attaqué.\n");
			break;
		}
	}
};

const char* SafetyGoogles::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Lunettes Filtre", // FRENCH
	"Safety Googles", // ENGLISH
	"Shutzbrille", // GERMAN
	"Gafa Protectora", // SPANISH
	"Visierantisabbia", // ITALIAN
};

class NormalGem : public PokeItem {
public:
	static const bool RNG = false;

	defctor(NormalGem);

	defdraw(2, normal);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->type == normal) ? 2 : 0;
	}

	virtual void attack_modifier(effectiveness& e, Piece* defenser) {
		if (holder->type == normal) {
			e++;
		}
	}
	
	virtual void after_move_effects(move_data& data) {
		if (not (data.do_miss or data.do_crit or data.defender == NULL or data.cancel)) {
			consume();
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Only for normal type pieces.\n");
			desc_add(" - One time use.\n");
			desc_add(" -Your first attack will be super effective\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Uniquement pour les pieces de type normal\n");
			desc_add(" - Utilisable une seule fois.\n");
			desc_add(" - Vôtre première attaque sera super efficace.\n")
			break;
		}
	}
};

const char* NormalGem::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Joyau Normal", // FRENCH
	"Normal Gem", // ENGLISH
	"Normaljuwel", // GERMAN
	"Gema Normal", // SPANISH
	"Bijounormale", // ITALIAN
};

class LeppaBerry : public PokeItem {
public:
	static const bool RNG = false;

	defctor(LeppaBerry);

	defdraw(3, 10);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->id == King::cls.id or piece->id == Pawn::cls.id) ? 2 : 0;
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
		move_data data = holder->base_move_to(target);
		consume();
		return data;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - One time use.\n");
			desc_add(" - Allows to perform special move a second time.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Utilisable une fois.\n");
			desc_add(" - Permet de refaire un deplacement special.\n");
			break;
		}
	}
};

const char* LeppaBerry::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Baie Mepo", // FRENCH
	"Leppa Berry", // ENGLISH
	"Jonagobeere", // GERMAN
	"Baya Zanama", // SPANISH
	"Baccamela", // ITALIAN
};

class BlackSludge : public PokeItem {
public:
	defctor(BlackSludge);

	defdraw(2, poison);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return -(piece->type == poison);
	}

	virtual void after_move_effects(Piece* defenser, move_data& data) {
		if (holder->type != poison and ((double)game.RNG() / (double)game.RNG.max()) < 0.125) {
			holder->square->to_graveyard();
			char buffer[256] = "\0";
			strcat_s(buffer, holder->name);
			strcat_s(buffer, "\nDied to\nBlackSludge");
			game.add_textbox(buffer);
			data.suicide = true;
		}
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Has a chance to kill non poison types piece when they move.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - A une chance de tuer les pièces qui ne sont pas de type poison quand elles se déplacent.\n");
			break;
		}
	}
};

const char* BlackSludge::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Boue Noire", // FRENCH
	"Black Sludge", // ENGLISH
	"Giftschleim", // GERMAN
	"Lodo Negro", // SPANISH
	"Fangopece", // ITALIAN
};

class HeavyDutyBoots : public PokeItem {
public:
	static const bool RNG = false;
	defctor(HeavyDutyBoots);

	defdraw(2, rock);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (typechart[piece->type][rock] == not_very_effective) ? 2 : 0;
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
			desc_add(" - Remove any not very effective against rock.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - L'utilisateur ne sera plus resité par le type roche.\n");
			break;
		}
	}
};

const char* HeavyDutyBoots::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Grosses Bottes", // FRENCH
	"Heavy-Duty Boots", // ENGLISH
	"Plateauschuhe", // GERMAN
	"Botas Gruesas", // SPANISH
	"Scarponi Robusti", // ITALIAN
};

class LightBall : public PokeItem {
public:
	defctor(LightBall);

	defdraw(3, electric);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->id == Pawn::cls.id) ? (piece->type == typeless) + 2 * (piece->type == electric) : 0;
	}
	
	virtual void crit_modifier(double& crit_rate, Piece* defenser) {
		if (holder->type == electric and holder->id == Pawn::cls.id)
			crit_rate *= 3;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[256];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - Only for electric type pawns.\n");
			desc_add(" - Increases critical hit chance.\n")
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Uniquement pour les pions de type électrique.\n");
			desc_add(" - Augmente le taux de coup critique.\n")
			break;
		}
	}

};

const char* LightBall::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Balle Lumière", // FRENCH
	"Light Ball", // ENGLISH
	"Kugelblitz", // GERMAN
	"Bola Luminosa", // SPANISH
	"Elettropalla", // ITALIAN
};

class Eviolite : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Eviolite);

	defdraw(3, 3);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2 * (piece->id == Pawn::cls.id);
	}

	virtual void defense_modifier(effectiveness& e, Piece* attacker) {
		if (holder->id == Pawn::cls.id) {
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
			desc_add(" - Only for pawns.\n")
			desc_add(" - Remove all weaknesses.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Uniquement pour les pions.\n");
			desc_add(" - Retire toute les faiblesses.\n");
			break;
		}
	}
};

const char* Eviolite::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Évoluroc", // FRENCH
	"Eviolite", // ENGLISH
	"Evolith", // GERMAN
	"Mineral Evolutivo", // SPANISH
	"Mineral Evol", // ITALIAN
};


class Baguette : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Baguette);

	defdraw(4, 3);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 0;
	}

	static void update_description(char*& description) {
		if (description == NULL)
			description = new char[64];

		int end = 0;
		switch (game.language) {
		case LANGUAGE::ENGLISH:
			desc_add(" - A baguette.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Une baguette.\n");
			break;
		case LANGUAGE::GERMAN:
			desc_add(" - Ein Baguette.\n");
			break;
		case LANGUAGE::SPANISH:
			desc_add(" - Una barra de pan.\n");
			break;
		case LANGUAGE::ITALIAN:
			desc_add(" - Una baguette.\n");
			break;
		}
	}
};

const char* Baguette::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Baguette", // FRENCH
	"Baguette", // ENGLISH
	"Baguette", // GERMAN
	"Barra de Pan", // SPANISH
	"Filoncino", // ITALIAN
};

class LeadersCrest : public PokeItem {
public:
	static const bool RNG = false;
	defctor(LeadersCrest);

	defdraw(2, steel);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->id == Pawn::cls.id) ? (piece->type == typeless) + 2 * (piece->type == steel or piece->type == dark) : 0;
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
			desc_add(" - Only for steel/dark type pawns.\n")
			desc_add(" - Allow promotion to King.\n");
			break;
		case LANGUAGE::FRENCH:
			desc_add(" - Uniquement pour les pions de type acier/ténèbres.\n");
			desc_add(" - Permet la promotion en Roi.\n");
			break;
		}
	}
};

const char* LeadersCrest::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Emblème du Général", // FRENCH
	"Leader’s Crest", // ENGLISH
	"Anführersymbol", // GERMAN
	"Distintivo de Líder", // SPANISH
	"Simbolo del capo", // ITALIAN
};

void* init_item_table() {
	int counter = 0;

#define Itemize(class) item_table[counter++] = ItemClass([](Piece* piece, ItemClass& IC) -> PokeItem* { return new class(piece, IC); }, class::usefulness_tier, class::draw, class::update_description, class::name, class::RNG);
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
	Itemize(Everstone);

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
	Itemize(ImmunityItem<fire>);

	
	EndLine();

	Itemize(ExpertBelt);
	Itemize(BlunderPolicy);
	Itemize(BrightPowder);
	Itemize(WideLens);
	Itemize(ScopeLens);
	Itemize(LifeOrb);
	Itemize(LoadedDice);
	Itemize(Metronome);
	
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
	usefulness_tier = [](Piece*) -> int { return -1024; };
	is_avaible = false;
	type = space_item;
	is_RNG_dependant = false;
	name = NULL;
	description = NULL;
}

ItemClass::ItemClass(PokeItem* (*ctor)(Piece*, ItemClass&), int (*w)(Piece*), void (*d)(Surface, SDL_Rect*, size, anchor), void (*update_desc)(char*&), const char** _name, bool RNG) {
	constructor = ctor;
	usefulness_tier = w;
	is_avaible = true;
	type = normal_item;
	_update_description = update_desc;
	_draw = d;
	is_RNG_dependant = RNG;
	description = NULL;
	name = _name;
	update_description();
}

PokeItem* ItemClass::operator()(Piece* piece) {
	return constructor(piece, self);
}

ItemClass item_table[NB_OF_ITEMS];

void* _table_initializer = init_item_table();