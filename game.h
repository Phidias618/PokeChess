#pragma once

#define FPS 60

#include <random>
#include <list>
#include <set>



class Game;

#include "board.h"
#include "board_eval.h"

class Textbox;

enum State {
	no_state,
	in_menu,
	in_selection,
	in_game,
	in_promotion,
	end_of_game,
	in_settings,
};

#include "assets.h"

#include "piece2.h"
#include "item2.h"
#include "Button.h"
#include "board.h"


struct interupted_music_saver {
	Mix_Music* music;
	double interupt_time;
	bool infinitely;
	interupted_music_saver* previous;
	~interupted_music_saver();
};

class Game {
public:
	LANGUAGE language;

	Display window;
	Surface screen;

	int corner_x;
	int corner_y;
	double scaling_factor;
	Surface drawing_board;

	Color bg_color;

	Uint32 mouse_state;
	double mouse_x;
	double mouse_y;

	bool interupt_eval = false;
	bool in_board_eval = false;
	int search_depth = 8;
	bool multi_threaded_eval = true;
	bool eval_failure = false;
	std::thread board_eval_thread;

	color_array<bool> is_a_bot;

	int bot_move_delay = 0;

	void make_bot_move();

	void start_bot_move();

	bot_move_data best_move;

	State state;
	ButtonCollection* global_buttons; // buttons for all or almost all state, they are loaded once at the begining of the game and never unloaded
	ButtonCollection* buttons; // buttons for the current state, change each time a switch of state occurs
	
	bool is_holding_something : 1;
	double selected_thing_sprite_x_offset;
	double selected_thing_sprite_y_offset;
	Surface selected_thing_sprite;


	Uint64 accessible_mask = 0;

	void draw(Surface, double, double, anchor c=top_left);
	void draw(Surface, double, double, double, double, double, double, anchor c=top_left);

	void draw_rect(Color color, double x, double y, double w, double h);

	union { // data for each states
		struct { // data for the state in_selection
			typing selected_type;         // 
			short nb_of_piece_with_type;  // keeps tracks of the number of piece who were given a type
			Uint32 is_type_avaible;  // a booleen array that store wether type-th element is still a type avaible
			PokeItem selected_item;
			bool type_selection;
		};

		Piece* selected_piece = NULL; // data for the state in_game
	};

	std::set<PokeItem> unavaible_items;

	bool enable_textbox = true;
	TextBoxDisplay * active_textbox, * last_textbox;

#if IN_DEBUG
	Uint64* displayed_mask;
#endif

	bool show_phone;
	PokeItem phone_displayed_item;
	Piece* phone_displayed_piece;
	typing phone_displayed_type;
	int phone_displayed_page;

	State previous_state; // use to exit the settings
	ButtonCollection* previous_buttons; // use to exit the settings

	double settings_music_time;

	Board& board;

	bool with_sounds;
	short music_volume;

	bool show_type_chart;

	std::mt19937 RNG;

	void select_piece(Piece* piece);
	void unselect_piece();
	void move_selected_piece_to(Sint8 target_pos);
	void promote(piece_id promotion_id);
	void add_move_cosmetics(move_data& data);
	void change_turn();
	void resume_move();

	void skip_bonus_turn();

	void to_menu();
	void to_selection();
	void to_game(bool resume=false);
	void to_promotion(Piece* promoting_piece);
	void to_end_of_game();

	void to_settings();
	void exit_settings();

	void add_textbox(const char* message);

	static void play_background(Mix_Music* music, int loop=-1);

	static void stop_background(int fadeout);

	static void interupt_background(Mix_Music* music, int loop);

	static void resume_background();

	void reset();
	
	Game();

	void init();

	~Game();
};

// extern Game game;