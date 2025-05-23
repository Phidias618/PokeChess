#include <cstdio>
#include <array>

#include <assert.h>

#include "poketyping.h"
#include "SDL+.h"
#include "assets.h"

int TILE_SIZE = 128;
int ITEM_SIZE = TILE_SIZE / 2;
int ITEM_MINI_SIZE = TILE_SIZE / 3;
int ITEM_MEGA_SIZE = 4 * TILE_SIZE / 4;

Surface sprite_sheet, item_sheet, item_sheet_mini, item_sheet_mega;
Surface rotom_dex;
Surface icon_sheet;

Surface check_mark;
Surface exit_cross;
Surface unown_questionmark_animated;
Surface clink_animated;

Surface tera_mosaic;
Surface start_button;

Surface typing_icons_sprite_sheet;

Surface typing_icon[18];

Surface pokemon_icons;
Surface psyduck_sprite, psyduck_active_sprite, AG_icon;

Surface pokeball_img, megaphone_img;


Font CSM_font_array[128];

Color type_color[18] = {
	0xff9099a1,
	0xffff9c54,
	0xff4d90d5,
	0xff63bb5b,
	0xfff3d23b,
	0xff74cec0,
	0xffce4069,
	0xffab6ac8,
	0xffd97746,
	0xff92aade,
	0xfff97176,
	0xff90c12c,
	0xffc7b78b,
	0xff5269ac,
	0xff096dc4,
	0xff5a5366,
	0xff5a8ea1,
	0xffec8fe6
};

std::array<sprite_column_type_enum, 43> sprite_column_type = {
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::promotion_no_king_padding,
	sprite_column_type_enum::promotion_no_king_padding,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::promotion_singleton,
	sprite_column_type_enum::promotion_singleton,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::promotion_with_king_padding,
	sprite_column_type_enum::promotion_with_king_padding,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,


	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,

	sprite_column_type_enum::nothing_special,
	sprite_column_type_enum::nothing_special,
};

void* load_all_fonts() {
	for (int i = 0; i < 128; i++) {
		CSM_font_array[i] = Font("assets\\Fonts\\ComicSansMS.TTF", i);
	}
	return NULL;
}

Surface textbox_frame;
Surface poke_charset;
Surface phone_frame;
Surface skip_button;

Surface boost_arrows;

Surface right_arrow;


Mix_Music* battle_music, * selection_music, * promotion_music, * end_music, * check_music, * promotion_end_music, * settings_music = NULL;
Mix_Chunk* slash_effect, * stat_increase_effect, * stat_decrease_effect = NULL;

void* load_all_sounds() {
	Mix_FreeMusic(battle_music);
	battle_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\battle.ogg");

	Mix_FreeMusic(selection_music);
	selection_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\gym.mp3");

	Mix_FreeMusic(promotion_music);
	promotion_music = Mix_LoadMUS("assets\\Sounds\\Events\\promotion.ogg");

	Mix_FreeMusic(end_music);
	end_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\end_music.ogg");

	Mix_FreeMusic(check_music);
	check_music = Mix_LoadMUS("assets\\Sounds\\Events\\check.ogg");

	Mix_FreeMusic(promotion_end_music);
	promotion_end_music = Mix_LoadMUS("assets\\Sounds\\Events\\promotion_end.ogg");

	Mix_FreeChunk(slash_effect);
	slash_effect = Mix_LoadWAV("assets\\Sounds\\Effects\\eat_piece.wav");

	Mix_FreeChunk(stat_increase_effect);
	stat_increase_effect = Mix_LoadWAV("assets\\Sounds\\Effects\\stat_increase.wav");

	Mix_FreeChunk(stat_decrease_effect);
	stat_decrease_effect = Mix_LoadWAV("assets\\Sounds\\Effects\\stat_decrease.wav");
	
	Mix_FreeMusic(settings_music);
	settings_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\settings.ogg");

	return NULL;
}

static int true_mod(int x, int y) {
	return (x % y + y) % y;
}

static SDL_Palette* GS_palette;

void* load_all_sprites(int __tile_size) {
	TILE_SIZE = __tile_size;
	ITEM_SIZE = __tile_size / 2;
	ITEM_MEGA_SIZE = 4 * TILE_SIZE / 4;
	ITEM_MINI_SIZE = 3 * TILE_SIZE / 8;
	textbox_frame = load_img("assets\\Sprites\\textbox_frame.png");

	if (GS_palette == NULL) {
		GS_palette = SDL_CreatePalette(256);
		for (int i = 0; i < 256; i++) {
			GS_palette->colors[i] = {(unsigned char)i, (unsigned char)i, (unsigned char)i, 255};
		}
	}



	poke_charset = load_img("assets\\Sprites\\charset.png");

	sprite_sheet = load_img("assets\\Sprites\\piece_sprite_sheet.png").scale_to(TILE_SIZE * 6, TILE_SIZE * 2, true);
	item_sheet_mega = load_img("assets\\Sprites\\items.png").scale_to(10 * ITEM_MEGA_SIZE, 18 * ITEM_MEGA_SIZE, true);

	item_sheet = item_sheet_mega.scale_to(10 * ITEM_SIZE, 18 * ITEM_SIZE, true);
	item_sheet_mini = item_sheet_mega.scale_to(10 * ITEM_MINI_SIZE, 18 * ITEM_MINI_SIZE, true);


	icon_sheet = load_img("assets\\Sprites\\button_icon.png").scale_to(2 * TILE_SIZE, TILE_SIZE, true);

	unown_questionmark_animated = load_img("assets\\Sprites\\unown_questionmark_animated.png").scale_to(TILE_SIZE * 16, TILE_SIZE, false);

	unown_questionmark_animated.set_colorkey(unown_questionmark_animated.map_rgb(64, 128, 0));

	start_button = load_img("assets\\Sprites\\start_button.png").scale_to(TILE_SIZE * 4, TILE_SIZE * 4, 0).chop({ 0, TILE_SIZE, 4 * TILE_SIZE, 2 * TILE_SIZE });

	typing_icons_sprite_sheet = load_img("assets\\Sprites\\typing.png").scale_to(19 * TILE_SIZE, TILE_SIZE, true);
	for (int i = 0; i < 18; i++) {
		typing_icon[i] = typing_icons_sprite_sheet.chop({ TILE_SIZE * (i+1), 0, TILE_SIZE, TILE_SIZE });
	}

	pokeball_img = load_img("assets\\Sprites\\pokeball.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	skip_button = load_img("assets\\Sprites\\skip_button.png").scale_to(TILE_SIZE * 2, TILE_SIZE * 2, true).chop({ 0, TILE_SIZE / 2, 2 * TILE_SIZE, TILE_SIZE });
	
	clink_animated = load_img("assets\\Sprites\\clink_animated.png").scale_to(TILE_SIZE*16, TILE_SIZE, false);

	clink_animated.set_colorkey(clink_animated.map_rgb(64, 128, 0));

	boost_arrows = load_img("assets\\Sprites\\boost_arrow.png").scale_to(TILE_SIZE * 8, TILE_SIZE, true);

	megaphone_img = load_img("assets\\Sprites\\megaphone.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	phone_frame = load_img("assets\\Sprites\\phone_frame.png").scale_to(4.5 * TILE_SIZE, 4.5 * 2 * TILE_SIZE, true);
	rotom_dex = load_img("assets\\Sprites\\rotom_dex.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	right_arrow = load_img("assets\\Sprites\\arrow.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	pokemon_icons = load_img("assets\\Sprites\\roster.png").scale_to(TILE_SIZE * 41 * 3 / 4, TILE_SIZE * 19 * 3 / 4, true);

	psyduck_sprite = load_img("assets\\Sprites\\psyduck.png").scale_to(TILE_SIZE, TILE_SIZE, true);
	psyduck_active_sprite = load_img("assets\\Sprites\\psyduck_active.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	AG_icon = load_img("assets\\Sprites\\AG_icon.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	tera_mosaic = load_img("assets\\Sprites\\tera_mosaic.png").scale_to(TILE_SIZE, TILE_SIZE, true).convertTo(SDL_PIXELFORMAT_XRGB8888);

	PRINT_DEBUG("\n\nFIN DU CHARGEMENT\n\n");
	return NULL;
}

Uint64 ability_array[18][40];

typedef union {
	Uint64 t;
	Uint8 arr[8];
} __Uint64_as_bytearray;

Uint64 reverse_endianess(Uint64 val) {
	__Uint64_as_bytearray x;
	x.t = val;
	
	for (int i = 0; i < 4; i++) {
		Uint8 tmp = x.arr[i];
		x.arr[i] = x.arr[7 - i];
		x.arr[7 - i] = tmp;
	}
	return x.t;
}

bool is_little_endian() {
	Uint16 n = 1;
	return *(Uint8*)&n == 1;
}

void* load_ability_array() {
	FILE* f;
	fopen_s(&f, "assets\\ability", "r");
	if (f == NULL)
		throw;
	fread(ability_array, sizeof(Uint64), 18 * 40, f);

	fclose(f);
	if (is_little_endian()) {
		iter_typing(t) {
			for (int i = 0; i < 40; i++) {
				ability_array[t][i] = reverse_endianess(ability_array[t][i]);
			}
		}
	}

	return NULL;
}

// void* __ = (void*)((int)load_all_sprites(TILE_SIZE) + (int)load_ability_array());