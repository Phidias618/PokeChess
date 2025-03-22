#include <array>

#include "item.h"
#include "game.h"
#include "board.h"

int current_id = 0;

move_data PokeItem::move_to(Square& target) {
	return holder->base_move_to(target);
}


PokeItem::PokeItem(Piece* piece, item_id id_, int prio, ItemClass& IC) : id(id_), holder(piece), priority(prio), cls(IC) {
	used = false;
	holder = piece;
}

PokeItem::PokeItem(Piece* piece, ItemClass& IC) : id(item_id::no_item), priority(0), cls(IC) {
	used = false;
}

PokeItem::~PokeItem() {
	holder->set_pokeicon(PokemonIcon(holder));
	holder->set_item(NULL);
}

void PokeItem::remove() {
	holder->item = NULL;
	holder->set_pokeicon(PokemonIcon(holder));
	holder->update_sprite();
}

void PokeItem::consume() {
	used = true;
	holder->item = NULL;
	holder->update_sprite();
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
		
	}

	virtual void update_pokeicon() {
		if (holder->type == old_type)
			holder->set_pokeicon(PokemonIcon(21 + holder->color, 1 + new_type));
	}

	virtual void remove() {
		holder->set_pokeicon(PokemonIcon(holder));
		holder = NULL;
	}

	defdraw(0, new_type);

	

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static bool would_be_useful(Piece* piece) {
		return piece->Class == Pawn::cls && piece->type == old_type;
	}

#define ctor(Class) [](Board& b, piece_color c, Square* sq, typing t, PokeItem* i) -> Piece* { \
	Piece* piece = new Class(b, c, sq, new_type, i); \
	piece->has_already_move = piece->evolved = true; \
	piece->set_pokeicon(PokemonIcon(23 + piece->color, 1 + new_type)); \
	return piece; \
}

	virtual bool prepare_promotion() {
		if (holder->type == old_type) {

			game.buttons->add(new PromotionButton(ctor(Queen), 7.0, 5.5));
			game.buttons->add(new PromotionButton(ctor(Rook), 8.0, 5.5));
			game.buttons->add(new PromotionButton(ctor(Bishop), 9.0, 5.5));
			game.buttons->add(new PromotionButton(ctor(Knight), 10.0, 5.5));

			return true;
		}
		return false;
	}

#undef ctor
	virtual void promote() {
		if (holder->type == new_type) {
			consume();
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};
template<typing t1, typing t2>
dstr_t EvolutionStone<t1, t2>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{evolution_stone_names[0][t2], 1},
	{evolution_stone_names[1][t2], 1},
	{evolution_stone_names[2][t2], 1},
	{evolution_stone_names[3][t2], 1},
	{evolution_stone_names[4][t2], 1},
};

constexpr char const* to_c_str(std::string str) {
	char* buffer = new char[str.length()+1];
	int i = 0;
	for (char c : str) {
		buffer[i++] = c;
	}
	buffer[i] = '\0';
	return buffer;
}

template<typing t1, typing t2>
dstr_t EvolutionStone<t1, t2>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{to_c_str(std::string(" - Uniquement pour les pions.\n\
 - Si l'utilisateur est de type ").append(type_str[(int)LANGUAGE::FRENCH][t1]).append(", il deviendra de type ").append(type_str[(int)LANGUAGE::FRENCH][t2]).append(" en évoluant.\n")), 1}, // FRENCH
	{to_c_str(std::string(" - Only for pawns.\n\
 - Makes ").append(type_str[(int)LANGUAGE::ENGLISH][t1]).append(" types, evolve into a ").append(type_str[(int)LANGUAGE::ENGLISH][t2]).append(" type.\n")), 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class Everstone : public PokeItem {
public:
	static const bool RNG = false;
	Everstone(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::everstone, 0, IC) {
		;
	}

	defdraw(0, 0);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->Class == Pawn::cls) ? -1 : 0;
	}

	virtual bool prepare_promotion() {
		game.to_game(true);
		game.resume_move();
		return true;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t Everstone::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{evolution_stone_names[0][0], 1},
	{evolution_stone_names[1][0], 1},
	{evolution_stone_names[2][0], 1},
	{evolution_stone_names[3][0], 1},
	{evolution_stone_names[4][0], 1},
};

dstr_t Everstone::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Empêche son porteur d'évoluer.\n", 1}, // FRENCH
	{" - Prevents its user from evolving.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
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
	static_assert(type != typeless);

	static const bool RNG = false;
	ResistanceBerry(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::resistance_berry, -1, IC) {
	
	}

	defdraw(1, type);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		const effectiveness e = typechart[type][piece->type];
		// the randomize wont draw a super effective berry from the item pool
		return (int) (e == super_effective or (type == normal and e == neutral));
	}

	virtual void defense_modifier(move_data& data) {
		if (data.attacker->type == type) {
			if (data.matchup == super_effective or type == normal)
				--data.matchup;
			else {
				--data.matchup.intensity;
			}
		}
	}

	virtual void revenge(move_data& data) {
		if (holder->is_in_graveyard and data.attacker->type == type) {
			consume();
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

template<typing type>
dstr_t ResistanceBerry<type>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{type != typeless ? resistance_berry_names[0][type] : "", 1},
	{type != typeless ? resistance_berry_names[1][type] : "", 1},
	{type != typeless ? resistance_berry_names[2][type] : "", 1},
	{type != typeless ? resistance_berry_names[3][type] : "", 1},
	{type != typeless ? resistance_berry_names[4][type] : "", 1},
};

template<typing type>
dstr_t ResistanceBerry<type>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{to_c_str(std::string(" - Retire une faiblesse contre le type ").append(type_str[(int)LANGUAGE::FRENCH][type]).append(".\n")), 1}, // FRENCH
	{to_c_str(std::string(" - Removes a ").append(type_str[(int)LANGUAGE::ENGLISH][type]).append(" type weakness.\n")), 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

dstr_t ResistanceBerry<normal>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Donne une resistance contre le type normal.\n", 1}, // FRENCH
	{" - Gives a normal type resistance.\n", 1}, // ENGLISH
	{"", 1},
	{"", 1},
	{"", 1},
};

class RingTarget : public PokeItem {
public:
	static const bool RNG = false;
	RingTarget(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::no_item, 0x7FFFFFFF, IC) {
	}

	defdraw(5, 0);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		int counter = 0;
		iter_typing(t) {
			if (typechart[t][piece->type] == immune) {
				counter--;
			}
		}
		return counter;
	}

	virtual void defense_modifier(move_data& data) {
		if (data.matchup == immune) {
			data.matchup.main = neutral;
			if (data.matchup.intensity > 0) {
				data.matchup.intensity--;
				data.matchup.main = super_effective;
			}
			else if (data.matchup.intensity < 0) {
				data.matchup.intensity++;
				data.matchup.main = not_very_effective;
			}
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t RingTarget::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Point de Mire", 1}, // FRENCH
	{"Ring Target", 1}, // ENGLISH
	{"Zielscheibe", 1}, // GERMAN
	{"Blanco", 1}, // SPANISH
	{"Facilsaglio", 1}, // ITALIAN
};

dstr_t RingTarget::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Retire toutes les immunités de son utilisateur.\n", 1}, // FRENCH
	{" - Removes all the immunity of its user.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

template <typing type>
class ImmunityItem : public PokeItem {
	
public:
	static const bool RNG = false;
	ImmunityItem(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::imunity, 1, IC) {
		;
	}

	defdraw(3, type);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];


	static int usefulness_tier(Piece* piece) {
		return (int)typechart[type][piece->type];
	}

	virtual void defense_modifier(move_data& data) {
		if (data.attacker->type == type) {
			data.matchup = immune;
		}
	}

	virtual void revenge(move_data& data) {
		if ((not holder->is_in_graveyard) and data.attacker->type == type) {
			consume();
		}
	}

	virtual void add_cosmetic(move_data& data) {
		if (used) {
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name[(int)game.language]);
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
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t ImmunityItem<ground>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Ballon", 1}, // FRENCH
	{"Air Balloon", 1}, // ENGLISH
	{"Luftballon", 1}, // GERMAN
	{"Globo Helio", 1}, // SPANISH
	{"Palloncino", 1}, // ITALIAN
};

dstr_t ImmunityItem<fire>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Boule de Neige", 1}, // FRENCH
	{"Snowball", 1}, // ENGLISH
	{"Schneeball", 1}, // GERMAN
	{"Bola de Nieve", 1}, // SPANISH
	{"Palla di Neve", 1}, // ITALIAN
};

dstr_t ImmunityItem<electric>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Pile", 1}, // FRENCH
	{"Cell Battery", 1}, // ENGLISH
	{"Akku", 1}, // GERMAN
	{"Pila", 1}, // SPANISH
	{"Ricaripila", 1}, // ITALIAN
};

template<typing type>
dstr_t ImmunityItem<type>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{to_c_str(std::string(" - Utilisable une unique fois.\n - Donne une immunité contre le type ").append(type_str[(int)LANGUAGE::FRENCH][type]).append(".\n")), 1}, // FRENCH
	{to_c_str(std::string(" - One time use.\n - Gives an immunity to ").append(type_str[(int)LANGUAGE::ENGLISH][type]).append(".\n")), 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class ExpertBelt : public PokeItem {
public:
	ExpertBelt(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::no_item, -2, IC) {
		;
	}

	defdraw(4, 13);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];


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

	virtual void accuracy_modifier(move_data& data) {
		if (data.matchup == super_effective)
			data.miss_rate = -INFINITY;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t ExpertBelt::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Ceinture Pro", 1}, // FRENCH
	{"Expert Belt", 1}, // ENGLISH
	{"Expertengurt", 1}, // GERMAN
	{"Cinta Experto", 1}, // SPANISH
	{"Abilcintura", 1}, // ITALIAN
};

dstr_t ExpertBelt::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Empeche d'echouer les attaques super efficace.\n", 1}, // FRENCH
	{" - Prevents super effective move from missing.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class BlunderPolicy : public PokeItem {
public:
	BlunderPolicy(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::no_item, 0, IC) {
		;
	}

	defdraw(4, 9);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	virtual void accuracy_modifier(move_data& data) {
		data.miss_rate = -INFINITY; // cannot miss
		consume();
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};



dstr_t BlunderPolicy::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Assurance Échec", 1}, // FRENCH
	{"Blunder Policy", 1}, // ENGLISH
	{"Fehlschlagschutz", 1}, // GERMAN
	{"Seguro Fallo", 1}, // SPANISH
	{"Fiascopolizza", 1}, // ITALIAN
};

dstr_t BlunderPolicy::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Utilisable une unique fois.\n\
 - Son utilisateur ne peut pas echoué sa premiere attaque.\n", 1}, // FRENCH
	{" - One time use.\n\
 - Its user cannot miss his first attack.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

#define defctor(name) name(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {}
class BrightPowder : public PokeItem {
public:
	defctor(BrightPowder);

	defdraw(4, 15);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];


	virtual void evasion_modifier(move_data& data) {
		data.miss_rate = 1 - ((1 - data.miss_rate) * 1.1);
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t BrightPowder::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Poudre Claire", 1}, // FRENCH
	{"Bright Powder", 1}, // ENGLISH
	{"Blendpuder", 1}, // GERMAN
	{"Polvo Brillo", 1}, // SPANISH
	{"Luminpolvere", 1}, // ITALIAN
};

dstr_t BrightPowder::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Augmente légerement l'ésquive.\n", 1}, // FRENCH
	{" - Increase slightly evasion.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class LifeOrb : public PokeItem {
	bool activated = false;
public:
	defctor(LifeOrb);

	defdraw(4, 5);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];


	virtual void crit_modifier(move_data& data) {
		data.crit_rate *= 2;
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void after_move_effects(Piece* defenser, move_data& data) {
		if (defenser != NULL and not data.suicide && ((double)game.RNG() / (double)game.RNG.max()) <= 0.1) {
			holder->square->to_graveyard();
			activated = true;
			data.suicide = true;
		}
	}

	virtual void add_cosmetic(move_data& data) {
		if (activated) {
			activated = false;
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to life orb");
			game.add_textbox(buffer);
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t LifeOrb::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Orbe Vie", 1}, // FRENCH
	{"Life Orb", 1}, // ENGLISH
	{"Leben-Orb", 1}, // GERMAN
	{"Vidasfera", 1}, // SPANISH
	{"Assorbisfera", 1}, // ITALIAN
};

dstr_t LifeOrb::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Son Utilisateur a 1/10 chance de mourir a chaque fois qu'il attaque.\n\
 - Augmente énormément le taux de coup critique.\n", 1}, // FRENCH
	{" - Its user has a 1 / 10 chance to die each time he attacks.\n\
 - Greatly increase critical hit rate.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class LoadedDice : public PokeItem {
public:
	LoadedDice(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::no_item, -1, IC) {
		;
	}

	defdraw(4, 6);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void crit_modifier(move_data& data) {
		if (data.crit_rate < 0)
			data.crit_rate = -INFINITY;
		else
			data.crit_rate = sqrt(data.crit_rate); // the equivallent of a roll with dnd advantage
	}

	virtual void accuracy_modifier(move_data& data) {
		if (data.miss_rate < 0) {
			data.miss_rate = -INFINITY;
		}
		else {
			data.miss_rate = 1 - sqrt(1 - data.miss_rate); // the equivallent of a roll with dnd advantage
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t LoadedDice::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"De Pipe", 1}, // FRENCH
	{"Loaded Dice", 1}, // ENGLISH
	{"Gezinkter Würfel", 1}, // GERMAN
	{"Dado Trucado", 1}, // SPANISH
	{"Dado Truccato", 1}, // ITALIAN
};

dstr_t LoadedDice::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Chaque utilisation d'aléatoire concernant son utilisateur est effectuée avec \"avantage\".\n", 1}, // FRENCH
	{" - Each rng roll affecting its user is perform with \"advantage\".\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class RockyHelmet : public PokeItem {
	bool activated = false;
public:
	defctor(RockyHelmet);
	
	defdraw(4, 0);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		if (holder->is_in_graveyard and not data.suicide && ((double)game.RNG() / (double)game.RNG.max()) < 1.0 / 6.0) {
			data.attacker->square->to_graveyard();
			activated = true;
			data.suicide = true;
		}
	}

	virtual void add_cosmetic(move_data& data) {
		if (activated) {
			char buffer[256] = "\0";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to Rcky Hlmt");
			game.add_textbox(buffer);
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t RockyHelmet::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Casque Brut", 1}, // FRENCH
	{"Rocky Helmet", 1}, // ENGLISH
	{"Beulenhelm", 1}, // GERMAN
	{"Casco dentado", 1}, // SPANISH
	{"Bitorzolelmo", 1}, // ITALIAN
};

dstr_t RockyHelmet::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Si son utilisateur meurt d'une piece enemie, cette piece a une chance de mourrir.\n", 1}, // FRENCH
	{" - If its user die to an enemy piece, this piece has a chance to die.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class Metronome : public PokeItem {
	short nb_of_consecutives;
public:
	static const bool RNG = true;

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];


	Metronome(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {
		nb_of_consecutives = 0;
	};

	defdraw(4, 14);

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void crit_modifier(move_data& data) {
		for (move_data const& d : holder->board.move_historic) {
			if (d.attacker->color == holder->color) {
				if (d.attacker == holder && d.defender != NULL) {
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
		data.crit_rate *= nb_of_consecutives;
	}

	virtual Uint32 get_hash() {
		return nb_of_consecutives;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t Metronome::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Métronome", 1}, // FRENCH
	{"Metronome", 1}, // ENGLISH
	{"Metronom", 1}, // GERMAN
	{"Metrónomo", 1}, // SPANISH
	{"Plessimetro", 1}, // ITALIAN
};

dstr_t Metronome::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Augmente le taux de critique pour chaques coups consecutifs.\n", 1}, // FRENCH
	{" - Increase critical hit rate for each consecutive moves.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class QuickClaw : public PokeItem {
public:
	static const bool RNG = true;

	QuickClaw(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::quick_claw, 0, IC) {};

	defdraw(3, 3);

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];
	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t QuickClaw::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Vive Griffe", 1}, // FRENCH
	{"Quick Claw", 1}, // ENGLISH
	{"Flinkklaue", 1}, // GERMAN
	{"Garra Rapida", 1}, // SPANISH
	{"Rapidartigli", 1}, // ITALIAN
};

dstr_t QuickClaw::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Rend son utilisateur capable d'infliger des coups critiques n'importe ou.\n", 1}, // FRENCH
	{" - Makes its user able to crit everywhere.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class RedCard : public PokeItem {
	bool activated = false;
public:
	static const bool RNG = false;

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	defctor(RedCard);

	defdraw(4, 1);

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		if (holder->is_in_graveyard and data.move_again and not data.suicide) {
			data.move_again = false;
			activated = true;
			game.board.in_bonus_move = false;
			data.attacker->base_move_to(*data.begin_square);
		}
	}

	virtual void add_cosmetic(move_data& data) {
		if (activated) {
			char buffer[256] = "\0";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nwas sent back.");
			game.add_textbox(buffer);
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t RedCard::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Carton Rouge", 1}, // FRENCH
	{"Red Card", 1}, // ENGLISH
	{"Rote Karte", 1}, // GERMAN
	{"tarjeta roja", 1}, // SPANISH
	{"Cartelrosso", 1}, // ITALIAN
};

dstr_t RedCard::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Si son utilisateur meurt d'une attaque qui résulte en un coup bonus, l'attaquant revient sur sa case de départ.\n", 1}, // FRENCH
	{" - If its user die to an attack that results in a bonus move, the attacker gets send back to its starting square.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class StickyBarbs : public PokeItem {
	bool activated;
public:
	defctor(StickyBarbs);

	defdraw(4, 2);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		if (holder->is_in_graveyard) {
			consume();
			used = false;
			if (data.attacker->item != NULL)
				data.attacker->item->consume();
			data.attacker->item = this;
			holder = data.attacker;
			holder->update_sprite();
		}
	}

	virtual void after_move_effects(move_data& data) {
		activated = false;
		if ((not data.suicide) && (((double)game.RNG() / (double)game.RNG.max()) < 0.125)) {
			holder->square->to_graveyard();
			activated = true;
			data.suicide = true;
			data.move_again = false;
		}
	}

	virtual void add_cosmetic(move_data& data) {
		if (activated) {
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to StckyBrbs");
			game.add_textbox(buffer);
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t StickyBarbs::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Piquants", 1}, // FRENCH
	{"Sticky Barb", 1}, // ENGLISH
	{"Klettdorn", 1}, // GERMAN
	{"Toxiestrella", 1}, // SPANISH
	{"Vischiopunta", 1}, // ITALIAN
};

dstr_t StickyBarbs::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Son utilisateur a une chance de mourrir quand il se déplace\n\
 - Colle aux pièces ennemie.\n", 1}, // FRENCH
	{" - Its user has a chance to die each time it moves\n\
 - Stick to enemy piece.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class AssaultVest : public PokeItem {
public:
	static const bool RNG = false;

	defctor(AssaultVest);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return -2;
	}

	defdraw(5, 1);
	virtual bool is_move_disallowed(Square& target) {
		return target.piece == NULL;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t AssaultVest::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Veste de Combat", 1}, // FRENCH
	{"Assault Vest", 1}, // ENGLISH
	{"Offensivweste", 1}, // GERMAN
	{"Chaleco Asalto", 1}, // SPANISH
	{"Corpetto assalto", 1}, // ITALIAN
};

dstr_t AssaultVest::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Empêche les coups qui ne sont pas des captures.\n", 1}, // FRENCH
	{" - Blocks non capturing moves.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class ShedShell : public PokeItem {
	bool activated = false;
public:
	static const bool RNG = false;

	defctor(ShedShell);

	defdraw(4, 16);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->Class == Knight::cls) ? 0 : 2;
	}

	static bool would_be_useful(Piece* piece) {
		return piece->Class != Knight::cls;
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
		consume();
		return data;
	}
	
	virtual void add_cosmetic(move_data& data) {
		if (used) {
			used = false;
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name[(int)game.language]);
			strcat_s(buffer, "\nfled successfuly");
			game.add_textbox(buffer);
			consume();
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t ShedShell::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Carapace Mue", 1}, // FRENCH
	{"Shed Shell", 1}, // ENGLISH
	{"Wechselhülle", 1}, // GERMAN
	{"Muda Concha", 1}, // SPANISH
	{"Disfoguscio", 1}, // ITALIAN
};

dstr_t ShedShell::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Utilisable une seule fois.\n\
 - Si vous êtes piegé vous pouvez vous deplacez en cavalier.\n", 1}, // FRENCH
	{" - One time use.\n\
 - If you are trapped you can perform a knight move.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class ScopeLens : public PokeItem {
public:
	defctor(ScopeLens);

	defdraw(4, 12);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void crit_modifier(move_data& data) {
		data.crit_rate *= 1.3;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t ScopeLens::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Lentilscope", 1}, // FRENCH
	{"Scope Lens", 1}, // ENGLISH
	{"Scope-Linse", 1}, // GERMAN
	{"Periscopio", 1}, // SPANISH
	{"Mirino", 1}, // ITALIAN
};

dstr_t ScopeLens::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Augmente légerement le taux de critique.\n", 1}, // FRENCH
	{" - Increase slightly critical hit rate.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class WideLens : public PokeItem {
public:
	defctor(WideLens);

	defdraw(4, 11);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void accuracy_modifier(move_data& data) {
		data.miss_rate /= 2;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t WideLens::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Loupe", 1}, // FRENCH
	{"Wide Lens", 1}, // ENGLISH
	{"Grosslinse", 1}, // GERMAN
	{"Lupa", 1}, // SPANISH
	{"Grandelente", 1}, // ITALIAN
};

dstr_t WideLens::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Augmente la precision.\n", 1}, // FRENCH
	{" - Increase accuracy.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class Honey : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(3, bug);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	Honey(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::honey, 0, IC) {
		;
	}

	static int usefulness_tier(Piece* piece) {
		return (typechart[bug][piece->type] <= not_very_effective) ? 2 : 1;
	}


	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t Honey::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Miel", 1}, // FRENCH
	{"Honey", 1}, // ENGLISH
	{"Honig", 1}, // GERMAN
	{"Miel", 1}, // SPANISH
	{"Miele", 1}, // ITALIAN
};

dstr_t Honey::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Les types insectes adverse ne peuvent resister a l'envie de miel.\n", 1}, // FRENCH
	{" - Opposing bug types cannot resist the attraction of honey.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class ProtectivePads : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(4, 8);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	ProtectivePads(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::protective_pads, 0, IC) {
		;
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t ProtectivePads::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Parre-Effet", 1}, // FRENCH
	{"Protective Pads", 1}, // ENGLISH
	{"Schutzpolster", 1}, // GERMAN
	{"Paracontacto", 1}, // SPANISH
	{"Smorzaurti", 1}, // ITALIAN
};

dstr_t ProtectivePads::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Ignore les objets ennemi quand vous attaquez.\n", 1}, // FRENCH
	{" - Ignore opposing items when attacking.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class SafetyGoogles : public PokeItem {
public:
	static const bool RNG = false;

	defdraw(4, 7);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	SafetyGoogles(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::safety_googles, 0, IC) {
		;
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t SafetyGoogles::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Lunettes Filtre", 1}, // FRENCH
	{"Safety Googles", 1}, // ENGLISH
	{"Shutzbrille", 1}, // GERMAN
	{"Gafa Protectora", 1}, // SPANISH
	{"Visierantisabbia", 1}, // ITALIAN
};

dstr_t SafetyGoogles::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Ignore les objets adverse vous êtes attaqué.\n", 1}, // FRENCH
	{" - Ignores opposing items when attacked.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class NormalGem : public PokeItem {
public:
	static const bool RNG = false;

	defctor(NormalGem);

	defdraw(3, normal);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->type == normal) ? 2 : 0;
	}

	virtual void attack_modifier(move_data& data) {
		if (holder->type == normal) {
			data.matchup++;
		}
	}
	
	virtual void after_move_effects(move_data& data) {
		if (not (data.do_miss or data.do_crit or data.defender == NULL or data.cancel)) {
			consume();
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t NormalGem::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Joyau Normal", 1}, // FRENCH
	{"Normal Gem", 1}, // ENGLISH
	{"Normaljuwel", 1}, // GERMAN
	{"Gema Normal", 1}, // SPANISH
	{"Bijounormale", 1}, // ITALIAN
};

dstr_t NormalGem::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Uniquement pour les pieces de type normal\n\
 - Utilisable une seule fois.\n\
 - Votre premiere attaque sera super efficace.\n", 1}, // FRENCH
	{" - Only for normal type pieces.\n\
 - One time use.\n\
 - Your first attack will gain one effectiveness.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class LeppaBerry : public PokeItem {
public:
	static const bool RNG = false;

	defctor(LeppaBerry);

	defdraw(4, 10);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->Class == King::cls or piece->Class == Pawn::cls) ? 2 : 0;
	}

	virtual bool is_move_allowed(Square& target) {
		if (not holder->has_already_move)
			return false;
		holder->has_already_move = false;
		bool res = false;
		if (holder->Class == Pawn::cls) {
			res = (dynamic_cast<Pawn*>(holder))->can_double_step(target);
		}
		else if (holder->Class == King::cls) {
			res = (dynamic_cast<King*>(holder)->can_castle(target));
		}
		holder->has_already_move = true;
		return res;
	}

	virtual move_data move_to(Square& target) {
		holder->has_already_move = false;
		move_data data = holder->base_move_to(target);
		holder->has_already_move = true;
		consume();
		return data;
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t LeppaBerry::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Baie Mepo", 1}, // FRENCH
	{"Leppa Berry", 1}, // ENGLISH
	{"Jonagobeere", 1}, // GERMAN
	{"Baya Zanama", 1}, // SPANISH
	{"Baccamela", 1}, // ITALIAN
};

dstr_t LeppaBerry::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Utilisable une seule fois.\n\
 - Permet de refaire un deplacement special.\n", 1}, // FRENCH
	{" - One time use.\n\
 - Allows to perform special move a second time.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class BlackSludge : public PokeItem {
	bool activated = false;
public:
	defctor(BlackSludge);

	defdraw(3, poison);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return -(piece->type == poison);
	}

	virtual void after_move_effects(move_data& data) {
		if (holder->type != poison and ((double)game.RNG() / (double)game.RNG.max()) < 0.125) {
			holder->square->to_graveyard();
			activated = true;
			data.suicide = true;
		}
	}

	virtual void add_cosmetic(move_data& data) {
		if (activated) {
			activated = false;
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to\nBlackSludge");
			game.add_textbox(buffer);
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t BlackSludge::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Boue Noire", 1}, // FRENCH
	{"Black Sludge", 1}, // ENGLISH
	{"Giftschleim", 1}, // GERMAN
	{"Lodo Negro", 1}, // SPANISH
	{"Fangopece", 1}, // ITALIAN
};

dstr_t BlackSludge::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - A une chance de tuer les pièces qui ne sont pas de type poison quand elles se déplacent.\n", 1}, // FRENCH
	{" - Has a chance to kill non poison types piece when they move.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class HeavyDutyBoots : public PokeItem {
public:
	static const bool RNG = false;
	defctor(HeavyDutyBoots);

	defdraw(3, rock);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (typechart[piece->type][rock] == not_very_effective) ? 2 : 0;
	}

	virtual void attack_modifier(move_data& data) {
		if (data.defender != NULL and data.defender->type == rock) {
			if (data.matchup == not_very_effective) {
				data.matchup++;
			}
			else {
				data.matchup.intensity++;
			}
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t HeavyDutyBoots::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Grosses Bottes", 1}, // FRENCH
	{"Heavy-Duty Boots", 1}, // ENGLISH
	{"Plateauschuhe", 1}, // GERMAN
	{"Botas Gruesas", 1}, // SPANISH
	{"Scarponi Robusti", 1}, // ITALIAN
};

dstr_t HeavyDutyBoots::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - L'utilisateur ne sera plus resité par le type roche.\n", 1}, // FRENCH
	{" - The user wont be resisted by rock types anymore.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class LightBall : public PokeItem {
public:
	defctor(LightBall);

	virtual void update_pokeicon() {
		if (holder->type == electric and holder->Class == Pawn::cls) {
			holder->set_pokeicon(PokemonIcon(25 + holder->color, 1 + electric));
		}
	}

	defdraw(4, electric);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return (piece->Class == Pawn::cls) ? (piece->type == typeless) + 2 * (piece->type == electric) : 0;
	}
	
	virtual void crit_modifier(move_data& data) {
		if (holder->type == electric and (holder->Class == Pawn::cls or holder->evolved))
			data.crit_rate *= 3;
	}

	void promote() {
		if (holder->type == electric) {
			holder->set_pokeicon(PokemonIcon(27 + 2 * holder->Class->id + holder->color, 1 + electric));
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t LightBall::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Balle Lumiere", 1}, // FRENCH
	{"Light Ball", 1}, // ENGLISH
	{"Kugelblitz", 1}, // GERMAN
	{"Bola Luminosa", 1}, // SPANISH
	{"Elettropalla", 1}, // ITALIAN
};

dstr_t LightBall::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Uniquement pour les pions de type électrique.\n\
 - Augmente le taux de coup critique.\n", 1}, // FRENCH
	{" - Only for electric type pawns.\n\
 - Increases critical hit chance.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class Eviolite : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Eviolite);

	defdraw(4, 3);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2 * (piece->Class == Pawn::cls);
	}

	virtual void defense_modifier(move_data& data) {
		if (holder->Class == Pawn::cls) {
			if (data.matchup == super_effective) {
				data.matchup--;
			}
			else {
				data.matchup.intensity--;
			}
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t Eviolite::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Évoluroc", 1}, // FRENCH
	{"Eviolite", 1}, // ENGLISH
	{"Evolith", 1}, // GERMAN
	{"Mineral Evolutivo", 1}, // SPANISH
	{"Mineral Evol", 1}, // ITALIAN
};

dstr_t Eviolite::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Uniquement pour les pions.\n\
 - Retire toute les faiblesses.\n", 1}, // FRENCH
	{" - Only for pawns.\n\
 - Remove all weaknesses.\n", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

class Baguette : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Baguette);

	defdraw(5, 3);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2 * (piece->Class == Pawn::cls);
	}

	virtual void attack_modifier(move_data& data) {
		if (data.en_passant) {
			data.matchup++;
		}
	}

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

dstr_t Baguette::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Baguette", 1}, // FRENCH
	{"Baguette", 1}, // ENGLISH
	{"Baguette", 1}, // GERMAN
	{"Barra de Pan", 1}, // SPANISH
	{"Filoncino", 1}, // ITALIAN
};

dstr_t Baguette::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Une baguette.\n - Les prises en passant son super efficace.\n", 1}, // FRENCH
	{" - A baguette.\n - En passant are super effective.\n", 1}, // ENGLISH
	{" - Ein Baguette.\n", 1}, // GERMAN
	{" - Una barra de pan.\n", 1}, // SPANISH
	{" - Una baguette.\n", 1}, // ITALIAN
};


template<typing t1, typing t2>
class KingPromotionItem : public PokeItem {
public:
	static const bool RNG = false;
	KingPromotionItem(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {
	}
	
	virtual void update_pokeicon() {
		if (holder->Class == Pawn::cls and (holder->type == t1 or holder->type == t2))
			holder->set_pokeicon(PokemonIcon(25 + holder->color, 1 + t1));
	}

	defdraw(3, t1);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];


	static int usefulness_tier(Piece* piece) {
		return (piece->Class == Pawn::cls) ? (piece->type == typeless) + 2 * (piece->type == t1 or piece->type == t2) : 0;
	}

#define ctor(Class) [](Board& b, piece_color color, Square* sq, typing type, PokeItem* item) -> Piece* { \
	Piece* piece = new Class(b, color, sq, type, item); \
	piece->has_already_move = piece->evolved = true; \
	piece->set_pokeicon(PokemonIcon(27 + 2 * Class::cls->id + piece->color, 1 + piece->type)); \
	return piece; \
}

	virtual bool prepare_promotion() {
		if (holder->type == t1 or holder->type == t2) {

			game.buttons->add(new PromotionButton(ctor(King), 6.5, 5.5));
			game.buttons->add(new PromotionButton(ctor(Queen), 7.5, 5.5));
			game.buttons->add(new PromotionButton(ctor(Rook), 8.5, 5.5));
			game.buttons->add(new PromotionButton(ctor(Bishop), 9.5, 5.5));
			game.buttons->add(new PromotionButton(ctor(Knight), 10.5, 5.5));

			return true;
		}
		return false;
	}
#undef ctor

	virtual void promote() {
		if (holder->Class == King::cls) {
			consume();
		}
	}
};

dstr_t KingPromotionItem<steel, dark>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Emblème du Général", 1}, // FRENCH
	{"Leader's Crest", 1}, // ENGLISH
	{"Anführersymbol", 1},// GERMAN
	{"Distintivo de Líder", 1}, // SPANISH
	{"Simbolo del capo", 1}, // ITALIAN
};

dstr_t KingPromotionItem<psychic, water>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Roche Royale", 1}, // FRENCH
	{"King's rock", 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};

template<typing t1, typing t2>
dstr_t KingPromotionItem<t1, t2>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{to_c_str(std::string(" - Uniquement pour les pions de type ").append(type_str[0][t1]).append((t1 != t2) ? std::string("/").append(type_str[0][t2]) : "").append(".\n\
 - Permet la promotion en Roi.\n")), 1}, // FRENCH
	{to_c_str(std::string(" - Only for ").append(type_str[1][t1]).append((t1 != t2) ? std::string("/").append(type_str[1][t2]) : "").append(" type pawns.\n\
 - Allow promotion to King.\n")), 1}, // ENGLISH
	{"", 1}, // GERMAN
	{"", 1}, // SPANISH
	{"", 1}, // ITALIAN
};


class IronBall : public PokeItem {
public:
	static bool const RNG = false;

	IronBall(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::no_item, -1, IC) {};

	defdraw(5, 2);

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return -1;
	}

	virtual void defense_modifier(move_data& data) {
		if (data.attacker->type == ground and data.matchup == super_effective) {
			data.matchup = neutral;
		}
	}

	virtual bool is_move_disallowed(Square& target) {
		if (holder->Class == Pawn::cls) {
			Pawn* piece = dynamic_cast<Pawn*>(holder);

			if (piece->can_double_step(target)) {
				return true;
			}
		}
		else if (holder->Class == King::cls) {
			King* piece = dynamic_cast<King*>(holder);

			if (piece->can_castle(target)) {
				return true;
			}
		}
		return false;
	}
};


dstr_t IronBall::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{"Balle Fer", 1}, // FRENCH
	{"Iron Ball", 1}, // ENGLISH
	{"Eisenkugel", 1}, // GERMAN
	{"Bola Férrea", 1}, // SPANISH
	{"Ferropalla", 1}, // ITALIAN
};

dstr_t IronBall::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{" - Retire une potentielle immunité sol.\n - Les rois ne peuvent plus roquer.\n - Les pions ne peuvent plus faire de double pas.\n", 1}, // FRENCH
	{" - Remove any ground immunity.\n - Kings/Pawns can no longer castle/double step.\n", 1}, // ENGLISH
	{" - Entfernt jegliche Bodenimmunität.\n", 1}, // GERMAN
	{" - Una barra de pan.\n", 1}, // SPANISH
	{" - Una baguette.\n", 1}, // ITALIAN
};


int number_of_drawed_terashard = 0;
template<typing t>
class TeraShard : public PokeItem {
public:
	static bool const RNG = false;

	TeraShard(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {
		number_of_drawed_terashard++;
	}

	defdraw(2, t);

	static int usefulness_tier(Piece* piece) {
		return 2 * (piece->type != t) - number_of_drawed_terashard;
	}

	virtual void select_holder() {
		if ((holder->color == white and game.board.white_tera) or (holder->color == black and game.board.black_tera))
			game.buttons->add(new TeraButton(9, 10.5, holder, t));
	}

	static dstr_t name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static dstr_t description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

template<typing t>
dstr_t TeraShard<t>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{to_c_str(std::string("Téra-Eclat ").append(type_str_cap[0][t])), 0.75}, // FRENCH
	{to_c_str(std::string(type_str_cap[1][t]).append(" Tera Shard")), 0.75}, // ENGLISH
	{to_c_str(std::string("Tera-Stück (").append(type_str_cap[2][t]).append(")")), 0.75}, // GERMAN
	{to_c_str(std::string("Teralito ").append(type_str_cap[3][t])), 0.75}, // SPANISH
	{to_c_str(std::string("Teralite ").append(type_str_cap[4][t])), 0.75}, // ITALIAN
};

template<typing t>
dstr_t TeraShard<t>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	{to_c_str(std::string(" - Permet son utilisateur de teracristal en type ").append(type_str[0][t]).append(".")), 1}, // FRENCH
	{to_c_str(std::string(" - Allow its user to terastalize into a ").append(type_str[1][t]).append(" type.")), 1}, // ENGLISH
	{to_c_str(std::string("Tera-Stück (").append(type_str[2][t]).append(")")), 1}, // GERMAN
	{to_c_str(std::string("Teralito ").append(type_str[3][t])), 1}, // SPANISH
	{to_c_str(std::string("Teralite ").append(type_str[4][t])), 1}, // ITALIAN
};

constexpr Uint32 get_name_hash(char const* name) {
	Uint32 res = 0;
	Uint32 p = 1;
	while (name[0] != '\0') {
		res += name[0] * p;
		p *= 1237;
		name++;
	}
	return res;
}

ItemClass item_table[NB_OF_ITEMS];

#include <exception>
#include <iostream>
void* init_item_table() {
	int counter = 0;

#define Itemize(Class) \
{ \
	Uint32 ch = get_name_hash(#Class); \
	std::cout << "hash(" << #Class << ") = " << ch << '\n'; \
	if (ch == 0) \
		std::cout << "Invalid name for Item " << #Class << " try to add some trailing _ after it's name\n"; \
	for (int i = 0; i < NB_OF_ITEMS; i++) { \
		ItemClass& IC = item_table[i]; \
		if (IC.type == normal_item and IC.class_hash == ch) { \
			std::cout << "Invalid name for Item " << #Class << " try to add some trailing _ after it's name\n"; \
			throw std::exception(""); \
		} \
	} \
	item_table[counter++] = ItemClass([](Piece* piece, ItemClass& IC) -> PokeItem* { return new Class(piece, IC); }, ch, Class::usefulness_tier, Class::draw, Class::name, Class::description, Class::RNG); \
}
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
	Itemize(KingPromotionItem<steel _ dark>);
	Itemize(KingPromotionItem<psychic _ water>);
	Itemize(Everstone);

	EndLine();

#define ItemizeAllTypes(Class) \
Itemize(Class<normal>); \
Itemize(Class<fire>); \
Itemize(Class<water>); \
Itemize(Class<grass>); \
Itemize(Class<electric>); \
Itemize(Class<ice>); \
Itemize(Class<fighting>); \
Itemize(Class<poison>); \
Itemize(Class<ground>); \
Itemize(Class<flying>); \
Itemize(Class<psychic>); \
Itemize(Class<bug>); \
Itemize(Class<rock>); \
Itemize(Class<ghost>); \
Itemize(Class<dragon>); \
Itemize(Class<dark>); \
Itemize(Class<steel>); \
Itemize(Class<fairy>);
	
	ItemizeAllTypes(ResistanceBerry);

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
	Itemize(LightBall);
	Itemize(LoadedDice);
	Itemize(Metronome);
	Itemize(QuickClaw);
	
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
	Itemize(IronBall);

	Itemize(Baguette);
	EndLine();

	ItemizeAllTypes(TeraShard);

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
}

ItemClass::ItemClass(PokeItem* (*ctor)(Piece*, ItemClass&), Uint32 ch, int (*w)(Piece*), void (*d)(Surface, SDL_Rect*, size, anchor), dstr_t _name[], dstr_t* desc, bool RNG) {
	constructor = ctor;
	usefulness_tier = w;
	is_avaible = true;
	type = normal_item;
	description = desc;
	_draw = d;
	is_RNG_dependant = RNG;
	name = _name;
	class_hash = ch;
}

PokeItem* ItemClass::operator()(Piece* piece) {
	auto item = constructor(piece, self);
	return item;
}



void* _table_initializer = init_item_table();