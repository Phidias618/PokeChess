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

auto Square::is_controlled_by(piece_color color) -> bool {
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
		piece->is_in_graveyard = true;
		if (piece->Class == King::cls) {
			board->nb_of_kings[piece->color]--;
			board->king_list[piece->color].remove(reinterpret_cast<King*>(piece));
		}
		piece = NULL;
	}
}


auto Square::draw() -> void {
	
	move_data const last_move_data = board->get_last_nonduck_move();
	Color color = board->light_square_color;
	if ((x + y) % 2 == 0)
		color = board->dark_square_color;
	if (last_move_data.begin_square == this or last_move_data.target_square == this)
		color = board->last_move_color;
	if (game.selected_piece != NULL && game.selected_piece == piece)
		color = board->selected_piece_color;

	game.draw_rect(color, 5 + x, 9 - y, 1.0, 1.0);

	//SDL_FillSurfaceRect(board->surface, &rect, ((x + y) % 2 == 0) ? board->dark_square_color : board->light_square_color);

	
	
	if (is_accessible) 
		game.drawing_board.draw_disk((5 + x) * TILE_SIZE + (TILE_SIZE / 2), (9 - y) * TILE_SIZE + (TILE_SIZE / 2), TILE_SIZE / 4, board->possible_move_color);

	if (piece != NULL and not (game.selected_piece == piece and game.is_holding_something) and (piece->color == board->active_player or game.state != in_selection)) {
		game.draw(piece->sprite, 5 + x, 9 - y);
	}
}

void Square::remove() {
	piece = NULL;
}

auto Square::clear() -> void {
	if (piece != NULL and piece != board->duck) {
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

	

	miss_rate = 0.1;
	crit_rate = 1.0 / 16.0;

	for (unsigned int y = 0; y < 8; y++) {
		grid[y].init(this, y);
	}


	reset();
}

auto Board::clear() -> void {
	turn_number = 0;
	
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
	white_tera = black_tera = true;

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
			file[0].piece = (*layout[i])(*this, white, &file[0], typeless, NULL);
		}
		file[0].is_accessible = false;

		file[1].piece = new Pawn(*this, white, &file[1], typeless, NULL);
		file[1].is_accessible = false;


		file[6].piece = new Pawn(*this, black, &file[6], typeless, NULL);
		file[6].is_accessible = false;

		if (layout[i] != NULL) {
			file[7].piece = (*layout[i])(*this, black, &file[7], typeless, NULL);
		}
		file[7].is_accessible = false;

		for (int j = 2; j < 6; j++) {
			file[j].is_accessible = false;
		}
	}

	for (Square const& s : self) {
		if (s.piece != NULL) {
			s.piece->set_reachable();
		}
	}
}


void Board::cancel_last_move() {
	if (move_historic.empty() or game.state == in_promotion)
		return;
	move_data const data = move_historic.front();
	move_historic.pop_front();
	in_bonus_move = false;

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

	if (not data.move_again) {
		active_player = not active_player;
	}

	if (data.cancel)
		goto restore_reachable;

	if (data.attacker->is_in_graveyard) {
		data.attacker->is_in_graveyard = false;
		if (data.attacker->color == white)
			white_death--;
		else
			black_death--;
	}

	if (data.defender != NULL and data.defender->is_in_graveyard) {
		data.defender->is_in_graveyard = false;
		if (data.defender->color == white)
			white_death--;
		else
			black_death--;
	}

	if (get_item_hash(data.attacker->item) != data.attacker_item_hash) {
		data.attacker->set_item(create_item_from_hash(data.attacker, data.attacker_item_hash));
	}
	if (data.defender != NULL and get_item_hash(data.defender->item) != data.defenser_item_hash) {
		data.defender->set_item(create_item_from_hash(data.defender, data.defenser_item_hash));
	}

	if (data.en_passant) {
		data.defender->square->piece = data.defender;
		
		data.attacker->square->piece = NULL;
		data.begin_square->piece = data.attacker;
		data.attacker->square = data.begin_square;
	}
	else if (data.tera) {
		data.attacker->un_tera();
	}
	else if (data.promotion) {
		PRINT_DEBUG(type_str_cap[0][data.attacker->base_type]);
		Piece* pawn = data.attacker;
		Piece* promoted = pawn->square->piece;

		pawn->square = data.begin_square;

		data.begin_square->piece = pawn;
		data.target_square->piece = data.defender;
		promoted->item = NULL;
		delete promoted;

		PRINT_DEBUG(type_str_cap[0][data.attacker->base_type]);

	}
	else if (data.castling) {
		
		if (data.target_square->x < data.attacker->x) {
			// _ _ K R _ _ _ _ -> R _ _ _ K _ _ _

			data.target_square->piece = self[data.attacker->x + 1][data.attacker->y].piece;
			self[data.attacker->x + 1][data.attacker->y].piece = NULL;

			data.target_square->piece->square = data.target_square;

			data.begin_square->piece = data.attacker;
			data.attacker->square->piece = NULL;
			data.attacker->square = data.begin_square;
			
		}
		else {
			// _ _ _ _ _ R K _ -> _ _ _ _ K _ _ R


			data.target_square->piece = self[data.attacker->x - 1][data.attacker->y].piece;
			data.target_square->piece->square = data.target_square;
			self[data.attacker->x - 1][data.attacker->y].piece = NULL;

			data.begin_square->piece = data.attacker;
			data.attacker->square->piece = NULL;
			data.attacker->square = data.begin_square;
		}

		data.target_square->piece->has_already_move = false;
	}
	else {
		
		if (data.defender != NULL) {
			data.defender->square->piece = data.defender;

			data.begin_square->piece = data.attacker;
			data.attacker->square = data.begin_square;
		}
		else {
			data.attacker->square->piece = NULL;

			data.begin_square->piece = data.attacker;
			data.attacker->square = data.begin_square;
		}
	}

restore_reachable:
	set_reachable();
}

void Board::resize_surface() {
	surface = Surface::createRGB(8 * TILE_SIZE, 8 * TILE_SIZE);
	for (Square& square : self) {
		if (square.piece != NULL)
			square.piece->resize_sprite();
	}

	for (int i = 0; i < white_death; i++)
		white_graveyard[i]->resize_sprite();

	for (int i = 0; i < black_death; i++)
		black_graveyard[i]->resize_sprite();

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

void Board::move_piece_to(Piece* piece, Square& target) {

}

void Board::set_reachable() {
	for (Square const& s : self) {
		if (s.piece != NULL) {
			s.piece->set_reachable();
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
	return &iterated_board[index & 0b111][index >> 3];
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