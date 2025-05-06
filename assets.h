#pragma once

extern int TILE_SIZE;
extern int ITEM_SIZE, ITEM_MINI_SIZE, ITEM_MEGA_SIZE;

#define or ||
#define and &&
#define not !

#include <utility>
#include <thread>
#include <array>

#include "SDL+.h"
#include "Debugger.h"


#define dstr_t std::pair<char const*, int>

void* load_all_sprites(int tile_size);

extern Surface sprite_sheet;

enum size {
	mega,
	regular,
	mini,
};

extern Uint64 ability_array[18][40];

extern Surface item_sheet_mega;
extern Surface item_sheet;
extern Surface item_sheet_mini;
extern Surface phone_frame;
extern Surface rotom_dex;

extern Surface pokemon_icons;
extern Surface psyduck_sprite, psyduck_active_sprite, AG_icon;

extern Surface right_arrow;
extern Surface tera_mosaic;
//extern Surface check_mark;
//extern Surface exit_cross;
extern Surface icon_sheet;
extern Surface unown_questionmark_animated;
extern Surface clink_animated;

extern Surface start_button;

extern Surface typing_icons_sprite_sheet;

extern Surface typing_icon[18];

extern Surface pokeball_img, megaphone_img;

extern Font CSM_font_array[128];

extern Surface textbox_frame;
extern Surface poke_charset;

extern Surface skip_button;

extern Surface boost_arrows;

extern Color type_color[18];

extern Mix_Music* battle_music;
extern Mix_Music* selection_music;
extern Mix_Music* promotion_music;
extern Mix_Music* end_music;
extern Mix_Music* check_music;
extern Mix_Music* promotion_end_music;
extern Mix_Music* settings_music;

extern Mix_Chunk *slash_effect, *stat_increase_effect, *stat_decrease_effect;

extern void* load_all_sounds();
extern void* load_all_fonts();


enum class sprite_column_type_enum {
	nothing_special, // for most column
	promotion_singleton, // when the sprite of the promoted piece does not depends of its new nature
	promotion_no_king_padding, // when the sprite of the promoted piece does depends of its new nature, and there is 2 extra column for a potential king sprite
	promotion_with_king_padding, // when the sprite of the promoted piece does depends of its new nature, and there is no extra column for a potential king sprite
};
extern std::array<sprite_column_type_enum, 43> sprite_column_type;