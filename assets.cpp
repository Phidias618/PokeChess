#include "SDL+.h"
#include "assets.h"

int TILE_SIZE = 64;
int ITEM_SIZE = TILE_SIZE / 2;

Surface sprite_sheet, item_sheet;

Surface icon_sheet;

Surface check_mark;
Surface exit_cross;
Surface unown_questionmark_animated;
Surface clink_animated;

Surface start_button;

Surface typing_icons_sprite_sheet;

Surface typing_icon[18];

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

Surface skip_button;

Surface boost_arrows;

void* load_all_sounds() {
	Mix_FreeMusic(battle_music);
	battle_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\battle.ogg");

	Mix_FreeMusic(selection_music);
	selection_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\gym.mp3");
	promotion_music = Mix_LoadMUS("assets\\Sounds\\Events\\promotion.ogg");
	end_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\end_music.ogg");
	check_music = Mix_LoadMUS("assets\\Sounds\\Events\\check.ogg");
	promotion_end_music = Mix_LoadMUS("assets\\Sounds\\Events\\promotion_end.ogg");
	slash_effect = Mix_LoadWAV("assets\\Sounds\\Effects\\eat_piece.wav");
	stat_increase_effect = Mix_LoadWAV("assets\\Sounds\\Effects\\stat_increase.wav");
	stat_decrease_effect = Mix_LoadWAV("assets\\Sounds\\Effects\\stat_decrease.wav");
	
	settings_music = Mix_LoadMUS("assets\\Sounds\\Backgrounds\\settings.ogg");

	return NULL;
}

void* load_all_sprites(int tile_size) {
	TILE_SIZE = tile_size;
	ITEM_SIZE = tile_size / 2;

	textbox_frame = load_img("assets\\Sprites\\textbox_frame.png");

	poke_charset = load_img("assets\\Sprites\\charset.png");

	sprite_sheet = load_img("assets\\Sprites\\piece_sprite_sheet.png").scale_to(TILE_SIZE * 6, TILE_SIZE * 2, true);
	item_sheet = load_img("assets\\Sprites\\items.png").scale_to(10 * ITEM_SIZE, 18 * ITEM_SIZE, true);

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

	boost_arrows = load_img("assets\\Sprites\\boost_arrow.png").scale_to(TILE_SIZE * 8, TILE_SIZE, true);//.convertTo(SDL_PIXELFORMAT_XRGB8888);

	megaphone_img = load_img("assets\\Sprites\\megaphone.png").scale_to(TILE_SIZE, TILE_SIZE, true);

	PRINT_DEBUG("\n\nFIN DU CHARGEMENT\n\n")
	return NULL;
}

void* __ = load_all_sprites(TILE_SIZE);

Mix_Music *battle_music, *selection_music, *promotion_music, *end_music, *check_music, *promotion_end_music, *settings_music = NULL;
Mix_Chunk *slash_effect, *stat_increase_effect, *stat_decrease_effect = NULL;