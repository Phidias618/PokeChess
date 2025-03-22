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

Surface start_button;

Surface typing_icons_sprite_sheet;

Surface typing_icon[18];

Surface pokemon_icons;
Surface psyduck_sprite, psyduck_active_sprite, AG_icon;

Surface pokeball_img, megaphone_img;


Font CSM_font_array[128];

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

void* load_all_sprites(int __tile_size) {
	TILE_SIZE = __tile_size;
	ITEM_SIZE = __tile_size / 2;
	ITEM_MEGA_SIZE = 4 * TILE_SIZE / 4;
	ITEM_MINI_SIZE = 3 * TILE_SIZE / 8;
	textbox_frame = load_img("assets\\Sprites\\textbox_frame.png");

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

	PRINT_DEBUG("\n\nFIN DU CHARGEMENT\n\n");
	return NULL;
}

void* __ = load_all_sprites(TILE_SIZE);

