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
			PRINT_DEBUG("Un blanc mort, c'est un drame national");
			board->white_graveyard[board->white_death] = piece;
			board->white_death++;
			break;
		case black:
			PRINT_DEBUG("Un noir de mort, bon un de plus un de moins ça change pas grand chose");
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
	
	
	Color color = board->light_square_color;
	if ((x + y) % 2 == 0)
		color = board->dark_square_color;
	if (board->last_move_begin_square == this or board->last_move_end_square == this)
		color = board->last_move_color;
	if (game.selected_piece != NULL && game.selected_piece == piece)
		color = board->selected_piece_color;

	game.draw_rect((x + y) % 2 == 1 ? board->light_square_color : board->dark_square_color, 5 + x, 9 - y, 1.0, 1.0);

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
	if (piece != NULL) {
		delete piece;
		piece = NULL;
	}
}

File::File() {
	board = NULL;
}



File::File(Board* b, int y){
	init(b, y);
}

auto File::init(Board* b, int x) -> void {
	board = b;
	for (int y = 0; y < 8; y++) {
		data[y].board = b;
		((short*)&data[y])[4] = x;
		((short*)&data[y])[5] = y;
	}
}

auto File::operator[](int i) -> Square& {
	if (0 <= i && i < 8)
		return data[i];
	else {
		PRINT_DEBUG("i: " << i);
		throw std::exception();
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
	File* row_ptr = grid;
	File* end_row_ptr = row_ptr + 8;
	for (; row_ptr < end_row_ptr; row_ptr++) {
		row_ptr->clear();
	}


	while (white_death --> 0) {
		delete white_graveyard[white_death];
		white_graveyard[white_death] = NULL;
	}
	white_death = 0;
	
	while (black_death --> 0) {
		delete black_graveyard[black_death];
		black_graveyard[black_death] = NULL;
	}

	nb_of_kings[white] = nb_of_kings[black] = 0;

	black_death = 0;

}

void Board::reset() {
	clear();

	turn_number = 0;

	layout[0] = layout[7] = Rook::cls;
	layout[1] = layout[6] = Knight::cls;
	layout[2] = layout[5] = Bishop::cls;
	layout[3] = Queen::cls;
	layout[4] = King::cls;

	active_player = white;
	last_move_begin_square = NULL;
	last_move_end_square = NULL;

	in_bonus_move = false;

	for (unsigned int i = 0; i < 8; i++) {
		File& file = grid[i];
		if (layout[i] != NULL)
			file[0].piece = (*layout[i])(*this, white, &file[0], typeless, NULL);
		file[0].is_accessible = false;

		file[1].piece = new Pawn(*this, white, &file[1], typeless, NULL);
		file[1].is_accessible = false;


		file[6].piece = new Pawn(*this, black, &file[6], typeless, NULL);
		file[6].is_accessible = false;

		if (layout[i] != NULL)
			file[7].piece = (*layout[i])(*this, black, &file[7], typeless, NULL);
		file[7].is_accessible = false;

		for (int j = 2; j < 6; j++) {
			file[j].is_accessible = false;
		}
	}
	
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

auto Board::operator[](int i) -> File& {
	if (0 <= i && i < 8)
		return grid[i];
	else
		throw std::exception();
}



bool Board::in_stalemate(piece_color color) {
	for (Square& begin_square : self) {
		if (begin_square.piece != NULL and begin_square.piece->color == color) {
			for (Square& target_square : self) {
				if (begin_square.piece->base_can_move_to(target_square)) {
					PRINT_DEBUG("piece at (" << begin_square.x << ", " << begin_square.y << ") can move to (" << target_square.x << ", " << target_square.y << ")");
					return false;
				}
			}
		}
	}
	return true;
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