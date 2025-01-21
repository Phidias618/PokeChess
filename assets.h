#pragma once

extern int TILE_SIZE;
extern int ITEM_SIZE;

#include "SDL+.h"

void* load_all_sprites(int tile_size);

extern Surface sprite_sheet;
extern Surface item_sheet;

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