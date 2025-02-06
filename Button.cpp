#include <cstring>
#include <random>

#include "Button.h"
#include "assets.h"

Button* Button::pressed_button = NULL;
int Button::pressed_button_mouse_button = 0;

Button::Button(double tile_x_, double tile_y_, double tile_w_, double tile_h_) { 
	x = tile_x_;
	y = tile_y_;
	w = tile_w_;
	h = tile_h_;
	currently_active = was_just_activated = false;
	collection = NULL;
}

bool Button::is_active() {
	return true;
}

void Button::resize() {
}

void ButtonCollection::add(Button* new_button) {
	if (new_button->collection != NULL) {
		PRINT_DEBUG("a button can only be part of one collection");
		throw std::exception("ButtonCollection::add()");
	}
	new_button->collection = this;
	if (first == NULL) {
		first = new Node(new_button);
		last = first;
	}
	else {
		last->next = new Node(new_button);
		last = last->next;
	}
}

void ButtonCollection::clear() {
	is_cleared = true;
	while (first != NULL) {
		Node* node = first;
		if (node->button == Button::pressed_button) {
			Button::pressed_button = NULL;
		}
		delete node->button;
		first = first->next;
		delete node;
	}
	last = NULL;
}

std::ostream& operator<<(std::ostream& os, ButtonCollection::Node* node) {
	if (node== NULL) {
		os << ";\n";
	}
	else {
		os << (void*)node << " -> ";
		os << node->next;
	}
	return os;
}
void ButtonCollection::draw() {
	Node* tmp;
#if SHOW_BUTTON_HITBOX
	static int counter = 0;
#endif
	for (Node* current = first; current != NULL; current = tmp) {
		tmp = current->next;
		if (game.state == end_of_game)
			PRINT_VAR(current->button);
		if (current->button->is_active()) {
			if (not current->button->currently_active) {
				current->button->currently_active = true;
				current->button->was_just_activated = true;
				current->button->activate();
			}
			else
				current->button->was_just_activated = false;

#if SHOW_BUTTON_HITBOX
			counter++;
			if (current->button->update() != Button::suicide) {
				current->button->draw();
				if (counter % 2)
					game.draw_rect(Color::red, current->button->x, current->button->y, current->button->w, current->button->h);
			}
#else
			if (current->button->update() != Button::suicide) {
				current->button->draw();
			}
#endif

		}
		else
			current->button->currently_active = false;
	}
}

void ButtonCollection::resize() {
	for (Node* current = first; current != NULL; current = current->next)
		current->button->resize();
}

bool ButtonCollection::click(int mouse_button, double x, double y) {
	is_cleared = false;
	for (Node* current = first; current != NULL; current = current->next) {
		if (current->button->is_active()) {
			double x_rel = x - current->button->x; // x coordinate relative to the top left corner of the button
			double y_rel = y - current->button->y; // y coordinate relative to the top left corner of the button

			if (0 <= x_rel and 0 <= y_rel and x_rel < current->button->w and y_rel < current->button->h) { // check wether the x and y coordinates are withing the button rect
				if (current->button->is_on_button(mouse_button, x_rel, y_rel)) {
					current->button->effect(mouse_button, x_rel, y_rel);
					if (not is_cleared) {
						Button::pressed_button = current->button;
						Button::pressed_button_mouse_button = mouse_button;
					}
					return true;
				}
			}

		}
	}
	return false;
}

void Button::kill() {
	PRINT_DEBUG("kill " << this);
	if (this == Button::pressed_button) {
		Button::pressed_button = NULL;
	}
	ButtonCollection::Node* previous = NULL;
	ButtonCollection::Node* next = NULL;
	for (ButtonCollection::Node* current = collection->first; current != NULL; previous = current, current = next) {
		next = current->next;
		if (current->button == this) {
			if (previous == NULL) {
				collection->first = current->next;
				if (collection->first = NULL) {
					collection->last = NULL;
				}
			}
			else {
				previous->next = current->next;
				if (previous->next == NULL) {
					collection->last = previous;
				}
			}
			
			current->next = NULL;
			delete current;
			break;
		}
	}
}


BeginGameButton::BeginGameButton() : Button(7.5, 5.5, 3.0, 1.0) {

}

bool BeginGameButton::is_active() {
	return not game.show_type_chart;
}

void BeginGameButton::draw() {
	game.draw(start_button, 9.0, 6.0, center);
}

void BeginGameButton::effect(int, double, double) {
	if (game.with_typing) {
		if (game.with_random_battle) {
			RandomTypingButton button = RandomTypingButton(0, 0);
			game.type_selection = true;
			button.effect(SDL_BUTTON_LEFT, 0, 0);
			game.type_selection = false;
			button.effect(SDL_BUTTON_LEFT, 0, 0);

			game.board.active_player = black;

			game.type_selection = true;
			button.effect(SDL_BUTTON_LEFT, 0, 0);
			game.type_selection = false;
			button.effect(SDL_BUTTON_LEFT, 0, 0);
			
			game.board.active_player = white;
			game.to_game();
		}
		else {
			game.to_selection();
		}
		game.with_check = false;
	}
	else {
		game.to_game();
		game.with_check = true;
	}
}

EndOfGameButton::EndOfGameButton() : Button(7.5, 5.5, 3.0, 1.0) {
	memset(message, '\0', 16);
}

void EndOfGameButton::activate() {
	switch (game.winner) {
	case white:
		bg_color = Color::white;
		txt_color = Color::black;
		memcpy(message, "White Wins!", 12);
		break;
	case black:
		bg_color = Color::black;
		txt_color = Color::white;
		memcpy(message, "Black Wins!", 12);
		break;
	case no_color:
		bg_color = Color::grey;
		txt_color = Color::black;
		memcpy(message, "Draw", 5);
		break;
	}
}

void EndOfGameButton::draw() {
	game.draw_rect(bg_color, x, y, w, h);
	Surface txt_surface = CSM_font_array[35].render_shaded(message, txt_color, bg_color);
	if (txt_surface != NULL)
		game.draw(txt_surface, 9.0 - (double)txt_surface->w / 2.0 / TILE_SIZE, 6.0 - (double)txt_surface->h / 2.0 / TILE_SIZE);

}

void EndOfGameButton::effect(int mouse_button, double, double) {
	if (mouse_button == SDL_BUTTON_LEFT)
		game.to_menu();
}

ExitGameButton::ExitGameButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {

}

bool ExitGameButton::is_active() {
	return game.state != in_menu and game.state != end_of_game and game.state != in_settings;
}

void ExitGameButton::draw() {
	game.draw(icon_sheet, x, y, 0, 0, 1, 1);
}

void ExitGameButton::effect(int mouse_button, double, double) {
	if (mouse_button == SDL_BUTTON_LEFT)
		game.to_menu();
}


PromotionButton::PromotionButton(std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> constructor, double tile_x_, double tile_y_) :
	Button(tile_x_, tile_y_, 1.0, 1.0), piece_constructor(constructor) {
	sprite = Surface::createRGBA(TILE_SIZE, TILE_SIZE);
}

void PromotionButton::resize() {
	sprite = Surface::createRGBA(TILE_SIZE, TILE_SIZE);

	Piece* piece = game.promoting_piece;
	Piece* temp = piece_constructor(game.board, piece->color, piece->square, piece->type, piece->item);
	SDL_Rect r(0, 0, TILE_SIZE, TILE_SIZE);
	sprite.blit(temp->sprite, &r, NULL);
	delete temp;
}

void PromotionButton::activate() {
	Piece* piece = game.promoting_piece;
	Piece* temp = piece_constructor(game.board, piece->color, piece->square, piece->type, piece->item);
	SDL_Rect r(0, 0, TILE_SIZE, TILE_SIZE);
	sprite.blit(temp->sprite, &r, NULL);
	delete temp;
}


void PromotionButton::draw() {
	game.draw(sprite, x, y);
}

void PromotionButton::effect(int mouse_button, double, double) {

	if (mouse_button != SDL_BUTTON_LEFT)
		return;
	Piece* piece = game.promoting_piece;
	Square* square = piece->square;
	PokeItem* item = piece->item;
	square->piece = piece_constructor(game.board, piece->color, square, piece->type, piece->item);
	if (item != NULL)
		item->holder = square->piece;
	
	delete piece;

	game.board.last_move_data.attacker = square->piece;
	game.promoting_piece = NULL;

	game.to_game(true);
	if (game.with_typing) {
		char buffer[64] = "Pawn evolved into\n";
		strcat_s(buffer, square->piece->Class->name);
		game.add_textbox(buffer);
	}

	game.resume_move();

	if (game.with_typing) {
		game.interupt_background(promotion_end_music, 0);
	}
}


TypingSelectionButton::TypingSelectionButton(double x_, double y_, typing type_) : Button(x_, y_, 1.0, 1.0) {
	type = type_;
}

bool TypingSelectionButton::is_active() {
	return game.type_selection and (game.is_type_avaible & (1 << type)) != 0;
}

void TypingSelectionButton::draw() {
	game.draw(typing_icon[type], x, y);
}

void TypingSelectionButton::effect(int mouse_button, double x_, double y_){
	if (mouse_button != SDL_BUTTON_LEFT)
		return;
	
	if (game.selected_type != typeless) {
		game.is_type_avaible |= (1 << game.selected_type);
	}

	if ((game.is_type_avaible >> type) & 1) {
		// the type is still avaible
		game.is_type_avaible &= ~(1 << type); // makes the typing no longer avaible
		game.is_holding_something = true;
		game.selected_type = type;
		game.selected_thing_sprite = typing_icon[type].scale_to(TILE_SIZE / 2, TILE_SIZE / 2, 1);

		game.selected_thing_sprite_x_offset = x_ / 2;
		game.selected_thing_sprite_y_offset = y_ / 2;

		game.phone_displayed_item = NULL;
		game.phone_displayed_piece = NULL;
		game.phone_displayed_type = type;
	}
}

ItemSelectionButton::ItemSelectionButton(double x_, double y_, ItemClass& Item_) : Button(x_, y_, 0.5, 0.5), Item(Item_) {
	Item.is_avaible = true;
}

bool ItemSelectionButton::is_active() {
	return not game.type_selection and Item.is_avaible;
}

bool ItemSelectionButton::is_on_button(int mouse_button, double x, double y) {
	return (mouse_button == SDL_BUTTON_LEFT) or (mouse_button == SDL_BUTTON_RIGHT);
}

void ItemSelectionButton::draw() {
	SDL_Rect rect(x * TILE_SIZE, y * TILE_SIZE, ITEM_SIZE, ITEM_SIZE);
	Item.draw(game.drawing_board, &rect);
}

Surface ItemSelectionButton::surface = Surface::createRGBA(ITEM_SIZE, ITEM_SIZE);

void ItemSelectionButton::effect(int mouse_click, double x_, double y_) {
	if (game.selected_item != NULL) {
		game.selected_item->is_avaible = true;
		game.is_holding_something = false;
		game.selected_item = NULL;
	}

	switch (mouse_click) {
	case SDL_BUTTON_LEFT:
		game.selected_item = &Item;
		Item.is_avaible = false;
		game.is_holding_something = true;
		game.selected_thing_sprite_x_offset = x_;
		game.selected_thing_sprite_y_offset = y_;
		if (surface == NULL or surface->w != ITEM_SIZE) {
			surface = Surface::createRGBA(ITEM_SIZE, ITEM_SIZE);
		}
		surface.clear();
		SDL_Rect rect(0, 0, ITEM_SIZE, ITEM_SIZE);
		Item.draw(surface, &rect);
		game.selected_thing_sprite = surface;

		if (game.show_phone) {
			game.phone_displayed_item = &Item;
			game.phone_displayed_piece = NULL;
		}

		if (Item.description != NULL)
			std::cout << Item.description << '\n';
		break;
	}
	
	
	
}

ConfirmSelectionButton::ConfirmSelectionButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {}

void ConfirmSelectionButton::draw() {
	game.draw(icon_sheet, x, y, 1, 0, 1, 1);
}

bool ConfirmSelectionButton::is_active() {
	return game.nb_of_piece_with_type == 16;

}

void ConfirmSelectionButton::effect(int mouse_button, double, double) {
	if (mouse_button != SDL_BUTTON_LEFT)
		return;

	game.is_type_avaible = 0xFFFFFFFF;
	game.type_selection = true;
	game.nb_of_piece_with_type = 0;
	for (ItemClass& item : item_table) {
		item.is_avaible = true;
	}
	
	game.phone_displayed_item = NULL;
	game.phone_displayed_type = typeless;
	game.phone_displayed_piece = NULL;

	if ((game.board.active_player = not game.board.active_player) == white){
		game.to_game();
	}
}

RandomTypingButton::RandomTypingButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {
	counter = 0;
	current_sprite = 0;
}

Button::update_return_code RandomTypingButton::update() {
	counter++;
	if (counter >= period) {
		counter = 0;
		current_sprite++;
		current_sprite %= 16;
	}
	return Button::nothing;
}

void RandomTypingButton::draw() {
	game.draw(unown_questionmark_animated, x, y, (double)current_sprite, 0.0, 1.0, 1.0);
}

void RandomTypingButton::effect(int mouse_button, double, double) {
	game.is_type_avaible = 0xFFFFFFFF; // makes all the types avaible again

	std::random_device rd;
	std::mt19937 gen(rd());

	if (game.type_selection) {
		typing list[18];
		for (typing t = normal; t <= fairy; t++) {
			list[t] = t;
		}

		std::shuffle(&list[normal], &list[fairy], gen);

		if (game.board.active_player == white)
			for (int i = 0; i < 16; i++) {
				Square& square = game.board[i % 8][i / 8];
				if (square.piece != NULL) {
					square.piece->set_type(list[i]);
					game.is_type_avaible &= ~(1 << list[i]); // makes the type not avaible
				}
			}
		else
			for (int i = 0; i < 16; i++) {
				Square& square = game.board[i % 8][i / 8 + 6];
				if (square.piece != NULL) {
					square.piece->set_type(list[i]);
					game.is_type_avaible &= ~(1 << list[i]); // makes the type not avaible
				}
			}

		game.nb_of_piece_with_type = 16;
	}
	else {
		int x = 0;
		int y = (game.board.active_player == white) ? 0 : 6;
		
		int j = 16;
		ItemClass* list[NB_OF_ITEMS];
		int current = 0;
		for (ItemClass& Item : item_table) {
			list[current++] = &Item;
			Item.is_avaible = true;
		}

		while (j > 0) {
			Piece* piece = game.board[x][y].piece;
			std::shuffle(list, list + NB_OF_ITEMS, gen);
			std::sort(
				list, 
				list + NB_OF_ITEMS, 
				[&piece]
				(const ItemClass* x, const ItemClass* y) -> bool {
					return x->usefulness_tier(piece) >= y->usefulness_tier(piece); 
				}
			);
			for (ItemClass* Item : list) {
				if (Item->type == normal_item and (game.with_RNG or not Item->is_RNG_dependant) and Item->is_avaible) {
					Item->is_avaible = false;
					piece->set_item((*Item)(piece));
					x++;
					if (x == 8) {
						x = 0;
						y++;
					}
					j--;
					break;
				}
			}
		}
	}
}

SwitchSelectionButton::SwitchSelectionButton(double x_, double y_) : Button(x_, y_, 3, 1) {
	;
}

void SwitchSelectionButton::draw() {
	const char* txt = "";
	switch (game.language) {
	case LANGUAGE::ENGLISH:
		txt = (game.type_selection) ? "Types" : "Items";
		break;
	case LANGUAGE::FRENCH:
		txt = (game.type_selection) ? "Types" : "Objets";
		break;
	case LANGUAGE::GERMAN:
		txt = (game.type_selection) ? "Typen" : "Objekte";
		break;
	case LANGUAGE::SPANISH:
		txt = (game.type_selection) ? "Typos" : "Objetos";
		break;
	}

	game.draw(CSM_font_array[TILE_SIZE / 2].render_shaded(txt, Color::black, game.bg_color), x, y);
}

void SwitchSelectionButton::effect(int, double, double) {
	game.type_selection = not game.type_selection;
	if (game.selected_item != NULL) {
		game.selected_item->is_avaible = true;
	}
	else if (game.selected_type != typeless) {
		game.is_type_avaible |= (1 << game.selected_type);
	}
	game.is_holding_something = false;

}

TextBoxDisplay::TextBoxDisplay(const char* text, bool _is_first) : Button(5.0, 0.0, 8.0, textbox_frame->h * 8.0 / textbox_frame->w) {
	sprite = Surface::createRGB(textbox_frame->w, textbox_frame->h);
	sprite.blit(textbox_frame, NULL, NULL);
	visual_index = text_index = 0;
	timer = 0;
	shift_timer = 0;
	shift_delay = 0;
	is_on_second_line = false;
	x = 5.0;
	side = black;
	y = 2.0;
	PRINT_VAR(this);
	strcpy_s(message, text);
	is_first = _is_first;
}

bool TextBoxDisplay::is_active() {
	return is_first;
}

void TextBoxDisplay::activate() {

	if (game.board.last_move_begin_square == NULL or game.board.last_move_end_square->y > 3) {
		y = 10;
		side = white;
	}
	else {
		y = 2;
		side = black;
	}
	timer = duration;
	sprite.blit(textbox_frame, NULL, NULL);
	is_on_second_line = false;
	visual_index = text_index = 0;
}

void TextBoxDisplay::destroy_all() {
	if (next != NULL) {
		next->destroy_all();
	}
	kill();
	delete this;
}

Button::update_return_code TextBoxDisplay::update() {
	if (shift_delay > 0) {
		shift_delay--;
		return Button::nothing;
	}
	if (shift_timer > 0) {
		shift_timer--;
		SDL_Rect dest(begin_x, begin_y, 0, 0);
		SDL_Rect area(begin_x, begin_y + 4, char_per_line * char_width, char_width + y_pixel_increment);
		sprite.blit(sprite, &dest, &area);

		return Button::nothing;
	}
	char displayed_char = (text_index >= BUFFER_SIZE) ? '\0' : message[text_index];
	if (displayed_char != '\0') {
		if (displayed_char == '\n') {
			visual_index = 0;
			if (not is_on_second_line)
				is_on_second_line = true;
			else {
				shift_delay = FPS / 4;
				shift_timer = y_pixel_increment / 4;
			}
		}
		else if (displayed_char < 128) {
			SDL_Rect r;
			r.x = begin_x + char_width * visual_index - (displayed_char=='\'')*2;
			r.y = begin_y + y_pixel_increment * is_on_second_line;

			Surface char_surface = poke_charset.chop({ (displayed_char % 16) * 16, (displayed_char / 16) * 16, 16, 16 });
			char_surface.set_colorkey(char_surface.map_rgba(255, 255, 255, 255), true);

			sprite.blit(char_surface, &r, NULL);

			if (displayed_char != '\'')
				visual_index++;
		}
		text_index++;
	}
	else {
		timer--;
		if (timer < 0) {
			kill();
			
			if (next == NULL) {
				game.active_textbox = game.last_textbox = NULL;
			}
			else {
				game.active_textbox = next;
				game.active_textbox->is_first = true;
			}

			delete this;
			return Button::suicide;
		}
	}
	return Button::nothing;
}

void TextBoxDisplay::draw() {
	Surface scaled = sprite.scale_by(8.0 * TILE_SIZE / sprite->w, true);
	game.draw(scaled, x, y, (side == white) ? bottom_left : top_left);
}

TextBoxDisplay::~TextBoxDisplay() {
	PRINT_DEBUG("free " << this);
}

SkipBonusMoveButton::SkipBonusMoveButton(double x_, double y_) : Button(x_ - skip_button->w / 2.0 / TILE_SIZE, y_ - skip_button->h / 2.0 / TILE_SIZE, (double)skip_button->w / TILE_SIZE, (double)skip_button->h / TILE_SIZE) {

}

bool SkipBonusMoveButton::is_active() { return game.board.in_bonus_move; }

void SkipBonusMoveButton::draw() {
	game.draw(skip_button, x, y);
}

void SkipBonusMoveButton::effect(int mouse_button, double, double) {
	game.board.in_bonus_move = false;
	game.unselect_piece();
	game.change_turn();
	game.check_for_end_of_game();
}

StatBoostDisplay::StatBoostDisplay(Piece* boosted_piece, pokestat boosted_stat, int delay_, int boost_or_debuff_) : Button(0.0, 0.0, 0.0, 0.0) {
	piece = boosted_piece;
	arrow_offset = 0;
	stat = boosted_stat;
	delay = delay_;
	counter = TextBoxDisplay::duration;
	boost_or_debuff = boost_or_debuff_;
}

Button::update_return_code StatBoostDisplay::update() {
	if (delay > 0) {
		delay--;
		if (delay == 0)
			if (boost_or_debuff == 1)
				Mix_PlayChannel(1, stat_increase_effect, 0);
			else
				;
	}
	else {
		counter--;
		arrow_offset += 0.75/16.0;
		if (counter < 0) {
			kill();
			delete this;
			return Button::suicide;
		}
	}
	return Button::nothing;
}

void StatBoostDisplay::draw() {
	if (delay > 0 or piece == NULL)
		return;

	Surface surface = game.drawing_board;
	Surface arrow = boost_arrows.chop({ TILE_SIZE * (int)stat, 0, TILE_SIZE, TILE_SIZE }).inplace_toric_scroll(0, (int)(- arrow_offset * TILE_SIZE * boost_or_debuff));
	if (surface.MUSTLOCK())
		surface.lock();

	Uint8* arrow_pixels = (Uint8*)arrow->pixels;
	auto surface_format = SDL_GetPixelFormatDetails(surface.get_format());
	auto arrow_format = SDL_GetPixelFormatDetails(arrow.get_format());
	int arrow_pitch = arrow->pitch;
	int surface_pitch = surface->pitch;


	Uint32* pixels = (Uint32*)surface->pixels;
	//Uint32* begin = &pixels[0];

	Uint32* begin = &pixels[TILE_SIZE * ((7-piece->y) + 2) * surface_pitch/4 + TILE_SIZE * (piece->x + 5)];
	Uint32* end = &pixels[TILE_SIZE * ((7-piece->y) + 3) * surface_pitch/4 + TILE_SIZE * (piece->x + 5)];

	Uint32 bg_color = begin[0];

	Uint32* row = begin;

	Uint8 arrow_R;
	Uint8 arrow_G;
	Uint8 arrow_B;

	Uint8 surface_R;
	Uint8 surface_G;
	Uint8 surface_B;
	
	for (int y = 0; y < TILE_SIZE; y++) {
		for (int x = 0; x < TILE_SIZE; x++) {
			Uint32 surface_px;
			if ((surface_px = row[x]) != bg_color) {
				Uint8 arrow_px = arrow_pixels[y * arrow_pitch + x];
				SDL_GetRGB(arrow_px, arrow_format, SDL_GetSurfacePalette(arrow), &arrow_R, &arrow_G, &arrow_B);
				
				SDL_GetRGB(surface_px, surface_format, SDL_GetSurfacePalette(surface), &surface_R, &surface_G, &surface_B);

				row[x] = SDL_MapRGB(surface_format, SDL_GetSurfacePalette(surface), arrow_R * 0.5 + surface_R * 0.5, arrow_G * 0.5 + surface_G * 0.5, arrow_B * 0.5 + surface_B * 0.5);
			}
		}

		row += surface_pitch/4;
	}


	if (surface.MUSTLOCK())
		surface.unlock();
}


ShowTypechartButton::ShowTypechartButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {
	int a = (TILE_SIZE - 19) / 18;
	resize();
}

bool ShowTypechartButton::is_active() {
	return game.with_typing and not (game.state == in_settings);
}

void ShowTypechartButton::resize() {
	int a = (TILE_SIZE - 19) / 18;
	sprite = Surface::createRGBA(18 * a + 19, 18 * a + 19);
	sprite.fill(Color::black);
	SDL_Rect r;
	r.w = r.h = a;
	r.x = 1;
	iter_typing(i) {
		r.y = 1;
		iter_typing(j) {

			switch (typechart[j][i]) {
			case neutral:
				SDL_FillSurfaceRect(sprite, &r, 0);
				break;
			case super_effective:
				SDL_FillSurfaceRect(sprite, &r, Color::green);
				break;
			case not_very_effective:
				SDL_FillSurfaceRect(sprite, &r, Color::red);
				break;
			case immune:
				SDL_FillSurfaceRect(sprite, &r, Color::black);
				break;
			}

			r.y += a + 1;
		}
		r.x += a + 1;
	}
}

void ShowTypechartButton::draw() {
	game.draw(sprite, x, y);
}

void ShowTypechartButton::effect(int mouse_button, double, double) {
	if (mouse_button == SDL_BUTTON_LEFT)
		game.show_type_chart = not game.show_type_chart;
}


TypechartDisplay::TypechartDisplay() : Button(5.0, 2.0, 8.0, 8.0) {
	;
}

bool TypechartDisplay::is_active() {
	return game.show_type_chart;
}

void TypechartDisplay::draw() {
	int a = TILE_SIZE * 8 / 19;
	int cx = (TILE_SIZE * 8 - 19 * a) / 2;
	int cy = cx;
	SDL_Rect rect;
	game.draw(CSM_font_array[a].render_shaded("Offensive Typing", Color::black, game.bg_color).rotate90(), 5.0, 6.0, middle_right);

	game.draw(CSM_font_array[a].render_shaded("Defensive Typing", Color::black, game.bg_color), 9.0, 2.0, bottom_middle);

	game.drawing_board.draw_disk(5 * TILE_SIZE + a / 2, 10 * TILE_SIZE + 11 * a / 4, 3 * a / 8, Color::black);
	rect.x = 5 * TILE_SIZE + a;
	rect.y = 10 * TILE_SIZE + 2 * a;
	game.drawing_board.blit(CSM_font_array[7 * a / 8].render_shaded("immune", Color::black, game.bg_color), &rect);

	game.drawing_board.draw_disk(5 * TILE_SIZE + a / 2, 10 * TILE_SIZE + 7 * a / 4, 3 * a / 8, Color::red);
	rect.x = 5 * TILE_SIZE + a;
	rect.y = 10 * TILE_SIZE + a;
	game.drawing_board.blit(CSM_font_array[7 * a / 8].render_shaded("not very effective", Color::black, game.bg_color), &rect);

	game.drawing_board.draw_disk(5 * TILE_SIZE + a / 2, 10 * TILE_SIZE + 3 * a / 4, 3 * a / 8, Color::green);
	rect.x = 5 * TILE_SIZE + a;
	rect.y = 10 * TILE_SIZE;
	game.drawing_board.blit(CSM_font_array[7 * a / 8].render_shaded("super effective", Color::black, game.bg_color), &rect);

	game.draw_rect(Color::silver, 5.0, 2.0, 8.0, 8.0);
	for (typing t = normal; t <= fairy; t++) {
		Surface icon = typing_icon[t].scale_to(a, a, true);
		SDL_Rect r(5 * TILE_SIZE + (t+1) * a + cx, 2*TILE_SIZE + cy, a, a);
		game.drawing_board.blit(icon, &r);
		r.w = 1;
		r.h = 19 * a;
		SDL_FillSurfaceRect(game.drawing_board, &r, Color::black);

		r.x = 5 * TILE_SIZE + cx;
		r.y += (t + 1) * a;
		game.drawing_board.blit(icon, &r);
		r.h = 1;
		r.w = 19 * a;
		SDL_FillSurfaceRect(game.drawing_board, &r, Color::black);

		r.x = cx + a + a/2 + 5 * TILE_SIZE;
		r.y = cx + a * (t + 1) + a/2 + 2 * TILE_SIZE;
		for (typing j = normal; j <= fairy; j++) {
			Uint32 color = 0;
			switch (typechart[t][j]) {
			case immune:
				color = Color::black;
				break;
			case not_very_effective:
				color = Color::red;
				break;
			case neutral:
				color = 0;
				break;
			case super_effective:
				color = Color::green;
				break;
			}
			if (color != 0)
				game.drawing_board.draw_disk(r.x, r.y, 3*a/8, color);

			r.x += a;
		}
	}
	
}

SettingsButton::SettingsButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {
	current_sprite = 0;
	counter = 0;
}

Button::update_return_code SettingsButton::update() {
	counter++;
	if (counter >= period) {
		counter = 0;
		current_sprite++;
		current_sprite %= 16;
	}
	return Button::nothing;
}

void SettingsButton::draw() {
	game.draw(clink_animated, x, y, (double)current_sprite, 0.0, 1.0, 1.0);
}

void SettingsButton::effect(int, double, double) {
	if (game.state == in_settings) {
		game.exit_settings();
	}
	else {
		game.to_settings();
	}
}


DisableRNGButton::DisableRNGButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {
	;
}

void DisableRNGButton::draw() {
	game.draw(pokeball_img, x, y);
	game.draw(icon_sheet, x, y, game.with_RNG, 0, 1, 1);
	game.draw(CSM_font_array[TILE_SIZE / 2].render_shaded("RNG", Color::black, game.bg_color), x + 1.0, y + 0.5, middle_left);
}

void DisableRNGButton::effect(int mouse_button, double, double) {
	if (mouse_button == SDL_BUTTON_LEFT) {
		game.with_RNG = not game.with_RNG;
	}
}


DisableSoundsButton::DisableSoundsButton(double x_, double y_) : Button(x_, y_, 3.0, 1.0) {
	;
}

void DisableSoundsButton::draw() {
	game.draw(CSM_font_array[TILE_SIZE / 2].render_shaded("Sounds: ", Color::black, game.bg_color), x, y+0.5, middle_left);
	game.draw(megaphone_img, x+2.0, y);
	game.draw(icon_sheet, x+2.0, y, game.with_sounds, 0, 1, 1);

}

bool DisableSoundsButton::is_on_button(int mouse_button, double x_, double y_) {
	return mouse_button == SDL_BUTTON_LEFT and x_ >= 2.0;
}

void DisableSoundsButton::effect(int mouse_button, double, double) {
	game.with_sounds = not game.with_sounds;
	Mix_VolumeMusic(game.with_sounds ? game.music_volume : 0);
	Mix_Volume(-1, game.with_sounds ? game.music_volume : 0);
}


VolumeSlider::VolumeSlider(double x_, double y_) : Button(x_, y_, 6.5, 1.0) {

}

bool VolumeSlider::is_on_button(int mouse_button, double x_, double y_) {
	return true;
}

void VolumeSlider::draw() {
	game.draw(CSM_font_array[TILE_SIZE / 2].render_shaded("Volume:", Color::black, game.bg_color), x, y+0.5, middle_left);
	game.draw_rect(Color::silver, x + 2.25, y+1./3., 2.5, 1./3.);
	double rate = (double)game.music_volume / 128.0;
	game.draw_rect(Color::black, x + 2.0 + rate * 2.5, y + 0.25, 0.5, 0.5);
	game.draw(CSM_font_array[TILE_SIZE / 2].render_shaded((std::to_string((int)(rate * 100)) += '%').c_str(), Color::black, game.bg_color), x + 5.1, y + 0.5, middle_left);
}

double clamp(double x, double min, double max) {
	return x < min ? min : x > max ? max : x;
}

void VolumeSlider::hold(int mouse_button, double x_, double) {
	PRINT_VAR(x_);
	game.music_volume = 128 * (clamp(x_-2.25, 0.0, 2.5) / 2.5);
	if (game.with_sounds) {
		Mix_VolumeMusic(game.music_volume);
		Mix_Volume(-1, game.music_volume);
	}
}

RandomBattleButton::RandomBattleButton(double x_, double y_) : Button(x_, y_, 1.0, 1.0) {
	counter = 0;
	current_sprite = 0;
}

Button::update_return_code RandomBattleButton::update() {
	counter++;
	if (counter >= period) {
		counter = 0;
		current_sprite++;
		current_sprite %= 16;
	}
	return Button::nothing;
}

void RandomBattleButton::draw() {
	game.draw(icon_sheet, x, y, game.with_random_battle, 0, 1, 1);
	game.draw(unown_questionmark_animated, x, y, current_sprite, 0, 1, 1);
	game.draw(CSM_font_array[TILE_SIZE / 3].render_shaded("Random battle", Color::black, game.bg_color), x + 1, y + 0.5, middle_left);
}

void RandomBattleButton::effect(int, double, double) {
	game.with_random_battle = not game.with_random_battle;
}


BoardButton::BoardButton(Board& b) : board(b), Button(5, 2, 8, 8) {
	is_first_unhold = true;
	no_unclick = false;
}

bool BoardButton::is_active() {
	return game.state != in_settings;
}

void BoardButton::draw() {
	//game.draw(board.surface, x, y, top_left);

	for (Square& square : board) {
		square.draw();
	}
}

void BoardButton::effect(int mouse_button, double x, double y) {
	int tile_x = (int)x;
	int tile_y = 7 - (int)y;

	Square& selected_square = game.board[tile_x][tile_y];
	Piece* piece = selected_square.piece;
	switch (game.state) {
	case in_game:
		if (selected_square.is_accessible and game.selected_piece != NULL) {
			game.move_selected_piece_to(selected_square);
		}
		else {
			if (game.selected_piece == piece && piece != NULL) {
				is_first_unhold = false;
				game.is_holding_something = true;
				game.selected_thing_sprite_x_offset = (x - tile_x);
				game.selected_thing_sprite_y_offset = y - (7 - tile_y);
				game.selected_thing_sprite = piece->sprite;

			}
			else {
				if (game.selected_piece != NULL) {
					game.unselect_piece();
				}
				if (piece != NULL && piece->color == game.board.active_player) {
					game.is_holding_something = true;
					game.selected_thing_sprite_x_offset = (x - tile_x);
					game.selected_thing_sprite_y_offset = y - (7 - tile_y);
					game.selected_thing_sprite = piece->sprite;
					game.select_piece(piece);
				}
			}
			
			if (piece != NULL) {
				game.phone_displayed_piece = piece;
				game.phone_displayed_type = typeless;
				game.phone_displayed_item = NULL;
			}
		}
		break;
	case in_selection:
		if (game.type_selection) {
			if (piece != NULL and piece->color == game.board.active_player) {
				if (game.selected_type != typeless) {
					if (piece->type != typeless) {
						game.is_type_avaible |= (1 << selected_square.piece->type); // add the old type of the piece to the type pool
						game.nb_of_piece_with_type--;
					}
					game.nb_of_piece_with_type++;
					piece->set_type(game.selected_type);
					no_unclick = true;
					game.selected_type = typeless;
					game.selected_thing_sprite = NULL;
					game.is_holding_something = false;
				}
			}
			else {
				if (game.selected_type != typeless) 
					game.is_type_avaible |= (1 << game.selected_type);
				game.is_holding_something = false;
			}
		}
		else {
			if (piece != NULL and piece->color == game.board.active_player) {
				if (game.selected_item != NULL) {
					if (piece->item != NULL) {
						piece->item->cls.is_avaible = true;
						piece->set_item(NULL);
					}

					piece->set_item((*game.selected_item)(piece));
					game.selected_item = NULL;
					game.is_holding_something = false;
					game.selected_thing_sprite = NULL;

					no_unclick = true;
				}
			}
			else {
				if (game.selected_item != NULL)
					game.selected_item->is_avaible = true;
				game.is_holding_something = false;
			}
		}
		break;
	}
}

void BoardButton::unhold(int mouse_button, double x, double y) {
	if (no_unclick) {
		no_unclick = false;
		return;
	}
	int tile_x;
	int tile_y;
	if (game.is_holding_something) {
		tile_x = (int)(x - game.selected_thing_sprite_x_offset + game.selected_thing_sprite->w / 2.0 / TILE_SIZE); // x_position, in tile coordinates, of the center of game.selected_thing_sprite, relative to the bottom left corner,
		tile_y = 7 - (int)(y - game.selected_thing_sprite_y_offset + game.selected_thing_sprite->h / 2.0 / TILE_SIZE); // y position, in tile coordinates, of the center of game.selected_thing_sprite, relative to the bottom left corner
	}
	else {
		tile_x = (int)x;
		tile_y = 7 - (int)y;
	}
	if (tile_x < 0 or tile_x >= 8 or tile_y < 0 or tile_y >= 8) {
		is_first_unhold = true;
		return;
	}

	Square& selected_square = game.board[tile_x][tile_y];
	Piece* piece = selected_square.piece;
	switch (game.state) {
	case in_game:
		if (game.is_holding_something and game.selected_piece != NULL) {
			if (selected_square.is_accessible) {
				is_first_unhold = true;
				game.move_selected_piece_to(selected_square);
			}
			else {
				game.is_holding_something = false;
				if (is_first_unhold) {
					// is_first_unhold = false;
				}
				else {
					game.unselect_piece();
					is_first_unhold = true;
				}
			}
		}
		break;
	case in_selection:
		if (game.type_selection) {
			if (game.selected_type == typeless and piece != NULL and piece->type != typeless and piece->color == game.board.active_player) {
				game.selected_type = piece->type;
				game.is_holding_something = true;
				game.selected_thing_sprite = typing_icon[game.selected_type].scale_by(0.5, true);
				piece->set_type(typeless);
				game.nb_of_piece_with_type--;

				game.phone_displayed_item = NULL;
				game.phone_displayed_type = game.selected_type;
				game.phone_displayed_piece = NULL;
			}
		}
		else {
			if (game.selected_item == NULL and piece != NULL and piece->item != NULL and piece->color == game.board.active_player) {
				ItemClass& Item = piece->item->cls;
				game.selected_item = &Item;
				game.is_holding_something = true;

				Surface square = Surface::createRGBA(ITEM_SIZE, ITEM_SIZE);
				Item.draw(square);
				game.selected_thing_sprite = square;

				piece->set_item(NULL);

				game.phone_displayed_item = &Item;
				game.phone_displayed_type = typeless;
				game.phone_displayed_piece = NULL;
			}
		}
	}
}


InformationDisplay::InformationDisplay(double x_, double y_) : Button(x_ - 1.25, y_ - 1.6, 2.5, 4.6) {
	displayed_type = typeless;
	displayed_item = NULL;
	displayed_piece = NULL;
	screen = Surface::createRGB(TILE_SIZE * w, TILE_SIZE * h);
	edge = 0.1 * TILE_SIZE;
	language = LANGUAGE::FRENCH;
}

#define CAPITALIZE(str) { \
	char* __str = (str); \
	if ('a' <= __str[0] and __str[0] <= 'z')\
		__str[0] += ('A' - 'a'); \
	\
}

void InformationDisplay::draw() {
	if (game.phone_displayed_item != displayed_item 
		or game.phone_displayed_type != displayed_type 
		or game.phone_displayed_piece != displayed_piece 
		or game.phone_displayed_page != displayed_page 
		or language != game.language) 
	{
		displayed_item = game.phone_displayed_item;
		displayed_piece = game.phone_displayed_piece;
		displayed_type = game.phone_displayed_type;
		displayed_page = game.phone_displayed_page;
		language = game.language;

		re_draw();
	}
	game.draw(screen, x, y);
	game.draw(phone_frame, x + w/2, y + h/2 - 0.45, center);
}

void InformationDisplay::resize() {
	edge = 0.1 * TILE_SIZE;
	screen = Surface::createRGB(TILE_SIZE * w, TILE_SIZE * h);
	re_draw();
}

void InformationDisplay::re_draw() {
	//game.draw_rect(Color::sky, x - 1.2, y - 1.9, 2.45, 4.8);
	screen.fill(Color::sky);
	PRINT_VAR((int)(game.language));
	// game.draw(phone_frame, x, y, center);
	if (displayed_item != NULL) {
		int X = w * TILE_SIZE / 2;
		int Y = 0.35 * TILE_SIZE;
		SDL_Rect rect(X, Y);
		Surface title = CSM_font_array[TILE_SIZE / 3].render_shaded_wrapped(displayed_item->name[(int)language], Color::black, Color::sky, 2.4 * TILE_SIZE);
		screen.blit(title, &rect, NULL, top_middle);
		Y += title->h;
		rect.y = Y;
		rect.x = X;

		displayed_item->draw(screen, &rect, mega, top_middle);
		Y += TILE_SIZE;
		X = edge;

		rect.y = Y;
		rect.x = X;

		screen.blit(CSM_font_array[TILE_SIZE / 5].render_blended_wrapped(displayed_item->description[(int)language], Color::black, TILE_SIZE * 2.3), &rect, NULL, top_left);
	}
	else if (displayed_type != typeless) {

		char buffer[64] = "";
		int i = 0;
		switch (language) {
		case LANGUAGE::FRENCH:
			for (const char* begin = "Type "; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			for (const char* begin = type_str[(int)language][displayed_type]; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			//buffer[i++] = ' '; buffer[i++] = ':'; buffer[i++] = '\0';
			
			break;
		case LANGUAGE::ENGLISH:
			for (const char* begin = type_str[(int)language][displayed_type]; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			for (const char* begin = " type"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			break;
		}
		

		CAPITALIZE(buffer);

		double const mini_size = 0.3;
		SDL_Rect rect;
		int Y = 0.4 * TILE_SIZE;
		int X = w * TILE_SIZE / 2;
		rect.x = X;
		rect.y = Y;

		screen.blit(CSM_font_array[(int)(TILE_SIZE / 3.1)].render_blended(buffer, Color::black), &rect, NULL, top_middle);
		Y += (int)(0.45 * TILE_SIZE);
		rect.x = X;
		rect.y = Y;
		screen.blit(typing_icon[displayed_type], &rect, NULL, top_middle);
		Y += (int)(1.15 * TILE_SIZE);

		bool any = false;
		iter_typing(type) {
			if (typechart[type][displayed_type] == immune) {
				any = true;
				break;
			}
		}
		if (any) {
			i = 0;
			for (const char* begin = "Immunity:"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			buffer[i] = '\0';
			Surface immunity_text = CSM_font_array[(int)(TILE_SIZE / 4)].render_blended(buffer, Color::black);
			X = edge;

			rect.x = X;
			rect.y = Y;
			screen.blit(immunity_text, &rect, NULL, middle_left);

			X = edge;
			X += immunity_text->w;

			iter_typing(type) {
				if (typechart[type][displayed_type] == immune) {
					rect.x = X;
					rect.y = Y;
					screen.blit(typing_icon[type].scale_by(mini_size, true), &rect, NULL, middle_left);

					X += (int)(mini_size * TILE_SIZE);
				}
			}
			Y += (int)(mini_size * TILE_SIZE);
			any = false;
		}
		int nb = 0;
		iter_typing(type) {
			if (typechart[type][displayed_type] == not_very_effective) {
				any = true;
				nb++;
			}
		}
		if (any) {
			i = 0;
			for (const char* begin = "Resistance:"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			buffer[i] = '\0';
			Surface text = CSM_font_array[(int)(TILE_SIZE / 4)].render_blended(buffer, Color::black);

			X = edge;

			rect.x = X;
			rect.y = Y;
			screen.blit(text, &rect, NULL, middle_left);

			if (nb > 3) {
				X = 2 * edge;
				Y += (int)(mini_size * TILE_SIZE);
			}
			else {
				X += text->w;
			}
			i = 0;
			iter_typing(type) {
				if (typechart[type][displayed_type] == not_very_effective) {
					rect.x = X;
					rect.y = Y;
					screen.blit(typing_icon[type].scale_by(mini_size, true), &rect, NULL, middle_left);
					X += (int)(mini_size * TILE_SIZE);
					i++;
					if (i >= 7 and i != nb) {
						X = 2 * edge;
						i = 0;
						Y += (int)(mini_size * TILE_SIZE);
					}
				}
			}
			Y += (int)(mini_size * TILE_SIZE);
			any = false;
		}
		
		{
			nb = 0;
			iter_typing(type) {
				if (typechart[type][displayed_type] == super_effective) {
					nb++;
				}
			}

			i = 0;
			for (const char* begin = "Weakness:"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			buffer[i] = '\0';
			Surface text = CSM_font_array[(int)(TILE_SIZE / 4)].render_blended(buffer, Color::black);

			X = edge;
			rect.x = X;
			rect.y = Y;
			screen.blit(text, &rect, NULL, middle_left);

			if (nb > 3) {
				X = 2 * edge;
				Y += (int)(mini_size * TILE_SIZE);

			}
			else {
				X += text->w;
			}

			i = 0;
			iter_typing(type) {
				if (typechart[type][displayed_type] == super_effective) {
					rect.x = X;
					rect.y = Y;
					screen.blit(typing_icon[type].scale_by(mini_size, true), &rect, NULL, middle_left);
					X += (int)(mini_size * TILE_SIZE);
					i++;
					if (i >= 7 and i != nb) {
						X = 2 * edge;
						i = 0;
						Y += (int)(mini_size * TILE_SIZE);
					}
				}
			}
			Y += (int)(mini_size * TILE_SIZE);

		}

		nb = 0;
		iter_typing(type) {
			if (typechart[displayed_type][type] == immune) {
				any = true;
			}
		}
		if (any) {
			i = 0;
			for (const char* begin = "Not effective:"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			buffer[i] = '\0';
			Surface text = CSM_font_array[(int)(TILE_SIZE / 4)].render_blended(buffer, Color::black);

			X = edge;
			rect.x = X;
			rect.y = Y;
			screen.blit(text, &rect, NULL, middle_left);

			X += text->w;
			

			i = 0;
			iter_typing(type) {
				if (typechart[displayed_type][type] == immune) {
					rect.x = X;
					rect.y = Y;
					screen.blit(typing_icon[type].scale_by(mini_size, true), &rect, NULL, middle_left);
					X += (int)(mini_size * TILE_SIZE);
					i++;
				}
			}
			Y += (int)(mini_size * TILE_SIZE);
		}

		nb = 0;
		iter_typing(type) {
			if (typechart[displayed_type][type] == not_very_effective) {
				any = true;
				nb++;
			}
		}

		if (any) {
			i = 0;
			for (const char* begin = "Resisted:"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			buffer[i] = '\0';
			Surface weakness_text = CSM_font_array[(int)(TILE_SIZE / 4)].render_blended(buffer, Color::black);

			X = edge;
			rect.x = X;
			rect.y = Y;
			screen.blit(weakness_text, &rect, NULL, middle_left);

			if (nb > 4) {
				X = 2*edge;
				Y += (int)(mini_size * TILE_SIZE);
			}
			else {
				X += weakness_text->w;
			}

			i = 0;
			iter_typing(type) {
				if (typechart[displayed_type][type] == not_very_effective) {
					rect.x = X;
					rect.y = Y;
					screen.blit(typing_icon[type].scale_by(mini_size, true), &rect, NULL, middle_left);
					X += (int)(mini_size * TILE_SIZE);
					i++;
					if (i >= 7 and i != nb) {
						X = 2 * edge;
						i = 0;
						Y += (int)(mini_size * TILE_SIZE);
					}
				}
			}
			Y += (int)(mini_size * TILE_SIZE);
			any = false;
		}

		nb = 0;
		iter_typing(type) {
			if (typechart[displayed_type][type] == super_effective) {
				any = true;
				nb++;
			}
		}
		if (any) {
			i = 0;
			for (const char* begin = "Super effective:"; *begin != '\0'; begin++) {
				buffer[i++] = *begin;
			}
			buffer[i] = '\0';
			Surface text = CSM_font_array[(int)(TILE_SIZE / 4)].render_blended(buffer, Color::black);

			X = edge;
			rect.x = X;
			rect.y = Y;
			screen.blit(text, &rect, NULL, middle_left);

			if (nb > 1) {
				X = 2 * edge;
				Y += (int)(mini_size * TILE_SIZE);
			}
			else {
				X += text->w;
			}

			i = 0;
			iter_typing(type) {
				if (typechart[displayed_type][type] == super_effective) {
					rect.x = X;
					rect.y = Y;
					screen.blit(typing_icon[type].scale_by(mini_size, true), &rect, NULL, middle_left);
					X += (int)(mini_size * TILE_SIZE);
					i++;
					if (i >= 7 and i != nb) {
						X = 2 * edge;
						i = 0;
						Y += (int)(mini_size * TILE_SIZE);
					}
				}
			}
			Y += (int)(mini_size * TILE_SIZE);
		}
	}
	else if (displayed_piece != NULL) {
		if (displayed_page == 0) {
			SDL_Rect rect;
			int Y = 0.4 * TILE_SIZE;
			int X = w * TILE_SIZE / 2;
			rect.x = X;
			rect.y = Y;

			screen.blit(CSM_font_array[(int)(TILE_SIZE / 2)].render_blended(displayed_piece->Class->name, Color::black), &rect, NULL, top_middle);
			
			Y += 3 * TILE_SIZE / 4;
			rect.x = X;
			rect.y = Y;
			screen.blit(displayed_piece->sprite, &rect, NULL, top_middle);
			
			X = edge;
			Y += 5 * TILE_SIZE / 4;

			if (displayed_piece->type != typeless) {
				char buffer[64] = "Type: ";
				int i = 6;
				for (const char* begin = type_str[(int)language][displayed_piece->type]; *begin != '\0'; begin++) {
					buffer[i++] = *begin;
				}

				buffer[i] = '\0';
				Surface text = CSM_font_array[(int)(TILE_SIZE / 3)].render_blended(buffer, Color::black);

				rect.x = X;
				rect.y = Y;
				screen.blit(text, &rect, NULL, middle_left);

			}
		}
		else if (displayed_page == 1) {
			displayed_type = displayed_piece->type;
			re_draw();
			displayed_type = typeless;
		}
		else if (displayed_page == 2) {
			if (displayed_piece->item != NULL) {
				displayed_item = &displayed_piece->item->cls;
				re_draw();
				displayed_item = NULL;
			}
		}

	}
}

bool InformationDisplay::is_active() {
	return game.show_phone;
}

Button::update_return_code InformationDisplay::update() {
	return Button::nothing;
}


ToggleInformationDisplay::ToggleInformationDisplay(double _x, double _y) : Button(_x, _y, 1, 1) {
	;
}

void ToggleInformationDisplay::draw() {
	game.draw(rotom_dex, x, y);
}

bool ToggleInformationDisplay::is_active() {
	return game.state != in_settings and game.state != in_menu;
}

void ToggleInformationDisplay::effect(int, double, double) {
	game.show_phone = not game.show_phone;
	game.phone_displayed_item = NULL;
	game.phone_displayed_piece = NULL;
	game.phone_displayed_type = typeless;
}

PhoneSwitchPage::PhoneSwitchPage(double x, double y, bool r) : Button(x-0.5, y-0.5, 1, 1) {
	is_right = r;
}

bool PhoneSwitchPage::is_active() {
	if (game.phone_displayed_piece == NULL or not game.show_phone) {
		return false;
	}
	if (is_right) {
		return (game.phone_displayed_page + (game.phone_displayed_piece->item == NULL)) <= 1;
	}
	else {
		return game.phone_displayed_page > 0;
	}
}

void PhoneSwitchPage::draw() {
	if (not is_right) {
		right_arrow.v_flip_inplace();
		game.draw(right_arrow, x, y);
		right_arrow.v_flip_inplace();
	}
	else {
		game.draw(right_arrow, x, y);
	}
}

void PhoneSwitchPage::effect(int, double, double) {
	if (is_right) {
		game.phone_displayed_page++;
	}
	else {
		game.phone_displayed_page--;
	}
}