#include <iostream>
#include <cstring>



#include "SDL+.h"

#include "board.h"

#include "game.h"
#include "board_eval.h"


interupted_music_saver::~interupted_music_saver() {
	if (previous != NULL)
		delete previous;
}

Mix_Music* current_music;
interupted_music_saver* interupted_music;
bool infinitely = false;

Game::Game() : board(*new Board()) {
	
}

void Game::init()  {
#if IN_DEBUG
	displayed_mask = &board.presence_mask;
#endif

	// is_a_bot[black] = true;
	language = LANGUAGE::ENGLISH;

	show_phone = false;
	phone_displayed_item = NO_ITEM;
	phone_displayed_piece = NULL;

	Mix_ReserveChannels(4);
	scaling_factor = 1.0;
	with_sounds = true;
	music_volume = 64;

	settings_music_time = 0.0;

	corner_x = corner_y = 0;
	drawing_board = Surface::createRGB((int)(18 * TILE_SIZE * scaling_factor), (int)(12 * TILE_SIZE * scaling_factor));
	bg_color = Color::beige;

	show_type_chart = false;
	current_music = NULL;

	global_buttons = new ButtonCollection();
	global_buttons->add(new BoardButton(board));
	global_buttons->add(new ShowTypechartButton(1.0, 0.0));
	global_buttons->add(new TypechartDisplay());
	global_buttons->add(new ExitGameButton(17.0, 0.0));
	global_buttons->add(new SettingsButton(0.0, 0.0));
	global_buttons->add(new ToggleInformationDisplay(2.0, 0.0));
	global_buttons->add(new InformationDisplay(2.5, 5.5));
	global_buttons->add(new PhoneSwitchPage(0.5, 6, false));
	global_buttons->add(new PhoneSwitchPage(4.5, 6, true));

	buttons = new ButtonCollection();

	std::random_device rand;
	RNG.seed(rand());

	window = Display("test", 18 * TILE_SIZE / 2, 12 * TILE_SIZE / 2, SDL_WINDOW_RESIZABLE|(SDL_WINDOW_FULLSCREEN&0));
	screen = window.get_surface();

	is_holding_something = false;

	selected_piece = NULL;


	nb_of_piece_with_type = 0;
	selected_type = typeless;
	is_type_avaible = 0xFFFFFFFF;

	board.init();

	to_menu();

	active_textbox = last_textbox = NULL;

	selected_thing_sprite_x_offset = selected_thing_sprite_y_offset = 0;
	mouse_x = mouse_y = 0.0;

	
}

void Game::reset() {
	show_phone = false;
	phone_displayed_item = NO_ITEM;
	phone_displayed_piece = NULL;
	is_holding_something = false;
	show_type_chart = false;

	accessible_mask = 0;

	std::random_device rand;
	RNG.seed(rand());

	selected_piece = NULL;

	nb_of_piece_with_type = 0;
	selected_type = typeless;
	is_type_avaible = 0xFFFFFFFF;

	active_textbox = last_textbox = NULL;

	board.reset();
}

#include <chrono>

void Game::select_piece(Piece* piece) {
	accessible_mask = 0;
	if (in_board_eval or not piece) {
		selected_piece = NULL;
		return;
	}

	selected_piece = piece;

	accessible_mask = board.reachable_mask_array[piece->pos];

	piece->item.select_holder(self, board, piece);
}

void Game::unselect_piece() {
	if (selected_piece == NULL) 
		return;
	if (board.piece_mask[(int)piece_id::duck] == 0 and board.active_player == no_color)
		return;

	selected_piece = NULL;
	is_holding_something = false;

	accessible_mask = 0;
}

void Game::draw(Surface surf, double x, double y, anchor c) {
	if (surf == NULL)
		return;
	SDL_Rect rect;
	rect.x = (int)(x * TILE_SIZE) - (surf->w * (c & 0b11)) / 2;
	rect.y = (int)(y * TILE_SIZE) - (surf->h * (c >> 2)) / 2;

	drawing_board.blit(surf, &rect, NULL);
}

void Game::draw(Surface surf, double x, double y, double ax, double ay, double aw, double ah, anchor c) {
	SDL_Rect rect;
	if (surf == NULL)
		return;
	rect.x = (int)(x * TILE_SIZE) - (surf->w * (c & 0b11) / 2);
	rect.y = (int)(y * TILE_SIZE) - (surf->h * (c >> 2) / 2);
	SDL_Rect a = { (int)(ax * TILE_SIZE), (int)(ay * TILE_SIZE), (int)(aw * TILE_SIZE), (int)(ah * TILE_SIZE) };
	drawing_board.blit(surf, &rect, &a);
}

void Game::draw_rect(Color color, double x, double y, double w, double h) {
	SDL_Rect rect = { (int)(x * TILE_SIZE), (int)(y * TILE_SIZE), (int)(w * TILE_SIZE), (int)(h * TILE_SIZE) };
	SDL_FillSurfaceRect(drawing_board, &rect, color);

}

void Game::move_selected_piece_to(Sint8 target_pos) {
	if (active_textbox != NULL) {
		active_textbox->destroy_all();
		active_textbox = last_textbox = NULL;
	}

	while (current_music != battle_music) {
		resume_background();
	}

	Piece* piece = selected_piece;
	selected_piece = NULL;
	is_holding_something = false;
	accessible_mask = 0;
	board.first_half_move(piece, target_pos);

	if (not board.last_move_data.interrupt_move) {
		resume_move();
	}
	else if (board.last_move_data.init_promotion) {
		to_promotion(board.last_move_data.attacker);
	}
}

void Game::resume_move() {
	move_data& data = board.last_move_data;
	piece_color const player = board.last_move_data.attacker_copy.color;
	
	add_move_cosmetics(data);

	board.second_half_move(board.last_move_data);

	Uint64 enemy_mask = board.color_mask[not board.active_player];
	Uint64 const king_mask = board.piece_mask[(int)piece_id::king] & board.color_mask[board.active_player];
	
	bool in_check = false;
	
	while (enemy_mask) {
		int const pos = bsf64(enemy_mask);
		CLEAR_LOWEST_BIT(enemy_mask);

		if (board.reachable_mask_array[pos] & king_mask) {
			in_check = true;
			break;
		}
	}

	if (in_check) {
		interupt_background(check_music, -1);
	}


	if (board.end_of_game) {
		to_end_of_game();
	}
	else {
		if (data.move_again and not is_a_bot[board.active_player]) {
			select_piece(data.attacker);
		}

		if (board.with_duck_chess and board.active_player == no_color and board.piece_mask[(int)piece_id::duck] == 0) {
			selected_piece = board.duck;
			accessible_mask = ~board.presence_mask;
		}
	}

	start_bot_move();
}

void Game::add_move_cosmetics(move_data& data) {

	if (board.with_typing) {
		if (data.defender != NULL)
			if (data.do_crit)
				;
			else if (data.do_miss)
				;
			else if (data.is_immune)
				;
			else
				Mix_PlayChannel(0, slash_effect, 0);

		if (data.en_passant)
			add_textbox("Pawn uses\nEn passant");

		if (data.castling) {
			add_textbox("King uses castle");
			add_textbox("That raises his\ndefense");
			global_buttons->add(new StatBoostDisplay(data.attacker, pokestat::defense, TextBoxDisplay::duration, +1));
		}

		if (data.do_miss) {
			char buffer[64] = { '\0' };
			strcpy_s(buffer, data.attacker->get_name(language));
			strcat_s(buffer, "'s\nattack's missed");
			add_textbox(buffer);
		}
		else if (data.is_immune) {
			char buffer[64] = "It doesn't affect\n";
			strcat_s(buffer, data.defender->get_name(language));
			add_textbox(buffer);
		}
		else if (data.do_crit and not data.do_miss)
			add_textbox("Critical hit!");

		else if (data.is_not_very_effective)
			add_textbox("It's not very\neffective...");
		else if (data.is_super_effective)
			add_textbox("It's super\neffective!");
		/*
		if (escaped_check) {
			add_textbox("King got away\nsafely");
		}*/
	}

	if (data.defender and not data.defender->item.is_safety_google()) {
		data.attacker->item.add_cosmetic(game, board, data.attacker, data);
	}

	if (data.defender and not data.attacker->item.is_protective_pads()) {
		data.defender->item.add_cosmetic(game, board, data.defender, data);
	}
}


void Game::promote(piece_id promotion_id) {
	Piece& piece = *board.promoting_piece;
	move_data interupted_move_data = board.last_move_data;
	board.promote(promotion_id);

	to_game(true);
	if (board.with_typing) {
		char buffer[64] = "Pawn evolved into\n";
		strcat_s(buffer, piece.get_name(game.language));
		add_textbox(buffer);
	}

	add_move_cosmetics(interupted_move_data);

	if (board.with_typing) {
		interupt_background(promotion_end_music, 0);
	}

	if (board.end_of_game)
		to_end_of_game();

	start_bot_move();
}

void Game::start_bot_move() {
	if ((not board.end_of_game) and board.active_player != no_color and is_a_bot[board.active_player]) {
		
		if (board_eval_thread.joinable()) {
			interupt_eval = true;
			board_eval_thread.join();
			interupt_eval = false;
		}
		board_eval_thread = std::thread([this]() -> void { if (multi_threaded_eval) start_threaded_eval(self, board); else start_eval(self, board); });
		// start_eval(self, board);
		bot_move_delay = 1 * FPS;
	}
}

void Game::change_turn() {
	// move_data const& data = board.last_move_data;
	
	board.change_turn();

	if (board.with_duck_chess and board.active_player == no_color and board.piece_mask[(int)piece_id::duck] == 0) {
		selected_piece = board.duck;
		accessible_mask = ~board.presence_mask;
	}

	start_bot_move();

	if (board.end_of_game) {
		to_end_of_game();
	}
}

void Game::skip_bonus_turn() {
	
	board.skip_bonus_turn();
	unselect_piece();
	start_bot_move();
}

void Game::make_bot_move() {
	if (bot_move_delay > 0) {
		bot_move_delay--;
		return;
	}

	if (eval_failure) {
		is_a_bot[white] = is_a_bot[black] = false;
	}

	if (state == in_game and board.active_player != no_color and is_a_bot[board.active_player] and not in_board_eval) {
		bot_move_data future = best_move;
		if (future.promotion) {
			promote(future.promotion_id);
		}
		else if (future.skip_bonus_turn) {
			board.skip_bonus_turn();
		}
		else if (future.tera) {
			board.tera_piece(board[future.begin_pos].piece);
		}
		else {
			// PRINT_DEBUG("move piece at (", future.begin_square->x, ", ", future.begin_square->y, ") to (", future.target_square->x, ", ", future.target_square->y, ")");
			if (future.begin_pos >= 0 and future.target_pos >= 0) {
				select_piece(board[future.begin_pos].piece);
				move_selected_piece_to(future.target_pos);
			}
		}
	}
}

void Game::add_textbox(const char* message) {
	if (not enable_textbox)
		return;
	if (active_textbox == NULL) {
		active_textbox = last_textbox = new TextBoxDisplay(message, true);
	}
	else {
		TextBoxDisplay* box = new TextBoxDisplay(message, false);
		last_textbox->add(box);
		last_textbox = box;
	}
	global_buttons->add(last_textbox);
}

void Game::to_menu() {
	if (state == in_settings) {
		exit_settings();
	}
	stop_background(100);
	state = in_menu;

	interupt_eval = true;
	if (board_eval_thread.joinable()) {
		board_eval_thread.join();
	}
	interupt_eval = false;

	buttons->clear();
	buttons->add(new BeginGameButton());
	buttons->add(new ChangeGameruleButton(14.0, 3.0, &board.with_RNG, pokeball_img, false, 1, 0, "avec de l'Aléatoire", "with RNG"));
	buttons->add(new ChangeGameruleButton(14.0, 4.0, &board.with_random_battle, unown_questionmark_animated, true, 16, FPS/8, "Team Aléatoire", "Random Teams"));
	buttons->add(new ChangeGameruleButton(14.0, 5.0, &board.with_items, NULL, true, 1, 0, "Objets", "Items"));
	buttons->add(new ChangeGameruleButton(14.0, 6.0, &board.with_AG, AG_icon, true, 1, 0, "", "Anything Goes"));
	buttons->add(new ChangeGameruleButton(14.0, 7.0, &board.with_antichess, pokeball_img, false, 1, 0, "Anti-echec", "Suicide Cup"));
	buttons->add(new ChangeGameruleButton(14.0, 8.0, &board.with_duck_chess, psyduck_sprite, true, 1, 0, "Echec Psykokwak", "Psyduck Chess"));
	buttons->add(new ChangeGameruleButton(14.0, 9.0, &board.with_reversed_typechart, NULL, true, 1, 0, "Reverse Battle", "Reverse Battle"));
	reset();
}



void Game::to_selection() {
	state = in_selection;
	show_phone = true;
	phone_displayed_item = NO_ITEM;
	phone_displayed_piece = NULL;
	phone_displayed_type = typeless;
	unavaible_items.clear();
	buttons->clear();
	iter_typing(type) {
		buttons->add(new TypingSelectionButton(14.0 + (type % 3), 3.0 + (type / 3), type));
	}
	
	buttons->add(new SwitchSelectionButton(14.0, 1.5));
	double x = 14.0;
	double y = 2.5;
	int i = 0;
	for (PokeItem item : item_table) {
		if (item.get_id() == item_ID::__terminator_placeholder__)
			break;
		if (board.with_RNG or not rng_dependant_items.contains(item)) {
			if (item.get_id() == item_ID::__space_placeholder__)
				goto space;
			else if (item.get_id() == item_ID::__newline_placeholder__)
				goto newline;
			buttons->add(new ItemSelectionButton(x, y, item));

		space:
			i++;
			if (i == 6) {
			newline:
				i = 0;
				x = 14.0;
				y += 0.5;
			}
			else {
				x += 0.5;
			}
		}
	}

	buttons->add(new RandomTypingButton(17.0, 10.0));
	buttons->add(new ConfirmSelectionButton(17.0, 11.0));
	
	nb_of_piece_with_type = 0;
	type_selection = true;
	selected_type = typeless;
	is_type_avaible = 0xFFFFFFFF;
	board.with_check = false;
}

void Game::to_game(bool resume) {
	state = in_game;

	buttons->clear();
	buttons->add(new SkipBonusMoveButton(9.0, 1.0));

	selected_piece = NULL;
	show_phone = false;

	std::random_device rd;

	RNG.seed(rd());

	board.set_reachable_squares();

	if (board.with_typing) {
		if (resume) {
			resume_background();
		}
		else {
			play_background(battle_music);
			// print_perft(board, 6);
		}
	}

}

void Game::to_end_of_game() {
	state = end_of_game;
	stop_background(1);
	delete interupted_music;
	play_background(end_music, -1);

	buttons->clear();
	buttons->add(new EndOfGameButton());

	accessible_mask = 0;
}

void Game::to_settings() {
	interupt_background(settings_music, -1);
	Mix_SetMusicPosition(settings_music_time);
	show_type_chart = false;
	previous_state = state;
	previous_buttons = buttons;
	state = in_settings;
	buttons = new ButtonCollection();

	buttons->add(new DisableSoundsButton(5.0+2.0, 2.0 + 2.0));
	buttons->add(new VolumeSlider(5.0 + 2.0, 2.0 + 3.0));
}

void Game::exit_settings() {
	settings_music_time = Mix_GetMusicPosition(settings_music);
	resume_background();
	state = previous_state;
	buttons->clear();
	delete buttons;
	buttons = previous_buttons;
	previous_buttons = NULL;
	previous_state = no_state;
}

template<class T>
Piece* constructor(Board& board_, piece_color color_, Square* sq, typing type_ = typeless, PokeItem* item=NULL) {
	return new T(board_, color_, sq, type_, item);
}

void Game::to_promotion(Piece* promoting_pawn) {
	board.promoting_piece = promoting_pawn;
	
	if (not promoting_pawn->item.prepare_promotion(board, promoting_pawn, &board.avaible_promotion)) {
		board.avaible_promotion = 0b0000'0000'0001'1110;
	}
	int count = popcount16(board.avaible_promotion);

	if (count == 0) {
		board.promoting_piece = NULL;
		return;
	}
	state = in_promotion;

	if (board.with_typing)
		interupt_background(promotion_music, -1);

	buttons->clear();
	
	double X = 9.0 - 0.5 * count;
	for (int i = 0; i < (8 * sizeof(board.avaible_promotion)); i++) {
		if ((board.avaible_promotion >> i) & 1) {
			buttons->add(new PromotionButton((piece_id)i, X, 5.5));
			X += 1.0;
		}
	}
}

void Game::play_background(Mix_Music* music, int loop) {
	current_music = music;
	infinitely = (loop == -1);
	Mix_PlayMusic(music, loop);
}

void Game::stop_background(int fadeout) {
	Mix_FadeOutMusic(fadeout);
	current_music = NULL;
	if (interupted_music != NULL) {
		delete interupted_music;
		interupted_music = NULL;
	}
}

void Game::interupt_background(Mix_Music* music, int loop) {
	interupted_music = new interupted_music_saver{current_music, Mix_GetMusicPosition(current_music), infinitely, interupted_music};

	play_background(music, loop);
	if (loop == -1)
		Mix_HookMusicFinished(NULL);
	else
		Mix_HookMusicFinished(resume_background);
}

void Game::resume_background() {
	if (interupted_music == NULL) {
		stop_background(0);
	}
	else
	{	
		if (interupted_music->music == NULL) {
			Mix_FadeOutMusic(2000);
		}
		else {
			Mix_PlayMusic(interupted_music->music, (interupted_music->infinitely == 1)?-1:1);
			Mix_SetMusicPosition(interupted_music->interupt_time);
			if (interupted_music->infinitely)
				Mix_HookMusicFinished(NULL);
			else {
				Mix_HookMusicFinished(resume_background);
			}
		}
		infinitely = interupted_music->infinitely;
		current_music = interupted_music->music;
		auto temp = interupted_music;
		interupted_music = interupted_music->previous;
		temp->previous = NULL;

		delete temp;
	}
}

Game::~Game() {
	interupt_eval = true;
	if (board_eval_thread.joinable())
		board_eval_thread.join();
}

Game game = Game();