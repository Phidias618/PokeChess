#include <iostream>
#include <exception>

#include "bit_manipulation.h"

#include "SDL+.h"
#include "board.h"
#include "piece2.h"

#define machin


constexpr int ABS_INT(int x) {
	return (x >= 0) ? x : -x;
}

class BoardIterator;

Square::Square() {
	piece = NULL;
}


Square::Square(Piece* p) : piece(p) {
	;
}

bool Square::is_controlled_by(Board& board, piece_color color) {
	// NOT INPLEMENTED
	return false;
}

auto Square::to_graveyard(Board& board) -> void {
	if (piece) {
		board.graveyard[piece->color][board.nb_of_death[piece->color]++] = piece;
		piece->to_graveyard(board);
	}
}


void Square::draw(Game& game, Board& board, int pos) {
	
	move_data const last_move_data = board.get_last_nonduck_move();
	int const x = (pos & 0b111);
	int const y = pos >> 3;
	Color color = board.light_square_color;
	if ((x + y) % 2 == 0)
		color = board.dark_square_color;
	if (last_move_data.begin_pos == pos)
		color = board.last_move_color;
	if (last_move_data.target_pos == pos)
		color = board.last_move_color;
	if (last_move_data.end_pos == pos)
		color = board.last_move_color;

	if (game.state == in_game and game.selected_piece != NULL and game.selected_piece->pos == pos)
		color = board.selected_piece_color;
#if IN_DEBUG
	if ((*game.displayed_mask) & get_square_mask(pos))
		color = 0xFFFF0000;
#endif
	game.draw_rect(color, 5 + x, 9 - y, 1.0, 1.0);

	//SDL_FillSurfaceRect(board->surface, &rect, ((x + y) % 2 == 0) ? board->dark_square_color : board->light_square_color);

	
	
	if (game.accessible_mask & get_square_mask(pos))
		game.drawing_board.draw_disk((5 + x) * TILE_SIZE + (TILE_SIZE / 2), (9 - y) * TILE_SIZE + (TILE_SIZE / 2), TILE_SIZE / 4, board.possible_move_color);

	if (piece and not (game.selected_piece == piece and game.is_holding_something)) {
		SDL_Rect pos = { (5 + x) * TILE_SIZE, (9 - y) * TILE_SIZE, TILE_SIZE, TILE_SIZE };
		piece->draw(board, game.drawing_board, &pos);
	}
}
/*
File::File() {
}



File::File(int y){
	init(y);
}

auto File::init(int x) -> void {
	for (int y = 0; y < 8; y++) {
		const_cast<Sint8&>(data[y].x) = x;
		const_cast<Sint8&>(data[y].y) = y;
	}
}

void File::clear() {
	for (Square& square : data) {
		square.clear();
	}
}

File::~File() {
	clear();
}*/

Board::Board() {
	for (int pos = 0; pos < 64; pos++) {
		grid[pos] = Square();
	}
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

	miss_rate = 0.1f;
	crit_rate = 1.0f / 16.0f;

	reset();
}

auto Board::clear() -> void {
	turn_number = 0;
	
	nb_of_piece = 0;

	end_of_game = false;
	winner = no_color;

	memset(&__first_mask, 0, (& __last_mask - &__first_mask + 1) * sizeof(Uint64*));
	

	move_historic.clear();
	{
		move_data d;
		last_move_data = d;
	}

	for (Square& square : grid) {
		square.clear();
	}

	nb_of_death[white] = nb_of_death[black] = 0;
}


piece_id Board::layout[8] = { piece_id::rook, piece_id::knight, piece_id::bishop, piece_id::queen, piece_id::king, piece_id::bishop, piece_id::knight, piece_id::rook };
// piece_id Board::layout[8] = { piece_id::king, piece_id::queen, piece_id::none, piece_id::none, piece_id::none, piece_id::none, piece_id::none, piece_id::none };

void Board::reset() {
	clear();

	turn_number = 0;
	is_tera_avaible[white] = is_tera_avaible[black] = true;

	active_player = white;
	
	in_bonus_move = false;
	first_turn = true;
	promoting_piece = NULL;

	for (int i = 0; i < 8; i++) {
		int pos;
		if (layout[i] != piece_id::none) {
			pos = i;
			grid[pos].piece = &(piece_pool[nb_of_piece++] = Piece(layout[i], white, typeless, NO_ITEM));
			grid[pos].piece->set_square(self, false, pos);
			pos |= 7 << 3;
			grid[pos].piece = &(piece_pool[nb_of_piece++] = Piece(layout[i], black, typeless, NO_ITEM));
			grid[pos].piece->set_square(self, false, pos);
		}
		
		pos = i | (1 << 3);
		grid[pos].piece = &(piece_pool[nb_of_piece++] = Piece(piece_id::pawn, white, typeless, NO_ITEM));
		grid[pos].piece->set_square(self, false, pos);

		pos = i | (6 << 3);
		grid[pos].piece = &(piece_pool[nb_of_piece++] = Piece(piece_id::pawn, black, typeless, NO_ITEM));
		grid[pos].piece->set_square(self, false, pos);
	}

	Uint64 const mask = (1ull << 16) - 1;
	__typeless_mask = 0;
	memset(type_mask, 0, 18 * 8);
	honey_holder_mask = 0;
}


TimerLock reachable_lock;
TimerLock copy_lock;

std::chrono::nanoseconds reachable_duration;
std::chrono::nanoseconds copy_duration;

void Board::set_reachable_squares() {

	auto t0 = std::chrono::high_resolution_clock::now();

	/*if (duck_mask == 0 and duck != NULL) {
		duck->set_reachable(self);
	}*/

	
	// shortcut for bonus move as only a single piece can move
	if (in_bonus_move) {
		int const pos = last_move_data.end_pos;
		Piece* const piece = self[pos].piece;

		reachable_mask_array[pos] = piece->get_reachable(self);

		auto const [bonus, banned] = piece->item.get_reachable_and_banned(self, piece, reachable_mask_array[pos]);
		reachable_mask_array[pos] |= bonus;
		reachable_mask_array[pos] &= ~banned;
		reachable_mask_array[pos] &= ~piece_mask[(int)piece_id::duck];

		if (with_antichess) {
			Uint64 const mask = reachable_mask_array[pos] & color_mask[-piece->color];
			if (mask != 0)
				reachable_mask_array[pos] = mask;
		}

		if (with_items and piece->type == bug and (honey_holder_mask & reachable_mask_array[pos]) != 0) {
			reachable_mask_array[pos] &= honey_holder_mask;
		}

		return;
	}

	// basic chess rules
	Uint64 iterated_mask = presence_mask;
	while (iterated_mask) {
		int const pos = bsf64(iterated_mask);
		CLEAR_LOWEST_BIT(iterated_mask);
		Sint8 const x = pos & 0b111;
		Sint8 const y = pos >> 3;
		reachable_mask_array[pos] = self[pos].piece->get_reachable(self);
	}

	// items movement
	iterated_mask = color_mask[active_player];
	while (iterated_mask) {
		int const pos = bsf64(iterated_mask);
		CLEAR_LOWEST_BIT(iterated_mask);

		Piece* const piece = self[pos].piece;
		auto const [bonus, banned] = piece->item.get_reachable_and_banned(self, piece, reachable_mask_array[pos]);
		reachable_mask_array[pos] |= bonus;
		reachable_mask_array[pos] &= ~banned;
	}

	// antichess movement
	if (with_antichess) {
		bool any = false;
		Uint64 iterated_mask = color_mask[active_player];
		while (iterated_mask) {
			int const pos = bsf64(iterated_mask);
			CLEAR_LOWEST_BIT(iterated_mask);

			if ((reachable_mask_array[pos] & color_mask[-active_player]) != 0) {
				any = true;
				break;
			}
		}

		if (any) {
			iterated_mask = color_mask[active_player];
			while (iterated_mask) {
				int const pos = bsf64(iterated_mask);
				CLEAR_LOWEST_BIT(iterated_mask);

				reachable_mask_array[pos] &= color_mask[-active_player];
			}
		}
	}

	// honey check
	Uint64 const bug_mask = type_mask[bug] & color_mask[active_player];
	Uint64 const honey_mask = honey_holder_mask & ~color_mask[active_player];
	if (bug_mask != 0 and honey_mask != 0) {
		for (int pos = 0; pos < 64; pos++) {
			if ((bug_mask & get_square_mask(pos)) != 0) {
				if ((reachable_mask_array[pos] & honey_mask) != 0)
					reachable_mask_array[pos] &= honey_mask;
			}
		}
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
	reachable_lock.lock();
	reachable_duration += elapsed;
	reachable_lock.unlock();
}

TimerLock cancel_lock;
std::chrono::nanoseconds cancel_duration;

void Board::cancel_last_move(bool restore) {
	auto t0 = std::chrono::high_resolution_clock::now();

	if (move_historic.empty())
		return;
	
	end_of_game = false;

	move_data data = move_historic.front();
	move_historic.pop_front();


	in_bonus_move = false;
	turn_number--;
	if (not move_historic.empty()) {
		last_move_data = move_historic.front();
		if (last_move_data.move_again) {
			in_bonus_move = true;
		}
		move_data const& non_duck = get_last_nonduck_move();
		if (ABS_INT(non_duck.end_pos - non_duck.begin_pos) == 16 and non_duck.attacker->id == piece_id::pawn) {
			avaible_en_passant_mask = get_square_mask((non_duck.end_pos + non_duck.begin_pos) / 2);
		}
		else {
			avaible_en_passant_mask = 0;
		}
	}
	else {
		move_data d;
		last_move_data = d;
		avaible_en_passant_mask = 0;
	}
	active_player = data.attacker->color;

	if (data.skip_bonus_turn) {
		goto restore_reachable;
	}

	if (data.init_promotion) {
		promoting_piece = NULL;
		data.move_again = false;
		data.interrupt_move = data.init_promotion = false;
	}

	else if (data.end_promotion) {
		data.attacker->to_limbo(self);
		*data.attacker = data.attacker_copy;
		data.attacker->revive(self);
		promoting_piece = data.attacker;
		avaible_promotion = move_historic.front().avaible_promotion;
		return;
	}


	/*if (data.cancel) {

		*data.attacker = data.attacker_copy;
		if (data.defender != NULL)
			*data.defender = data.defender_copy;

		goto restore_reachable;

	}*/
	if (data.tera) {
		is_tera_avaible[data.attacker_copy.color] = true;
	}


	if (data.castling) {
		int const direction = (data.end_pos > data.begin_pos) ? -1 : 1;
		Piece* rook = self[data.end_pos + direction].piece;
		rook->set_square(self, false, data.target_pos);
	}

	if (data.suicide) {
		nb_of_death[data.attacker_copy.color]--;
	}
	else {
		data.attacker->to_limbo(self);
	}
	*data.attacker = data.attacker_copy;
	data.attacker->set_square(self, false, data.begin_pos);

	if (data.defender and data.defender->is_dead) {
		nb_of_death[data.defender_copy.color]--;
		*data.defender = data.defender_copy;
		data.defender->revive(self);
	}

	

restore_reachable:

	auto t1 = std::chrono::high_resolution_clock::now();
	cancel_lock.lock();
	cancel_duration += t1 - t0;
	cancel_lock.unlock();

	if (not restore)
		return;
	set_reachable_squares();
}

void Board::resize_surface() {
	surface = Surface::createRGB(8 * TILE_SIZE, 8 * TILE_SIZE);
}

move_data const Board::get_last_nonduck_move() {
	for (move_data move : move_historic) {
		/*if (reinterpret_cast<Uint64>(move.attacker) == 0xdddd'dddd'dddd'ddddull)
			throw std::exception();*/
		if (move.attacker->color != no_color)
			return move;
	}
	static move_data empty;
	return empty;
}

bool Board::in_stalemate(piece_color color) {
	return false;
	// NOT INPLEMENTED
	return true;
}
TimerLock movement_lock;

std::chrono::nanoseconds movement_duration;

void Board::move_piece_to(Piece* piece, Sint8 target_pos) {

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	piece_color player = active_player;

	last_move_data = piece->move_to(self, target_pos);

	if ((not last_move_data.interrupt_move))
		resume_move();

	if (last_move_data.init_promotion) {
		if (not last_move_data.attacker->item.prepare_promotion(self, piece, &avaible_promotion)) {
			avaible_promotion = 0b0000'0000'0001'1110;
		}

		if (avaible_promotion != 0) {
			promoting_piece = piece;
			last_move_data.avaible_promotion = avaible_promotion;
			move_historic.push_front(last_move_data);
		}
	}
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	movement_lock.lock();
	movement_duration += t1 - t0;
	movement_lock.unlock();
}



void Board::first_half_move(Piece* piece, Sint8 target_pos) {
	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	piece_color const player = active_player;

	last_move_data = piece->move_to(self, target_pos);
	move_historic.push_front(last_move_data);

	if (ABS_INT(last_move_data.end_pos - last_move_data.begin_pos) == 16 and piece->id == piece_id::pawn) {
		avaible_en_passant_mask = get_square_mask((last_move_data.end_pos + last_move_data.begin_pos) / 2);
	}
	else if (player != no_color) {
		avaible_en_passant_mask = 0;
	}

	if (last_move_data.init_promotion) {
		if (not last_move_data.attacker->item.prepare_promotion(self, piece, &avaible_promotion)) {
			avaible_promotion = 0b0000'0000'0001'1110;
		}

		if (avaible_promotion != 0) {
			promoting_piece = piece;
			last_move_data.avaible_promotion = avaible_promotion;
		}
	}
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	movement_lock.unlock();
	movement_duration += t1 - t0;
	movement_lock.unlock();
}

void Board::second_half_move(move_data& first_half_move_data) {

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

	move_data& data = first_half_move_data;

	if (data.end_promotion) {
		;
	}
	else {
		if (not data.defender or not data.defender->item.is_safety_google()) {
			data.attacker->item.update_status(self, data.attacker, data);
		}

		if (data.defender and not data.attacker->item.is_protective_pads()) {
			data.defender->item.update_status(self, data.defender, data);
		}

		if (not data.move_again) {
			in_bonus_move = false;
			change_turn();
		}
		else {
			in_bonus_move = true;
		}
		check_for_end_of_game();

		turn_number++;
		if (not end_of_game) {
			std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
			set_reachable_squares();
			std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
			t0 += t4 - t3;
		}
	}

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	movement_lock.unlock();
	movement_duration += t1 - t0;
	movement_lock.unlock();
}


void Board::check_for_end_of_game() {
	if (with_check) {
		if (in_stalemate(active_player)) {
			end_of_game = true;
			winner = no_color;
		}
	}
	else if (with_antichess) {
		if (nb_of_death[white] == 16) {
			winner = (piece_color)(not (nb_of_death[black] == 16));
			end_of_game = true;
		}
		else if (nb_of_death[black] == 16) {
			winner = black;
			end_of_game = true;
		}
	}
	else {
		Uint64 const white_king_mask = piece_mask[(int)piece_id::king] & color_mask[white];
		Uint64 const black_king_mask = piece_mask[(int)piece_id::king] & color_mask[black];

		if (white_king_mask == 0 and black_king_mask == 0) {
			winner = no_color;
			end_of_game = true;
		}
		else if (white_king_mask == 0) {
			winner = black;
			end_of_game = true;
		}
		else if (black_king_mask == 0) {
			winner = white;
			end_of_game = true;
		}
	}
}

void Board::promote(piece_id id) {
#if ENABLE_SAFETY_CHECKS
	if (promoting_piece == NULL or id >= 16 or ((1 << id) & avaible_promotion) == 0) {
		PRINT_DEBUG("Unauthorized promotion");
		throw;
	}
#endif

	move_data data;
	data.attacker = promoting_piece;
	data.attacker_copy = *promoting_piece;
	data.end_promotion = true;
	data.begin_pos = data.end_pos = data.target_pos = promoting_piece->pos;


	promoting_piece->promote(self, id);

	promoting_piece = NULL;

	second_half_move(data);

	second_half_move(last_move_data);

	move_historic.push_front(data);

	last_move_data = data;
}

void Board::resume_move() {
	move_data& data = last_move_data;

	if (not data.defender or not data.defender->item.is_safety_google()) {
		data.attacker->item.update_status(self, data.attacker, data);
	}

	if (data.defender and not data.attacker->item.is_protective_pads()) {
		data.defender->item.update_status(self, data.defender, data);
	}

	if (not data.move_again) {
		in_bonus_move = false;
		change_turn();
	}
	else {
		in_bonus_move = true;
	}

	check_for_end_of_game();

	turn_number++;
	move_historic.push_front(data);
	if (not end_of_game) {
		set_reachable_squares();
	}
}

void Board::change_turn() {
	if (with_duck_chess) {
		if (active_player == no_color) {
			first_turn = false;
			active_player = -get_last_nonduck_move().attacker->color;
		}
		else {
			if (piece_mask[(int)piece_id::duck] == 0) {
				Duck(duck = &__duck__);
			}
			active_player = no_color;
		}
	}
	else {
		active_player = -active_player;
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
	data.attacker = self[last_move_data.end_pos].piece;
	data.target_pos = data.begin_pos = data.end_pos = data.attacker->pos;

	data.skip_bonus_turn = true;
	move_historic.push_front(data);
	
	change_turn();
	check_for_end_of_game();

	set_reachable_squares();
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
	data.tera = true;
	data.attacker = piece;
	data.attacker_copy = *piece;

	data.target_pos = data.begin_pos = data.end_pos = data.attacker->pos;

	piece->tera(self, new_type);
	piece->set_item(self, NO_ITEM);
	
	move_historic.push_front(data);

	change_turn();

	set_reachable_squares();
}

Board& Board::operator=(Board& other) {

	std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
	
	Uint64 const offset = reinterpret_cast<Uint8*>(this) - reinterpret_cast<Uint8*>(&other);
	Uint64 const move_historic_offset = reinterpret_cast<Uint8*>(&move_historic) - reinterpret_cast<Uint8*>(this);

	memcpy(this, &other, move_historic_offset);
	
	move_historic = other.move_historic;

	memcpy(&move_historic + 1, &other.move_historic + 1, sizeof(Board) - move_historic_offset - sizeof(move_historic));
	
	
	// memcpy(piece_pool, other.piece_pool, 40 * sizeof(Piece));

	if (duck != NULL) {
		duck = &__duck__;
	}
	
	if (promoting_piece != NULL) {
		promoting_piece = reinterpret_cast<Piece*>(reinterpret_cast<Uint8*>(promoting_piece) + offset);
	}
	else {
		promoting_piece = NULL;
	}

	for (int pos = 0; pos < 64; pos++) {
		if (self[pos].piece) {
			self[pos].piece = reinterpret_cast<Piece*>(reinterpret_cast<Uint8*>(self[pos].piece) + offset);
		}
	}

	for (int i = 0; i < nb_of_death[white]; i++) {
		graveyard[white][i] = reinterpret_cast<Piece*>(reinterpret_cast<Uint8*>(graveyard[white][i]) + offset);
	}

	for (int i = 0; i < nb_of_death[black]; i++) {
		graveyard[black][i] = reinterpret_cast<Piece*>(reinterpret_cast<Uint8*>(graveyard[black][i]) + offset);
	}

	/*nb_of_death[white] = other.nb_of_death[white];
	nb_of_death[black] = other.nb_of_death[black];

	in_bonus_move = other.in_bonus_move;

	move_historic = other.move_historic;

	last_move_data = other.last_move_data;
	
	active_player = other.active_player;

	with_ability = other.with_ability;
	with_antichess = other.with_antichess;
	with_check = other.with_check;
	with_duck_chess = other.with_duck_chess;
	with_items = other.with_items;
	with_reversed_typechart = other.with_reversed_typechart;
	with_RNG = other.with_RNG;
	with_typing = other.with_typing;
	
	crit_rate = other.crit_rate;
	miss_rate = other.miss_rate;


	memcpy(&__first_mask, &other.__first_mask, (&__last_mask - &__first_mask + 1) * sizeof(Uint64*));

	winner = other.winner;
	end_of_game = other.end_of_game;*/

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	copy_duration += t1 - t0;

	return self;
}


Board::Board(Board& other) {
	self = other;
}

Board::~Board() {
	clear();
}


MoveContainer<bot_move_data, MOVE_CONTAINER_CAPACITY> Board::get_avaible_move() {
	MoveContainer<bot_move_data, MOVE_CONTAINER_CAPACITY> vec;
	bot_move_data move;
	move.tera = move.skip_bonus_turn = move.promotion = false;
	if (promoting_piece != NULL) {

		decltype(avaible_promotion) avaible = avaible_promotion;
		move.promotion = true;
		for (int i = 0; i < 8 * sizeof(avaible_promotion); i++) {
			if ((avaible & 1) != 0) {
				move.promotion_id = (piece_id)i;
				vec.push(move);
			}
			avaible >>= 1;
		}

		return vec;
	}

	Uint64 iterated_mask_1 = in_bonus_move ? get_square_mask(last_move_data.end_pos) : color_mask[active_player];
	while (iterated_mask_1) {
		int const begin = bsf64(iterated_mask_1);
		CLEAR_LOWEST_BIT(iterated_mask_1);
		
		move.begin_pos = begin;

		Piece* const piece = self[move.begin_pos].piece;
		
		Uint64 iterated_mask_2 = reachable_mask_array[begin];
		while (iterated_mask_2) {
			int const target = bsf64(iterated_mask_2);
			CLEAR_LOWEST_BIT(iterated_mask_2);

			move.target_pos = target;

			vec.push(move);
		}

		if (piece->item.get_id() == item_ID::tera_shard) {
			move.tera = true;
			vec.push(move);
			move.tera = false;
		}
	}
	if (in_bonus_move) {
		move.skip_bonus_turn = true;
		vec.push(move);
	}
	return vec;
}

void Board::execute_move(bot_move_data move) {
	if (move.promotion) [[unlikely]] {
		promote(move.promotion_id);
	}
	else if (move.skip_bonus_turn) [[unlikely]] {
		skip_bonus_turn();
	}
	else if (move.tera) [[unlikely]] {
		tera_piece(self[move.begin_pos].piece);
	}
	else [[likely]] {
		if (self[move.begin_pos].piece == NULL) {
			std::cout << (int)move.begin_pos << ", " << (int)move.target_pos << "\n";
		}
		first_half_move(self[move.begin_pos].piece, move.target_pos);
		if (not last_move_data.interrupt_move) [[unlikely]] {
			second_half_move(last_move_data);
		}
	}
}

BoardIterator::BoardIterator(Board& board, int pos) : iterated_board(board) {
	index = pos;
}

BoardIterator::BoardIterator(const BoardIterator& other) : iterated_board(other.iterated_board) {
	index = other.index;
}

auto BoardIterator::operator*() -> reference {
	return iterated_board[index];
}

auto BoardIterator::operator->() -> pointer {
	return  &(iterated_board[index]);
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

BoardIterator Board::begin() {
	return BoardIterator(self, 0);
}

BoardIterator Board::end() {
	return BoardIterator(self, 64);
}