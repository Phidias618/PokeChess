#include "piece2.h"

constexpr int ABS_INT(int x) {
	return (x >= 0) ? x : -x;
}



PokemonIcon::PokemonIcon(Piece const* const piece) {
	x = 1 + 2 * ((int)piece->id + piece->evolved * 5) + (1 - piece->color) / 2;
	y = (short)piece->base_type + 1;
	if (y == 0) {
		x = y = -1;
	}
}

void PokemonIcon::promote(Piece* piece) {
	switch (sprite_column_type[x]) {
	case sprite_column_type_enum::promotion_no_king_padding:
		x += 2 * (int)piece->id;
		break;
	case sprite_column_type_enum::promotion_singleton:
		x += 2;
		break;
	case sprite_column_type_enum::promotion_with_king_padding:
		x += 2 * (int)piece->id + 2;
		break;
	default:
		throw;
	}
}

Piece::Piece() {
	memset(this, 0, sizeof(Piece));
	id = piece_id::none;
	type = base_type = typeless;
	in_limbo = true;
}

Piece::~Piece() {
	;
}

Piece::Piece(piece_id _id, piece_color _color, typing _type, PokeItem _item) {
	memset(this, 0, sizeof(Piece));
	id = _id;
	color = _color;
	base_type = type = _type;
	item = _item;
	is_piece = true;
	in_limbo = true;
}

void Piece::draw(Board const& board, Surface dest, SDL_Rect* dest_rect) const {
	SDL_Rect pos = { 0, 0, 0, 0 };
	if (not is_piece)
		return;
	if (id == piece_id::duck) {
		dest.blit(board.active_player == no_color ? psyduck_active_sprite : psyduck_sprite, dest_rect);
	}
	else {
		SDL_Rect area(TILE_SIZE * (int)id, TILE_SIZE * ((1 - color) / 2), TILE_SIZE, TILE_SIZE);

		if (dest_rect != NULL) {
			pos.x = dest_rect->x;
			pos.y = dest_rect->y;
		}
		else {
			pos.x = pos.y = 0;
		}

		dest.blit(sprite_sheet, &pos, &area);
	}
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
				if (p1[X] != background_color) {
					float mix_factor = (float)(p2[X] & 0xff) / 255.f;
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
					Uint32 col = p1[X];
					col = col;
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


Uint64 Piece::get_reachable(Board const& board) const {
	Uint64 reachable_mask;

	switch (id) {
	case piece_id::duck:
		reachable_mask = ~board.presence_mask;
		break;
	case piece_id::king: {
		reachable_mask = king_attack_patern[pos];

		reachable_mask &= ~board.color_mask[color];

		if (not has_already_move) {
			Sint8 rook_pos;
			int x = pos & 0b111;
			if (x <= 6 and light_can_castle(board, pos + 2, &rook_pos)) {
				reachable_mask |= 1ull << (pos + 2);
				reachable_mask |= 1ull << rook_pos;
			}
			if (x >= 2 and light_can_castle(board, pos - 2, &rook_pos)) {
				reachable_mask |= 1ull << (pos - 2);
				reachable_mask |= 1ull << rook_pos;
			}
		}
		break;
	}
	case piece_id::pawn: {
		reachable_mask = 0;
		Sint8 const step = 8 * color;
		reachable_mask = get_square_mask(pos + step);
		if ((not has_already_move) and not (board.presence_mask & get_square_mask(pos + step))) {
			reachable_mask |= get_square_mask(pos + 2 * step);
		}
		reachable_mask &= ~board.presence_mask;

		reachable_mask |= (pawn_capture_patern[color][pos] & (board.presence_mask | board.avaible_en_passant_mask));
		
		reachable_mask &= ~board.color_mask[color];
		break;
	}
	case piece_id::knight:
		reachable_mask = knight_attack_patern[pos];
		reachable_mask &= ~board.color_mask[color];
		break;
	case piece_id::bishop:
		
		reachable_mask = get_bishop_attack(pos, board.presence_mask);
		reachable_mask &= ~board.color_mask[color];
		break;
	case piece_id::rook:
		reachable_mask = get_rook_attack(pos, board.presence_mask);
		reachable_mask &= ~board.color_mask[color];
		break;

	case piece_id::queen:
		reachable_mask = get_rook_attack(pos, board.presence_mask);
		reachable_mask |= get_bishop_attack(pos, board.presence_mask);
		reachable_mask &= ~board.color_mask[color];
		break;
	case piece_id::amazon:
		reachable_mask = get_rook_attack(pos, board.presence_mask);
		reachable_mask |= get_bishop_attack(pos, board.presence_mask);
		reachable_mask |= knight_attack_patern[pos];
		reachable_mask &= ~board.color_mask[color];
		break;
	case piece_id::empress:
		reachable_mask = get_rook_attack(pos, board.presence_mask);
		reachable_mask |= knight_attack_patern[pos];
		reachable_mask &= ~board.color_mask[color];
		break;
	case piece_id::princess:
		reachable_mask = get_bishop_attack(pos, board.presence_mask);
		reachable_mask |= knight_attack_patern[pos];
		reachable_mask &= ~board.color_mask[color];
		break;
	}
	
	return reachable_mask;
}

// assume there is not vertical movement
// assume horizontal movement is by 2 squares
// assume the piece is royal
// assume the piece has not already move
// does not check for check
bool Piece::light_can_castle(Board const& board, Sint8 target_pos, Sint8* rook_pos) const {
	if (board.with_antichess)
		return false;

	int const x_step = 2 * (target_pos > pos) - 1; 
	int ray_pos = pos + x_step;

	while ((ray_pos & ~0b111111) == 0 and 0 <= (ray_pos & 0b111) and (ray_pos & 0b111) < 8) {
		if (Piece* p = board[ray_pos].piece) {
			if (p->id == piece_id::rook and p->color == color and (not p->has_already_move)) { // a rook is avaible to castle with
				*rook_pos = p->pos;
				return true;
			}
			return false; // cannot castle through piece
		}
		ray_pos += x_step;
	}
	return false; // the while loop ended up looking outside of the board*/
}

// assume the movement is diagonal
// assume the piece is a pawn
// assume board[target_x][target_y] is empty

/*
bool Piece::light_can_en_passant(Board& board, Sint8 target_x, Sint8 target_y) {
	move_data const last_move_data = board.get_last_nonduck_move();

	if ((not last_move_data.attacker) or
		last_move_data.cancel or
		last_move_data.attacker->color == color or
		last_move_data.attacker->id != piece_id::pawn)
		return false;

	return board.last_move_data.end_x == target_x and board.last_move_data.end_y == y and ABS_INT(last_move_data.end_y - last_move_data.begin_y) == 2;
}


bool Piece::can_en_passant(Board& board, Sint8 target_x, Sint8 target_y) {
	return target_y - y == color and ABS_INT(x - target_x) == 1 and (not board[target_x][target_y].piece) and light_can_en_passant(board, target_x, target_y);
}
*/

bool Piece::can_castle(Board const& board, Sint8 target_pos, Sint8* rook_pos) const {
	if (id != piece_id::king or has_already_move or (pos >> 3) != (target_pos >> 3)) {
		return false;
	}
	if ((ABS_INT(target_pos - pos) == 2 and light_can_castle(board, target_pos, rook_pos)))
		return true;

	if ((board.piece_mask[(int)piece_id::rook] & board.color_mask[color] & get_square_mask(target_pos) & board.reachable_mask_array[pos])) {
		*rook_pos = target_pos;
		return true;
	}
	return false;
}

void Piece::tera(Board& board, typing new_type) {
#if ENABLE_SAFETY_CHECKS
	if (not board.is_tera_avaible[color]) {
		PRINT_DEBUG("unauthorized tera");
		throw;
	}
#endif
	board.is_tera_avaible[color] = false;
	is_tera = true;

	Uint64 const pos_mask = 1ull << pos;
	
	board.type_mask[type] &= ~pos_mask;
	board.type_mask[new_type] |= pos_mask;

	type = new_type;
}

void Piece::un_tera(Board& board) {
#if ENABLE_SAFETY_CHECKS
	if (not is_tera) {
		PRINT_DEBUG("Invalid un_tera");
		throw;
	}
#endif

	Uint64 const pos_mask = 1ull << pos;
	board.type_mask[type] &= ~pos_mask;

	board.type_mask[base_type] |= pos_mask;

	type = base_type;
	is_tera = false;
	board.is_tera_avaible[color] = true;
}


void Piece::set_square(Board& board, bool limbo, Sint8 new_pos) {
	Uint64 const remove_mask = ~get_square_mask(in_limbo, pos);


	board.presence_mask &= remove_mask;
	board.color_mask[color] &= remove_mask;
	board.piece_mask[(int)id] &= remove_mask;
	board.type_mask[type] &= remove_mask;
	
	if (not in_limbo)
		board[pos].piece = NULL;
	
	if (is_dead and not limbo) {
		is_dead = in_limbo = false;
	}
	item.move_with(board, this, in_limbo, pos, limbo, new_pos);
	
	in_limbo = limbo;
	pos = new_pos;

	if (not limbo) {
		board[pos].piece = this;
	}

	Uint64 const add_mask = get_square_mask(in_limbo, pos);

	board.presence_mask |= add_mask;
	board.color_mask[color] |= add_mask;
	board.piece_mask[(int)id] |= add_mask;
	board.type_mask[type] |= add_mask;
}


void Piece::to_limbo(Board& board) {
	Uint64 const remove_mask = ~get_square_mask(pos);

	board.presence_mask &= remove_mask;
	board.color_mask[color] &= remove_mask;
	board.piece_mask[(int)id] &= remove_mask;
	board.type_mask[type] &= remove_mask;

	board[pos].piece = NULL;

	in_limbo = true;
}

void Piece::revive(Board& board) {
	is_dead = in_limbo = false;
	Uint64 const add_mask = get_square_mask(in_limbo, pos);

	board.presence_mask |= add_mask;
	board.color_mask[color] |= add_mask;
	board.piece_mask[(int)id] |= add_mask;
	board.type_mask[type] |= add_mask;
	board[pos].piece = this;
}

char const* Piece::get_name(LANGUAGE language) const {
	static char const* piece_name[(int)piece_id::__nb_of_id__][NB_OF_LANGUAGE] = {
		{ "Roi", "King", "Konig", "Rey", "Re" },
		{ "Dame", "Queen", "Dame", "Dama", "Donna", },
		{ "Fou", "Bishop", "Laufer", "Alfil", "Alfiere" },
		{ "Cavalier", "Knight", "Springer", "Caballo", "Cavallo", },
		{ "Tour", "Rook", "Turm", "Torre", "Torre", },
		{ "Pion", "Pawn", "Bauer", "Peon", "Pedone", },
		{ "Psykokwak", "Psyduck", "Enton", "Psyduck", "Psyduck", },
		{ "Amazone", "Amazon", "", "", "", },
		{ "Princesse", "Princess", "", "", "", },
		{ "Imperatrice", "Empress", "", "", "", },
	};

	return piece_name[(int)id][(int)language];
}

move_data Piece::base_move_to(Board& board, Sint8 new_pos) {
	move_data data;
	if (can_castle(board, new_pos, &new_pos)) {
		data.castling = true;
		
		int x_step = (new_pos > pos) ? 1 : -1;

		data.end_pos = pos + 2 * x_step;

		data.target_pos = new_pos;
		
		data.begin_pos = pos;

		data.attacker = this;
		data.attacker_copy = self;

		board[new_pos].piece->set_square(board, false, pos + x_step);
		set_square(board, false, pos + 2 * x_step);
	}
	else if (id == piece_id::pawn and board.avaible_en_passant_mask & get_square_mask(new_pos)) {
		data.en_passant = true;
		move_data const& last = board.get_last_nonduck_move();
		data.set_type_matchup_data(board, this, board[last.end_pos].piece, new_pos);
		if (data.end_pos != pos) {
			board[last.end_pos].to_graveyard(board);
			if (data.suicide) {
				board[pos].to_graveyard(board);
			}
			else {
				set_square(board, false, data.end_pos);
			}
		}
		
	}
	else {
		// regular move
		data.set_type_matchup_data(board, this, board[new_pos].piece, new_pos);

		if (data.end_pos == pos)
			;
		else if (data.suicide) {
			board[data.end_pos].to_graveyard(board);
			board[pos].to_graveyard(board);
		}
		else {
			board[data.end_pos].to_graveyard(board);
			set_square(board, false, data.end_pos);
		}
	}
	if (data.suicide)
		return data;

	has_already_move = true;

	data.init_promotion = (id == piece_id::pawn and (promotion_rank[color] == (pos >> 3)));
	data.interrupt_move = data.init_promotion;

	return data;
}

move_data Piece::move_to(Board& board, Sint8 new_pos) {
	move_data data;

	if ((item.get_reachable_and_banned(board, this, get_reachable(board)).first & get_square_mask(in_limbo, new_pos)) == 0) {
		data = base_move_to(board, new_pos);
	}
	else {
		data = item.move_to(board, this, new_pos);
	}
	
	if ((not data.defender) or not data.defender->item.is_safety_google()) {
		data.attacker->item.after_move_effects(board, data);
	}

	if (data.defender and not data.attacker->item.is_protective_pads()) {
		data.defender->item.revenge(board, data.defender, data);
	}

	return data;
}

void Piece::set_type(Board& board, typing new_type) {
	Uint64 const pos_mask = get_square_mask(in_limbo, pos);
	board.type_mask[type] &= ~pos_mask;
	base_type = type = new_type;
	board.type_mask[type] |= pos_mask;

	set_pokeicon(PokemonIcon(this));
}

void Piece::set_item(Board& board, PokeItem new_item) {
	item.remove_from(board, this);
	item = new_item;
	item.put_on(board, this);
}

void Piece::to_graveyard(Board& board) {
	item.to_graveyard(board, this);
	to_limbo(board);
	is_dead = true;
}



Piece const NO_PIECE;
color_array<int> promotion_rank = std::array<int, 3>({0, 1237, 7});


void move_data::set_type_matchup_data(Board& board, Piece* _attacker, Piece* _defender, Sint8 _target_pos) {
	attacker = _attacker;
	defender = _defender;
	begin_pos = attacker->pos;
	target_pos = _target_pos;

	attacker_copy = *attacker;
	if (defender != NULL)
		defender_copy = *defender;

	end_pos = target_pos;

	miss_rate = board.miss_rate;
	crit_rate = board.crit_rate;

	was_piece_first_move = not attacker->has_already_move;

	if (not defender)
		return;

	matchup = get_effectiveness(attacker->type, defender->type, board.with_reversed_typechart);

	if (board.with_ability) {
		if ((attacker->type == fire and defender->ability_torch) or
			(attacker->type == water and defender->ability_water_absorb) or
			(attacker->type == grass and defender->ability_sap_sipper) or
			(attacker->type == electric and defender->ability_volt_absorb))
		{
			matchup = immune;
			is_immune = true;
		}
	}
	if (board.with_items) {
		if (not defender->item.is_safety_google()) {
			if (not attacker->item.is_protective_pads()) {
				if (attacker->item.priority() >= defender->item.priority()) {
					attacker->item.attack_modifier(board, self);
					defender->item.defense_modifier(board, self);

					attacker->item.crit_modifier(board, self);

					attacker->item.accuracy_modifier(board, self);
					defender->item.evasion_modifier(board, self);
				}
				else {
					defender->item.defense_modifier(board, self);
					attacker->item.attack_modifier(board, self);

					attacker->item.crit_modifier(board, self);

					defender->item.evasion_modifier(board, self);
					attacker->item.accuracy_modifier(board, self);
				}
			}
			else {
				attacker->item.attack_modifier(board, self);
				attacker->item.crit_modifier(board, self);
				attacker->item.accuracy_modifier(board, self);
			}
		}
		else {
			if (not attacker->item.is_protective_pads()) {
				defender->item.defense_modifier(board, self);
				defender->item.evasion_modifier(board, self);
			}
		}
	}

	if (matchup.main == immune) {
		is_immune = true;
		// cancel = true;
		end_pos = begin_pos;
		return;
	}

	if (board.with_RNG and ((not board.with_ability) or (not defender->ability_shell_armor)) and
		game.RNG() < crit_rate * game.RNG.max()) {
		do_crit = true;
		move_again = true;
	}
	else if (board.with_RNG and  ((not board.with_ability) or not (attacker->ability_no_guard or defender->ability_no_guard or attacker->ability_compound_eye)) and
		 game.RNG() < miss_rate * game.RNG.max()) {
		do_miss = true;
		// cancel = true;
		end_pos = begin_pos;
	}
	else {
		suicide = is_not_very_effective = (matchup.main == not_very_effective);
		move_again = is_super_effective = (matchup.main == super_effective);
	}
}


std::ostream& operator<<(std::ostream& os, move_data const& data) {
	if (data.attacker != NULL) {
		os << "attacker: " << data.attacker->get_name(LANGUAGE::ENGLISH) << " at (" << (int)(data.begin_pos & 0b111) << ", " << (int)(data.begin_pos >> 3) << ")\n";
	}
	else {
		os << "attacker: None\n";
	}
	os << "target: (" << (int)(data.target_pos & 0b111) << ", " << (int)(data.target_pos >> 3) << ")\n";

	os << "defender: " << ((data.defender == NULL) ? "None" : data.defender->get_name(LANGUAGE::ENGLISH)) << '\n';
	os << '\n';
	if (data.castling)
		os << "castling\n";
	/*if (data.cancel)
		os << "canceled\n";
	*/
	if (data.move_again)
		os << "bonus move\n";
	if (data.init_promotion)
		os << "init_promotion\n";
	if (data.end_promotion) {
		os << "promotion to " << data.attacker->get_name(LANGUAGE::ENGLISH) << '\n';
	}
	if (data.en_passant)
		os << "en passant\n";
	if (data.skip_bonus_turn)
		os << "skip\n";
	if (data.tera)
		os << "tera into " << type_str[data.attacker->type][1] << '\n';
	os << "---------------------\n";
	return os;
}



Uint64 sliding_attack_lookup[56320];


MagicBitboardData const rook_magic[64] = {
	{ 0x000101010101017eull, 0x01010101010101feull, 0x0280004002816010ull,  2048, 52 },
	{ 0x000202020202027cull, 0x02020202020202fdull, 0x0840100420044000ull, 13312, 53 },
	{ 0x000404040404047aull, 0x04040404040404fbull, 0x0100090044102001ull, 19456, 53 },
	{ 0x0008080808080876ull, 0x08080808080808f7ull, 0x0880080180100004ull, 34816, 53 },
	{ 0x001010101010106eull, 0x10101010101010efull, 0x0300040800500300ull, 37888, 53 },
	{ 0x002020202020205eull, 0x20202020202020dfull, 0x2080018012005400ull, 23552, 53 },
	{ 0x004040404040403eull, 0x40404040404040bfull, 0x4280048006000100ull, 16384, 53 },
	{ 0x008080808080807eull, 0x808080808080807full, 0x0100120640822100ull,  8192, 52 },

	{ 0x0001010101017e00ull, 0x010101010101fe01ull, 0x071d802040028000ull, 13312, 53 },
	{ 0x0002020202027c00ull, 0x020202020202fd02ull, 0x2020400020005000ull,  2048, 54 },
	{ 0x0004040404047a00ull, 0x040404040404fb04ull, 0x5800801000802002ull, 34816, 54 },
	{ 0x0008080808087600ull, 0x080808080808f708ull, 0x0001002010000900ull, 19456, 54 },
	{ 0x0010101010106e00ull, 0x101010101010ef10ull, 0x8010800800040281ull, 23552, 54 },
	{ 0x0020202020205e00ull, 0x202020202020df20ull, 0x0009800201140080ull, 37888, 54 },
	{ 0x0040404040403e00ull, 0x404040404040bf40ull, 0x0004800100020080ull,  8192, 54 },
	{ 0x0080808080807e00ull, 0x8080808080807f80ull, 0x8018800100005080ull, 16384, 53 },

	{ 0x00010101017e0100ull, 0x0101010101fe0101ull, 0x0020008080004000ull, 31744, 53 },
	{ 0x00020202027c0200ull, 0x0202020202fd0202ull, 0x0000464002201001ull, 46080, 54 },
	{ 0x00040404047a0400ull, 0x0404040404fb0404ull, 0x0203010012442000ull, 49152, 54 },
	{ 0x0008080808760800ull, 0x0808080808f70808ull, 0x1001010008100420ull, 53248, 54 },
	{ 0x00101010106e1000ull, 0x1010101010ef1010ull, 0x8000808008000400ull, 55296, 54 },
	{ 0x00202020205e2000ull, 0x2020202020df2020ull, 0x5240818004000e00ull, 51200, 54 },
	{ 0x00404040403e4000ull, 0x4040404040bf4040ull, 0x0050040001029008ull, 44032, 54 },
	{ 0x00808080807e8000ull, 0x80808080807f8080ull, 0x0040020001108044ull, 29696, 53 },

	{ 0x000101017e010100ull, 0x01010101fe010101ull, 0x1140004180008028ull, 46080, 53 },
	{ 0x000202027c020200ull, 0x02020202fd020202ull, 0x00104000c0201000ull, 31744, 54 },
	{ 0x000404047a040400ull, 0x04040404fb040404ull, 0x0020001010040200ull, 53248, 54 },
	{ 0x0008080876080800ull, 0x08080808f7080808ull, 0x0412080080100280ull, 49152, 54 },
	{ 0x001010106e101000ull, 0x10101010ef101010ull, 0x0021018500080010ull, 51200, 54 },
	{ 0x002020205e202000ull, 0x20202020df202020ull, 0x1004010040020040ull, 55296, 54 },
	{ 0x004040403e404000ull, 0x40404040bf404040ull, 0x0a14108400020801ull, 29696, 54 },
	{ 0x008080807e808000ull, 0x808080807f808080ull, 0x0110802080084500ull, 44032, 53 },

	{ 0x0001017e01010100ull, 0x010101fe01010101ull, 0x0402008102002044ull, 41984, 53 },
	{ 0x0002027c02020200ull, 0x020202fd02020202ull, 0x0000806001804004ull, 27648, 54 },
	{ 0x0004047a04040400ull, 0x040404fb04040404ull, 0x4301002001001040ull, 52224, 54 },
	{ 0x0008087608080800ull, 0x080808f708080808ull, 0x4520828804801000ull, 48128, 54 },
	{ 0x0010106e10101000ull, 0x101010ef10101010ull, 0x0500040080800802ull, 50176, 54 },
	{ 0x0020205e20202000ull, 0x202020df20202020ull, 0x8021800401800200ull, 54272, 54 },
	{ 0x0040403e40404000ull, 0x404040bf40404040ull, 0x2001000405001200ull, 25600, 54 },
	{ 0x0080807e80808000ull, 0x8080807f80808080ull, 0x1101040042000081ull, 39936, 53 },

	{ 0x00017e0101010100ull, 0x0101fe0101010101ull, 0x4005804004658000ull, 27648, 53 },
	{ 0x00027c0202020200ull, 0x0202fd0202020202ull, 0x0002002100820041ull, 41984, 54 },
	{ 0x00047a0404040400ull, 0x0404fb0404040404ull, 0x0801001020010040ull, 48128, 54 },
	{ 0x0008760808080800ull, 0x0808f70808080808ull, 0x0009000810010020ull, 52224, 54 },
	{ 0x00106e1010101000ull, 0x1010ef1010101010ull, 0x0114008008008004ull, 54272, 54 },
	{ 0x00205e2020202000ull, 0x2020df2020202020ull, 0x2482001004020008ull, 50176, 54 },
	{ 0x00403e4040404000ull, 0x4040bf4040404040ull, 0x6801221008040041ull, 39936, 54 },
	{ 0x00807e8080808000ull, 0x80807f8080808080ull, 0x00000c8441220004ull, 25600, 53 },

	{ 0x007e010101010100ull, 0x01fe010101010101ull, 0x48fffe99fecfaa00ull, 12288, 54 },
	{ 0x007c020202020200ull, 0x02fd020202020202ull, 0x48fffe99fecfaa00ull,     0, 55 },
	{ 0x007a040404040400ull, 0x04fb040404040404ull, 0x497fffadff9c2e00ull, 33792, 55 },
	{ 0x0076080808080800ull, 0x08f7080808080808ull, 0x613fffddffce9200ull, 18432, 55 },
	{ 0x006e101010101000ull, 0x10ef101010101010ull, 0xffffffe9ffe7ce00ull, 21504, 55 },
	{ 0x005e202020202000ull, 0x20df202020202020ull, 0xfffffff5fff3e600ull, 36864, 55 },
	{ 0x003e404040404000ull, 0x40bf404040404040ull, 0x0003ff95e5e6a4c0ull,  6144, 55 },
	{ 0x007e808080808000ull, 0x807f808080808080ull, 0x510ffff5f63c96a0ull, 15360, 54 },

	{ 0x7e01010101010100ull, 0xfe01010101010101ull, 0xebffffb9ff9fc526ull,     0, 53 },
	{ 0x7c02020202020200ull, 0xfd02020202020202ull, 0x61fffeddfeedaeaeull, 12288, 54 },
	{ 0x7a04040404040400ull, 0xfb04040404040404ull, 0x53bfffedffdeb1a2ull, 18432, 54 },
	{ 0x7608080808080800ull, 0xf708080808080808ull, 0x127fffb9ffdfb5f6ull, 33792, 54 },
	{ 0x6e10101010101000ull, 0xef10101010101010ull, 0x411fffddffdbf4d6ull, 36864, 54 },
	{ 0x5e20202020202000ull, 0xdf20202020202020ull, 0x8002000110080402ull, 21504, 53 },
	{ 0x3e40404040404000ull, 0xbf40404040404040ull, 0x0003ffef27eebe74ull, 15360, 54 },
	{ 0x7e80808080808000ull, 0x7f80808080808080ull, 0x7645fffecbfea79eull,  6144, 53 },

};

MagicBitboardData const bishop_magic[64] = {
	{ 0x0040201008040200ull, 0x8040201008040200ull, 0xffedf9fd7cfcffffull,  6288, 59 },
	{ 0x0000402010080400ull, 0x0080402010080500ull, 0xfc0962854a77f576ull,  2048, 60 },
	{ 0x0000004020100a00ull, 0x0000804020110a00ull, 0x2812089401090000ull, 25600, 59 },
	{ 0x0000000040221400ull, 0x0000008041221400ull, 0x0008060042020044ull, 19456, 59 },
	{ 0x0000000002442800ull, 0x0000000182442800ull, 0x0002021101288880ull, 23552, 59 },
	{ 0x0000000204085000ull, 0x0000010204885000ull, 0x40120202200018c0ull, 27648, 59 },
	{ 0x0000020408102000ull, 0x000102040810a000ull, 0xfc0a66c64a7ef576ull,  8192, 60 },
	{ 0x0002040810204000ull, 0x0102040810204000ull, 0x7ffdfdfcbd79ffffull,   144, 59 },

	{ 0x0020100804020000ull, 0x4020100804020002ull, 0xfc0846a64a34fff6ull,  2048, 60 },
	{ 0x0040201008040000ull, 0x8040201008050005ull, 0xfc087a874a3cf7f6ull,  6272, 60 },
	{ 0x00004020100a0000ull, 0x00804020110a000aull, 0x402010708c810000ull, 19488, 59 },
	{ 0x0000004022140000ull, 0x0000804122140014ull, 0x0400c40404820043ull, 34816, 59 },
	{ 0x0000000244280000ull, 0x0000018244280028ull, 0x9281084840001184ull, 37888, 59 },
	{ 0x0000020408500000ull, 0x0001020488500050ull, 0x4400022820080802ull, 23584, 59 },
	{ 0x0002040810200000ull, 0x0102040810a000a0ull, 0xfc0864ae59b4ff76ull,   128, 60 },
	{ 0x0004081020400000ull, 0x0204081020400040ull, 0x3c0860af4b35ff76ull,  8192, 60 },

	{ 0x0010080402000200ull, 0x2010080402000204ull, 0x73c01af56cf4cffbull, 21504, 60 },
	{ 0x0020100804000400ull, 0x4020100805000508ull, 0x41a01cfad64aaffcull, 31776, 60 },
	{ 0x004020100a000a00ull, 0x804020110a000a11ull, 0x8408004112040011ull,  6144, 57 },
	{ 0x0000402214001400ull, 0x0080412214001422ull, 0x9800803802810003ull, 49280, 57 },
	{ 0x0000024428002800ull, 0x0001824428002844ull, 0x1002800400e00a02ull, 51328, 57 },
	{ 0x0002040850005000ull, 0x0102048850005088ull, 0x4422000922100200ull,     0, 57 },
	{ 0x0004081020002000ull, 0x02040810a000a010ull, 0x7c0c028f5b34ff76ull, 29728, 60 },
	{ 0x0008102040004000ull, 0x0408102040004020ull, 0xfc0a028e5ab4df76ull, 18432, 60 },

	{ 0x0008040200020400ull, 0x1008040200020408ull, 0x4920080004900440ull, 31744, 59 },
	{ 0x0010080400040800ull, 0x2010080500050810ull, 0x2009210010044100ull, 46080, 59 },
	{ 0x0020100a000a1000ull, 0x4020110a000a1120ull, 0x6802240212080200ull, 49152, 57 },
	{ 0x0040221400142200ull, 0x8041221400142241ull, 0x4804080300220040ull, 53248, 55 },
	{ 0x0002442800284400ull, 0x0182442800284482ull, 0x180404001041004cull, 55296, 55 },
	{ 0x0004085000500800ull, 0x0204885000508804ull, 0x8810012050c41004ull, 51200, 57 },
	{ 0x0008102000201000ull, 0x040810a000a01008ull, 0x00a08420420a1224ull, 44032, 59 },
	{ 0x0010204000402000ull, 0x0810204000402010ull, 0x0006020208c04641ull, 29696, 59 },

	{ 0x0004020002040800ull, 0x0804020002040810ull, 0x881004040010b108ull, 27648, 59 },
	{ 0x0008040004081000ull, 0x1008050005081020ull, 0x0201243004200922ull, 41984, 59 },
	{ 0x00100a000a102000ull, 0x20110a000a112040ull, 0x0002012200142800ull, 48128, 57 },
	{ 0x0022140014224000ull, 0x4122140014224180ull, 0xb040202020080080ull, 52224, 55 },
	{ 0x0044280028440200ull, 0x8244280028448201ull, 0x6040008020920020ull, 54272, 55 },
	{ 0x0008500050080400ull, 0x0488500050880402ull, 0x00a0408100028044ull, 50176, 57 },
	{ 0x0010200020100800ull, 0x0810a000a0100804ull, 0x2151080206030500ull, 39936, 59 },
	{ 0x0020400040201000ull, 0x1020400040201008ull, 0x1484010204602091ull, 25600, 59 },

	{ 0x0002000204081000ull, 0x0402000204081020ull, 0xdcefd9b54bfcc09full, 23552, 60 },
	{ 0x0004000408102000ull, 0x0805000508102040ull, 0xf95ffa765afd602bull, 27680, 60 },
	{ 0x000a000a10204000ull, 0x110a000a11204080ull, 0x8040084402201000ull,  8192, 57 },
	{ 0x0014001422400000ull, 0x2214001422418000ull, 0x042468a011140800ull, 48256, 57 },
	{ 0x0028002844020000ull, 0x4428002844820100ull, 0x0004081010400402ull, 50304, 57 },
	{ 0x0050005008040200ull, 0x8850005088040201ull, 0x4004212546000100ull,  2048, 57 },
	{ 0x0020002010080400ull, 0x10a000a010080402ull, 0x43ff9a5cf4ca0c01ull, 25632, 60 },
	{ 0x0040004020100800ull, 0x2040004020100804ull, 0x4bffcd8e7c587601ull, 19456, 60 },

	{ 0x0000020408102000ull, 0x0200020408102040ull, 0xfc0ff2865334f576ull,     0, 60 },
	{ 0x0000040810204000ull, 0x0500050810204080ull, 0xfc0bf6ce5924f576ull,  8320, 60 },
	{ 0x00000a1020400000ull, 0x0a000a1120408000ull, 0x80140c8088210400ull, 18464, 59 },
	{ 0x0000142240000000ull, 0x1400142241800000ull, 0x160400a084040100ull, 33792, 59 },
	{ 0x0000284402000000ull, 0x2800284482010000ull, 0x0040004059220009ull, 36864, 59 },
	{ 0x0000500804020000ull, 0x5000508804020100ull, 0x0a88041002120120ull, 21536, 59 },
	{ 0x0000201008040200ull, 0xa000a01008040201ull, 0xc3ffb7dc36ca8c89ull,  2176, 60 },
	{ 0x0000402010080400ull, 0x4000402010080402ull, 0xc3ff8a54f4ca2c89ull,  6144, 60 },

	{ 0x0002040810204000ull, 0x0002040810204080ull, 0xfffffcfcfd79edffull,  8336, 59 },
	{ 0x0004081020400000ull, 0x0005081020408000ull, 0xfc0863fccb147576ull,     0, 60 },
	{ 0x000a102040000000ull, 0x000a112040800000ull, 0x000204030080d002ull, 29696, 59 },
	{ 0x0014224000000000ull, 0x0014224180000000ull, 0xe028020000420880ull, 18432, 59 },
	{ 0x0028440200000000ull, 0x0028448201000000ull, 0x2086010010420220ull, 21504, 59 },
	{ 0x0050080402000000ull, 0x0050880402010000ull, 0x8094010404082208ull, 31744, 59 },
	{ 0x0020100804020000ull, 0x00a0100804020100ull, 0xfc087e8e4bb2f736ull,  6144, 60 },
	{ 0x0040201008040200ull, 0x0040201008040201ull, 0x43ff9e4ef4ca2c89ull,  2192, 59 },

};

static Uint64 get_rook_premask(int sq) {
	Uint64 result = 0ULL;
	int rk = sq / 8, fl = sq % 8, r, f;
	for (r = rk + 1; r <= 6; r++) result |= (1ULL << (fl + r * 8));
	for (r = rk - 1; r >= 1; r--) result |= (1ULL << (fl + r * 8));
	for (f = fl + 1; f <= 6; f++) result |= (1ULL << (f + rk * 8));
	for (f = fl - 1; f >= 1; f--) result |= (1ULL << (f + rk * 8));
	return result;
}

Uint64 get_bishop_premask(int sq) {
	Uint64 result = 0ULL;
	int rk = sq / 8, fl = sq % 8, r, f;
	for (r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) result |= (1ULL << (f + r * 8));
	for (r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) result |= (1ULL << (f + r * 8));
	for (r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) result |= (1ULL << (f + r * 8));
	for (r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) result |= (1ULL << (f + r * 8));
	return result;
}


void init_magic_attack() {
	memset(sliding_attack_lookup, 0, sizeof(sliding_attack_lookup));

	for (int pos = 0; pos < 64; pos++) {

		int bits = popcount64(rook_magic[pos].pre_mask);

		for (Uint16 j = 0; j < (1 << bits); j++) {
			Uint64 presence_mask = pdep64(j, rook_magic[pos].pre_mask);
			Uint64 key = ((presence_mask & rook_magic[pos].pre_mask) * rook_magic[pos].magic) >> rook_magic[pos].shift;
			(sliding_attack_lookup + rook_magic[pos].lookup_offset)[key] |= get_rook_movement(pos, presence_mask);
		}
		
		bits = popcount64(bishop_magic[pos].pre_mask);
		
		for (Uint16 j = 0; j < (1 << bits); j++) {
			Uint64 presence_mask = pdep64(j, bishop_magic[pos].pre_mask);
			Uint64 key = (presence_mask * bishop_magic[pos].magic) >> bishop_magic[pos].shift;
			(sliding_attack_lookup + bishop_magic[pos].lookup_offset)[key] |= get_bishop_movement(pos, presence_mask);
		}
	}

	return;
}



color_array<Uint64[64], true> const pawn_step_patern = std::array<Uint64[64], 2>({
	{ // black pawn
		0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
		0x0000000000000001ull, 0x0000000000000002ull, 0x0000000000000004ull, 0x0000000000000008ull, 0x0000000000000010ull, 0x0000000000000020ull, 0x0000000000000040ull, 0x0000000000000080ull,
		0x0000000000000100ull, 0x0000000000000200ull, 0x0000000000000400ull, 0x0000000000000800ull, 0x0000000000001000ull, 0x0000000000002000ull, 0x0000000000004000ull, 0x0000000000008000ull,
		0x0000000000010000ull, 0x0000000000020000ull, 0x0000000000040000ull, 0x0000000000080000ull, 0x0000000000100000ull, 0x0000000000200000ull, 0x0000000000400000ull, 0x0000000000800000ull,
		0x0000000001000000ull, 0x0000000002000000ull, 0x0000000004000000ull, 0x0000000008000000ull, 0x0000000010000000ull, 0x0000000020000000ull, 0x0000000040000000ull, 0x0000000080000000ull,
		0x0000000100000000ull, 0x0000000200000000ull, 0x0000000400000000ull, 0x0000000800000000ull, 0x0000001000000000ull, 0x0000002000000000ull, 0x0000004000000000ull, 0x0000008000000000ull,
		0x0000010000000000ull, 0x0000020000000000ull, 0x0000040000000000ull, 0x0000080000000000ull, 0x0000100000000000ull, 0x0000200000000000ull, 0x0000400000000000ull, 0x0000800000000000ull,
		0x0001000000000000ull, 0x0002000000000000ull, 0x0004000000000000ull, 0x0008000000000000ull, 0x0010000000000000ull, 0x0020000000000000ull, 0x0040000000000000ull, 0x0080000000000000ull,
	},
	{ // white pawn
		0x0000000000000100ull, 0x0000000000000200ull, 0x0000000000000400ull, 0x0000000000000800ull, 0x0000000000001000ull, 0x0000000000002000ull, 0x0000000000004000ull, 0x0000000000008000ull,
		0x0000000000010000ull, 0x0000000000020000ull, 0x0000000000040000ull, 0x0000000000080000ull, 0x0000000000100000ull, 0x0000000000200000ull, 0x0000000000400000ull, 0x0000000000800000ull,
		0x0000000001000000ull, 0x0000000002000000ull, 0x0000000004000000ull, 0x0000000008000000ull, 0x0000000010000000ull, 0x0000000020000000ull, 0x0000000040000000ull, 0x0000000080000000ull,
		0x0000000100000000ull, 0x0000000200000000ull, 0x0000000400000000ull, 0x0000000800000000ull, 0x0000001000000000ull, 0x0000002000000000ull, 0x0000004000000000ull, 0x0000008000000000ull,
		0x0000010000000000ull, 0x0000020000000000ull, 0x0000040000000000ull, 0x0000080000000000ull, 0x0000100000000000ull, 0x0000200000000000ull, 0x0000400000000000ull, 0x0000800000000000ull,
		0x0001000000000000ull, 0x0002000000000000ull, 0x0004000000000000ull, 0x0008000000000000ull, 0x0010000000000000ull, 0x0020000000000000ull, 0x0040000000000000ull, 0x0080000000000000ull,
		0x0100000000000000ull, 0x0200000000000000ull, 0x0400000000000000ull, 0x0800000000000000ull, 0x1000000000000000ull, 0x2000000000000000ull, 0x4000000000000000ull, 0x8000000000000000ull,
		0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
	}
	});
color_array<Uint64[64], true> const pawn_capture_patern = std::array<Uint64[64], 2>({
	{ // black pawn
		0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
		0x0000000000000002ull, 0x0000000000000005ull, 0x000000000000000aull, 0x0000000000000014ull, 0x0000000000000028ull, 0x0000000000000050ull, 0x00000000000000a0ull, 0x0000000000000040ull,
		0x0000000000000200ull, 0x0000000000000500ull, 0x0000000000000a00ull, 0x0000000000001400ull, 0x0000000000002800ull, 0x0000000000005000ull, 0x000000000000a000ull, 0x0000000000004000ull,
		0x0000000000020000ull, 0x0000000000050000ull, 0x00000000000a0000ull, 0x0000000000140000ull, 0x0000000000280000ull, 0x0000000000500000ull, 0x0000000000a00000ull, 0x0000000000400000ull,
		0x0000000002000000ull, 0x0000000005000000ull, 0x000000000a000000ull, 0x0000000014000000ull, 0x0000000028000000ull, 0x0000000050000000ull, 0x00000000a0000000ull, 0x0000000040000000ull,
		0x0000000200000000ull, 0x0000000500000000ull, 0x0000000a00000000ull, 0x0000001400000000ull, 0x0000002800000000ull, 0x0000005000000000ull, 0x000000a000000000ull, 0x0000004000000000ull,
		0x0000020000000000ull, 0x0000050000000000ull, 0x00000a0000000000ull, 0x0000140000000000ull, 0x0000280000000000ull, 0x0000500000000000ull, 0x0000a00000000000ull, 0x0000400000000000ull,
		0x0002000000000000ull, 0x0005000000000000ull, 0x000a000000000000ull, 0x0014000000000000ull, 0x0028000000000000ull, 0x0050000000000000ull, 0x00a0000000000000ull, 0x0040000000000000ull,
	},
	{ // white pawn
		0x0000000000000200ull, 0x0000000000000500ull, 0x0000000000000a00ull, 0x0000000000001400ull, 0x0000000000002800ull, 0x0000000000005000ull, 0x000000000000a000ull, 0x0000000000004000ull,
		0x0000000000020000ull, 0x0000000000050000ull, 0x00000000000a0000ull, 0x0000000000140000ull, 0x0000000000280000ull, 0x0000000000500000ull, 0x0000000000a00000ull, 0x0000000000400000ull,
		0x0000000002000000ull, 0x0000000005000000ull, 0x000000000a000000ull, 0x0000000014000000ull, 0x0000000028000000ull, 0x0000000050000000ull, 0x00000000a0000000ull, 0x0000000040000000ull,
		0x0000000200000000ull, 0x0000000500000000ull, 0x0000000a00000000ull, 0x0000001400000000ull, 0x0000002800000000ull, 0x0000005000000000ull, 0x000000a000000000ull, 0x0000004000000000ull,
		0x0000020000000000ull, 0x0000050000000000ull, 0x00000a0000000000ull, 0x0000140000000000ull, 0x0000280000000000ull, 0x0000500000000000ull, 0x0000a00000000000ull, 0x0000400000000000ull,
		0x0002000000000000ull, 0x0005000000000000ull, 0x000a000000000000ull, 0x0014000000000000ull, 0x0028000000000000ull, 0x0050000000000000ull, 0x00a0000000000000ull, 0x0040000000000000ull,
		0x0200000000000000ull, 0x0500000000000000ull, 0x0a00000000000000ull, 0x1400000000000000ull, 0x2800000000000000ull, 0x5000000000000000ull, 0xa000000000000000ull, 0x4000000000000000ull,
		0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
	},
});

Uint64 const king_attack_patern[64] = {
	0x0000000000000302ull, 0x0000000000000705ull, 0x0000000000000e0aull, 0x0000000000001c14ull, 0x0000000000003828ull, 0x0000000000007050ull, 0x000000000000e0a0ull, 0x000000000000c040ull,
	0x0000000000030203ull, 0x0000000000070507ull, 0x00000000000e0a0eull, 0x00000000001c141cull, 0x0000000000382838ull, 0x0000000000705070ull, 0x0000000000e0a0e0ull, 0x0000000000c040c0ull,
	0x0000000003020300ull, 0x0000000007050700ull, 0x000000000e0a0e00ull, 0x000000001c141c00ull, 0x0000000038283800ull, 0x0000000070507000ull, 0x00000000e0a0e000ull, 0x00000000c040c000ull,
	0x0000000302030000ull, 0x0000000705070000ull, 0x0000000e0a0e0000ull, 0x0000001c141c0000ull, 0x0000003828380000ull, 0x0000007050700000ull, 0x000000e0a0e00000ull, 0x000000c040c00000ull,
	0x0000030203000000ull, 0x0000070507000000ull, 0x00000e0a0e000000ull, 0x00001c141c000000ull, 0x0000382838000000ull, 0x0000705070000000ull, 0x0000e0a0e0000000ull, 0x0000c040c0000000ull,
	0x0003020300000000ull, 0x0007050700000000ull, 0x000e0a0e00000000ull, 0x001c141c00000000ull, 0x0038283800000000ull, 0x0070507000000000ull, 0x00e0a0e000000000ull, 0x00c040c000000000ull,
	0x0302030000000000ull, 0x0705070000000000ull, 0x0e0a0e0000000000ull, 0x1c141c0000000000ull, 0x3828380000000000ull, 0x7050700000000000ull, 0xe0a0e00000000000ull, 0xc040c00000000000ull,
	0x0203000000000000ull, 0x0507000000000000ull, 0x0a0e000000000000ull, 0x141c000000000000ull, 0x2838000000000000ull, 0x5070000000000000ull, 0xa0e0000000000000ull, 0x40c0000000000000ull,
};

Uint64 const knight_attack_patern[64] = {
	0x0000000000020400ull, 0x0000000000050800ull, 0x00000000000a1100ull, 0x0000000000142200ull, 0x0000000000284400ull, 0x0000000000508800ull, 0x0000000000a01000ull, 0x0000000000402000ull,
	0x0000000002040004ull, 0x0000000005080008ull, 0x000000000a110011ull, 0x0000000014220022ull, 0x0000000028440044ull, 0x0000000050880088ull, 0x00000000a0100010ull, 0x0000000040200020ull,
	0x0000000204000402ull, 0x0000000508000805ull, 0x0000000a1100110aull, 0x0000001422002214ull, 0x0000002844004428ull, 0x0000005088008850ull, 0x000000a0100010a0ull, 0x0000004020002040ull,
	0x0000020400040200ull, 0x0000050800080500ull, 0x00000a1100110a00ull, 0x0000142200221400ull, 0x0000284400442800ull, 0x0000508800885000ull, 0x0000a0100010a000ull, 0x0000402000204000ull,
	0x0002040004020000ull, 0x0005080008050000ull, 0x000a1100110a0000ull, 0x0014220022140000ull, 0x0028440044280000ull, 0x0050880088500000ull, 0x00a0100010a00000ull, 0x0040200020400000ull,
	0x0204000402000000ull, 0x0508000805000000ull, 0x0a1100110a000000ull, 0x1422002214000000ull, 0x2844004428000000ull, 0x5088008850000000ull, 0xa0100010a0000000ull, 0x4020002040000000ull,
	0x0400040200000000ull, 0x0800080500000000ull, 0x1100110a00000000ull, 0x2200221400000000ull, 0x4400442800000000ull, 0x8800885000000000ull, 0x100010a000000000ull, 0x2000204000000000ull,
	0x0004020000000000ull, 0x0008050000000000ull, 0x00110a0000000000ull, 0x0022140000000000ull, 0x0044280000000000ull, 0x0088500000000000ull, 0x0010a00000000000ull, 0x0020400000000000ull,
};