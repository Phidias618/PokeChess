#include "poketyping.h"

#include <cstdint>

/*
This file contains all things that are related to pokemon in pokemon chess
*/

const char* type_str[NB_OF_LANGUAGE][18] = {
	{ // FRENCH
		"normal",
		"feu",
		"eau",
		"plante",
		"electrique",
		"glace",
		"combat",
		"poison",
		"sol",
		"vol",
		"psy",
		"insecte",
		"roche",
		"spectre",
		"dragon",
		"tenebres",
		"acier",
		"fee",
	},
	{ // ENGLISH
		"normal",
		"fire",
		"water",
		"grass",
		"electric",
		"ice",
		"fighting",
		"poison",
		"ground",
		"flying",
		"psychic",
		"bug",
		"rock",
		"ghost",
		"dragon",
		"dark",
		"steel",
		"fairy",
	},
	{ // GERMAN
		"normal",
		"feuer",
		"wasser",
		"pflanze",
		"elektro",
		"eis",
		"kampf",
		"gift",
		"boden",
		"flug",
		"psycho",
		"kafer",
		"gestein",
		"geist",
		"drache",
		"unlicht",
		"stahl",
		"fee"
	},
	{ // SPANISH
		"normal",
		"fuego",
		"agua",
		"planta",
		"eléctrico",
		"hielo",
		"lucha",
		"veneno",
		"tierra",
		"volador",
		"psiquico",
		"bicho",
		"roca",
		"fantasma",
		"dragon",
		"siniestro",
		"acero",
		"hada",
	},
	{ // ITALIAN
		"normale",
		"fuoco",
		"acqua",
		"erba",
		"elettro",
		"ghiaccio",
		"lotta",
		"veleno",
		"terra",
		"volante",
		"psico",
		"coleottero",
		"roccia",
		"ombra",
		"drago",
		"buio",
		"acciaio",
		"folletto",
	}
};

const char* type_str_cap[NB_OF_LANGUAGE][18] = {
	{ // FRENCH
		"Normal",
		"Feu",
		"Eau",
		"Plante",
		"Electrique",
		"Glace",
		"Combat",
		"Poison",
		"Sol",
		"Vol",
		"Psy",
		"Insecte",
		"Roche",
		"Spectre",
		"Dragon",
		"Tenebres",
		"Acier",
		"Fee",
	},
	{ // ENGLISH
		"Normal",
		"Fire",
		"Water",
		"Grass",
		"Electric",
		"Ice",
		"Fighting",
		"Poison",
		"Ground",
		"Flying",
		"Psychic",
		"Bug",
		"Rock",
		"Ghost",
		"Dragon",
		"Dark",
		"Steel",
		"Fairy",
	},
	{ // GERMAN
		"Normal",
		"Feuer",
		"Wasser",
		"Pflanze",
		"Elektro",
		"Eis",
		"Kampf",
		"Gift",
		"Boden",
		"Flug",
		"Psycho",
		"Kafer",
		"Gestein",
		"Geist",
		"Drache",
		"Unlicht",
		"Stahl",
		"Fee"
	},
	{ // SPANISH
		"Normal",
		"Fuego",
		"Agua",
		"Planta",
		"Electrico",
		"Hielo",
		"Lucha",
		"Veneno",
		"Tierra",
		"Volador",
		"Psiquico",
		"Bicho",
		"Roca",
		"Fantasma",
		"Dragon",
		"Siniestro",
		"Acero",
		"Hada",
	},
	{ // ITALIAN
		"Normale",
		"Fuoco",
		"Acqua",
		"Erba",
		"Elettro",
		"Ghiaccio",
		"Lotta",
		"Veleno",
		"Terra",
		"Volante",
		"Psico",
		"Coleottero",
		"Roccia",
		"Ombra",
		"Drago",
		"Buio",
		"Acciaio",
		"Folletto",
	}
};

effectiveness& operator++(effectiveness& x) {
	if (x.main == immune || x.main == super_effective || x.intensity < 0) {
		++x.intensity;
		return x;
	}
	else {
		++(char&)x;
		return x;
	}
}

effectiveness operator++(effectiveness& x, int) {
	effectiveness old = x;
	++x;
	return old;
}

effectiveness& operator--(effectiveness& x) {
	if (x.main == immune || x.main == not_very_effective || x.intensity > 0) {
		x.intensity--;
		return x;
	}
	else {
		--(char&)x;
		return x;
	}
}

effectiveness operator--(effectiveness& x, int) {
	effectiveness old = x;
	--x;
	return old;
}

#define NVE not_very_effective
#define SE super_effective

/*the line typing corresponds to the attacking typing while the column typing corresponds to the defensive typing*/
effectiveness_enum const typechart[18][18] =
/*             normal   fire     water    grass    electrik ice      fighting poison   ground   flying   psychic  bug      rock     ghost    dragon   dark     steel    fairy */
{/*normal*/   {neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, NVE    , immune , neutral, neutral, NVE    , neutral},
 /*fire*/     {neutral, NVE    , NVE    , SE     , neutral, SE     , neutral, neutral, neutral, neutral, neutral, SE     , NVE    , neutral, NVE    , neutral, SE     , neutral},
 /*water*/    {neutral, SE     , NVE    , NVE    , neutral, neutral, neutral, neutral, SE     , neutral, neutral, neutral, SE     , neutral, NVE    , neutral, neutral, neutral},
 /*grass*/    {neutral, NVE    , SE     , NVE    , neutral, neutral, neutral, NVE    , SE     , NVE    , neutral, NVE    , SE     , neutral, NVE    , neutral, NVE    , neutral},
 /*electrik*/ {neutral, neutral, SE     , NVE    , NVE    , neutral, neutral, neutral, immune , SE     , neutral, neutral, neutral, neutral, NVE    , neutral, neutral, neutral},
 /*ice*/      {neutral, NVE    , NVE    , SE     , neutral, NVE    , neutral, neutral, SE     , SE     , neutral, neutral, neutral, neutral, SE     , neutral, NVE    , neutral},
 /*fighting*/ {SE     , neutral, neutral, neutral, neutral, SE     , neutral, NVE    , neutral, NVE    , NVE    , NVE    , SE     , immune , neutral, SE     , SE     , NVE    },
 /*poison*/   {neutral, neutral, neutral, SE     , neutral, neutral, neutral, NVE    , NVE    , neutral, neutral, neutral, NVE    , NVE    , neutral, neutral, immune , SE     },
 /*ground*/   {neutral, SE     , neutral, NVE    , SE     , neutral, neutral, SE     , neutral, immune , neutral, NVE    , SE     , neutral, neutral, neutral, SE     , neutral},
 /*flying*/   {neutral, neutral, neutral, SE     , NVE    , neutral, SE     , neutral, neutral, neutral, neutral, SE     , NVE    , neutral, neutral, neutral, NVE    , neutral},
 /*psychic*/  {neutral, neutral, neutral, neutral, neutral, neutral, SE     , SE     , neutral, neutral, NVE    , neutral, neutral, neutral, neutral, immune , NVE    , neutral},
 /*bug*/      {neutral, NVE    , neutral, SE     , neutral, neutral, NVE    , NVE    , neutral, NVE    , SE     , neutral, neutral, NVE    , neutral, SE     , NVE    , NVE    },
 /*rock*/     {neutral, SE     , neutral, neutral, neutral, SE     , NVE    , neutral, NVE    , SE     , neutral, SE     , neutral, neutral, neutral, neutral, NVE    , neutral},
 /*ghost*/    {immune , neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, SE     , neutral, neutral, SE     , neutral, NVE    , neutral, neutral},
 /*dragon*/   {neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, neutral, SE     , neutral, NVE    , immune },
 /*dark*/     {neutral, neutral, neutral, neutral, neutral, neutral, NVE    , neutral, neutral, neutral, SE     , neutral, neutral, SE     , neutral, NVE    , neutral, NVE    },
 /*steel*/    {neutral, NVE    , NVE    , neutral, NVE    , SE     , neutral, neutral, neutral, neutral, neutral, neutral, SE     , neutral, neutral, neutral, NVE    , SE     },
 /*fairy*/    {neutral, NVE    , neutral, neutral, neutral, neutral, SE     , NVE    , neutral, neutral, neutral, neutral, neutral, neutral, SE     , SE     , NVE    , neutral}
};

#undef NVE
#undef SE


effectiveness get_effectiveness(typing attacking_typing, typing defensing_typing, bool reverse) {
	effectiveness_enum res;
	if (attacking_typing == typeless || defensing_typing == typeless) {
		res = neutral;
	}
	else {
		res = typechart[attacking_typing][defensing_typing];
	}
	if (reverse) {
		if (res <= not_very_effective)
			res = super_effective;
		else if (res == super_effective)
			res = not_very_effective;
	}
	
	return res;
}