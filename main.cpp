#include <SDL.h>
#include <iostream>
#include <string>

#include "SDL+.h"
#include "Debugger.h"

#include "assets.h"

#include "piece.h"

#include "Button.h"

#include "poketyping.h"
#include "game.h"
#include "board.h"


constexpr Uint64 TPF_ms = 1000 / FPS;

SDL_Event event;
bool running = true;

void loop();

int main(int argc, char* args[])
{	
	game.init();
	if (SDL_plus_init(SDL_INIT_VIDEO | SDL_INIT_AUDIO, IMG_INIT_PNG, MIX_INIT_WAVPACK)) {

		if (game.window == NULL)
		{
			//printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{	
			Mix_Volume(-1, game.music_volume);
			Mix_VolumeMusic(game.music_volume);
			load_all_sounds();
			load_all_fonts();

			while (running) {
				loop();
			}
		}
	}

	SDL_plus_quit();

	return 0;
}

void get_drawing_board_coordinates(Sint32 x, Sint32 y, double* new_x, double* new_y) {
	// sets *new_x and *new_y with the tile coordinates of (x, y) relative to the top left corner
	*new_x = ((double)x - game.corner_x) / (double)TILE_SIZE;
	*new_y = ((double)y - game.corner_y) / (double)TILE_SIZE;
}

auto draw_frame() -> void {
	//Game& game = *game_ptr;
	SDL_Rect dest_rect;

	Uint64 t1 = SDL_GetTicks();

	game.screen.fill(game.bg_color);
	game.drawing_board.fill(game.bg_color);

	Uint64 t2 = SDL_GetTicks();

	dest_rect.x = TILE_SIZE * 5;
	dest_rect.y = TILE_SIZE * 2;

	Uint64 t3 = SDL_GetTicks();


	SDL_Rect r(TILE_SIZE * 3, TILE_SIZE * 8, TILE_SIZE, TILE_SIZE);
	if (game.state != in_settings) {
		for (int i = 0; i < game.board.white_death; i++) {

			game.drawing_board.blit(game.board.white_graveyard[i]->sprite, &r, NULL);
			r.x -= TILE_SIZE;
			if (i % 3 == 2) {
				r.x += 3 * TILE_SIZE;
				r.y -= TILE_SIZE;
			}
		}
		r.x = 14 * TILE_SIZE;
		r.y = 3 * TILE_SIZE;
		for (int i = 0; i < game.board.black_death; i++) {
			game.drawing_board.blit(game.board.black_graveyard[i]->sprite, &r, NULL);
			r.x += TILE_SIZE;
			if (i % 3 == 2) {
				r.x -= 3 * TILE_SIZE;
				r.y += TILE_SIZE;
			}
		}
	}



	Uint64 t4 = SDL_GetTicks();

	game.global_buttons->draw();
	game.buttons->draw();

	Uint64 t5 = SDL_GetTicks();


	if (game.is_holding_something and game.selected_thing_sprite != NULL) {
		dest_rect.x = (int)(game.mouse_x * TILE_SIZE);
		dest_rect.y = (int)(game.mouse_y * TILE_SIZE);
		dest_rect.x -= (int)(TILE_SIZE * game.selected_thing_sprite_x_offset);
		dest_rect.y -= (int)(TILE_SIZE * game.selected_thing_sprite_y_offset);
		game.drawing_board.blit(game.selected_thing_sprite, &dest_rect, NULL);
	}

	Uint64 t6 = SDL_GetTicks();

	dest_rect.x = game.corner_x;
	dest_rect.y = game.corner_y;

	game.screen.blit(game.drawing_board, &dest_rect, NULL);

	Uint64 t7 = SDL_GetTicks();

	if (false) {
		PRINT_DEBUG("time to clear the previous surface: " << t2 - t1 << "ms");
		PRINT_DEBUG("time to draw the graveyards: " << t4 - t3 << "ms");
		PRINT_DEBUG("time to draw the buttons: " << t5 - t4 << "ms");
		PRINT_DEBUG("time to draw the selected thing: " << t6 - t5 << "ms");
		PRINT_DEBUG("time draw the drawing board onto the screen: " << t7 - t6 << "ms\n");
	}
}



void handle_event(SDL_Event e) {
	//Game& game = *game_ptr;

	switch (e.type) {
	case SDL_EVENT_QUIT:
		running = false;
		break;

	case SDL_EVENT_WINDOW_RESIZED: {

		Uint64 t5 = SDL_GetTicks();

		game.screen = game.window.get_surface();

		double ratio = (double)game.screen->w / (double)game.screen->h;
		if (ratio <= 1.5) {
			game.resize_drawing_board(game.screen->w / 18);
		}
		else {
			game.resize_drawing_board(game.screen->h / 12);
		}
		game.corner_x = (game.screen->w - game.drawing_board->w) / 2;
		game.corner_y = (game.screen->h - game.drawing_board->h) / 2;
		Uint64 t7 = SDL_GetTicks();

		printf("\nTemps du redimensionnage: %lldms\n", t7 - t5);
		break;
	}

	case SDL_EVENT_MOUSE_BUTTON_DOWN: {

		SDL_MouseButtonEvent* mbe /*mbe for mouse button event*/ = (SDL_MouseButtonEvent*)&e;

		double x, y;
		get_drawing_board_coordinates((Sint32)mbe->x, (Sint32)mbe->y, &x, &y);

		if (game.buttons->click(mbe->button, x, y))
			break;
		else if (game.global_buttons->click(mbe->button, x, y))
			break;
		else
			game.show_type_chart = false;

		if (mbe->button == SDL_BUTTON_LEFT) {
			if (game.state == in_game)
				game.unselect_piece();
		}
		else if (mbe->button == SDL_BUTTON_RIGHT) {
			switch (game.state) {
			case in_game: {
				int tile_x = (int)x; // x_position in tile coordinates,  relative to the bottom left corner
				int tile_y = 11 - (int)y; // y position in tile coordinates, relative to the bottom left corner

				if ((5 <= tile_x && tile_x < 13) && (2 <= tile_y && tile_y < 10)) {
					// the click is on the board congratulation
					Square& selected_square = game.board[tile_x - 5][tile_y - 2];
					if (selected_square.piece != NULL) {
						if (false) { // outil de debug pour voir les case controller par une piece
							for (Square& square : game.board) {
								if (selected_square.piece->do_control(square)) {
									square.is_accessible = true;
								}
							}
						}
					}
				}
				break;
			}
			}
		}
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_UP: {
		SDL_MouseButtonEvent* mbe = (SDL_MouseButtonEvent*)&e;

		if (Button::pressed_button != NULL and mbe->button == Button::pressed_button_mouse_button) {
			double x, y;
			get_drawing_board_coordinates((Sint32)mbe->x, (Sint32)mbe->y, &x, &y);
			Button::pressed_button->unhold(Button::pressed_button_mouse_button, x- Button::pressed_button->x, y - Button::pressed_button->y);
			Button::pressed_button = NULL;
			break;
		}

		double x, y;
		get_drawing_board_coordinates((Sint32)mbe->x, (Sint32)mbe->y, &x, &y);

		if (mbe->button == SDL_BUTTON_LEFT and game.is_holding_something) {
			game.is_holding_something = false;
			if (game.state == in_game) {
				game.selected_piece = NULL;
			}
			else if (game.state == in_selection) {
				if (game.type_selection and game.selected_type != typeless) {
					game.is_type_avaible |= (1 << game.selected_type);
					game.selected_type = typeless;
				}
				else if (game.selected_item != NULL) {
					game.selected_item->is_avaible = true;
					game.selected_item = NULL;

				}
			}
		}
		break;
	}

	case SDL_EVENT_KEY_DOWN: {
		SDL_KeyboardEvent* kbe = (SDL_KeyboardEvent*)&e;
		switch (kbe->key) {
		default:
			break;
		}
	}
	}
}
int counter = 0;
void loop() {
	Uint64 start_time = SDL_GetTicks();
	
	float x, y;
	auto mouse_state = SDL_GetMouseState(&x, &y);
	get_drawing_board_coordinates(x, y, &game.mouse_x, &game.mouse_y);

	while (SDL_PollEvent(&event)) {
		handle_event(event);
	}

	if (Button::pressed_button != NULL) {
		
		Button::pressed_button->hold(Button::pressed_button_mouse_button, game.mouse_x - Button::pressed_button->x, game.mouse_y - Button::pressed_button->y);
	}
	Uint64 time2 = SDL_GetTicks();

	draw_frame();
	
	Uint64 time3 = SDL_GetTicks();

	game.window.refresh();

	Uint64 time4 = SDL_GetTicks();


	if (TPF_ms > SDL_GetTicks() - start_time)
		SDL_Delay((int)(TPF_ms + start_time - SDL_GetTicks()));

	if (Button::pressed_button != NULL) {
		char title[100] = "";
		strcat_s(title, "(");
		strcat_s(title, std::to_string(game.screen->w).c_str());
		strcat_s(title, ", ");
		strcat_s(title, std::to_string(game.screen->h).c_str());
		strcat_s(title, ") ");
		strcat_s(title, std::to_string(1000 / (SDL_GetTicks() - start_time)).c_str());

		game.window.set_title(title);
	}
}
