#pragma once
/*je dois manger au moin une piece du range précedant avant de pouvour manger un piece d'un certain rang (pion, cavalier, fou, tour, dame, roi)*/
#include <cstdint>
#include <string>

enum class LANGUAGE : int8_t {
	FRENCH,
	ENGLISH,
	GERMAN,
	SPANISH,
	ITALIAN,
	// maybe more later, or not
	__NB_OF_LANGUAGE__,
};
#define NB_OF_LANGUAGE ((int)LANGUAGE::__NB_OF_LANGUAGE__)

enum typing : int8_t {
	typeless=-1,
	normal,
	fire,
	water,
	grass,
	electric,
	ice,
	fighting,
	poison,
	ground,
	flying,
	psychic,
	bug,
	rock,
	ghost,
	dragon,
	dark,
	steel,
	fairy,
};

extern const char* type_str[NB_OF_LANGUAGE][18];
extern const char* type_str_cap[NB_OF_LANGUAGE][18];


inline typing& operator++(typing& x) {
	return (typing&)++(char&)x;
}

inline typing& operator--(typing& x) {
	return (typing&)--(char&)x;
}

inline typing operator++(typing& x, int) {
	auto old = x;
	++x;
	return old;
}

inline typing operator--(typing& x, int) {
	auto old = x;
	--x;
	return old;
}

enum class pokestat : int8_t {
	mix = 0,
	attack = 1,
	defense = 2,
	sp_attack = 3,
	sp_defense = 4,
	speed = 5,
	accuracy = 6,
	evasion = 7,
};

enum effectiveness_enum : int8_t {
	immune = -1,
	not_very_effective = 0,
	neutral = 1,
	super_effective = 2,
};
#if false
const char* defensive_effectiveness_str[NB_OF_LANGUAGE][4] = {
	{ // FRENCH
		"immunitées",
		"resiste",
		"",
		"faiblesses"
	},
	{ // ENGLISH
		"immunity",
		"resist",
		"",
		"weaknesses",
	}
};

const char* offensive_effectiveness_str[NB_OF_LANGUAGE][4] = {
	{ // FRENCH
		"inefficace",
		"resisté",
		"",
		"super efficace"
	},
	{ // ENGLISH
		"uneffective",
		"resisted",
		"",
		"super effective",
	}
};
#endif

struct effectiveness {
	effectiveness_enum main;
	int8_t intensity;
	inline effectiveness(effectiveness_enum e) {
		main = e;
		intensity = 0;
	}

	inline effectiveness() {
		main = neutral;
		intensity = 0;
	}

	inline effectiveness(effectiveness_enum e, int intensity_) {
		main = e;
		intensity = intensity_;
	}

	inline operator effectiveness_enum() {
		return main;
	}
};

effectiveness& operator++(effectiveness& x);
effectiveness operator++(effectiveness& x, int);

effectiveness& operator--(effectiveness& x);
effectiveness operator--(effectiveness& x, int);

inline bool operator==(const effectiveness x, effectiveness_enum e) {
	return x.main == e;
}

inline bool operator==(effectiveness_enum e, const effectiveness x) {
	return x == e;
}

/*the line typing corresponds to the attacking typing while the column typing corresponds to the defensive typing*/
extern effectiveness_enum const typechart[18][18];

#define iter_typing(var) for (typing var = normal; var <= fairy; var++)

effectiveness get_effectiveness(typing attacking_typing, typing defensing_typing, bool reverse);