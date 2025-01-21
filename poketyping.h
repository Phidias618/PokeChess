#pragma once
/*je dois manger au moin une piece du range précedant avant de pouvour manger un piece d'un certain rang (pion, cavalier, fou, tour, dame, roi)*/
#include <cstdint>
#include <string>
#include "float32_enum.cpp"
#include "pythonic.h"

enum class LANGUAGE : long {
	FRENCH,
	ENGLISH,
	GERMAN,
	SPANISH,
	// maybe more later
	NB_OF_LANGUAGE,
};

enum typing : char {
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

extern const char* type_str[(long)LANGUAGE::NB_OF_LANGUAGE][18];

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

enum class pokestat : char {
	mix = 0,
	attack = 1,
	defense = 2,
	special_attack = 3,
	special_defense = 4,
	speed = 5,
	accuracy = 6,
	evasion = 7,
};

enum effectiveness_enum : char {
	immune = 0,
	not_very_effective = 1,
	neutral = 2,
	super_effective = 3,
};

struct effectiveness {
	effectiveness_enum main;
	int intensity;
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

#define for_typing(var) for (typing var = normal; var <= fairy; var++)