/*This files contains all the code related to chess piece*/

#include "SDL+.h"
#include "Debugger.h"

#include "assets.h"

#include "poketyping.h"
#include "piece.h"

#define set_cls(Class, id, name) PieceClass  Class::__cls = PieceClass([](Board& board, piece_color color, Square* sq, typing type, PokeItem* item) -> Piece* { return new Class(board, color, sq, type, item);  }, id, name); PieceClass* const Class::cls = &Class::__cls;

set_cls(King, 0, "King");
set_cls(Queen, 1, "Queen");
set_cls(Bishop, 2, "Bishop");
set_cls(Knight, 3, "Knight");
set_cls(Rook, 4, "Rook");
set_cls(Pawn, 5, "Pawn");


#undef __set_cls
#undef set_cls


#include "game.h"
#include "board.h"
#include "item.h"

constexpr int ABS_INT(int x) {
	return (x >= 0) ? x : -x;
}


void move_data::set_type_matchup_data(Piece* attacker_, Piece* defender_, Square* target_square_) {
	attacker = attacker_;
	defender = defender_;
	begin_square = attacker->square;
	target_square = target_square_;

	miss_rate = attacker->board.miss_rate;
	crit_rate = attacker->board.crit_rate;

	attacker_item_slot = attacker->item;
	if (defender != NULL)
		defenser_item_slot = defender->item;

	if (not game.with_typing or defender == NULL)
		return;

	if (attacker->type != typeless and defender->type != typeless)
		matchup = typechart[attacker->type][defender->type];

	Board& board = attacker->board;


	if (attacker->item != NULL and not IS_SAFETY_GOOGLES(defenser_item_slot)) {
		if (defender->item != NULL and not IS_PROTECTIVE_PADS(attacker_item_slot)) {
			if (attacker->item->priority >= defender->item->priority) {
				attacker_item_slot->attack_modifier(self);
				defenser_item_slot->defense_modifier(self);

				attacker_item_slot->crit_modifier(self);
				
				attacker_item_slot->accuracy_modifier(self);
				defenser_item_slot->evasion_modifier(self);
			}
			else {
				defenser_item_slot->defense_modifier(self);
				defenser_item_slot->attack_modifier(self);
				
				attacker_item_slot->crit_modifier(self);

				defenser_item_slot->evasion_modifier(self);
				attacker_item_slot->accuracy_modifier(self);
			}
		}
		else {
			attacker_item_slot->attack_modifier(self);
			attacker_item_slot->crit_modifier(self);
			attacker_item_slot->accuracy_modifier(self);
		}
	}
	else {
		if (defender->item != NULL and not IS_PROTECTIVE_PADS(attacker_item_slot)) {
			defenser_item_slot->defense_modifier(self);
			defenser_item_slot->evasion_modifier(self);
		}
	}

	if (matchup == immune) {
		is_immune = true;
		cancel = true;
		return;
	}

	
	if (game.with_RNG and (double)game.RNG() / (double)game.RNG.max() < crit_rate) {
		do_crit = true;
		move_again = true;
	}
	else if (game.with_RNG and (double)game.RNG() / (double)game.RNG.max() < miss_rate) {
		do_miss = true;
		cancel = true;
	}
	else {
		switch (matchup) {
		case not_very_effective:
			is_not_very_effective = true;
			suicide = true;
			break;
		case super_effective:
			is_super_effective = true;
			if (not do_miss)
				move_again = true;
			break;
		}
	}

}

PieceClass::PieceClass() : constructor(NULL), id(-2), name("") {
	;
}

PieceClass::PieceClass(std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> ctor, int id_, const char* const _name) : id(id_), constructor(ctor), name(_name) {
	;
}

PieceClass& PieceClass::operator=(const PieceClass& other) {
	constructor = other.constructor;
	*(int*)&id = other.id;
	return self;
}


bool PieceClass::operator==(void* other) const {
	return constructor == NULL;
}

bool operator==(void* other, PieceClass const Class) {
	return Class.operator==(NULL);
}

Piece* PieceClass::operator()(Board& board, piece_color color, Square* sq, typing type, PokeItem* item) {
	if (self != NULL)
		return constructor(board, color, sq, type, item);
	return NULL;
}




Piece::Piece(Board& board_, PieceClass* const _Class, piece_color color_, Square* sq, typing type_, PokeItem* item) :
	board(board_),
	Class(_Class),
	color(color_),
	sprite(Surface::createRGBA(TILE_SIZE, TILE_SIZE)),
	square(sq),
	type(type_),
	item(item)
{	
	has_already_move = false;
	is_in_graveyard = false;

	update_sprite();
}

void Piece::resize_sprite() {
	sprite = Surface::createRGBA(TILE_SIZE, TILE_SIZE);

	update_sprite();
}

void Piece::update_sprite() {
	sprite.fill(0x00000000);

	SDL_Rect dest(0, 0, TILE_SIZE, TILE_SIZE);
	SDL_Rect area(TILE_SIZE * Class->id, TILE_SIZE * color, TILE_SIZE, TILE_SIZE);

	sprite.blit(sprite_sheet, &dest, &area);
	dest = { 5 * TILE_SIZE / 8, 5 * TILE_SIZE / 8, 0, 0 };
	if (type != typeless)
		sprite.blit(typing_icon[type].scale_to(TILE_SIZE / 3, TILE_SIZE / 3, true), &dest, NULL);

	if (item != NULL) {
		dest = { 0, TILE_SIZE - ITEM_MINI_SIZE, 0, 0 };
		(item->cls).draw(sprite, &dest, mini);
	}
}

void Piece::set_type(typing new_type) {
	type = new_type;
	update_sprite();
}

void Piece::set_item(PokeItem* new_item) {
	if (item != NULL)
		delete item;
	item = new_item;
	if (item != NULL)
		item->holder = this;

	update_sprite();
}

bool Piece::base_can_move_to(Square& target_square) {
	if (board.in_bonus_move and board.last_move_data.attacker != this)
		return false; // when you are allowed to play twice or more, you can only move the same piece

	auto target_piece = target_square.piece;
	if (target_piece == NULL or (target_piece->color != this->color)) {
		// need to check wether the move will put your own king in check
		if (game.with_check) {
			// simulate the move, the modification to the board will need to be cancel later
			Square* temp = square;
			square->piece = NULL;
			target_square.piece = this;
			square = &target_square;

			bool in_check = false;
			for (King* king : board.king_list[color]) {
				if (king->is_in_check()) {
					in_check = true;
					break;
				}
			}
			// cancel the simulated move
			square = temp;
			square->piece = this;
			target_square.piece = target_piece;
			

			return not in_check;
		}
		else {
			return true;
		}
	}
	else
		return false;
}

bool Piece::base_do_control(Square& target_square) { return false; }

move_data Piece::move_to(Square& square) {
	move_data data;

	if (item == NULL or can_move_to(square, ignore_item)) {
		data = base_move_to(square);
	}
	else {
		data = item->move_to(square);
	}

	if (data.attacker_item_slot != NULL and not IS_SAFETY_GOOGLES(data.defenser_item_slot)) {
		data.attacker_item_slot->after_move_effects(data);
	}

	if (data.defenser_item_slot != NULL and not IS_PROTECTIVE_PADS(data.attacker_item_slot)) {
		data.defenser_item_slot->revenge(data);
	}

	return data;
}


move_data Piece::base_move_to(Square& target_square) {
	move_data data;
	data.set_type_matchup_data(this, target_square.piece, &target_square);
	data.was_in_check = false;
	for (King* king : board.king_list[color]) {
		if (king->is_in_check()) {
			data.was_in_check = true;
			break;
		}
	}

	if (data.cancel)
		;
	else {
		has_already_move = true;

		target_square.to_graveyard(); // kills the opposing piece

		if (data.suicide) {
			square->to_graveyard(); // kills itself because of not_very_effective move
			square = &target_square; // change its current square, that's not useless as this gives the game information about the square you would have landed
		}
		else {
			square->piece = NULL; // removes itself from its previous location
			target_square.piece = this; // moves itself to its new location
			square = &target_square; // change its current square
		}
	}

	bool is_in_check = false;
	for (King* king : board.king_list[color]) {
		if (king->is_in_check()) {
			is_in_check = true;
			break;
		}
	}
	data.escaped_check = (data.was_in_check and not is_in_check);

	return data;
}

bool Piece::can_move_to(Square& target, Uint64 flags) {
	Piece* adv = target.piece;

	static bool check_for_is_move_disallowed = true, check_for_is_move_allowed = true;
	static bool check_for_antichess = true;


	if (board.in_bonus_move and board.last_move_data.attacker != this)
		return false; // when you are allowed to play twice or more, you can only move the same piece

	if ((flags & ignore_honey) == 0 and type == bug and not IS_SAFETY_GOOGLES(item) and not HOLDS_HONEY(adv)) {
		for (Square& square : board) {
			if (can_move_to(square, flags | ignore_honey)) {
				if (HOLDS_HONEY(square.piece)) {
					// congratulation, you are a bug type that can reach opposing honey
					return false;
				}
			}
		}
	}
	bool base = base_can_move_to(target);

	if (game.with_antichess and check_for_antichess and target.piece == NULL and (Class != Pawn::cls or not dynamic_cast<Pawn*>(this)->can_en_passant(target))) {
		check_for_antichess = false;

		for (Square& s1 : board) {
			Piece* piece = s1.piece;
			if (piece != NULL and piece->color == color) {
				for (Square& s2 : board) {
					if (s2.piece != NULL and piece->can_move_to(s2)) {
						check_for_antichess = true;
						return false;
					}
				}
			}
		}

		check_for_antichess = true;
	}


	if (item != NULL and (flags & ignore_item) == 0) {
		if (base) {
			if ((flags & ignore_movement_restriction) == 0 and check_for_is_move_allowed) {
				check_for_is_move_disallowed = false;
				base = not item->is_move_disallowed(target);
				check_for_is_move_disallowed = true;
			}
		}
		else {
			if ((flags & ignore_movement_bonus) == 0 and check_for_is_move_allowed) {
				check_for_is_move_allowed = false;
				base = item->is_move_allowed(target);
				check_for_is_move_allowed = true;
			}
		}
	}

	return base;
}

bool Piece::do_control(Square& target) {
	bool base = base_do_control(target);
	return base;
}


Piece::~Piece() {
}
#define useless0 1
#define useless1 useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0
#define useless2 useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1
#define useless3 useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2

King::King(Board& board_, piece_color color, Square* sq, typing type_, PokeItem* item) : Piece(board_, King::cls, color, sq, type_, item) {
	//board.kings[color] = this;
	board.nb_of_kings[color]++;
	board.king_list[color].push_front(this);
}

King::~King() {
	if (not is_in_graveyard) {
		board.nb_of_kings[color]--;
		board.king_list[color].remove(this);
	}
}

auto King::base_can_move_to(Square& target) -> bool {
	constexpr bool base_rule = true;
	bool base_movement = (Piece::base_can_move_to(target) and ABS_INT(x - target.x) <= 1 and ABS_INT(y - target.y) <= 1) or can_castle(target, base_rule);
	if (base_rule)
		return base_movement;
	else {
		if (base_movement) {
			return item == NULL or not item->is_move_disallowed(target);
		}
		else {
			return item != NULL and item->is_move_allowed(target);
		}
	}
}

auto King::base_move_to(Square& target_square) -> move_data {
	if (can_castle(target_square)) {
		move_data data;
		data.set_type_matchup_data(this, NULL, &target_square);
		data.was_in_check = is_in_check();

		if (not data.cancel) {
			castle(target_square);
			data.castling = true;
		}
		data.escaped_check = data.was_in_check and not is_in_check();

		if (data.suicide)
			square->to_graveyard();

		return data;
	}
	else
		return Piece::base_move_to(target_square);
}

auto King::base_do_control(Square& target_square) -> bool {
	auto dx = target_square.x - x;
	auto dy = target_square.y - y;
	return (dx != 0 or dy != 0) and -1 <= dx and dx <= 1 and -1 <= dy and dy <= 1;
}


Pawn::Pawn(Board& board_, piece_color color, Square* sq, typing type_, PokeItem* item) : Piece(board_, Pawn::cls, color, sq, type_, item) {
}

inline bool Pawn::base_do_control(Square& square) {
	return (square.y - y) == ((color == white) ? 1 : -1) and ABS_INT(square.x - x) == 1;
}

bool Pawn::base_can_move_to(Square& target_square) {
	if (can_en_passant(target_square))
		return true;

	if (not Piece::base_can_move_to(target_square))
		return false;

	Piece* target = target_square.piece;

	int dx = target_square.x - x;
	int dy = target_square.y - y;
	int direction = ((color == white) ? 1 : -1);

	if (target == NULL) {
		if (dx != 0)
			return false; // can only move forward
		if (dy == direction)
			return true; // just a simple step
		else if (dy == 2*direction)
			return (not has_already_move) and board[x][y + direction].piece == NULL; // check for a double step
		return false; // nor a simple step nor a double step
	}
	else {
		if (target->color == color)
			return false; // cannot move on a friendly piece
		else
			return base_do_control(target_square); // check if you try to move diagonaly
	}
}

bool Pawn::can_double_step(Square& target, bool base_rule) {
	int direction = ((color == white) ? 1 : -1);
	int dy = target.y - y;
	int dx = target.x - x;
	PRINT_VAR(dx);
	PRINT_VAR(dy);
	PRINT_VAR(has_already_move);
	return 
		(dx == 0) and 
		(dy == 2 * direction) and 
		(not has_already_move) and 
		board[x][y + direction].piece == NULL and 
		board[x][target.y].piece == NULL
	;
}

auto Pawn::can_en_passant(Square& target_square, bool base_rule) -> bool {
	if (board.last_move_data.attacker == NULL or board.last_move_data.attacker->color == color or board.last_move_data.attacker->Class != Pawn::cls)
		return false; // you can only en passant if the last piece move is an enemy pawn
	
	if (not base_do_control(target_square)) 
		return false; // you can only en passant in diagonal
	Piece* target_piece = board[target_square.x][y].piece;
	if (target_piece != board.last_move_data.attacker)
		return false; // the last piece moved needs to be your target
	if (ABS_INT(board.last_move_end_square->y - board.last_move_begin_square->y) != 2)
		return false; // the opposing pawn must have perform a double step
	
	// must check wether the move wille put our king in danger

	if (game.with_check) {
		// simulate the move, the modification to the board will need to be cancel later
		Square* begin_pos = square;
		square->piece = NULL;
		target_square.piece = this;
		square = &target_square;
		board.last_move_end_square->piece = NULL;

		bool in_check = false;
		for (King* king : board.king_list[board.active_player]) {
			if (king->is_in_check()) {
				in_check = true;
				break;
			}
		}

		// cancel the simulated move
		square = begin_pos;
		square->piece = this;
		target_square.piece = NULL;
		board.last_move_end_square->piece = target_piece;

		return not in_check;
	}
	else {
		return true;
	}
		
}

auto Pawn::base_move_to(Square& target_square) -> move_data {
	move_data data;

	data.was_in_check = false;
	for (King* king : board.king_list[board.active_player]) {
		if (king->is_in_check()) {
			data.was_in_check = true;
			break;
		}
	}

	if (can_en_passant(target_square)) {
		data.en_passant = true;
		data.set_type_matchup_data(this, board.last_move_data.attacker, &target_square);
		if (not data.cancel) {
			board.last_move_end_square->to_graveyard();

			if (data.suicide) {
				square->to_graveyard();
			}
			else {
				square->remove();
				target_square.piece = this;
				square = &target_square;
			}
		}
		
	}
	else {
		data = Piece::base_move_to(target_square);
	}

	if ((color == white and y == 7) or (color == black and y == 0)) {
		if (not data.suicide)
			data.promotion = data.interrupt_move = true;
	}

	bool is_in_check = false;
	for (King* king : board.king_list[board.active_player]) {
		if (king->is_in_check()) {
			is_in_check = true;
			break;
		}
	}

	data.escaped_check = data.was_in_check and not is_in_check;
	return data;
}


Knight::Knight(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem* item) : Piece(board_, Knight::cls, color_, sq, type_, item) {
}

auto Knight::base_do_control(Square& target_square) -> bool {
	int dx = target_square.x - x;
	int dy = target_square.y - y;
	return dx * dx + dy * dy == 5;
}

auto Knight::base_can_move_to(Square& target_square) -> bool {
	return Piece::base_can_move_to(target_square) and base_do_control(target_square);
}

Rook::Rook(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem* item) : Piece(board_, Rook::cls, color_, sq, type_, item) {
}

auto Rook::base_can_move_to(Square& target_square) -> bool {
	return Piece::base_can_move_to(target_square) and base_do_control(target_square);
}

auto Rook::base_do_control(Square& target_square) -> bool {
	int dx = target_square.x - x;
	int dy = target_square.y - y;

	if (dx != 0) {
		if (dy != 0)
			// not a cross movement
			return false;
		else {
			//horizontal movement
			int x_step = (dx > 0) ? 1: -1;
			for (int x_temp = x + x_step; x_temp != target_square.x; x_temp += x_step)
				if (board[x_temp][y].piece != NULL)
					return false;
			return true;
		}
	}
	else {
		if (dy == 0) // do not control its own square
			return false;
		// vertical movement
		int y_step = (dy > 0) ? 1 : -1;
		for (int y_temp = y + y_step; y_temp != target_square.y; y_temp += y_step)
			if (board[x][y_temp].piece != NULL)
				return false;
		return true;
	}
}

Bishop::Bishop(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem* item) : Piece(board_, Bishop::cls, color_, sq, type_, item) {
}


auto Bishop::base_do_control(Square& target_square) -> bool{
	int dx = target_square.x - x;
	int dy = target_square.y - y;

	if (dx == 0 || ABS_INT(dx) != ABS_INT(dy))
		// the movement does not follow an X patern
		return false;
	else {
		signed char x_step = (dx > 0) ? (1) : (-1);
		signed char y_step = (dy > 0) ? (1) : (-1);

		auto x_temp = x + x_step;
		auto y_temp = y + y_step;

		for (; x_temp != target_square.x; x_temp += x_step, y_temp += y_step) {
			if (board[x_temp][y_temp].piece != NULL) {
				return false;
			}

		}
		return true;

	}
}	



auto Bishop::base_can_move_to(Square& target_square) -> bool {
	return Piece::base_can_move_to(target_square) and this->base_do_control(target_square);
}

Queen::Queen(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem* item) :
	Bishop(board_, color_, sq, type_, item), Rook(board_, color_, sq, type_, item), Piece(board_, Queen::cls, color_, sq, type_, item)
{
}

auto inline Queen::base_do_control(Square& target_square) -> bool {
	return Rook::base_do_control(target_square) or Bishop::base_do_control(target_square);
}

auto Queen::base_can_move_to(Square& target_square) -> bool {
	return Piece::base_can_move_to(target_square) and base_do_control(target_square);
}


auto King::can_castle(Square& target_square, bool base_rule) -> bool {
	int dx = target_square.x - x;

	if (game.with_antichess)
		return false;

	if (has_already_move or target_square.y != y) // cannot castle if the king already moved, or if it's not on the same row
		return false;

	if (game.with_check and board[x][y].is_controlled_by(not color)) // cannot castle out of check
		return false;

	int x_step = (dx > 0) ? 1 : -1;
	int x_temp = x + x_step;

	if (target_square.piece == NULL) { // check for a regular castle, when you click on the space the king will go
		if (ABS_INT(dx) != 2)
			return false;
		
		while (0 <= x_temp and x_temp < 8) {
			Square& intermediate_square = board[x_temp][y];
			if (intermediate_square.piece != NULL) {
				Piece* p = intermediate_square.piece;
				if (p->Class == Rook::cls and p->color == color and not p->has_already_move) // a rook is avaible to castle with
					return true;
				return false; // cannot castle through piece
			}

			if (game.with_check and intermediate_square.is_controlled_by(not color)) // cannot castle through check
				return false;

			x_temp += x_step;
		}
		return false; // the while loop ended up looking outside of the board
	}
	else if (target_square.piece->Class == Rook::cls and target_square.piece->color == color and not target_square.piece->has_already_move) { // check for a castle when you click directly on a rook
		Piece* rook = target_square.piece;
		for (; x_temp != rook->x; x_temp += x_step) {
			Square& intermediate_square = board[x_temp][y];
			if (intermediate_square.piece != NULL or (game.with_check and intermediate_square.is_controlled_by(not color))) // cannot castle through a piece/through check
				return false;
		}
		return true; // there isn't any obstacle and the castling path is safe
	}
	else
		return false; // cannot castle onto another piece
}

auto King::castle(Square& target_square) -> void { // assume King::can_castle(target_square) is true
	Piece* rook;
	int dx = target_square.x - x;
	int x_step = (dx > 0) ? 1 : -1;
	if (target_square.piece == NULL) {
		// first the program find the rook the king is castling with
		
		int x_temp = x + x_step;

		while (board[x_temp][y].piece == NULL)
			x_temp += x_step;
		rook = board[x_temp][y].piece;
	}
	else {
		rook = target_square.piece; // this assumes the piece is a rook as King::can_castle(target_square) is suppose as true
	}

	square->remove(); // removes the king from its previous position
	board[x + 2 * x_step][y].piece = this; // moves the king to the corresponding square
	square = &board[x + 2 * x_step][y];// updates the king position

	rook->square->remove(); // removes the rook from it's previous position
	board[x - x_step][y].piece = rook; // moves the rook to the corresponding square
	rook->square = &board[x - x_step][y]; // updates the position of the rook
	
	has_already_move = true;
	rook->has_already_move = true;
}

bool King::is_in_check(bool base_rule) {
	return square->is_controlled_by(not color);
}