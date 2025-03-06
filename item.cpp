#include <array>

#include "item.h"
#include "game.h"
#include "board.h"

move_data PokeItem::move_to(Square& target) {
	return holder->base_move_to(target);
}


PokeItem::PokeItem(Piece* piece, item_id id_, int prio, ItemClass& IC) : id(id_), holder(piece), priority(prio), cls(IC) {
	used = false;
	holder = piece;
}

PokeItem::PokeItem(Piece* piece, ItemClass& IC) : id(item_id::basic), priority(0), cls(IC) {
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

	

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};
template<typing t1, typing t2>
const char* EvolutionStone<t1, t2>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	evolution_stone_names[0][t2],
	evolution_stone_names[1][t2],
	evolution_stone_names[2][t2],
	evolution_stone_names[3][t2],
	evolution_stone_names[4][t2],
};

constexpr const char* to_c_str(std::string str) {
	char* buffer = new char[str.length()+1];
	int i = 0;
	for (char c : str) {
		buffer[i++] = c;
	}
	buffer[i] = '\0';
	return buffer;
}

template<typing t1, typing t2>
const char* EvolutionStone<t1, t2>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	to_c_str(std::string(" - Uniquement pour les pions.\n\
 - Si l'utilisateur est de type ").append(type_str[(int)LANGUAGE::FRENCH][t1]).append(", il deviendra de type ").append(type_str[(int)LANGUAGE::FRENCH][t2]).append(" en évoluant.\n")), // FRENCH
	to_c_str(std::string(" - Only for pawns.\n\
 - Makes ").append(type_str[(int)LANGUAGE::ENGLISH][t1]).append(" types, evolve into a ").append(type_str[(int)LANGUAGE::ENGLISH][t2]).append(" type.\n")), // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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
		return (piece->Class == Pawn::cls) ? -1 : 0;
	}

	virtual bool prepare_promotion() {
		game.to_game(true);
		game.resume_move();
		return true;
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* Everstone::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	evolution_stone_names[0][0],
	evolution_stone_names[1][0],
	evolution_stone_names[2][0],
	evolution_stone_names[3][0],
	evolution_stone_names[4][0],
};

const char* Everstone::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Empêche son porteur d'évoluer.\n", // FRENCH
	" - Prevents its user from evolving.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

template<typing type>
const char* ResistanceBerry<type>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	type != typeless ? resistance_berry_names[0][type] : "",
	type != typeless ? resistance_berry_names[1][type] : "",
	type != typeless ? resistance_berry_names[2][type] : "",
	type != typeless ? resistance_berry_names[3][type] : "",
	type != typeless ? resistance_berry_names[4][type] : "",
};

template<typing type>
const char* ResistanceBerry<type>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	to_c_str(std::string(" - Retire une faiblesse contre le type ").append(type_str[(int)LANGUAGE::FRENCH][type]).append(".\n")), // FRENCH
	to_c_str(std::string(" - Removes a ").append(type_str[(int)LANGUAGE::ENGLISH][type]).append(" type weakness.\n")), // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

const char* ResistanceBerry<normal>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Donne une resistance contre le type normal.\n", // FRENCH
	" - Gives a normal type resistance.\n", // ENGLISH
	"",
	"",
	"",
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* RingTarget::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Point de Mire", // FRENCH
	"Ring Target", // ENGLISH
	"Zielscheibe", // GERMAN
	"Blanco", // SPANISH
	"Facilsaglio", // ITALIAN
};

const char* RingTarget::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Retire toutes les immunités de son utilisateur.\n", // FRENCH
	" - Removes all the immunity of its user.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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
			strcat_s(buffer, holder->Class->name);
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
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

template<typing type>
const char* ImmunityItem<type>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	to_c_str(std::string(" - Utilisable une unique fois.\n - Donne une immunité contre le type ").append(type_str[(int)LANGUAGE::FRENCH][type]).append(".\n")), // FRENCH
	to_c_str(std::string(" - One time use.\n - Gives an immunity to ").append(type_str[(int)LANGUAGE::ENGLISH][type]).append(".\n")), // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	virtual void accuracy_modifier(move_data& data) {
		if (data.matchup == super_effective)
			data.miss_rate = -INFINITY;
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* ExpertBelt::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Ceinture Pro", // FRENCH
	"Expert Belt", // ENGLISH
	"Expertengurt", // GERMAN
	"Cinta Experto", // SPANISH
	"Abilcintura", // ITALIAN
};

const char* ExpertBelt::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Empeche d'echouer les attaques super efficace.\n", // FRENCH
	" - Prevents super effective move from missing.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class BlunderPolicy : public PokeItem {
public:
	BlunderPolicy(Piece* piece, ItemClass& IC) : PokeItem(piece, item_id::basic, 0, IC) {
		;
	}

	defdraw(3, 9);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	virtual void accuracy_modifier(move_data& data) {
		data.miss_rate = -INFINITY; // cannot miss
		consume();
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};



const char* BlunderPolicy::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Assurance Échec", // FRENCH
	"Blunder Policy", // ENGLISH
	"Fehlschlagschutz", // GERMAN
	"Seguro Fallo", // SPANISH
	"Fiascopolizza", // ITALIAN
};

const char* BlunderPolicy::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Utilisable une unique fois.\n\
 - Son utilisateur ne peut pas echoué sa premiere attaque.\n", // FRENCH
	" - One time use.\n\
 - Its user cannot miss his first attack.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

#define defctor(name) name(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {}
class BrightPowder : public PokeItem {
public:
	defctor(BrightPowder);

	defdraw(3, 15);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


	virtual void evasion_modifier(move_data& data) {
		data.miss_rate = 1 - ((1 - data.miss_rate) * 1.1);
	}

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* BrightPowder::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Poudre Claire", // FRENCH
	"Bright Powder", // ENGLISH
	"Blendpuder", // GERMAN
	"Polvo Brillo", // SPANISH
	"Luminpolvere", // ITALIAN
};

const char* BrightPowder::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Augmente légerement l'ésquive.\n", // FRENCH
	" - Increase slightly evasion.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class LifeOrb : public PokeItem {
	bool activated = false;
public:
	defctor(LifeOrb);

	defdraw(3, 5);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];


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
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name);
			strcat_s(buffer, "\nDied to life orb");
			game.add_textbox(buffer);
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* LifeOrb::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Orbe Vie", // FRENCH
	"Life Orb", // ENGLISH
	"Leben-Orb", // GERMAN
	"Vidasfera", // SPANISH
	"Assorbisfera", // ITALIAN
};

const char* LifeOrb::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Son Utilisateur a 1/10 chance de mourir a chaque fois qu'il attaque.\n\
 - Augmente énormément le taux de coup critique.\n", // FRENCH
	" - Its user has a 1 / 10 chance to die each time he attacks.\n\
 - Greatly increase critical hit rate.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* LoadedDice::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"De Pipe", // FRENCH
	"Loaded Dice", // ENGLISH
	"Gezinkter Würfel", // GERMAN
	"Dado Trucado", // SPANISH
	"Dado Truccato", // ITALIAN
};

const char* LoadedDice::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Chaque utilisation d'aléatoire concernant son utilisateur est effectuée avec \"avantage\".\n", // FRENCH
	" - Each rng roll affecting its user is perform with \"advantage\".\n" // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class RockyHelmet : public PokeItem {
	bool activated = false;
public:
	defctor(RockyHelmet);
	
	defdraw(3, 0);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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
			strcat_s(buffer, data.attacker->Class->name);
			strcat_s(buffer, "\nDied to Rcky Hlmt");
			game.add_textbox(buffer);
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* RockyHelmet::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Casque Brut", // FRENCH
	"Rocky Helmet", // ENGLISH
	"Beulenhelm", // GERMAN
	"Casco dentado", // SPANISH
	"Bitorzolelmo", // ITALIAN
};

const char* RockyHelmet::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Si son utilisateur meurt d'une piece enemie, cette piece a une chance de mourrir.\n", // FRENCH
	" - If its user die to an enemy piece, this piece has a chance to die.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	virtual void crit_modifier(move_data& data) {
		for (move_data& const d : holder->board.move_historic) {
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* Metronome::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Métronome", // FRENCH
	"Metronome", // ENGLISH
	"Metronom", // GERMAN
	"Metrónomo", // SPANISH
	"Plessimetro", // ITALIAN
};

const char* Metronome::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Augmente le taux de critique pour chaques coups consecutifs.\n", // FRENCH
	" - Increase critical hit rate for each consecutive moves.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class RedCard : public PokeItem {
	bool activated = false;
public:
	static const bool RNG = false;

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	defctor(RedCard);

	defdraw(3, 1);

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
			strcat_s(buffer, data.attacker->Class->name);
			strcat_s(buffer, "\nwas sent back.");
			game.add_textbox(buffer);
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* RedCard::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Carton Rouge", // FRENCH
	"Red Card", // ENGLISH
	"Rote Karte", // GERMAN
	"tarjeta roja", // SPANISH
	"Cartelrosso", // ITALIAN
};

const char* RedCard::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Si son utilisateur meurt d'une attaque qui résulte en un coup bonus, l'attaquant revient sur sa case de départ.\n", // FRENCH
	" - If its user die to an attack that results in a bonus move, the attacker gets send back to its starting square.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class StickyBarbs : public PokeItem {
	bool activated;
public:
	defctor(StickyBarbs);

	defdraw(3, 2);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void revenge(move_data& data) {
		if (holder->is_in_graveyard) {
			consume();
			used = false;
			if (data.attacker_item_slot != NULL)
				data.attacker_item_slot->consume();
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
			strcat_s(buffer, holder->Class->name);
			strcat_s(buffer, "\nDied to StckyBrbs");
			game.add_textbox(buffer);
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* StickyBarbs::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Piquants", // FRENCH
	"Sticky Barb", // ENGLISH
	"Klettdorn", // GERMAN
	"Toxiestrella", // SPANISH
	"Vischiopunta", // ITALIAN
};

const char* StickyBarbs::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Son utilisateur a une chance de mourrir quand il se déplace\n\
 - Colle aux pièces ennemie.\n", // FRENCH
	" - Its user has a chance to die each time it moves\n\
 - Stick to enemy piece.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* AssaultVest::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Veste de Combat", // FRENCH
	"Assault Vest", // ENGLISH
	"Offensivweste", // GERMAN
	"Chaleco Asalto", // SPANISH
	"Corpetto assalto", // ITALIAN
};

const char* AssaultVest::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Empêche les coups qui ne sont pas des captures.\n", // FRENCH
	" - Blocks non capturing moves.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class ShedShell : public PokeItem {
	bool activated = false;
public:
	static const bool RNG = false;

	defctor(ShedShell);

	defdraw(3, 16);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name);
			strcat_s(buffer, "\nfled successfuly");
			game.add_textbox(buffer);
			consume();
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* ShedShell::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Carapace Mue", // FRENCH
	"Shed Shell", // ENGLISH
	"Wechselhülle", // GERMAN
	"Muda Concha", // SPANISH
	"Disfoguscio", // ITALIAN
};

const char* ShedShell::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Utilisable une seule fois.\n\
 - Si vous êtes piegé vous pouvez vous deplacez en cavalier.\n", // FRENCH
	" - One time use.\n\
 - If you are trapped you can perform a knight move.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class ScopeLens : public PokeItem {
public:
	defctor(ScopeLens);

	defdraw(3, 12);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void crit_modifier(move_data& data) {
		data.crit_rate *= 1.3;
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* ScopeLens::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Lentilscope", // FRENCH
	"Scope Lens", // ENGLISH
	"Scope-Linse", // GERMAN
	"Periscopio", // SPANISH
	"Mirino", // ITALIAN
};

const char* ScopeLens::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Augmente légerement le taux de critique.\n", // FRENCH
	" - Increase slightly critical hit rate.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class WideLens : public PokeItem {
public:
	defctor(WideLens);

	defdraw(3, 11);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2;
	}

	virtual void accuracy_modifier(move_data& data) {
		data.miss_rate /= 2;
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* WideLens::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Loupe", // FRENCH
	"Wide Lens", // ENGLISH
	"Grosslinse", // GERMAN
	"Lupa", // SPANISH
	"Grandelente", // ITALIAN
};

const char* WideLens::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Augmente la precision.\n", // FRENCH
	" - Increase accuracy.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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


	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* Honey::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Miel", // FRENCH
	"Honey", // ENGLISH
	"Honig", // GERMAN
	"Miel", // SPANISH
	"Miele", // ITALIAN
};

const char* Honey::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Les types insectes adverse ne peuvent resister a l'envie de miel.\n", // FRENCH
	" - Opposing bug types cannot resist the attraction of honey.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* ProtectivePads::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Parre-Effet", // FRENCH
	"Protective Pads", // ENGLISH
	"Schutzpolster", // GERMAN
	"Paracontacto", // SPANISH
	"Smorzaurti", // ITALIAN
};

const char* ProtectivePads::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Ignore les objets ennemi quand vous attaquez.\n", // FRENCH
	" - Ignore opposing items when attacking.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* SafetyGoogles::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Lunettes Filtre", // FRENCH
	"Safety Googles", // ENGLISH
	"Shutzbrille", // GERMAN
	"Gafa Protectora", // SPANISH
	"Visierantisabbia", // ITALIAN
};

const char* SafetyGoogles::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Ignore les objets adverse vous êtes attaqué.\n", // FRENCH
	" - Ignores opposing items when attacked.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* NormalGem::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Joyau Normal", // FRENCH
	"Normal Gem", // ENGLISH
	"Normaljuwel", // GERMAN
	"Gema Normal", // SPANISH
	"Bijounormale", // ITALIAN
};

const char* NormalGem::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Uniquement pour les pieces de type normal\n\
 - Utilisable une seule fois.\n\
 - Vôtre première attaque sera super efficace.\n", // FRENCH
	" - Only for normal type pieces.\n\
 - One time use.\n\
 - Vôtre première attaque sera super efficace.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class LeppaBerry : public PokeItem {
public:
	static const bool RNG = false;

	defctor(LeppaBerry);

	defdraw(3, 10);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* LeppaBerry::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Baie Mepo", // FRENCH
	"Leppa Berry", // ENGLISH
	"Jonagobeere", // GERMAN
	"Baya Zanama", // SPANISH
	"Baccamela", // ITALIAN
};

const char* LeppaBerry::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Utilisable une seule fois.\n\
 - Permet de refaire un deplacement special.\n", // FRENCH
	" - One time use.\n\
 - Allows to perform special move a second time.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class BlackSludge : public PokeItem {
	bool activated = false;
public:
	defctor(BlackSludge);

	defdraw(2, poison);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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
			char buffer[256] = "\0";
			strcat_s(buffer, holder->Class->name);
			strcat_s(buffer, "\nDied to\nBlackSludge");
			game.add_textbox(buffer);
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* BlackSludge::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Boue Noire", // FRENCH
	"Black Sludge", // ENGLISH
	"Giftschleim", // GERMAN
	"Lodo Negro", // SPANISH
	"Fangopece", // ITALIAN
};

const char* BlackSludge::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - A une chance de tuer les pièces qui ne sont pas de type poison quand elles se déplacent.\n", // FRENCH
	" - Has a chance to kill non poison types piece when they move.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* HeavyDutyBoots::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Grosses Bottes", // FRENCH
	"Heavy-Duty Boots", // ENGLISH
	"Plateauschuhe", // GERMAN
	"Botas Gruesas", // SPANISH
	"Scarponi Robusti", // ITALIAN
};

const char* HeavyDutyBoots::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - L'utilisateur ne sera plus resité par le type roche.\n", // FRENCH
	" - The user wont be resisted by rock types anymore.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class LightBall : public PokeItem {
public:
	defctor(LightBall);

	virtual void update_pokeicon() {
		if (holder->type == electric and holder->Class == Pawn::cls) {
			holder->set_pokeicon(PokemonIcon(25 + holder->color, 1 + electric));
		}
	}

	defdraw(3, electric);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* LightBall::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Balle Lumière", // FRENCH
	"Light Ball", // ENGLISH
	"Kugelblitz", // GERMAN
	"Bola Luminosa", // SPANISH
	"Elettropalla", // ITALIAN
};

const char* LightBall::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Uniquement pour les pions de type électrique.\n\
 - Augmente le taux de coup critique.\n", // FRENCH
	" - Only for electric type pawns.\n\
 - Increases critical hit chance.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class Eviolite : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Eviolite);

	defdraw(3, 3);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

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

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* Eviolite::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Évoluroc", // FRENCH
	"Eviolite", // ENGLISH
	"Evolith", // GERMAN
	"Mineral Evolutivo", // SPANISH
	"Mineral Evol", // ITALIAN
};

const char* Eviolite::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Uniquement pour les pions.\n\
 - Retire toute les faiblesses.\n", // FRENCH
	" - Only for pawns.\n\
 - Remove all weaknesses.\n", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

class Baguette : public PokeItem {
public:
	static const bool RNG = false;

	defctor(Baguette);

	defdraw(4, 3);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static int usefulness_tier(Piece* piece) {
		return 2 * (piece->Class == Pawn::cls);
	}

	virtual void attack_modifier(move_data& data) {
		if (data.en_passant) {
			data.move_again = true;
			data.is_super_effective = true;
		}
	}

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];
};

const char* Baguette::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Baguette", // FRENCH
	"Baguette", // ENGLISH
	"Baguette", // GERMAN
	"Barra de Pan", // SPANISH
	"Filoncino", // ITALIAN
};

const char* Baguette::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	" - Une baguette.\n", // FRENCH
	" - A baguette.\n", // ENGLISH
	" - Ein Baguette.\n", // GERMAN
	" - Una barra de pan.\n", // SPANISH
	" - Una baguette.\n", // ITALIAN
};


template<typing t1, typing t2>
class KingPromotionItem : public PokeItem {
public:
	static const bool RNG = false;
	KingPromotionItem(Piece* piece, ItemClass& IC) : PokeItem(piece, IC) {
	}
	
	virtual void update_pokeicon() {
		if (holder->Class == Pawn::cls and (holder->type == t1 or holder->type == t2))
			holder->set_pokeicon(PokemonIcon(25 + holder->color, 1 + holder->type));
	}

	defdraw(2, t1);

	static const char* name[(int)LANGUAGE::NB_OF_LANGUAGE];

	static const char* description[(int)LANGUAGE::NB_OF_LANGUAGE];


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

const char* KingPromotionItem<steel, dark>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Emblème du Général", // FRENCH
	"Leader's Crest", // ENGLISH
	"Anführersymbol", // GERMAN
	"Distintivo de Líder", // SPANISH
	"Simbolo del capo", // ITALIAN
};

const char* KingPromotionItem<psychic, water>::name[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	"Roche Royale", // FRENCH
	"King's rock", // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

template<typing t1, typing t2>
const char* KingPromotionItem<t1, t2>::description[(int)LANGUAGE::NB_OF_LANGUAGE] = {
	to_c_str(std::string(" - Uniquement pour les pions de type ").append(type_str[0][t1]).append((t1 != t2) ? std::string("/").append(type_str[0][t2]) : "").append(".\n\
 - Permet la promotion en Roi.\n")), // FRENCH
	to_c_str(std::string(" - Only for ").append(type_str[1][t1]).append((t1 != t2) ? std::string("/").append(type_str[1][t2]) : "").append(" type pawns.\n\
 - Allow promotion to King.\n")), // ENGLISH
	"", // GERMAN
	"", // SPANISH
	"", // ITALIAN
};

void* init_item_table() {
	int counter = 0;

#define Itemize(class) item_table[counter++] = ItemClass([](Piece* piece, ItemClass& IC) -> PokeItem* { return new class(piece, IC); }, class::usefulness_tier, class::draw, class::name, class::description, class::RNG);
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
	item_table[counter++] = ItemClass([](Piece* piece, ItemClass& IC) -> PokeItem* { return new LightBall(piece, IC); }, LightBall::usefulness_tier, LightBall::draw, LightBall::name, LightBall::description, LightBall::RNG);;
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
}

ItemClass::ItemClass(PokeItem* (*ctor)(Piece*, ItemClass&), int (*w)(Piece*), void (*d)(Surface, SDL_Rect*, size, anchor), const char* _name[], const char** desc, bool RNG) {
	constructor = ctor;
	usefulness_tier = w;
	is_avaible = true;
	type = normal_item;
	description = desc;
	_draw = d;
	is_RNG_dependant = RNG;
	name = _name;
}

PokeItem* ItemClass::operator()(Piece* piece) {
	auto item = constructor(piece, self);
	return item;
}

ItemClass item_table[NB_OF_ITEMS];

void* _table_initializer = init_item_table();