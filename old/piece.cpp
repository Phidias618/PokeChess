/*This files contains all the code related to chess piece*/

#include <cstdarg>
#include <functional>

#include "SDL+.h"

#include "assets.h"

#include "poketyping.h"
#include "piece.h"

#define set_cls(Class, id, ...) PieceClass  Class::__cls = PieceClass(std::function<Class* (Board&, piece_color, Square*, typing, PokeItem)>([](Board& board, piece_color color, Square* sq, typing type, PokeItem item) -> Class* { return new Class(board, color, sq, type, item);  }), id, __VA_ARGS__); PieceClass* const Class::cls = &Class::__cls;

set_cls(King, 0, "Roi", "King");
set_cls(Queen, 1, "Dame", "Queen");
set_cls(Bishop, 2, "Fou", "Bishop");
set_cls(Knight, 3, "Cavalier", "Knight");
set_cls(Rook, 4, "Tour", "Rook");
set_cls(Pawn, 5, "Pion", "Pawn");
PieceClass Duck::__cls = PieceClass(std::function<Duck * (Board&, piece_color, Square*, typing, PokeItem)>([](Board& board, piece_color color, Square* sq, typing type, PokeItem item) -> Duck* { return new Duck(board, sq); }), 6, "Psykokwak", "Psyduck"); PieceClass* const Duck::cls = &Duck::__cls;;

#undef set_cls

#include "game.h"
#include "board.h"

constexpr int ABS_INT(int x) {
	return (x >= 0) ? x : -x;
}


std::ostream& operator<<(std::ostream& os, move_data const data) {
	if (data.attacker != NULL) {
		os << "attacker: " << data.attacker->Class->name[1] << " at (" << data.attacker->x << ", " << data.attacker->y << ")\n";
	}
	else {
		os << "attacker: None\n";
	}
	if (data.target_square != NULL)
		os << "target: (" << data.target_square->x << ", " << data.target_square->y << ")\n";
	else
		os << "target: None\n";
	os << "defender: " << ((data.defender == NULL) ? "None" : data.defender->Class->name[1]) << '\n';
	os << '\n';
	if (data.castling)
		os << "castling\n";
	if (data.cancel)
		os << "canceled\n";
	if (data.move_again)
		os << "bonus move\n";
	if (data.promotion)
		os << "promotion\n";
	if (data.en_passant)
		os << "en passant\n";
	if (data.skip_bonus_turn)
		os << "skip\n";
	if (data.tera)
		os << "tera into " << type_str[data.attacker->type][1] << '\n';
	os << "---------------------\n";
	return os;
}


void move_data::set_type_matchup_data(Piece* attacker_, Piece* defender_, Square* target_square_) {
	attacker = attacker_;
	defender = defender_;
	begin_square = attacker->square;
	target_square = target_square_;
	
	miss_rate = attacker->board.miss_rate;
	crit_rate = attacker->board.crit_rate;

	was_piece_first_move = not attacker->has_already_move;

	attacker_item = attacker->item;

	if (defender != NULL)
		defender_item = defender->item;

	if (not attacker->board.with_typing or defender == NULL)
		return;

	matchup = get_effectiveness(attacker->type, defender->type, attacker->board.with_reversed_typechart);

	if (attacker->board.with_ability) {
		if ((attacker->type == fire and defender->ability_torch) or
			(attacker->type == water and defender->ability_water_absorb) or
			(attacker->type == grass and defender->ability_sap_sipper) or
			(attacker->type == electric and defender->ability_volt_absorb)) 
		{
			matchup = immune;
			is_immune = true;
		}
	}

	Board& board = attacker->board;


	if (not defender->item.is_safety_google()) {
		if (not attacker->item.is_protective_pads()) {
			if (attacker->item.priority() >= defender->item.priority()) {
				attacker->item.attack_modifier(self);
				defender->item.defense_modifier(self);

				attacker->item.crit_modifier(self);
				
				attacker->item.accuracy_modifier(self);
				defender->item.evasion_modifier(self);
			}
			else {
				defender->item.defense_modifier(self);
				attacker->item.attack_modifier(self);
				
				attacker->item.crit_modifier(self);

				defender->item.evasion_modifier(self);
				attacker->item.accuracy_modifier(self);
			}
		}
		else {
			attacker->item.attack_modifier(self);
			attacker->item.crit_modifier(self);
			attacker->item.accuracy_modifier(self);
		}
	}
	else {
		if (not attacker->item.is_protective_pads()) {
			defender->item.defense_modifier(self);
			defender->item.evasion_modifier(self);
		}
	}

	if (matchup == immune) {
		is_immune = true;
		cancel = true;
		return;
	}

	if ((not attacker->board.with_ability or not defender->ability_shell_armor) and
		attacker->board.with_RNG and (double)game.RNG() / (double)game.RNG.max() < crit_rate) {
		do_crit = true;
		move_again = true;
	}
	else if ((not attacker->board.with_ability or not defender->ability_no_guard or not attacker->ability_no_guard or not attacker->ability_compound_eye) and
		attacker->board.with_RNG and (double)game.RNG() / (double)game.RNG.max() < miss_rate) {
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

PieceClass::PieceClass() : constructor(NULL), id(-2) {
	for (int i = 1; i < (int)LANGUAGE::NB_OF_LANGUAGE; i++) {
		name[i] = "";
	}
}

PieceClass::PieceClass(int _id, char const* names...) : constructor(NULL), id(_id) {
	va_list args;
	va_start(args, _id);
	name[0] = names;
	for (int i = 1; i < (int)LANGUAGE::NB_OF_LANGUAGE; i++) {
		name[i] = va_arg(args, char const*);
	}
	va_end(args);
}

template<typename T>
PieceClass::PieceClass(std::function<T* (Board&, piece_color, Square*, typing, PokeItem)> ctor, int id_, const char* names...) : id(id_), constructor(ctor) {
	base_promotion_constructor =
		[this](Piece* pawn) -> Piece* {
			Piece* res = constructor(pawn->board, pawn->color, NULL, pawn->type, pawn->item);
			res->has_already_move = res->evolved = true;
			PokemonIcon icon = pawn->pokeicon;
			icon.promote(res);
			res->set_pokeicon(icon);
			return res;
		};

	va_list args;
	va_start(args, names);
	name[0] = names;
	for (int i = 1; i < (int)LANGUAGE::NB_OF_LANGUAGE; i++) {
		name[i] = va_arg(args, char const*);
	}
	va_end(args);

	T* piece = ctor(game.board, no_color, NULL, typeless, NO_ITEM);
	offset = (Uint8*)(Piece*)piece - (Uint8*)piece;
	delete piece;
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

Piece* PieceClass::operator()(Board& board, piece_color color, Square* sq, typing type, PokeItem item) {
	if (self != NULL)
		return constructor(board, color, sq, type, item);
	return NULL;
}

PokemonIcon::PokemonIcon(Piece const* const piece) {
	x = 1 + 2 * (piece->Class->id + piece->evolved * 5) + (1 - piece->color) / 2;
	y = (short)piece->base_type + 1;
	if (y == 0) {
		x = y = -1;
	}
}

void PokemonIcon::promote(Piece* const promoted_piece) {
	// for the eveelitions
	if (x == 21 or x == 22)
		x += 2;
	else {
		x += 2 * (promoted_piece->Class->id + (x > 13));
	}
}

Piece::Piece(Board& board_, PieceClass* const _Class, piece_color color_, Square* sq, typing type_, PokeItem item) :
	board(board_),
	Class(_Class),
	color(color_),
	// sprite(Surface::createRGBA(TILE_SIZE, TILE_SIZE)),
	square(NULL),
	base_type(type_), type(type_),
	item(item)
{	
	has_already_move = false;
	is_in_graveyard = false;
	set_square(sq, false);
	update_sprite();
}

void Piece::update_sprite() {
	/*// sprite.fill(0x00000000);

	SDL_Rect dest(0, 0, TILE_SIZE, TILE_SIZE);
	SDL_Rect area(TILE_SIZE * Class->id, TILE_SIZE * color, TILE_SIZE, TILE_SIZE);

	// sprite.blit(sprite_sheet, &dest, &area);

	if (pokeicon) {
		pokeicon.draw(sprite, TILE_SIZE/8, TILE_SIZE/8);
	}

	if (is_tera) {
		if (sprite.MUSTLOCK())
			sprite.lock();

		Uint32* p1 = (Uint32*)sprite->pixels;
		Uint32* p2 = (Uint32*)tera_mosaic->pixels;

		Uint32* end = p1 + sprite->h * sprite->pitch / 4;

		while (p1 != end) {
			
			for (int X = 0; X < sprite->w; X++) {
				Uint8 a1 = p1[X] >> 24;
				if (a1 != 0) {
					float mix_factor = (float)(p2[X] & 0xff) / 255;
					mix_factor *= mix_factor; // ^2
					mix_factor *= mix_factor * mix_factor; // ^6

					Uint32 shade = type_color[type];
					Uint8 r1 = p1[X];
					Uint8 r2 = shade;

					Uint8 g1 = p1[X] >> 8;
					Uint8 g2 = (shade >> 8);

					Uint8 b1 = p1[X] >> 16;
					Uint8 b2 = (shade >> 16);

					Uint8 a2 = 0xff;

					p1[X] &= 0x00000000;
					p1[X] |= (Uint32)(mix_factor * r2 + (1 - mix_factor) * r1);
					p1[X] |= (Uint32)(mix_factor * g2 + (1 - mix_factor) * g1) << 8;
					p1[X] |= (Uint32)(mix_factor * b2 + (1 - mix_factor) * b1) << 16;
					p1[X] |= (Uint32)(mix_factor * a2 + (1 - mix_factor) * a1) << 24;

				}

			}

			p1 += sprite->pitch / 4;
			p2 += tera_mosaic->pitch / 4;
		}

		if (sprite.MUSTLOCK())
			sprite.unlock();

	}

	dest = { 5 * TILE_SIZE / 8, 5 * TILE_SIZE / 8, 0, 0 };
	if (type != typeless)
		sprite.blit(typing_icon[type].scale_to(TILE_SIZE / 3, TILE_SIZE / 3, true), &dest, NULL);

	dest = { 0, TILE_SIZE - ITEM_MINI_SIZE, 0, 0 };
	item.draw(sprite, &dest, mini);*/
}

void Piece::set_type(typing new_type) {
	Uint64 const pos_mask = 1ull << (x | (y << 3));
	board.type_mask[type] &= ~pos_mask;
	base_type = type = new_type;
	board.type_mask[type] |= pos_mask;

	set_pokeicon(PokemonIcon(this));
	item.sync_holder(this);
}

void Piece::tera(typing new_type) {
	type = new_type;
	is_tera = true;

	Uint64 const pos_mask = 1ull << (x | (y << 3));
	
	board.type_mask[base_type] &= ~pos_mask;
	board.type_mask[new_type] |= pos_mask;

	board.is_tera_avaible[color] = false;
}

void Piece::un_tera() {
	Uint64 const pos_mask = 1ull << (x | (y << 3));
	board.type_mask[base_type] |= pos_mask;
	board.type_mask[type] &= ~pos_mask;

	type = base_type;
	is_tera = false;
	// update_sprite();

	board.is_tera_avaible[color] = true;
}

void Piece::set_item(PokeItem new_item) {
	item.change_holder(this, NULL);

	item = new_item;

	item.change_holder(NULL, this);

	// update_sprite();
}

void Piece::set_pokeicon(PokemonIcon icon) {
	pokeicon = icon;
	ability = icon.get_ability();
}

bool Piece::base_can_move_to(Square const& target) const {
	Piece* p = target.piece;
	if (p != NULL and p->Class == Duck::cls)
		return false;

	return ((reachable_mask >> (target.x | (target.y << 3))) & 1) == 1;

	/*auto target_piece = target_square.piece;
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
		return false;*/
}

bool Piece::base_do_control(Square const& target_square) const { return false; }

move_data Piece::move_to(Square& square) {
	move_data data;
	

	data.set_type_matchup_data(this, square.piece, &square);
	if ((item_reachable_mask & square.get_mask()) == 0) {
		data = base_move_to(square);
	}
	else {
		data = item.move_to(this, square);
	}
	


	if (data.defender == NULL or not data.defender->item.is_safety_google()) {
		data.attacker->item.after_move_effects(data);
	}

	if (data.defender != NULL and not data.attacker->item.is_protective_pads()) {
		data.defender->item.revenge(data);
	}

	return data;
}


void Piece::set_square(Square* target) {

	if (square != NULL) {
		Uint64 const remove_mask = ~(1ull << (x | (y << 3)));

		board.presence_mask &= remove_mask;
		board.color_mask[color] &= remove_mask;
		board.type_mask[type] &= remove_mask;
		board.piece_mask[Class->id] &= remove_mask;

		square->piece = NULL;
	}

	if (target != NULL) {
		target->to_graveyard(); // kills the opposing piece
		target->piece = this; // moves itself to its new location

		item.move_with(this, *square, *target);
	}

	square = target; // change its current square

	if (square != NULL) {
		Uint64 const add_mask = 1ull << (x | (y << 3));

		board.presence_mask |= add_mask;
		board.color_mask[color] |= add_mask;
		board.type_mask[type] |= add_mask;
		board.piece_mask[Class->id] |= add_mask;
	}
}

move_data Piece::base_move_to(Square& target_square) {
	move_data data;
	data.set_type_matchup_data(this, target_square.piece, &target_square);

	if (data.cancel)
		;
	else {
		set_square(&target_square);
		has_already_move = true;
		if (data.suicide) {
			square->to_graveyard(); // kills itself because of not_very_effective move
		}
	}

	return data;
}



bool Piece::can_move_to(Square const& target, Uint64 flags) const {
	/*Piece* adv = target.piece;

	static bool check_for_is_move_disallowed = true, check_for_is_move_allowed = true;
	static bool check_for_antichess = true;


	if (adv != NULL and adv->Class == Duck::cls)
		return false;

	bool base = base_can_move_to(target);

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

	if (not base)
		return false;

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
	

	if (game.with_antichess and check_for_antichess and not board.in_bonus_move and target.piece == NULL and (Class != Pawn::cls or not dynacast<Pawn>(this)->can_en_passant(target))) {
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
	*/

	

	return (reachable_mask >> (target.x | (target.y << 3)) & 1) == 1;
}

bool Piece::do_control(Square const& target) const {
	bool base = base_do_control(target);
	return base;
}


void Piece::draw(Surface dest, SDL_Rect* dest_rect) {
	// SDL_Rect dest(0, 0, TILE_SIZE, TILE_SIZE);
	SDL_Rect area(TILE_SIZE * Class->id, TILE_SIZE * ((1 - color) / 2), TILE_SIZE, TILE_SIZE);
	SDL_Rect pos = { 0, 0, 0, 0 };

	if (dest_rect != NULL) {
		pos.x = dest_rect->x;
		pos.y = dest_rect->y;
	}
	else {
		pos.x = pos.y = 0;
	}

	dest.blit(sprite_sheet, &pos, &area);

	if (dest_rect != NULL) {
		pos.x = dest_rect->x;
		pos.y = dest_rect->y;
	}
	else {
		pos.x = pos.y = 0;
	}

	if (pokeicon) {
		pokeicon.draw(dest, pos.x + TILE_SIZE / 8, pos.y + TILE_SIZE / 8);
	}



	if (is_tera) {
		if (dest.MUSTLOCK())
			dest.lock();

		Uint32* p1 = (Uint32*)dest->pixels + pos.x + pos.y * dest->pitch / 4;
		Uint32* p2 = (Uint32*)tera_mosaic->pixels;

		Uint32 background_color = *p1;

		Uint32* end = p1 + TILE_SIZE * dest->pitch / 4;

		while (p1 != end) {

			for (int X = 0; X < TILE_SIZE; X++) {
				Uint8 a1 = p1[X] >> 24;
				if (a1 != 0 and p1[X] != background_color) {
					float mix_factor = (float)(p2[X] & 0xff) / 255;
					mix_factor *= mix_factor; // ^2
					mix_factor *= mix_factor * mix_factor; // ^6

					Uint32 shade = type_color[type];
					Uint8 r1 = p1[X];
					Uint8 r2 = shade;

					Uint8 g1 = p1[X] >> 8;
					Uint8 g2 = (shade >> 8);

					Uint8 b1 = p1[X] >> 16;
					Uint8 b2 = (shade >> 16);

					Uint8 a2 = 0xff;

					p1[X] &= 0x00000000;
					p1[X] |= (Uint32)(mix_factor * r2 + (1 - mix_factor) * r1);
					p1[X] |= (Uint32)(mix_factor * g2 + (1 - mix_factor) * g1) << 8;
					p1[X] |= (Uint32)(mix_factor * b2 + (1 - mix_factor) * b1) << 16;
					p1[X] |= (Uint32)(mix_factor * a2 + (1 - mix_factor) * a1) << 24;

				}

			}

			p1 += dest->pitch / 4;
			p2 += tera_mosaic->pitch / 4;
		}

		if (dest.MUSTLOCK())
			dest.unlock();
	}

	if (dest_rect != NULL) {
		pos.x = dest_rect->x + 5 * TILE_SIZE / 8;
		pos.y = dest_rect->y + 5 * TILE_SIZE / 8;
	}
	else {
		pos.x = pos.y = 5 * TILE_SIZE / 8;
	}
	if (board.with_typing and type != typeless)
		dest.blit(typing_icon[type].scale_to(TILE_SIZE / 3, TILE_SIZE / 3, true), &pos, NULL);


	if (dest_rect != NULL) {
		pos.x = dest_rect->x;
		pos.y = dest_rect->y + TILE_SIZE - ITEM_MINI_SIZE;
	}
	else {
		pos.x = 0;
		pos.y = TILE_SIZE - ITEM_MINI_SIZE;
	}

	item.draw(dest, &pos, mini);
}

Piece* Piece::copy(Board& new_board) {
	Piece* p;
	switch (Class->id) {
	case 0:
		p = new King(new_board, color, square == NULL ? NULL : &new_board[x][y], type, item);
		break;
	case 1:
		p = new Queen(new_board, color, square == NULL ? NULL : &new_board[x][y], type, item);
		break;
	case 2:
		p = new Bishop(new_board, color, square == NULL ? NULL : &new_board[x][y], type, item);
		break;
	case 3:
		p = new Knight(new_board, color, square == NULL ? NULL : &new_board[x][y], type, item);
		break;
	case 4:
		p = new Rook(new_board, color, square == NULL ? NULL : &new_board[x][y], type, item);
		break;
	case 5:
		p = new Pawn(new_board, color, square == NULL ? NULL : &new_board[x][y], type, item);
		break;
	case 6:
		p = new Duck(new_board, square == NULL ? NULL : &new_board[x][y]);
		break;
	}
	p->reachable_mask = reachable_mask;
	p->item_reachable_mask = item_reachable_mask;
	p->ability = ability;
	p->evolved = evolved;
	p->has_already_move = has_already_move;
	p->is_in_graveyard = is_in_graveyard;
	p->is_tera = is_tera;
	p->base_type = base_type;
	p->type = type;
	p->pokeicon = pokeicon;
}

Piece::~Piece() {

}
#define useless0 1
#define useless1 useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0
#define useless2 useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1
#define useless3 useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2

King::King(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item) : Piece(board_, King::cls, color, sq, type_, item) {
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

void King::set_reachable() {
	reachable_mask = 0;
	short const X = x;
	short const Y = y;

	Uint64 const ham = ((X < 7) << 2) | (X > 0);
	Uint64 const bread = ham | 0b010;

	if (Y < 7) {
		reachable_mask |= bread << ((X | ((Y + 1) << 3)) - 1);
	}

	reachable_mask |= ham << ((X | (Y << 3)) - 1);

	if (Y > 0) {
		reachable_mask |= bread << ((X | ((Y - 1) << 3)) - 1);
	}

	reachable_mask &= ~board.color_mask[color];

	if (not has_already_move) {
		Square* rook_pos;
		if (X <= 6 and can_castle(board[X + 2][Y], &rook_pos)) {
			reachable_mask |= 1ull << ((X | (Y << 3)) + 2);
			reachable_mask |= 1ull << (rook_pos->x | (rook_pos->y << 3));
		}
		if (X >= 2 and can_castle(board[X - 2][Y], &rook_pos)) {
			reachable_mask |= 1ull << ((X | (Y << 3)) - 2);
			reachable_mask |= 1ull << (rook_pos->x | (rook_pos->y << 3));
		}
	}
}

auto King::base_can_move_to(Square const& target) -> bool {
	return (Piece::base_can_move_to(target) and ABS_INT(x - target.x) <= 1 and ABS_INT(y - target.y) <= 1) or can_castle(target, NULL);
}

auto King::base_move_to(Square& target_square) -> move_data {
	
	Square* rook_pos;
	
	if (can_castle(target_square, &rook_pos)) {
		move_data data;
		data.set_type_matchup_data(this, NULL, &target_square);
		
		data.was_in_check = is_in_check();

		if (not data.cancel) {
			castle(rook_pos);
			data.target_square = rook_pos;
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

// rook_pos must contain a valid Rook to castle with
auto King::castle(Square* rook_pos) -> void {
	Piece* rook = rook_pos->piece;
#if ENABLE_SAFETY_CHECKS
	if (rook == NULL or rook->Class != Rook::cls) {
		PRINT_DEBUG("WHO IS THE IDIOT(probably me) NOT ABLE TO READ THE CONDITION ON King::castle");
		throw;
	}
#endif

	int dx = rook_pos->x - x;
	int x_step = (dx > 0) ? 1 : -1;
	/*;
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
	*rook_pos = rook->square;*/


	/*Uint64 const king_remove_mask = ~(1ull << (x | (y << 3)));
	board.presence_mask &= king_remove_mask;
	board.color_mask[color] &= king_remove_mask;
	board.type_mask[type] &= king_remove_mask;

	square->piece = NULL; // removes the king from its previous position
	board[x + 2 * x_step][y].piece = this; // moves the king to the corresponding square
	square = &board[x + 2 * x_step][y];// updates the king position

	Uint64 const king_add_mask = 1ull << (x | (y << 3));
	board.presence_mask |= king_add_mask;
	board.color_mask[color] |= king_add_mask;
	board.type_mask[type] |= king_add_mask;


	Uint64 const rook_remove_mask = ~(1ull << (rook->x | (rook->y << 3)));
	board.presence_mask &= rook_remove_mask;
	board.color_mask[color] &= rook_remove_mask;
	board.type_mask[type] &= rook_remove_mask;

	rook->square->piece = NULL; // removes the rook from it's previous position
	.piece = rook; // moves the rook to the corresponding square
	rook->square = &board[x - x_step][y]; // updates the position of the rook

	Uint64 const rook_add_mask = 1ull << (x | (y << 3));
	board.presence_mask |= rook_add_mask;
	board.color_mask[color] |= rook_add_mask;
	board.type_mask[type] |= rook_add_mask;
	*/
	
	set_square(&board[x + 2 * x_step][y]);
	rook->set_square(&board[x - x_step][y]);
}

bool King::base_do_control(Square const& target_square) const {
	auto dx = target_square.x - x;
	auto dy = target_square.y - y;
	return (dx != 0 or dy != 0) and -1 <= dx and dx <= 1 and -1 <= dy and dy <= 1;
}


Pawn::Pawn(Board& board_, piece_color color, Square* sq, typing type_, PokeItem item) : Piece(board_, Pawn::cls, color, sq, type_, item) {
}

inline bool Pawn::base_do_control(Square const& square) const {
	return (square.y - y) == ((color == white) ? 1 : -1) and ABS_INT(square.x - x) == 1;
}

void Pawn::set_reachable() {
	reachable_mask = 0;

	short X = x, Y = y + color;

	if (Y < 0 or Y >= 8)
		return;

	Piece* p;
	Square* s;
	if (X > 0) {
		s = &board[X - 1][Y];
		p = s->piece;
		if (p != NULL or light_en_passant_check(X - 1, Y))
			reachable_mask |= (1ull << ((X - 1) | (Y << 3)));
	}
	
	if (X < 7) {
		s = &board[X + 1][Y];
		p = s->piece;
		if (p != NULL or light_en_passant_check(X + 1, Y))
			reachable_mask |= (1ull << ((X + 1) | (Y << 3)));
	}

	p = board[X][Y].piece;
	if (p == NULL) {
		reachable_mask |= (1ull << (X | (Y << 3)));
		if (not has_already_move) {
			Y += color;
			p = board[X][Y].piece;
			if (p == NULL) {
				reachable_mask |= (1ull << (X | (Y << 3)));
			}
		}
	}

	reachable_mask &= ~board.color_mask[color];
}

bool Pawn::base_can_move_to(Square const& target_square) const {
	if (can_en_passant(target_square))
		return true;

	if (not Piece::base_can_move_to(target_square))
		return false;

	Piece* target = target_square.piece;

	int dx = target_square.x - x;
	int dy = target_square.y - y;

	if (target == NULL) {
		if (dx != 0)
			return false; // can only move forward
		if (dy == color)
			return true; // just a simple step
		else if (dy == 2*color)
			return (not has_already_move) and board[x][y + color].piece == NULL; // check for a double step
		return false; // nor a simple step nor a double step
	}
	else {
		if (target->color == color)
			return false; // cannot move on a friendly piece
		else
			return base_do_control(target_square); // check if you try to move diagonaly
	}
}

bool Pawn::can_double_step(Square const& target) const {
	int dy = target.y - y;
	int dx = target.x - x;
	return 
		(dx == 0) and 
		(dy == 2 * color) and 
		(not has_already_move) and 
		board[x][y + color].piece == NULL and 
		board[x][target.y].piece == NULL
	;
}

// assumes the move is already diagonal and the targeted square is empty
bool Pawn::light_en_passant_check(short X, short Y) const {
	move_data const last_move_data = board.get_last_nonduck_move();

	if (last_move_data.attacker == NULL or
		last_move_data.cancel or
		last_move_data.attacker->color == color or
		last_move_data.attacker->Class != Pawn::cls)
		return false;

	if (board[X][y].piece != last_move_data.attacker) {
		return false;
	}
	return ABS_INT(last_move_data.target_square->y - last_move_data.begin_square->y) == 2;
}

bool Pawn::can_en_passant(Square const& target_square) const {
	move_data const last_move_data = board.get_last_nonduck_move();

	if (last_move_data.attacker == NULL or
		last_move_data.cancel or
		last_move_data.attacker->color == color or 
		last_move_data.attacker->Class != Pawn::cls)
		return false; // you can only en passant if the last piece moved is an enemy pawn
	
	if (not base_do_control(target_square)) 
		return false; // you can only en passant in diagonal
	Piece* target_piece = board[target_square.x][y].piece;
	if (target_piece != last_move_data.attacker)
		return false; // the last piece moved needs to be your target
	if (ABS_INT(last_move_data.target_square->y - last_move_data.begin_square->y) != 2)
		return false; // the opposing pawn must have perform a double step
	
	// must check wether the move wille put our king in danger

	if (board.with_check) {
		// simulate the move, the modification to the board will need to be cancel later
		Square* begin_pos = square;
		square->piece = NULL;
		const_cast<Square&>(target_square).piece = const_cast<Pawn*>(this);
		const_cast<Square*&>(square) = &const_cast<Square&>(target_square);
		board.last_move_data.target_square->piece = NULL;

		bool in_check = false;
		for (King* king : board.king_list[board.active_player]) {
			if (king->is_in_check()) {
				in_check = true;
				break;
			}
		}

		// cancel the simulated move
		const_cast<Square*&>(square) = begin_pos;
		const_cast<Square*>(square)->piece = const_cast<Pawn*>(this);
		const_cast<Square&>(target_square).piece = NULL;
		board.last_move_data.target_square->piece = target_piece;

		return not in_check;
	}
	else {
		return true;
	}	
}

auto Pawn::base_move_to(Square& target_square) -> move_data {
	move_data data;

	if (can_en_passant(target_square)) {
		data.en_passant = true;

		move_data const last_move_data = board.get_last_nonduck_move();

		data.set_type_matchup_data(this, last_move_data.attacker, &target_square);
		if (not data.cancel) {


			last_move_data.target_square->to_graveyard();

			if (data.suicide) {
				square->to_graveyard();
			}
			else {
				set_square(&target_square);
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

	return data;
}


Knight::Knight(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem item) : Piece(board_, Knight::cls, color_, sq, type_, item) {
}

bool Knight::base_do_control(Square const& target_square) const {
	int dx = target_square.x - x;
	int dy = target_square.y - y;
	return dx * dx + dy * dy == 5;
}

void Knight::set_reachable() {
	reachable_mask = 0;
	short const X = x, const Y = y;
	
	Uint64 const small = (Uint64)(X <= 6) << 2ull | (X >= 1);
	Uint64 const large = (Uint64)(X <= 5) << 4ull | (X >= 2);

	if (Y < 7) {
		reachable_mask |= large << ((X - 2) + ((Y + 1) << 3));
		if (Y < 6) {
			reachable_mask |= small << ((X - 1) + ((Y + 2) << 3));
		}
	}

	if (Y > 0) {
		reachable_mask |= large << ((X - 2) + ((Y - 1) << 3));
		if (Y > 1) {
			reachable_mask |= small << ((X - 1) + ((Y - 2) << 3));
		}
	}

	reachable_mask &= ~board.color_mask[color];
}

bool Knight::base_can_move_to(Square const& target_square) const {
	return Piece::base_can_move_to(target_square) and base_do_control(target_square);
}

Rook::Rook(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem item) : Piece(board_, Rook::cls, color_, sq, type_, item) {
}

bool Rook::base_can_move_to(Square const& target_square) const {
	return Piece::base_can_move_to(target_square) and base_do_control(target_square);
}

void Rook::set_reachable() {
	reachable_mask = 0;
	
	short X = x - 1, Y = y;

	while (X >= 0 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		X--;
	}

	if (X >= 0) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	X = x + 1, Y = y;

	while (X < 8 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		X++;
	}

	if (X < 8) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	X = x, Y = y - 1;

	while (Y >= 0 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		Y--;
	}

	if (Y >= 0) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	X = x, Y = y + 1;

	while (Y < 8 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		Y++;
	}

	if (Y < 8) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	reachable_mask &= ~board.color_mask[color];
}

bool Rook::base_do_control(Square const& target_square) const {
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

Bishop::Bishop(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem item) : Piece(board_, Bishop::cls, color_, sq, type_, item) {
}


bool Bishop::base_do_control(Square const& target_square) const {
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

void Bishop::set_reachable() {
	reachable_mask = 0;

	short X = x - 1, Y = y - 1;

	while (X >= 0 and Y >= 0 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		X--;
		Y--;
	}

	if (X >= 0 and Y >= 0) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	X = x + 1, Y = y + 1;

	while (X < 8 and Y < 8 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		X++;
		Y++;
	}

	if (X < 8 and Y < 8) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	X = x + 1, Y = y - 1;

	while (X < 8 and Y >= 0 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		X++;
		Y--;
	}

	if (X < 8 and Y >= 0) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	X = x - 1, Y = y + 1;

	while (X >= 0 and Y < 8 and board[X][Y].piece == NULL) {
		reachable_mask |= 1ull << (X | (Y << 3));
		X--;
		Y++;
	}

	if (X >= 0 and Y < 8) {
		reachable_mask |= 1ull << (X | (Y << 3));
	}

	reachable_mask &= ~board.color_mask[color];
}


bool Bishop::base_can_move_to(Square const& target_square) const {
	return Piece::base_can_move_to(target_square) and this->base_do_control(target_square);
}

Queen::Queen(Board& board_, piece_color color_, Square* sq, typing type_, PokeItem item) :
	Bishop(board_, color_, sq, type_, item), Rook(board_, color_, sq, type_, item), Piece(board_, Queen::cls, color_, sq, type_, item)
{
}

void Queen::set_reachable() {
	Rook::set_reachable();
	Uint64 save = reachable_mask;
	Bishop::set_reachable();
	reachable_mask |= save;
}

inline bool Queen::base_do_control(Square const& target_square) const {
	return Rook::base_do_control(target_square) or Bishop::base_do_control(target_square);
}

bool Queen::base_can_move_to(Square const& target_square) const {
	return Piece::base_can_move_to(target_square) and base_do_control(target_square);
}


bool King::can_castle(Square const& target_square, Square** rook_pos) const {
	int dx = target_square.x - x;

	if (board.with_antichess)
		return false;

	if (has_already_move or target_square.y != y) // cannot castle if the king already moved, or if it's not on the same row
		return false;

	if (board.with_check and board[x][y].is_controlled_by(not color)) // cannot castle out of check
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
				if (p->Class == Rook::cls and p->color == color and not p->has_already_move) { // a rook is avaible to castle with
					if (rook_pos != NULL)
						*rook_pos = &intermediate_square;
					return true;
				}
				return false; // cannot castle through piece
			}

			if (board.with_check and intermediate_square.is_controlled_by(not color)) // cannot castle through check
				return false;

			x_temp += x_step;
		}
		return false; // the while loop ended up looking outside of the board
	}
	else if (target_square.piece->Class == Rook::cls and target_square.piece->color == color and not target_square.piece->has_already_move) { // check for a castle when you click directly on a rook
		
		if (rook_pos != NULL)
			*rook_pos = &const_cast<Square&>(target_square);

		Piece* rook = target_square.piece;
		for (; x_temp != rook->x; x_temp += x_step) {
			Square& intermediate_square = board[x_temp][y];
			if (intermediate_square.piece != NULL or (board.with_check and intermediate_square.is_controlled_by(not color))) // cannot castle through a piece/through check
				return false;
		}
		return true; // there isn't any obstacle and the castling path is safe
	}
	else
		return false; // cannot castle onto another piece
}



bool King::is_in_check(bool base_rule) {
	return square->is_controlled_by(not color);
}


Duck::Duck(Board& b, Square* sq) : Piece(b, Duck::cls, no_color, sq, typeless, PokeItem()) {
	// sprite = psyduck_sprite;
	board.duck = this;
}

bool Duck::base_can_move_to(Square& target) {
	return target.piece == NULL;
}

void Duck::draw(Surface dest, SDL_Rect* dest_rect) {
	if (board.active_player == no_color)
		dest.blit(psyduck_active_sprite, dest_rect);
	else
		dest.blit(psyduck_sprite, dest_rect);
}

void Duck::set_reachable() {
	reachable_mask = -1;

	reachable_mask &= ~board.presence_mask;
}


PieceClass* piece_class_tab[7] = { King::cls, Queen::cls, Bishop::cls, Knight::cls, Bishop::cls, Pawn::cls, Duck::cls };