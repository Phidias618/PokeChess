#include <iostream>
#include <cstring>

#include "game.h"
#include "Debugger.h"

#include "SDL+.h"

#include "board.h"

interupted_music_saver::~interupted_music_saver() {
	if (previous != NULL)
		delete previous;
}

Mix_Music* current_music;
interupted_music_saver* interupted_music;
bool infinitely = false;

Game::Game() : board(*new Board()) {
	
}

void Game::init() {
	language = LANGUAGE::ENGLISH;

	show_phone = false;
	phone_displayed_item = NULL;
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

	window = Display("test", 18 * TILE_SIZE, 12 * TILE_SIZE, SDL_WINDOW_RESIZABLE);
	screen = window.get_surface();

	is_holding_something = false;

	selected_piece = NULL;


	nb_of_piece_with_type = 0;
	selected_type = typeless;
	is_type_avaible = 0xFFFFFFFF;

	to_menu();
	with_check = true;
	with_RNG = true;
	with_typing = true;
	with_items = false;
	with_random_battle = false;

	winner = no_color;

	active_textbox = last_textbox = NULL;

	selected_thing_sprite_x_offset = selected_thing_sprite_y_offset = 0;
	mouse_x = mouse_y = 0.0;

	board.init();
}

void Game::reset() {
	show_phone = false;
	phone_displayed_item = NULL;
	phone_displayed_piece = NULL;
	is_holding_something = false;
	show_type_chart = false;

	std::random_device rand;
	RNG.seed(rand());

	selected_piece = NULL;
	winner = no_color;

	nb_of_piece_with_type = 0;
	selected_type = typeless;
	is_type_avaible = 0xFFFFFFFF;

	with_check = true;

	active_textbox = last_textbox = NULL;

	board.reset();
}

void Game::resize_drawing_board(int new_tile_size) {
	double old_factor = scaling_factor;
	scaling_factor = (double)new_tile_size / 64;
	load_all_sprites(new_tile_size);

	drawing_board = Surface::createRGB(18 * TILE_SIZE, 12 * TILE_SIZE);
	board.resize_surface();
	buttons->resize();
	global_buttons->resize();
	if (previous_buttons != NULL) {
		previous_buttons->resize();
	}
}


void Game::select_piece(Piece* piece) {
	selected_piece = piece;

	for (Square& square : board) {
		if (piece->can_move_to(square)) {
			square.is_accessible = true;
		}
	}
}

void Game::unselect_piece() {
	if (selected_piece == NULL) 
		return;
	if (board.duck != NULL and board.duck->square == NULL)
		return;
	int x = selected_piece->x; int y = selected_piece->y;
	selected_piece = NULL;
	is_holding_something = false;

	for (Square& square : board) {
		if (square.is_accessible) {
			square.is_accessible = false;
		}
	}
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

void Game::move_selected_piece_to(Square& square) {
	if (active_textbox != NULL) {
		active_textbox->destroy_all();
		active_textbox = last_textbox = NULL;
	}

	if (current_music == promotion_end_music)
		resume_background();

	piece_color player = board.active_player;
	Square* begin_square = selected_piece->square;

	board.last_move_data = selected_piece->move_to(square);

	// Square* temp = board.last_move_begin_square;
	// board.last_move_begin_square = begin_square;

	// temp = board.last_move_end_square;
	// board.last_move_end_square = selected_piece->square;

	unselect_piece();

	if (not board.last_move_data.interrupt_move)
		resume_move();

	check_for_end_of_game();

	if (state != end_of_game and board.last_move_data.promotion)
		to_promotion(board.last_move_data.attacker);

}

void Game::resume_move() {
	move_data& data = board.last_move_data;
	piece_color player = board.last_move_data.attacker->color;

	if (with_typing) {
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

		bool is_in_check = false;
		for (King* king : board.king_list[board.active_player]) {
			if (king->is_in_check()) {
				is_in_check = true;
				break;
			}
		}
		bool escaped_check = board.last_move_data.escaped_check;

		if (data.castling) {
			add_textbox("King uses castle");
			if (not escaped_check and not is_in_check) {
				add_textbox("That raises his\ndefense");
				global_buttons->add(new StatBoostDisplay(data.attacker, pokestat::defense, TextBoxDisplay::duration, +1));
			}
		}

		if (data.do_miss) {
			char buffer[64] = { '\0' };
			strcpy_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "'s\nattack's missed");
			add_textbox(buffer);
		}
		else if (data.is_immune) {
			char buffer[64] = "It doesn't affect\n";
			strcat_s(buffer, data.defender->Class->name[(int)game.language]);
			add_textbox(buffer);
		}
		else if (data.do_crit and not data.do_miss)
			add_textbox("Critical hit!");

		else if (data.is_not_very_effective)
			add_textbox("It's not very\neffective...");
		else if (data.is_super_effective)
			add_textbox("It's super\neffective!");

		if (escaped_check) {
			add_textbox("King got away\nsafely");
		}
	}

	if (data.attacker_item_slot != NULL and not IS_SAFETY_GOOGLES(data.defenser_item_slot)) {
		data.attacker_item_slot->add_cosmetic(data);
	}

	if (data.defenser_item_slot != NULL and not IS_PROTECTIVE_PADS(data.attacker_item_slot)) {
		data.defenser_item_slot->add_cosmetic(data);
	}

	if (data.move_again) {
		board.in_bonus_move = true;
		select_piece(data.attacker);
		if (with_typing) {
			if (data.was_in_check)
				resume_background();
			else if (current_music != check_music)
				for (King* king : board.king_list[board.active_player]) {
					if (king->is_in_check()) {
						interupt_background(check_music, -1);
						break;
					}
				}
		}
	}
	else {
		board.in_bonus_move = false;
		change_turn();
	}
	
	check_for_end_of_game();

	board.turn_number++;
	board.move_historic.push_front(data);
}

void Game::change_turn() {
	move_data& data = board.last_move_data;
	if (with_duck_chess) {
		if (board.active_player == no_color) {
			board.first_turn = false;
			board.active_player = not board.move_historic.front().attacker->color;
			board.duck->sprite = psyduck_sprite;
		}
		else {
			if (board.duck == NULL) {
				select_piece(new Duck(board, NULL));
			}
			else {
				select_piece(board.duck);
				board.duck->sprite = psyduck_active_sprite;
			}

			board.active_player = no_color;
		}
	}
	else {
		board.active_player = not board.active_player;
	}



	if (with_typing) {
		if (data.was_in_check)
			resume_background();
		else if (current_music != check_music) {
			for (King* king : board.king_list[board.active_player]) {
				if (king->is_in_check()) {
					interupt_background(check_music, -1);
					break;
				}
			}
		}
	}
}

void Game::check_for_end_of_game() {
	if (with_check) {
		if (board.in_stalemate(board.active_player)) {
			to_end_of_game();
			winner = no_color;
			for (King* king : board.king_list[board.active_player]) {
				if (king->is_in_check()) {
					// checkmate congratulations
					winner = not board.active_player;
					break;
				}
			}
		}
	}
	else if (with_antichess) {
		if (board.white_death == 16) {
			winner = white;
			to_end_of_game();
		}
		else if (board.black_death == 16) {
			winner = black;
			to_end_of_game();
		}
	}
	else {
		if (board.nb_of_kings[white] == 0 and board.nb_of_kings[black] == 0) {
			winner = no_color;
			to_end_of_game();
		}
		else if (board.nb_of_kings[white] == 0) {
			winner = black;
			to_end_of_game();
		}
		else if (board.nb_of_kings[black] == 0) {
			winner = white;
			to_end_of_game();
		}
	}
}

void Game::add_textbox(const char* message) {
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

	buttons->clear();
	buttons->add(new BeginGameButton());
	buttons->add(new ChangeGameruleButton(14.0, 3.0, &with_RNG, &pokeball_img, false, 1, 0, "avec de l'Aléatoire", "with RNG"));
	buttons->add(new ChangeGameruleButton(14.0, 4.0, &with_random_battle, &unown_questionmark_animated, true, 16, FPS/8, "Team Aléatoire", "Random Teams"));
	buttons->add(new ChangeGameruleButton(14.0, 5.0, &with_items, new Surface(), true, 1, 0, "Objets", "Items"));
	buttons->add(new ChangeGameruleButton(14.0, 6.0, &with_AG, &AG_icon, true, 1, 0, "", "Anything Goes"));
	buttons->add(new ChangeGameruleButton(14.0, 7.0, &with_antichess, &pokeball_img, false, 1, 0, "Anti-echec", "Suicide Cup"));
	buttons->add(new ChangeGameruleButton(14.0, 8.0, &with_duck_chess, &psyduck_sprite, true, 1, 0, "Echec Psykokwak", "Psyduck Chess"));
	
	reset();
}



void Game::to_selection() {
	state = in_selection;
	show_phone = true;
	phone_displayed_item = NULL;
	phone_displayed_piece = NULL;
	phone_displayed_type = typeless;

	buttons->clear();
	iter_typing(type) {
		buttons->add(new TypingSelectionButton(14.0 + (type % 3), 3.0 + (type / 3), type));
	}
	
	buttons->add(new SwitchSelectionButton(14.0, 1.5));
	double x = 14.0;
	double y = 2.5;
	int i = 0;
	for (ItemClass& Item : item_table) {
		if (Item.type == terminator_item)
			break;
		if (game.with_RNG or not Item.is_RNG_dependant) {
			if (Item.type == space_item)
				goto space;
			else if (Item.type == newline_item)
				goto newline;
			buttons->add(new ItemSelectionButton(x, y, Item));

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
	with_check = false;
}

void Game::to_game(bool resume) {
	state = in_game;

	buttons->clear();
	buttons->add(new SkipBonusMoveButton(9.0, 1.0));

	selected_piece = NULL;
	show_phone = false;

	std::random_device rd;

	RNG.seed(rd());

	if (with_typing) {
		if (resume) {
			resume_background();
		}
		else {
			play_background(battle_music);
		}
	}
}

void Game::to_end_of_game() {
	state = end_of_game;
	stop_background(1);
	play_background(end_music, -1);

	buttons->clear();
	buttons->add(new EndOfGameButton());

	for (Square& square : board) {
		square.is_accessible = false;
	}
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
	promoting_piece = promoting_pawn;
	
	state = in_promotion;

	if (with_typing)
		interupt_background(promotion_music, -1);

	buttons->clear();
	if (promoting_pawn->item == NULL or not promoting_pawn->item->prepare_promotion()) {
		buttons->add(new PromotionButton(Queen::cls->base_promotion_constructor, 7.0, 5.5));
		buttons->add(new PromotionButton(Rook::cls->base_promotion_constructor, 8.0, 5.5));
		buttons->add(new PromotionButton(Bishop::cls->base_promotion_constructor, 9.0, 5.5));
		buttons->add(new PromotionButton(Knight::cls->base_promotion_constructor, 10.0, 5.5));
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
	PRINT_DEBUG("resume");
	if (interupted_music == NULL) {
		PRINT_DEBUG("pas de musique a reprendre");
		stop_background(0);
	}
	else
	{	
		if (interupted_music->music == NULL) {
			Mix_FadeOutMusic(2000);
		}
		else {
			PRINT_VAR(interupted_music->infinitely);
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
}

Game game = Game();