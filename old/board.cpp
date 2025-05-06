#include <iostream>
#include <exception>

#include "SDL+.h"
#include "board.h"
#include "piece.h"

#define machin

class BoardIterator;


Square::Square() : x(0), y(0) {
	board = NULL;
	piece = NULL;
	is_accessible = false;
}

Square::Square(Board* b) : x(0), y(0){
	board = b;
	piece = NULL;
	is_accessible = false;
}

Square::Square(Board* b, int x_, int y_) : x(x_), y(y_) {
	board = b;
	piece = NULL;
	is_accessible = false;
}

Square::Square(Board* b, Piece* p, int x_, int y_) :x(x_), y(y_) {
	board = b;
	piece = p;
	is_accessible = false;
}

bool Square::is_controlled_by(piece_color color) const {

	for (Square& square : *board) {
		if (square.piece != NULL and square.piece->color == color) {
			if (square.piece->do_control(self))
				return true;
		}
	}
	return false;
}

auto Square::to_graveyard() -> void {
	if (piece != NULL) {
		switch (piece->color) {
		case white:
			// PRINT_DEBUG("Un blanc mort, c'est un drame national");
			board->white_graveyard[board->white_death] = piece;
			board->white_death++;
			break;
		case black:
			// PRINT_DEBUG("Un noir de mort, bon un de plus un de moins ça change pas grand chose");
			board->black_graveyard[board->black_death] = piece;
			board->black_death++;
			break;
		}

		Uint64 const remove_mask = ~(1ull << (x | (y << 3)));

		board->presence_mask &= remove_mask;
		board->color_mask[piece->color] &= remove_mask;
		board->type_mask[piece->type] &= remove_mask;
		board->piece_mask[piece->Class->id] &= remove_mask;

		piece->is_in_graveyard = true;
		if (piece->Class == King::cls) {
			board->nb_of_kings[piece->color]--;
			board->king_list[piece->color].remove(dynacast<King>(piece));
		}

		piece->item.to_graveyard(piece);

		piece = NULL;
	}
}


void Square::draw() const {
	
	move_data const last_move_data = board->get_last_nonduck_move();
	Color color = board->light_square_color;
	if ((x + y) % 2 == 0)
		color = board->dark_square_color;
	if (last_move_data.begin_square == this or last_move_data.target_square == this)
		color = board->last_move_color;
	if (game.selected_piece != NULL && game.selected_piece == piece)
		color = board->selected_piece_color;
#if IN_DEBUG
	if ((*game.displayed_mask) & get_mask())
		color = 0xFFFF0000;
#endif
	game.draw_rect(color, 5 + x, 9 - y, 1.0, 1.0);

	//SDL_FillSurfaceRect(board->surface, &rect, ((x + y) % 2 == 0) ? board->dark_square_color : board->light_square_color);

	
	
	if (is_accessible) 
		game.drawing_board.draw_disk((5 + x) * TILE_SIZE + (TILE_SIZE / 2), (9 - y) * TILE_SIZE + (TILE_SIZE / 2), TILE_SIZE / 4, board->possible_move_color);

	if (piece != NULL and not (game.selected_piece == piece and game.is_holding_something) and (piece->color == board->active_player or game.state != in_selection)) {
		SDL_Rect pos = { (5 + x) * TILE_SIZE, (9 - y) * TILE_SIZE, TILE_SIZE, TILE_SIZE };
		piece->draw(game.drawing_board, &pos);
	}
}

void Square::remove() {
	piece = NULL;
}

auto Square::clear() -> void {
	if (piece != NULL) {
		if (piece != board->duck)
			delete piece;
		piece = NULL;
	}
}

File::File() {
}



File::File(Board* b, int y){
	init(b, y);
}

auto File::init(Board* b, int x) -> void {
	for (int y = 0; y < 8; y++) {
		data[y].board = b;
		const_cast<short&>(data[y].x) = x;
		const_cast<short&>(data[y].y) = y;
		//((short*)&data[y])[4] = x;
		//((short*)&data[y])[5] = y;
	}
}

auto File::clear() -> void {
	for (int i = 0; i < 8; i++) {
		if (data[i].piece != NULL) {
			data[i].clear();
		}
	}
}

File::~File() {
	clear();
}


template<class T>
Piece* constructor(Board& board_, piece_color color_, Square* sq, typing type_ = typeless, PokeItem* item = NULL) {
	return new T(board_, color_, sq, type_, item);
}

Piece* null_constructor(Board& board_, piece_color color_, Square* sq, typing type_ = typeless, PokeItem* item=NULL) {
	return NULL;
}

Board::Board() {

}

void Board::init() {
	light_square_color = Color::silver;
	dark_square_color = Color::blue;
	selected_piece_color = Color::orange;
	possible_move_color = Color::aqua;
	last_move_color = Color::pale_yellow;

	with_check = true;
	with_RNG = true;
	with_typing = true;
	with_items = false;
	with_random_battle = false;

	miss_rate = 0.1;
	crit_rate = 1.0 / 16.0;

	for (unsigned int y = 0; y < 8; y++) {
		grid[y].init(this, y);
	}

	reset();
}

auto Board::clear() -> void {
	turn_number = 0;
	
	end_of_game = false;
	winner = no_color;

	for (move_data const& move : move_historic) {
		if (move.promotion) {
			delete move.attacker;
		}
	}

	move_historic.clear();
	{
		move_data d;
		last_move_data = d;
	}

	File* const end_row_ptr = grid + 8;
	for (File* row_ptr = grid; row_ptr < end_row_ptr; row_ptr++) {
		row_ptr->clear();
	}
	if (duck != NULL)
		delete duck;
	duck = NULL;

	while (white_death --> 0) {
		delete white_graveyard[white_death];
		white_graveyard[white_death] = NULL;
	}
	white_death = 0;
	
	while (black_death --> 0) {
		delete black_graveyard[black_death];
		black_graveyard[black_death] = NULL;
	}
	black_death = 0;

	nb_of_kings[white] = nb_of_kings[black] = 0;

	king_list[white].clear();
	king_list[black].clear();
}

void Board::reset() {
	clear();

	turn_number = 0;
	is_tera_avaible[white] = is_tera_avaible[black] = true;

	layout[0] = layout[7] = Rook::cls;
	layout[1] = layout[6] = Knight::cls;
	layout[2] = layout[5] = Bishop::cls;
	layout[3] = Queen::cls;
	layout[4] = King::cls;

	active_player = white;
	// last_move_begin_square = NULL;
	// last_move_end_square = NULL;
	

	in_bonus_move = false;
	first_turn = true;

	for (unsigned int i = 0; i < 8; i++) {
		File& file = grid[i];
		if (layout[i] != NULL) {
			file[0].piece = (*layout[i])(*this, white, &file[0], typeless, NO_ITEM);
		}
		file[0].is_accessible = false;

		file[1].piece = new Pawn(*this, white, &file[1], typeless, NO_ITEM);
		file[1].is_accessible = false;


		file[6].piece = new Pawn(*this, black, &file[6], typeless, NO_ITEM);
		file[6].is_accessible = false;

		if (layout[i] != NULL) {
			file[7].piece = (*layout[i])(*this, black, &file[7], typeless, NO_ITEM);
		}
		file[7].is_accessible = false;

		for (int j = 2; j < 6; j++) {
			file[j].is_accessible = false;
		}
	}

	Uint64 const mask = (1ull << 16) - 1;
	color_mask[white] = mask;
	presence_mask = mask;
	color_mask[black] = mask << 48;
	presence_mask |= (mask << 48);
	color_mask[no_color] = 0;
	__typeless_mask = 0;
	memset(type_mask, 0, 18 * 8);
	honey_holder_mask = 0;

	set_reachable_squares();
}


void Board::set_reachable_squares() {

	// auto t0 = std::chrono::high_resolution_clock::now();

	if (duck != NULL) {
		if (duck->square == NULL) {
			duck->set_reachable();
			if (active_player == no_color)
				game.select_piece(duck);
		}
	}

	// basic chess rules
	for (Square const& s : self) {
		if (s.piece != NULL) {
			s.piece->set_reachable();
		}
	}

	// items movement
	if (with_items) {
		for (Square const& s : self) {
			if (s.piece != NULL) {
				s.piece->item_reachable_mask = 0;
				if (s.piece->item != NULL) {
					s.piece->item.adjust_reachable(s.piece);
				}
			}
		}
	}

	// antichess movement
	if (with_antichess) {
		bool any = false;
		for (Square const& s : self) {
			Piece* p = s.piece;
			if (p != NULL and p->color == active_player) {
				if ((p->
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					& color_mask[not p->color]) != 0) {
					any = true;
					break;
				}
			}
		}

		if (any) {
			for (Square const& s : self) {
				Piece* p = s.piece;
				if (p != NULL and p->color == active_player) {
					p->reachable_mask &= color_mask[not p->color];
				}
			}
		}
	}

	// honey check
	Uint64 const bug_mask = type_mask[bug] & color_mask[active_player];
	Uint64 const honey_mask = honey_holder_mask & ~color_mask[active_player];
	if (bug_mask != 0 and honey_mask != 0) {
		for (Square const s : self) {
			if ((bug_mask & s.get_mask()) != 0) {
				if ((s.piece->reachable_mask & honey_mask) != 0)
					s.piece->reachable_mask &= honey_mask;
			}
		}
	}

	// auto t1 = std::chrono::high_resolution_clock::now();
	// auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
	// PRINT_DEBUG(elapsed);
}

void Board::cancel_last_move(bool restore) {
	if (move_historic.empty())
		return;

	end_of_game = false;
	
	if (promoting_piece != NULL) {
		promoting_piece = NULL;
		last_move_data.promotion = false;
		last_move_data.promotion_id = 0;
		last_move_data.interrupt_move = false;
		move_historic.push_front(last_move_data);
	}

	move_data const data = move_historic.front();
	move_historic.pop_front();
	in_bonus_move = false;
	turn_number--;
	if (not move_historic.empty()) {
		last_move_data = move_historic.front();
		if (move_historic.front().move_again) {
			in_bonus_move = true;
		}
	}
	else {
		move_data d;
		last_move_data = d;
	}

	if (data.skip_bonus_turn) {
		active_player = data.attacker->color;
		goto restore_reachable;
	}

	if (data.was_piece_first_move) {
		data.attacker->has_already_move = false;
	}

	active_player = data.attacker->color;

	if (data.attacker->item != data.attacker_item) {
		data.attacker->set_item(data.attacker_item);
	}
	if (data.defender != NULL and data.defender->item != data.defender_item) {
		data.defender->set_item(data.defender_item);
	}

	if (data.cancel)
		goto restore_reachable;

	else if (data.tera) {
		data.attacker->un_tera();
		goto restore_reachable;
	}

	if (data.attacker->is_in_graveyard) {
		data.attacker->is_in_graveyard = false;
		if (data.attacker->Class == King::cls) {
			king_list[data.attacker->color].push_front(dynacast<King>(data.attacker));
			nb_of_kings[data.attacker->color]++;
		}
		if (data.attacker->color == white)
			white_death--;
		else
			black_death--;
	}

	if (data.defender != NULL and data.defender->is_in_graveyard) {
		data.defender->is_in_graveyard = false;

		if (data.defender->Class == King::cls) {
			king_list[data.defender->color].push_front(dynacast<King>(data.defender));
			nb_of_kings[data.defender->color]++;
		}

		if (data.defender->color == white)
			white_death--;
		else
			black_death--;
	}

	if (data.promotion) {
		Piece* pawn = data.attacker;
		Square* square = data.target_square;
		Piece* promoted = square->piece;
		

		promoted->set_square(NULL);
		pawn->set_square(square);
		
		delete promoted;
	}
	else if (data.castling) {
		Square& rook_square = self[(data.attacker->x + ((data.target_square->x < data.attacker->x) ? 1 : -1))][data.attacker->y];
		Piece* const rook = rook_square.piece;
		rook->set_square(data.target_square, false);
		data.target_square->piece->has_already_move = false;
	}

	data.attacker->set_square(data.begin_square, false);
	
	if (data.defender != NULL) {
		data.defender->set_square(data.defender->square, false);
	}

	if (with_duck_chess and turn_number == 0) {
		delete duck;
		duck = NULL;
	}

	else if (data.begin_square == NULL) {
		if (data.attacker->Class == Duck::cls) {
			set_reachable_squares();
			game.select_piece(duck);
			return;
		}
	}
	
restore_reachable:
	if (not restore)
		return;
	set_reachable_squares();
	for (Square& s : self) {
		s.is_accessible = false;
	}
	game.selected_piece = NULL;
}

void Board::resize_surface() {
	surface = Surface::createRGB(8 * TILE_SIZE, 8 * TILE_SIZE);
}

move_data const Board::get_last_nonduck_move() {
	for (move_data const move : move_historic) {
		if (move.attacker->color != no_color)
			return move;
	}
	static move_data empty;
	return empty;
}

bool Board::in_stalemate(piece_color color) {
	for (Square& begin_square : self) {
		if (begin_square.piece != NULL and begin_square.piece->color == color) {
			for (Square& target_square : self) {
				if (begin_square.piece->base_can_move_to(target_square)) {
					// PRINT_DEBUG("piece at (", begin_square.x, ",", begin_square.y, ") can move to (", target_square.x, ",", target_square.y, ")");
					return false;
				}
			}
		}
	}
	return true;
}

void Board::move_piece_to(Piece* piece, Square* target) {
	piece_color player = active_player;
	Square* begin_square = piece->square;

	last_move_data = piece->move_to(*target);

	if (not last_move_data.interrupt_move)
		resume_move();

	check_for_end_of_game();

	if (not end_of_game and last_move_data.promotion) {
		if (not piece->item.prepare_promotion(piece, &avaible_promotion)) {
			avaible_promotion = 0b00011110;
		}

		if (avaible_promotion != 0) {
			promoting_piece = piece;
		}
	}
}


void Board::check_for_end_of_game() {
	if (with_check) {
		if (in_stalemate(active_player)) {
			end_of_game = true;
			winner = no_color;
			for (King* king : king_list[active_player]) {
				if (king->is_in_check()) {
					// checkmate congratulations
					winner = not active_player;
					break;
				}
			}
		}
	}
	else if (with_antichess) {
		if (white_death == 16) {
			winner = white;
			end_of_game = true;
		}
		else if (black_death == 16) {
			winner = black;
			end_of_game = true;
		}
	}
	else {
		if (nb_of_kings[white] == 0 and nb_of_kings[black] == 0) {
			winner = no_color;
			end_of_game = true;
		}
		else if (nb_of_kings[white] == 0) {
			winner = black;
			end_of_game = true;
		}
		else if (nb_of_kings[black] == 0) {
			winner = white;
			end_of_game = true;
		}
	}
}

void Board::promote(Uint8 id) {
#if ENABLE_SAFETY_CHECKS
	if (promoting_piece == NULL or id >= 7 or ((1 << id) & avaible_promotion) == 0) {
		PRINT_DEBUG("Unauthorized promotion");
		throw;
	}
#endif
	Piece* promoted = piece_class_tab[id]->base_promotion_constructor(promoting_piece);
	Square* square = promoting_piece->square;
	last_move_data.promotion_id = id;

	promoting_piece->set_square(NULL);
	promoted->set_square(square);
	promoted->item.promote(promoted);

	promoting_piece = NULL;

	resume_move();
}

void Board::resume_move() {
	move_data& data = last_move_data;
	if (data.defender == NULL or not data.defender->item.is_safety_google()) {
		data.attacker->item.update_status(data);
	}

	if (data.defender != NULL and not data.attacker->item.is_protective_pads()) {
		data.defender->item.update_status(data);
	}

	
	if (not data.move_again) {
		in_bonus_move = false;
		change_turn();
	}

	// board.check_for_end_of_game();

	turn_number++;
	move_historic.push_front(data);

	set_reachable_squares();

	
}

void Board::change_turn() {
	if (with_duck_chess) {
		if (active_player == no_color) {
			first_turn = false;
			active_player = not get_last_nonduck_move().attacker->color;
			duck->reachable_mask = 0;
		}
		else {
			if (duck == NULL) {
				duck = new Duck(self, NULL);
			}
			active_player = no_color;
		}
	}
	else {
		active_player = not active_player;
	}
}

void Board::skip_bonus_turn() {
#if ENABLE_SAFETY_CHECKS
	if (not in_bonus_move) {
		PRINT_DEBUG("Invalid attempt to skip a turn");
		throw;
	}
#endif
	in_bonus_move = false;

	move_data data;
	data.attacker = game.board.last_move_data.attacker;
	data.begin_square = data.attacker->square;
	data.target_square = data.attacker->square;
	data.skip_bonus_turn = true;
	move_historic.push_front(data);
	set_reachable_squares();

	change_turn();
	check_for_end_of_game();
}

void Board::tera_piece(Piece* piece) {
#if ENABLE_SAFETY_CHECKS
	if (piece->item.get_id() != item_ID::tera_shard) {
		PRINT_DEBUG("Invalid attempt to tera a piece");
		throw;
	}
#endif
	move_data data;
	typing new_type = (typing)piece->item.get_slot1();
	data.set_type_matchup_data(piece, NULL, piece->square);

	piece->tera(new_type);
	piece->set_item(NO_ITEM);

	data.tera = true;
	data.attacker = piece;
	data.begin_square = data.target_square = game.selected_piece->square;
	
	change_turn();

	move_historic.push_front(data);
	set_reachable_squares();
}

Board Board::copy() {
	Board new_board;
	new_board.init();
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {
			new_board[x][y].piece = self[x][y].piece->copy(new_board);
		}
	}

	for (int i = 0; i < 16; i++) {
		if (white_graveyard[i] != NULL) {
			new_board.white_graveyard[i] = white_graveyard[i]->copy(new_board);
		}
	}
}

Board::~Board() {
	clear();
}

BoardIterator::BoardIterator(Board& board, int index_) : iterated_board(board) {
	index = index_;
}

BoardIterator::BoardIterator(Board& board, int x, int y) : iterated_board(board) {
	index = (x & 0b111) + ((y & 0b111) << 3); // index = x + 8y
}

BoardIterator::BoardIterator(const BoardIterator& other) : iterated_board(other.iterated_board) {
	index = other.index;
}

auto BoardIterator::operator*() -> reference {
	return iterated_board[index & 0b111][index >> 3];
}

auto BoardIterator::operator->() -> pointer {
	return  &(iterated_board[index & 0b111][index >> 3]);
}

#define useless0 1
#define useless1 useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0
#define useless2 useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1
#define useless3 useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2



auto BoardIterator::operator++() -> BoardIterator& {
	index++;
	if (index > 64)
		index = 64;
	return self;
}

auto BoardIterator::operator++(int) -> BoardIterator {
	auto prev(self);
	++self;
	return prev;
}

auto BoardIterator::operator--() -> BoardIterator& {
	index--;
	if (index < 0)
		index = 0;

	return self;
}
auto BoardIterator::operator--(int) -> BoardIterator {
	auto prev(self);
	--self;
	return prev;
}

auto inline BoardIterator::operator+=(difference_type step) -> BoardIterator& {
	index += step;
	if (index > 64)
		index = 64;

	else if (index < 0)
		index = 0;

	return self;
}

auto inline BoardIterator::operator+(difference_type step) -> BoardIterator {
	return BoardIterator(self) += step;
}

auto inline BoardIterator::operator-=(difference_type step) -> BoardIterator& {
	return self += -step;
}

auto inline BoardIterator::operator-(difference_type step) -> BoardIterator {
	return BoardIterator(self) -= step;
}



auto operator==(const BoardIterator& lhs, const BoardIterator& rhs) -> bool {
	return lhs.index == rhs.index;
}

auto operator!=(const BoardIterator& lhs, const BoardIterator& rhs) -> bool {
	return lhs.index != rhs.index;
}

auto Board::begin() -> BoardIterator {
	return BoardIterator(self, 0);
}

auto Board::end() -> BoardIterator {
	return BoardIterator(self, 64);
}