#include <array>
#include <bit>

#include "assets.h"

#include "item2.h"

#include "piece2.h"
#include "Button.h"

/*
case item_ID::evolution_stone:
	break;
case item_ID::king_promotion:
	break;
case item_ID::everstone:
	break;
case item_ID::resistance_berry:
	break;
case item_ID::immunity:
	break;
case item_ID::HDB:
	break;
case item_ID::eviolite:
	break;
case item_ID::normal_gem:
	break;
case item_ID::baguette:
	break;
case item_ID::tera_shard:
	break;
case item_ID::honey:
	break;
case item_ID::rocky_helmet:
	break;
case item_ID::red_card:
	break;
case item_ID::sticky_barbs:
	break;
case item_ID::life_orb:
	break;
case item_ID::loaded_dice:
	break;
case item_ID::scope_lens:
	break;
case item_ID::metronome:
	break;
case item_ID::quick_claw:
	break;
case item_ID::blunder_policy:
	break;
case item_ID::wide_lens:
	break;
case item_ID::expert_belt:
	break;
case item_ID::bright_powder:
	break;
case item_ID::safety_googles:
	break;
case item_ID::protective_pads:
	break;
case item_ID::shed_shell:
	break;
case item_ID::leppa_berry:
	break;
case item_ID::ring_target:
	break;
case item_ID::assault_vest:
	break;
case item_ID::iron_ball:
	break;
case item_ID::black_sludge:
	break;
case item_ID::flame_orb:
	break;
case item_ID::light_ball:
	break;
case item_ID::deep_sea_scale:
	break;
case item_ID::deep_sea_tooth:
	break;
default:
	return;
*/

int PokeItem::usefulness_tier(Board const& board, Piece const* piece) const {
	switch (id) {
	case item_ID::evolution_stone:
		return piece->id == piece_id::pawn and piece->base_type == old_type ? 2 : 0;
	case item_ID::king_promotion:
		return (piece->id == piece_id::pawn) ? (piece->type == typeless) + 2 * (piece->type == t1 or piece->type == t2) : 0;
	case item_ID::everstone:
		return -(piece->id == piece_id::pawn);
	case item_ID::resistance_berry: {
		effectiveness const e = get_effectiveness(type, piece->type, board.with_reversed_typechart);
		int res = (int)(e == super_effective or (t1 == normal and e == neutral));
		return res - number_of_drawed_resistance_berry;
	}
	case item_ID::immunity:
		return get_effectiveness(type, piece->type, board.with_reversed_typechart) == super_effective ? 2 : 1;
	case item_ID::HDB:
		return get_effectiveness(piece->type, type, board.with_reversed_typechart) == not_very_effective ? 2 : 1;
	case item_ID::eviolite:
		return 2 * (piece->id == piece_id::pawn);
	case item_ID::normal_gem:
		return 2 * (piece->type == type);
	case item_ID::baguette:
		return 2 * (piece->id == piece_id::pawn);
	case item_ID::tera_shard:
		return 2 * (piece->type != type) - number_of_drawed_terashard;
	case item_ID::honey:
		return (get_effectiveness(type, piece->type, board.with_reversed_typechart) <= not_very_effective) ? 2 : 1;
	case item_ID::rocky_helmet:
		return 2;
	case item_ID::red_card:
		return 2;
	case item_ID::sticky_barbs:
		return 2 - (piece->id != piece_id::pawn) - 2 * (piece->id == piece_id::king);
	case item_ID::life_orb:
		return 2;
	case item_ID::loaded_dice:
		return 2;
	case item_ID::scope_lens:
		return (board.with_ability and piece->ability_no_guard) ? 0 : 2;
	case item_ID::metronome:
		return 2;
	case item_ID::quick_claw:
		return 2;
	case item_ID::blunder_policy:
		return (board.with_ability and piece->ability_no_guard) ? 0 : 2;
	case item_ID::wide_lens:
		return 2;
	case item_ID::expert_belt: {
		int counter = 0;
		iter_typing(t) {
			if (get_effectiveness(piece->type, t, board.with_reversed_typechart) == super_effective)
				counter++;
		}
		if (counter == 0)
			return 0;
		else if (counter < 2)
			return 1;
		else
			return 2;
	}
	case item_ID::bright_powder:
		return 2;
	case item_ID::safety_googles:
		return 2;
	case item_ID::protective_pads:
		return 2;
	case item_ID::shed_shell:
		return 2 * (piece->id != piece_id::knight or piece->id != piece_id::princess or piece->id != piece_id::empress or piece->id != piece_id::amazon);
	case item_ID::leppa_berry:
		return 2 * (piece->id == piece_id::king or piece->id == piece_id::pawn);
	case item_ID::ring_target: {
		int counter = 0;
		iter_typing(t) {
			if (get_effectiveness(t, piece->type, board.with_reversed_typechart) == immune) {
				counter--;
			}
		}
		return counter;
	}
	case item_ID::assault_vest:
		return -2;
	case item_ID::iron_ball:
		return -1;
	case item_ID::black_sludge:
		return -(piece->type == poison);
	case item_ID::flame_orb:
		if (piece->type == type)
			return 0;
		else {
			return (board.with_ability and piece->ability_guts) ? 2 : -1;
		}
	case item_ID::light_ball:
		return (piece->id == piece_id::pawn) ? (piece->type == typeless) + 2 * (piece->type == electric) : 0;
	case item_ID::deep_sea_scale:
	case item_ID::deep_sea_tooth:
		return (piece->id == piece_id::pawn) ? (piece->type == typeless) + 2 * (piece->type == water) : 0;

	case item_ID::rare_candy:
		return (piece->id == piece_id::pawn) ? 2 : 0;
	default:
		return 0;
	}
}

void PokeItem::draw(Surface dest, SDL_Rect* rect, size s, anchor a) const {
	int x = 0, y = 0;
	switch (id) {
	case item_ID::evolution_stone:
		y = (int)new_type;
		break;
	case item_ID::king_promotion:
		x = 3;
		y = (int)t1;
		break;
	case item_ID::everstone:
		break;
	case item_ID::resistance_berry:
		x = 1;
		y = (int)type;
		break;
	case item_ID::immunity:
		x = 3;
		y = (int)type;
		break;
	case item_ID::HDB:
		x = 3;
		y = (int)type;
		break;
	case item_ID::eviolite:
		x = 4;
		y = 3;
		break;
	case item_ID::normal_gem:
		x = 3;
		y = (int)type;
		break;
	case item_ID::baguette:
		x = 5;
		y = 3;
		break;
	case item_ID::tera_shard:
		x = 2;
		y = (int)type;
		break;
	case item_ID::honey:
		x = 3;
		y = (int)type;
		break;
	case item_ID::rocky_helmet:
		x = 4;
		break;
	case item_ID::red_card:
		x = 4;
		y = 1;
		break;
	case item_ID::sticky_barbs:
		x = 4;
		y = 2;
		break;
	case item_ID::life_orb:
		x = 4;
		y = 5;
		break;
	case item_ID::loaded_dice:
		x = 4;
		y = 6;
		break;
	case item_ID::scope_lens:
		x = 4;
		y = 12;
		break;
	case item_ID::metronome:
		x = 4;
		y = 14;
		break;
	case item_ID::quick_claw:
		x = 3;
		y = 3;
		break;
	case item_ID::blunder_policy:
		x = 4;
		y = 9;
		break;
	case item_ID::wide_lens:
		x = 4;
		y = 11;
		break;
	case item_ID::expert_belt:
		x = 4;
		y = 13;
		break;
	case item_ID::bright_powder:
		x = 4;
		y = 15;
		break;
	case item_ID::safety_googles:
		x = 4;
		y = 7;
		break;
	case item_ID::protective_pads:
		x = 4;
		y = 8;
		break;
	case item_ID::shed_shell:
		x = 4;
		y = 16;
		break;
	case item_ID::leppa_berry:
		x = 4;
		y = 10;
		break;
	case item_ID::ring_target:
		x = 5;
		y = 0;
		break;
	case item_ID::assault_vest:
		x = 5;
		y = 1;
		break;
	case item_ID::iron_ball:
		x = 5;
		y = 2;
		break;
	case item_ID::black_sludge:
		x = 3;
		y = (int)type;
		break;
	case item_ID::flame_orb:
		x = 6;
		y = (int)type;
		break;
	case item_ID::light_ball:
		x = 4;
		y = 4;
		break;
	case item_ID::deep_sea_scale:
		x = 5;
		y = 16;
		break;
	case item_ID::deep_sea_tooth:
		x = 5;
		y = 17;
		break;
	case item_ID::rare_candy:
		x = 5;
		y = 4;
		break;
	default:
		return;
	}

	Surface sheet = item_sheet;
	int w = ITEM_SIZE;
	if (s == mini) {
		sheet = item_sheet_mini;
		w = ITEM_MINI_SIZE;
	}
	else if (s == mega) {
		sheet = item_sheet_mega;
		w = ITEM_MEGA_SIZE;
	}
	SDL_Rect r((x)*w, (y)*w, w, w);
	dest.blit(sheet, rect, &r, a);
}

void PokeItem::attack_modifier(Board const& board, move_data& data) {
	switch (id) {
	case item_ID::HDB:
		if (data.defender != NULL and data.defender->type == t1) {
			if (data.matchup == not_very_effective) {
				data.matchup++;
			}
			else {
				data.matchup.intensity++;
			}
		}
		break;
	case item_ID::normal_gem:
		if (data.attacker->type == t1) {
			data.matchup++;
		}
		break;
	case item_ID::baguette:
		if (data.en_passant) {
			data.matchup++;
		}
		break;
	default:
		return;
	}
}

void PokeItem::defense_modifier(Board const& board, move_data& data) {
	switch (id) {
	case item_ID::resistance_berry:
		if (data.attacker->type == type) {
			if (data.matchup == super_effective or type == normal)
				--data.matchup;
			else {
				--data.matchup.intensity;
			}
		}
		break;
	case item_ID::immunity:
		if (data.attacker->type == type) {
			data.matchup = immune;
		}
		break;
	case item_ID::eviolite:
		if (data.defender->id == piece_id::pawn) {
			if (data.matchup == super_effective) {
				data.matchup--;
			}
			else {
				data.matchup.intensity--;
			}
		}
		break;
	case item_ID::ring_target:
		if (data.matchup == immune) {
			data.matchup.main = neutral;
			if (data.matchup.intensity > 0) {
				data.matchup.intensity--;
				data.matchup.main = super_effective;
			}
			else if (data.matchup.intensity < 0) {
				data.matchup.intensity++;
				data.matchup.main = not_very_effective;
			}
		}
		break;
	case item_ID::iron_ball:
		if (data.attacker->type == ground and data.matchup == super_effective) {
			data.matchup = neutral;
		}
		break;
	default:
		return;
	}
}

void PokeItem::accuracy_modifier(Board const& board, move_data& data) {
	switch (id) {
	case item_ID::baguette:
		if (data.en_passant)
			data.miss_rate = -INFINITY;
		break;
	case item_ID::loaded_dice:
		if (data.miss_rate <= 0) {
			data.miss_rate = -INFINITY;
		}
		else {
			data.miss_rate *= data.miss_rate;
		}
		break;
	case item_ID::blunder_policy:
		data.miss_rate = -INFINITY;
		break;
	case item_ID::wide_lens:
		data.miss_rate /= 2;
		break;
	case item_ID::expert_belt:
		if (data.matchup == super_effective)
			data.miss_rate = -INFINITY;
		break;
	default:
		return;
	}
}

void PokeItem::crit_modifier(Board const& board, move_data& data) {
	Piece const* holder = data.attacker;
	switch (id) {
	case item_ID::life_orb:
		data.crit_rate *= 2.f;
		break;
	case item_ID::loaded_dice:
		if (data.crit_rate <= 0)
			data.crit_rate = -INFINITY;
		else {
			data.crit_rate = 1 - data.crit_rate;
			data.crit_rate *= data.crit_rate;
			data.crit_rate = 1 - data.crit_rate;
		}
		break;
	case item_ID::scope_lens:
		data.crit_rate *= 1.3f;
		break;
	case item_ID::metronome: {
		for (move_data const& d : board.move_historic) {
			if (d.attacker->color == holder->color) {
				if (d.attacker == holder and d.defender) {
					consecutive_counter++;
					if (consecutive_counter >= 5)
						consecutive_counter = 5;
				}
				else {
					consecutive_counter = 1;
				}
				break;
			}
		}
		data.crit_rate *= consecutive_counter;
		break;
	}
	case item_ID::flame_orb:
		if (board.with_ability and holder->ability_guts) {
			data.crit_rate *= 2;
		}
		break;
	case item_ID::light_ball: {
		if (holder->type == electric and (holder->id == piece_id::pawn or holder->evolved))
			data.crit_rate *= 2;
		break;
	}
	case item_ID::deep_sea_tooth:
		if (holder->type == water and holder->id == piece_id::pawn)
			data.crit_rate *= 2;
		break;
	default:
		return;
	}
}


void PokeItem::evasion_modifier(Board const& board, move_data& data) {
	Piece const* holder = data.defender;

#if ENABLE_SAFETY_CHECKS
	if (holder->item != self)
		throw;
#endif

	switch (id) {
	case item_ID::loaded_dice:
		data.miss_rate = 1 - data.miss_rate;
		data.miss_rate *= data.miss_rate;
		data.miss_rate = 1 - data.miss_rate;
		break;
	case item_ID::bright_powder:
		data.miss_rate = 1 - data.miss_rate;
		data.miss_rate *= 0.9;
		data.miss_rate = 1 - data.miss_rate;
		break;
	case item_ID::assault_vest:
		data.miss_rate = 1 - data.miss_rate;
		data.miss_rate *= 0.75;
		data.miss_rate = 1 - data.miss_rate;
		break;
	case item_ID::deep_sea_scale:
		if (holder->type == water and holder->id == piece_id::pawn) {
			data.miss_rate = 1 - data.miss_rate;
			data.miss_rate *= 0.825f;
			data.miss_rate = 1 - data.miss_rate;
		}
		break;
	default:
		return;
	}
}

std::pair<Uint64, Uint64> PokeItem::get_reachable_and_banned(Board const& board, Piece const* holder, Uint64 const avaible_moves) {
	std::pair<Uint64, Uint64> result = { 0, 0 };
	switch (id) {
	case item_ID::shed_shell: {
		result.first = knight_attack_patern[holder->pos] & ~board.presence_mask;
	}
	case item_ID::leppa_berry: {
		if (not holder->has_already_move)
			break;
		
		const_cast<Piece*>(holder)->has_already_move = false;
		result.first = holder->get_reachable(board);
		const_cast<Piece*>(holder)->has_already_move = true;
		
		break;
	}
	case item_ID::assault_vest:
		result.second = ~board.presence_mask;
		break;
	case item_ID::iron_ball:
		if (not const_cast<Piece*>(holder)->has_already_move) {
			const_cast<Piece*>(holder)->has_already_move = true;
			result.second = ~holder->get_reachable(board);
			const_cast<Piece*>(holder)->has_already_move = false;
		}
		break;
	default:
		break;
	}

	result.first &= ~avaible_moves;
	result.second &= ~avaible_moves;
	return result;
}


move_data PokeItem::move_to(Board& board, Piece* holder, Sint8 target_pos) {
	move_data data;
	
	switch (id) {
	case item_ID::shed_shell: {
		data = holder->base_move_to(board, target_pos);
		holder->item.used = true;
		break;
	}
	case item_ID::leppa_berry: {
		holder->has_already_move = false;
		data = holder->base_move_to(board, target_pos);
		holder->has_already_move = true;
		data.was_piece_first_move = false;
		holder->set_item(board, NO_ITEM);
		break;
	}
	default:
		throw;
	}
	return data;
}

void PokeItem::move_with(Board& board, Piece* holder, bool was_in_limbo, Sint8 old_pos, bool to_limbo, Sint8 new_pos) {
	switch (id) {
	case item_ID::honey:
		board.honey_holder_mask &= ~get_square_mask(was_in_limbo, old_pos);
		board.honey_holder_mask |= get_square_mask(to_limbo, new_pos);
		break;
	case item_ID::metronome:

		break;
	default:
		return;
	}
}

void PokeItem::after_move_effects(Board& board, move_data& data) {
	Piece* holder = data.attacker;
#if ENABLE_SAFETY_CHECKS
	if (holder->item != self)
		throw;
#endif
	switch (id) {
	case item_ID::everstone:
		if (data.init_promotion) {
			data.init_promotion = false;
			data.interrupt_move = false;
		}
		break;
	case item_ID::normal_gem:
		if (not (data.do_miss or data.do_crit or data.defender == NULL or data.end_pos == data.begin_pos)) {
			holder->set_item(board, NO_ITEM);
		}
		break;
	case item_ID::sticky_barbs:
		if (board.with_RNG and (not data.suicide) and game.RNG() < game.RNG.max() / 8) {
			board[holder->pos].to_graveyard(board);
			used = true;
			data.suicide = true;
			data.move_again = false;
		}
		break;
	case item_ID::life_orb:
		if (board.with_RNG and data.defender != NULL and (not data.suicide) and game.RNG() <= game.RNG.max() / 10) {
			board[holder->pos].to_graveyard(board);
			used = true;
			data.suicide = true;
		}
		break;
	case item_ID::quick_claw:
		if (board.with_RNG and (not data.defender) and (not data.move_again) and game.RNG() <= data.crit_rate * game.RNG.max()) {
			data.do_crit = data.move_again = true;
		}
		break;
	case item_ID::black_sludge:
	case item_ID::flame_orb:
		if (holder->type != type and game.RNG() < game.RNG.max() / 8) {
			board[holder->pos].to_graveyard(board);
			used = true;
			data.suicide = true;
		}
		break;
	case item_ID::rare_candy:
		if (holder->id == piece_id::pawn and promotion_rank[holder->color] == ((holder->pos >> 3) + holder->color)) {
			data.init_promotion = data.interrupt_move = true;
		}
		break;
	default:
		return;
	}
}


void PokeItem::revenge(Board& board, Piece* holder, move_data& data) {
#if ENABLE_SAFETY_CHECKS
	if (holder->item != self)
		throw;
#endif

	switch (id) {
	case item_ID::resistance_berry: {
		if (holder->is_dead and data.attacker->type == type and ((type != normal and data.matchup.main <= neutral) or (type == normal and data.matchup == not_very_effective)))
			used = true;
		break;
	}
	case item_ID::immunity:
		if ((not holder->is_dead) and data.attacker->type == type) {
			used = true;
		}
		break;
	case item_ID::rocky_helmet:
		if (holder->is_dead and not data.suicide and board.with_RNG and game.RNG() < game.RNG.max() / 6) {
			board[data.end_pos].to_graveyard(board);
			used = true;
			data.suicide = true;
		}
		break;
	case item_ID::red_card:
		if (holder->is_dead and data.move_again and not data.suicide) {
			data.move_again = false;
			used = true;
			game.board.in_bonus_move = false;

			board[data.end_pos].piece->set_square(board, false, data.begin_pos);
			data.end_pos = data.begin_pos;
		}
		break;
	case item_ID::sticky_barbs:
		if (holder->is_dead and not data.suicide) {
			used = false;
			board[data.end_pos].piece->set_item(board, self);
			holder->set_item(board, NO_ITEM);
		}
		break;
	default:
		return;
	}
}

void PokeItem::promote(Board& board, Piece* new_holder) {
	switch (id) {
	case item_ID::evolution_stone:
		if (new_holder->type == old_type) {
			new_holder->type = new_type;
			new_holder->set_item(board, NO_ITEM);
		}
		break;
	case item_ID::king_promotion:
		if (new_holder->id == piece_id::king) {
			new_holder->set_item(board, NO_ITEM);
		}
		break;
	case item_ID::light_ball:
		if (new_holder->type == electric) {
			new_holder->set_pokeicon(PokemonIcon(27 + 2 * (int)new_holder->id + new_holder->color, 1 + electric));
		}
		break;
	case item_ID::deep_sea_scale:
		break;
	case item_ID::deep_sea_tooth:
		break;
	case item_ID::rare_candy:
		new_holder->set_item(board, NO_ITEM);
		break;
	default:
		return;
	}
}


bool PokeItem::prepare_promotion(Board const&, Piece const* holder, Uint16* avaible) {
	switch (id) {
	case item_ID::king_promotion:
		if (holder->base_type == t1 or holder->base_type == t2) {
			*avaible = 0b0000'0001'0101'0110;
			// 8-th bit for king
			// 6-th bit for queen
			// 4-th bit for bishop
			// 1-th bit for knight
			// 2-th bit for rook
			return true;
		}
		return false;
	case item_ID::everstone:
		*avaible = 0;
		return true;
	default:
		return false;
	}
}


void PokeItem::add_cosmetic(Game& game, Board const& board, Piece const* holder, move_data const& data) {
	switch (id) {
	case item_ID::immunity:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, holder->get_name(game.language));
			switch (type) {
			case ground:
				strcat_s(buffer, "\'s\nBalloon poped.");
				break;
			case electric:
				strcat_s(buffer, "\'s\nBattery surchaged");
				break;
			case fire:
				strcat_s(buffer, "\'s\nSnowball melted");
				break;
			case water:
				strcat_s(buffer, "\'s\nBulb died");
				break;
			}
			game.add_textbox(buffer);
		}
		break;
	case item_ID::rocky_helmet:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->get_name(game.language));
			strcat_s(buffer, "\nDied to Rcky Hlmt");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::red_card:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->get_name(game.language));
			strcat_s(buffer, "\nwas sent back.");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::sticky_barbs:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, holder->get_name(game.language));
			strcat_s(buffer, "\nDied to StckyBrbs");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::life_orb:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, holder->get_name(game.language));
			strcat_s(buffer, "\nDied to life orb");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::shed_shell:
		/*
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, holder->get_name(game.language));
			strcat_s(buffer, "\nfled successfuly");
			game.add_textbox(buffer);
		}*/
		break;
	case item_ID::black_sludge:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, holder->get_name(game.language));
			strcat_s(buffer, "\nDied to\nBlack Sludge");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::flame_orb:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, holder->get_name(game.language));
			strcat_s(buffer, "\nDied to Flame Orb");
			game.add_textbox(buffer);
		}
		break;
	default:
		return;
	}
}



void PokeItem::update_status(Board& board, Piece* holder, move_data& data) {
	switch (id) {
	case item_ID::resistance_berry:
	case item_ID::immunity:
	case item_ID::red_card:
		if (used) {
			used = false;
			holder->set_item(board, NO_ITEM);
		}
		break;
	case item_ID::shed_shell:
		if (used) {
			used = false;
			holder->set_item(board, NO_ITEM);
		}
		break;
	}
}

void PokeItem::select_holder(Game& game, Board& board, Piece* holder) {
	switch (id) {
	case item_ID::tera_shard:
		if (board.is_tera_avaible[holder->color])
			game.buttons->add(new TeraButton(9, 10.5, holder, type));
		break;
	default:
		return;
	}
}

void PokeItem::put_on(Board& board, Piece* holder) {
	switch (id) {
	case item_ID::resistance_berry:
		number_of_drawed_resistance_berry++;
		break;
	case item_ID::tera_shard:
		number_of_drawed_terashard++;
		break;
	case item_ID::honey:
		board.honey_holder_mask |= get_square_mask(holder->in_limbo, holder->pos);
		break;
	case item_ID::metronome:
		consecutive_counter = 0;
		break;
	default:
		return;
	}
}



void PokeItem::sync_with_holder(Board& board, Piece* holder) {
	switch (id) {
	case item_ID::evolution_stone:
		if (holder->base_type == old_type)
			holder->set_pokeicon(PokemonIcon(21 + (1 - holder->color) / 2, 1 + new_type));
		else
			holder->set_pokeicon(PokemonIcon(holder));
		break;
	case item_ID::king_promotion:
		if (holder->id == piece_id::pawn and (holder->type == t1 or holder->type == t2))
			holder->set_pokeicon(PokemonIcon(25 + (1 - holder->color) / 2, 1 + t1));
		else
			holder->set_pokeicon(PokemonIcon(holder));
		break;
	case item_ID::light_ball:
		if (holder->type == electric and holder->id == piece_id::pawn) {
			holder->set_pokeicon(PokemonIcon(25 + (1 - holder->color) / 2, 1 + electric));
		}
		else
			holder->set_pokeicon(PokemonIcon(holder));
		break;
	case item_ID::deep_sea_scale:
	case item_ID::deep_sea_tooth:

		break;
	default:
		return;
	}
}




void PokeItem::remove_from(Board& board, Piece* holder) {
	switch (id) {
	case item_ID::honey:
		board.honey_holder_mask &= ~get_square_mask(holder->in_limbo, holder->pos);
		break;
	default:
		return;
	}
}


void PokeItem::remove_from_during_selection(Board& board, Piece* holder) {
	holder->set_pokeicon(PokemonIcon(holder));
	switch (id) {
	case item_ID::evolution_stone:
		break;
	case item_ID::king_promotion:
		break;
	case item_ID::everstone:
		break;
	case item_ID::tera_shard:
		number_of_drawed_terashard--;
		break;
	case item_ID::honey:
		board.honey_holder_mask &= ~get_square_mask(holder->in_limbo, holder->pos);
		break;
	case item_ID::light_ball:
		break;
	case item_ID::deep_sea_scale:
		break;
	case item_ID::deep_sea_tooth:
		break;
	default:
		return;
	}
}

template<typename T>
struct ItemSearchTree {
public:
	Uint8 depth;
	Uint8 nb_of_child;
	ItemSearchTree<T>* next;
	T data;
	
	ItemSearchTree() {
		depth = 0;
		nb_of_child = 0;
		next = NULL;
	}

	ItemSearchTree(T const& value) : data(value) {
		depth = 0;
		nb_of_child = 0;
		next = NULL;
	}


	ItemSearchTree(ItemSearchTree const& other) {
		self = other;
	}

	ItemSearchTree<T>& operator=(ItemSearchTree<T> const& other) {
		nb_of_child = other.nb_of_child;
		depth = other.depth;

		if (other.next != NULL) {
			next = new ItemSearchTree[nb_of_child];
			int i = nb_of_child;
			while (i-- > 0) {
				next[i] = other.next[i];
			}
		}
		else {
			next = NULL;
		}
		data = other.data;
		return self;
	}

	ItemSearchTree<T>& operator=(ItemSearchTree<T>&& other) noexcept {
		next = other.next;
		other.next = NULL;
		nb_of_child = other.nb_of_child;
		data = other.data;
		depth = other.depth;

		return self;
	}

	template<size_t N>
	ItemSearchTree(std::array<ItemSearchTree, N> children) {
		depth = 0;
		nb_of_child = N;
		next = new ItemSearchTree[N];

		for (int i = 0; i < N; i++) {
			next[i] = children[i];
			next[i].depth++;
		}
	}

	~ItemSearchTree() {
		if (next != NULL) {
			delete[] next;
		}
	}

	T& operator[](PokeItem item) {
		if (next == NULL)
			return data;
		else {
			int key = std::bit_cast<std::array<Uint8, 4>>(item)[depth];
			return next[key][item];
		}
	}

	T const& operator[](PokeItem item) const {
		if (next == NULL)
			return data;
		else {
			int key = std::bit_cast<std::array<Uint8, 4>>(item)[depth];
			return next[key][item];
		}
	}
};
#define MEDIUM_SIZE_ITEM_NAME (TILE_SIZE / 3)
ItemSearchTree<dstr_t> __french_name_tree =
ItemSearchTree<dstr_t>(std::array<ItemSearchTree<dstr_t>, (int)item_ID::__nb_of_item__>({
	ItemSearchTree<dstr_t>({ "NULL", 0 }),
	ItemSearchTree<dstr_t>( // evolution stone
		std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", 0), // to normal
		dstr_t("Pierre Feu", MEDIUM_SIZE_ITEM_NAME), // to fire
		dstr_t("Pierre Eau", MEDIUM_SIZE_ITEM_NAME), // to water
		dstr_t("Pierre Plante", MEDIUM_SIZE_ITEM_NAME), // to grass
		dstr_t("Pierre Foudre", MEDIUM_SIZE_ITEM_NAME), // to electrik
		dstr_t("Pierre Glace", MEDIUM_SIZE_ITEM_NAME), // to ice
		dstr_t("", 0), // to fighting
		dstr_t("", 0), // to poison
		dstr_t("", 0), // to ground
		dstr_t("", 0), // to flying
		dstr_t("Pierre Eclat", MEDIUM_SIZE_ITEM_NAME), // to psychic
		dstr_t("", 0), // to bug
		dstr_t("Obsidienne", MEDIUM_SIZE_ITEM_NAME), // to rock
		dstr_t("", 0), // to ghost
		dstr_t("Ecaille Draco", MEDIUM_SIZE_ITEM_NAME), // to dragon
		dstr_t("Pierre Nuit", MEDIUM_SIZE_ITEM_NAME), // to dark
		dstr_t("Peau Metal", MEDIUM_SIZE_ITEM_NAME), // to steel
		dstr_t("Grelot Zen", MEDIUM_SIZE_ITEM_NAME), // to fairy
	})),
	std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", 0), // to normal
		dstr_t("", 0), // to fire
		dstr_t("Roche Royale", MEDIUM_SIZE_ITEM_NAME), // to water
		dstr_t("", 0), // to grass
		dstr_t("", 0), // to electrik
		dstr_t("", 0), // to ice
		dstr_t("", 0), // to fighting
		dstr_t("", 0), // to poison
		dstr_t("", 0), // to ground
		dstr_t("", 0), // to flying
		dstr_t("Roche Royale", MEDIUM_SIZE_ITEM_NAME), // to psychic
		dstr_t("", 0), // to bug
		dstr_t("", 0), // to rock
		dstr_t("", 0), // to ghost
		dstr_t("", 0), // to dragon
		dstr_t("Embleme du General", MEDIUM_SIZE_ITEM_NAME), // to dark
		dstr_t("Embleme du General", MEDIUM_SIZE_ITEM_NAME), // to steel
		dstr_t("", 0), // to fairy
	}),
	dstr_t({"Pierre Stase", MEDIUM_SIZE_ITEM_NAME}),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t("Baie Zalis", MEDIUM_SIZE_ITEM_NAME), // normal
		dstr_t("Baie Chocco", MEDIUM_SIZE_ITEM_NAME), // fire
		dstr_t("Baie Pocpoc", MEDIUM_SIZE_ITEM_NAME), // water
		dstr_t("Baie Ratam", MEDIUM_SIZE_ITEM_NAME), // grass
		dstr_t("Baie Parma", MEDIUM_SIZE_ITEM_NAME), // electrik
		dstr_t("Baie Nanone", MEDIUM_SIZE_ITEM_NAME), // ice
		dstr_t("Baie Pomroz", MEDIUM_SIZE_ITEM_NAME), // fighting
		dstr_t("Baie Kebia", MEDIUM_SIZE_ITEM_NAME), // poison
		dstr_t("Baie Jouca", MEDIUM_SIZE_ITEM_NAME), // ground
		dstr_t("Baie Cobaba", MEDIUM_SIZE_ITEM_NAME), // flying
		dstr_t("Baie Yapap", MEDIUM_SIZE_ITEM_NAME), // psychic
		dstr_t("Baie Panga", MEDIUM_SIZE_ITEM_NAME), // bug
		dstr_t("Baie Charti", MEDIUM_SIZE_ITEM_NAME), // rock
		dstr_t("Baie Sedra", MEDIUM_SIZE_ITEM_NAME), // ghost
		dstr_t("Baie Fraigo", MEDIUM_SIZE_ITEM_NAME), // dragon
		dstr_t("Baie Lampou", MEDIUM_SIZE_ITEM_NAME), // dark
		dstr_t("Baie Babiri", MEDIUM_SIZE_ITEM_NAME), // steel
		dstr_t("Baie Selro", MEDIUM_SIZE_ITEM_NAME), // fairy
	}),
	std::array<ItemSearchTree<dstr_t>, 9>({
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // normal
		dstr_t("Boule de Neige", MEDIUM_SIZE_ITEM_NAME), // fire
		dstr_t("Bulbe", MEDIUM_SIZE_ITEM_NAME), // water
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // grass
		dstr_t("Pile", MEDIUM_SIZE_ITEM_NAME), // electrik
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // ice
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // fighting
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // poison
		dstr_t("Ballon", MEDIUM_SIZE_ITEM_NAME), // ground
#if false
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // flying
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // psychic
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // bug
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // rock
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // ghost
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // dragon
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // dark
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // steel
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // fairy
#endif
	}),
	dstr_t("Grosses Bottes", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Evoluroc", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Joyau Normal", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Baguette", MEDIUM_SIZE_ITEM_NAME),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t("Tera-Eclat Normal", MEDIUM_SIZE_ITEM_NAME), // normal
		dstr_t("Tera-Eclat Feu", MEDIUM_SIZE_ITEM_NAME), // fire
		dstr_t("Tera-Eclat Eau", MEDIUM_SIZE_ITEM_NAME), // water
		dstr_t("Tera-Eclat Plante", MEDIUM_SIZE_ITEM_NAME), // grass
		dstr_t("Tera-Eclat Electrique", MEDIUM_SIZE_ITEM_NAME), // electrik
		dstr_t("Tera-Eclat Glace", MEDIUM_SIZE_ITEM_NAME), // ice
		dstr_t("Tera-Eclat Combat", MEDIUM_SIZE_ITEM_NAME), // fighting
		dstr_t("Tera-Eclat Poison", MEDIUM_SIZE_ITEM_NAME), // poison
		dstr_t("Tera-Eclat Sol", MEDIUM_SIZE_ITEM_NAME), // ground
		dstr_t("Tera-Eclat Vol", MEDIUM_SIZE_ITEM_NAME), // flying
		dstr_t("Tera-Eclat Psy", MEDIUM_SIZE_ITEM_NAME), // psychic
		dstr_t("Tera-Eclat Insecte", MEDIUM_SIZE_ITEM_NAME), // bug
		dstr_t("Tera-Eclat Roche", MEDIUM_SIZE_ITEM_NAME), // rock
		dstr_t("Tera-Eclat Spectre", MEDIUM_SIZE_ITEM_NAME), // ghost
		dstr_t("Tera-Eclat Dragon", MEDIUM_SIZE_ITEM_NAME), // dragon
		dstr_t("Tera-Eclat Tenebres", MEDIUM_SIZE_ITEM_NAME), // dark
		dstr_t("Tera-Eclat Acier", MEDIUM_SIZE_ITEM_NAME), // steel
		dstr_t("Tera-Eclat Fee", MEDIUM_SIZE_ITEM_NAME), // fairy
	}),
	dstr_t("Miel", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Casque Brut", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Carton Rouge", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Piquants", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Orbe Vie", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("De pipe", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Lentilscope", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Metronome", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Vive Griffe", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Assurance Echec", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Loupe", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Ceinture Pro", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Poudre Claire", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Lunettes Filtre", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Parre-Effet", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Carapace Mue", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Baie Mepo", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Point de Mire", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Veste de Combat", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Balle Fer", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Boue Noire", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Orbe Flamme", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Balle Lumiere", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Ecaille Ocean", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Dent Ocean", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Super Bonbon", MEDIUM_SIZE_ITEM_NAME),
}));

ItemSearchTree<dstr_t> __english_name_tree =
ItemSearchTree<dstr_t>(std::array<ItemSearchTree<dstr_t>, (int)item_ID::__nb_of_item__>({
	ItemSearchTree<dstr_t>({ "NULL", 0 }),
	ItemSearchTree<dstr_t>( // evolution stone
		std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", 0), // to normal
		dstr_t("Fire Stone", MEDIUM_SIZE_ITEM_NAME), // to fire
		dstr_t("Water Stone", MEDIUM_SIZE_ITEM_NAME), // to water
		dstr_t("Leaf Stone", MEDIUM_SIZE_ITEM_NAME), // to grass
		dstr_t("Thunder Stone", MEDIUM_SIZE_ITEM_NAME), // to electrik
		dstr_t("Ice Stone", MEDIUM_SIZE_ITEM_NAME), // to ice
		dstr_t("", 0), // to fighting
		dstr_t("", 0), // to poison
		dstr_t("", 0), // to ground
		dstr_t("", 0), // to flying
		dstr_t("Shiny Stone", MEDIUM_SIZE_ITEM_NAME), // to psychic
		dstr_t("", 0), // to bug
		dstr_t("Black Augurite", MEDIUM_SIZE_ITEM_NAME), // to rock
		dstr_t("", 0), // to ghost
		dstr_t("Dragon Scale", MEDIUM_SIZE_ITEM_NAME), // to dragon
		dstr_t("Dusk Stone", MEDIUM_SIZE_ITEM_NAME), // to dark
		dstr_t("Metal Coat", MEDIUM_SIZE_ITEM_NAME), // to steel
		dstr_t("Soothe Bell", MEDIUM_SIZE_ITEM_NAME), // to fairy
	})),
	std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", 0), // to normal
		dstr_t("", 0), // to fire
		dstr_t("King's Rock", MEDIUM_SIZE_ITEM_NAME), // to water
		dstr_t("", 0), // to grass
		dstr_t("", 0), // to electrik
		dstr_t("", 0), // to ice
		dstr_t("", 0), // to fighting
		dstr_t("", 0), // to poison
		dstr_t("", 0), // to ground
		dstr_t("", 0), // to flying
		dstr_t("King's Rock", MEDIUM_SIZE_ITEM_NAME), // to psychic
		dstr_t("", 0), // to bug
		dstr_t("", 0), // to rock
		dstr_t("", 0), // to ghost
		dstr_t("", 0), // to dragon
		dstr_t("Leader's Crest", MEDIUM_SIZE_ITEM_NAME), // to dark
		dstr_t("Leader's Crest", MEDIUM_SIZE_ITEM_NAME), // to steel
		dstr_t("", 0), // to fairy
	}),
	dstr_t({"Pierre Stase", MEDIUM_SIZE_ITEM_NAME}),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t("Chilan berry", MEDIUM_SIZE_ITEM_NAME), // normal
		dstr_t("Occa berry", MEDIUM_SIZE_ITEM_NAME), // fire
		dstr_t("Passho berry", MEDIUM_SIZE_ITEM_NAME), // water
		dstr_t("Rindo berry", MEDIUM_SIZE_ITEM_NAME), // grass
		dstr_t("Wacan berry", MEDIUM_SIZE_ITEM_NAME), // electrik
		dstr_t("Yache berry", MEDIUM_SIZE_ITEM_NAME), // ice
		dstr_t("Chople berry", MEDIUM_SIZE_ITEM_NAME), // fighting
		dstr_t("Kebia berry", MEDIUM_SIZE_ITEM_NAME), // poison
		dstr_t("Shuca berry", MEDIUM_SIZE_ITEM_NAME), // ground
		dstr_t("Coba berry", MEDIUM_SIZE_ITEM_NAME), // flying
		dstr_t("Payapa berry", MEDIUM_SIZE_ITEM_NAME), // psychic
		dstr_t("Tanga berry", MEDIUM_SIZE_ITEM_NAME), // bug
		dstr_t("Charti berry", MEDIUM_SIZE_ITEM_NAME), // rock
		dstr_t("Kasib berry", MEDIUM_SIZE_ITEM_NAME), // ghost
		dstr_t("Haban berry", MEDIUM_SIZE_ITEM_NAME), // dragon
		dstr_t("Colbur berry", MEDIUM_SIZE_ITEM_NAME), // dark
		dstr_t("Babiri berry", MEDIUM_SIZE_ITEM_NAME), // steel
		dstr_t("Roseli berry", MEDIUM_SIZE_ITEM_NAME), // fairy
	}),
	std::array<ItemSearchTree<dstr_t>, 9>({
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // normal
		dstr_t("Snowball", MEDIUM_SIZE_ITEM_NAME), // fire
		dstr_t("Water Bulb", MEDIUM_SIZE_ITEM_NAME), // water
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // grass
		dstr_t("Cell Battery", MEDIUM_SIZE_ITEM_NAME), // electrik
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // ice
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // fighting
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // poison
		dstr_t("Air Balloon", MEDIUM_SIZE_ITEM_NAME), // ground
#if false
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // flying
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // psychic
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // bug
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // rock
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // ghost
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // dragon
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // dark
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // steel
		dstr_t("", MEDIUM_SIZE_ITEM_NAME), // fairy
#endif
	}),
	dstr_t("Heavy Duty Boots", TILE_SIZE / 4),
	dstr_t("Eviolite", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Normal Gem", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Baguette", MEDIUM_SIZE_ITEM_NAME),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t("Normal Tera Shard", TILE_SIZE / 4), // normal
		dstr_t("Fire Tera Shard", TILE_SIZE / 4), // fire
		dstr_t("Water Tera Shard", TILE_SIZE / 4), // water
		dstr_t("Grass Tera Shard", TILE_SIZE / 4), // grass
		dstr_t("Electrik Tera Shard", TILE_SIZE / 4), // electrik
		dstr_t("Ice Tera Shard", TILE_SIZE / 4), // ice
		dstr_t("Fighting Tera Shard", TILE_SIZE / 4), // fighting
		dstr_t("Poison Tera Shard", TILE_SIZE / 4), // poison
		dstr_t("Ground Tera Shard", TILE_SIZE / 4), // ground
		dstr_t("Flying Tera Shard", TILE_SIZE / 4), // flying
		dstr_t("Psychic Tera Shard", TILE_SIZE / 4), // psychic
		dstr_t("Bug Tera Shard", TILE_SIZE / 4), // bug
		dstr_t("Rock Tera Shard", TILE_SIZE / 4), // rock
		dstr_t("Ghost Tera Shard", TILE_SIZE / 4), // ghost
		dstr_t("Dragon Tera Shard", TILE_SIZE / 4), // dragon
		dstr_t("Dark Tera Shard", TILE_SIZE / 4), // dark
		dstr_t("Steel Tera Shard", TILE_SIZE / 4), // steel
		dstr_t("Fairy Tera Shard", TILE_SIZE / 4), // fairy
	}),
	dstr_t("Honey", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Rocky Helmet", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Red Card", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Sticky Barbs", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Life Orb", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Loaded Dice", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Scope Lens", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Metronome", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Quick Claw", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Blunder Policy", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Wide Lens", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Expert Belt", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Bright Powder", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Safety Googles", TILE_SIZE / 4),
	dstr_t("Protective Pads", TILE_SIZE / 4),

	dstr_t("Shed Shell", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Leppa Berry", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Ring Target", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Assault Vest", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Iron Ball", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Black Sludge", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Flame Orb", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Light Ball", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Deep Sea Scale", MEDIUM_SIZE_ITEM_NAME),
	dstr_t("Deep Sea Tooth", MEDIUM_SIZE_ITEM_NAME),

	dstr_t("Rare Candy", MEDIUM_SIZE_ITEM_NAME),
}));


ItemSearchTree<dstr_t> const item_name_tree[NB_OF_LANGUAGE] = {
	__french_name_tree,
	__english_name_tree,
};

dstr_t PokeItem::get_name(LANGUAGE language) {
	return item_name_tree[(int)language][self];
}

#define MEDIUM_SIZE_DESC_TEXT (TILE_SIZE / 5)

ItemSearchTree<dstr_t> __french_desc_tree =
ItemSearchTree<dstr_t>(std::array<ItemSearchTree<dstr_t>, (int)item_ID::__nb_of_item__>({
	ItemSearchTree<dstr_t>({ "NULL", 0 }),
	ItemSearchTree<dstr_t>( // evolution stone
		std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type feu en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type eau en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type plante en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type electrique en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type glace en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ground
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type insecte il deviendra de type roche en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type eau il deviendra de type dragon en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type tenebres en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type insecte il deviendra de type acier en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t(" - Uniquement pour les pions.\n - Si l\'utilisateur est de type normal il deviendra de type fee en evoluant.\n", MEDIUM_SIZE_DESC_TEXT), // fairy
	})),
	std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", 0), // to normal
		dstr_t("", 0), // to fire
		dstr_t(" - Uniquement pour les pions de type eau/psy\n - Permet la promotion en Roi.\n", MEDIUM_SIZE_DESC_TEXT), // to water
		dstr_t("", 0), // to grass
		dstr_t("", 0), // to electrik
		dstr_t("", 0), // to ice
		dstr_t("", 0), // to fighting
		dstr_t("", 0), // to poison
		dstr_t("", 0), // to ground
		dstr_t("", 0), // to flying
		dstr_t(" - Uniquement pour les pions de type eau/psy\n - Permet la promotion en Roi.\n", MEDIUM_SIZE_DESC_TEXT), // to psychic
		dstr_t("", 0), // to bug
		dstr_t("", 0), // to rock
		dstr_t("", 0), // to ghost
		dstr_t("", 0), // to dragon
		dstr_t(" - Uniquement pour les pions de type acier/tenebres\n - Permet la promotion en Roi.\n", MEDIUM_SIZE_DESC_TEXT), // to dark
		dstr_t(" - Uniquement pour les pions de type acier/tenebres\n - Permet la promotion en Roi.\n", MEDIUM_SIZE_DESC_TEXT), // to steel
		dstr_t("", 0), // to fairy
	}),
	dstr_t({" - Empeche son porteur d'evoluer.\n", MEDIUM_SIZE_DESC_TEXT}),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t(" - Retire une faiblesse contre le type normal.\n", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Retire une faiblesse contre le type feu.\n", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Retire une faiblesse contre le type eau.\n", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t(" - Retire une faiblesse contre le type plante.\n", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Retire une faiblesse contre le type electrique.\n", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t(" - Retire une faiblesse contre le type glace.\n", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t(" - Retire une faiblesse contre le type combat.\n", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t(" - Retire une faiblesse contre le type poison.\n", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t(" - Retire une faiblesse contre le type sol.\n", MEDIUM_SIZE_DESC_TEXT), // ground
		dstr_t(" - Retire une faiblesse contre le type vol.\n", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t(" - Retire une faiblesse contre le type psy.\n", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t(" - Retire une faiblesse contre le type insecte.\n", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t(" - Retire une faiblesse contre le type roche.\n", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t(" - Retire une faiblesse contre le type spectre.\n", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t(" - Retire une faiblesse contre le type dragon.\n", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t(" - Retire une faiblesse contre le type tenebres.\n", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t(" - Retire une faiblesse contre le type acier.\n", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t(" - Retire une faiblesse contre le type fee.\n", MEDIUM_SIZE_DESC_TEXT), // fairy
	}),
	std::array<ItemSearchTree<dstr_t>, 9>({
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Utilisable une unique fois.\n - Donne une immunite contre le type feu.\n", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Utilisable une unique fois.\n - Donne une immunite contre le type eau.\n", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Utilisable une unique fois.\n - Donne une immunite contre le type electrique.\n", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t(" - Utilisable une unique fois.\n - Donne une immunite contre le type sol.\n", MEDIUM_SIZE_DESC_TEXT), // ground
#if false
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // fairy
#endif
	}),
	dstr_t(" - L'utilisateur ne sera plus resite par le type roche.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Uniquement pour les pions.\n - Retire toute les faiblesses.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Uniquement pour les pieces de type normal\n - Utilisable une seule fois.\n - Votre premiere attaque sera super efficace.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Une baguette.\n - Les prises en passant son super efficace.\n", MEDIUM_SIZE_DESC_TEXT),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t(" - Permet son utilisateur de teracristal en type normal.", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Permet son utilisateur de teracristal en type feu.", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Permet son utilisateur de teracristal en type eau.", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t(" - Permet son utilisateur de teracristal en type plante.", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Permet son utilisateur de teracristal en type electrique.", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t(" - Permet son utilisateur de teracristal en type glace.", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t(" - Permet son utilisateur de teracristal en type combat.", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t(" - Permet son utilisateur de teracristal en type poison.", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t(" - Permet son utilisateur de teracristal en type sol.", MEDIUM_SIZE_DESC_TEXT), // ground
		dstr_t(" - Permet son utilisateur de teracristal en type vol.", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t(" - Permet son utilisateur de teracristal en type psy.", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t(" - Permet son utilisateur de teracristal en type insecte.", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t(" - Permet son utilisateur de teracristal en type roche.", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t(" - Permet son utilisateur de teracristal en type spectre.", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t(" - Permet son utilisateur de teracristal en type dragon.", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t(" - Permet son utilisateur de teracristal en type tenebres.", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t(" - Permet son utilisateur de teracristal en type acier.", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t(" - Permet son utilisateur de teracristal en type fee.", MEDIUM_SIZE_DESC_TEXT), // fairy
	}),
	dstr_t(" - Les types insectes adverse ne peuvent resister a l'envie de miel.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Si son utilisateur meurt d'une piece enemie, cette piece a une chance de mourrir.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Si son utilisateur meurt d'une attaque qui resulte en un coup bonus, l'attaquant revient sur sa case de depart.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Son utilisateur a une chance de mourrir quand il se deplace\n - Colle aux pièces ennemie.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Son Utilisateur a 1/10 chance de mourir a chaque fois qu'il attaque.\n - Augmente enormement le taux de coup critique.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Chaque utilisation d'aleatoire concernant son utilisateur est effectuee avec \"avantage\".\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Augmente legerement le taux de critique.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Augmente le taux de critique pour chaques coups consecutifs.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Rend son utilisateur capable d'infliger des coups critiques n'importe ou.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Utilisable une unique fois.\n - Son utilisateur ne peut pas echoue sa premiere attaque.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Augmente la precision.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Empeche les attaques super efficace d'echouer.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Augmente legerement l'esquive.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Ignore les objets adverse vous êtes attaque.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Ignore les objets ennemi quand vous attaquez.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Utilisable une seule fois.\n - Vous pouvez vous deplacez en cavalier.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Utilisable une seule fois.\n - Permet de refaire un deplacement special.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Retire toutes les immunites de son utilisateur.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Empêche les coups qui ne sont pas des captures\n - Augmente beacoup l'esquive.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Retire une potentielle immunite sol.\n - Les rois ne peuvent plus roquer.\n - Les pions ne peuvent plus faire de double pas.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - A une chance de tuer les pièces qui ne sont pas de type poison quand elles se deplacent.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - A une chance de tuer les pièces qui ne sont pas de type feu quand elles se deplacent\n - Peux augmenter le critique de certaines pieces.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Uniquement pour les pions de type electrique.\n - Augmente le taux de coup critique.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t("NotInplementedError", MEDIUM_SIZE_DESC_TEXT),
	dstr_t("NotInplementedError", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Permet d'evoluer un rang plus tot.", MEDIUM_SIZE_DESC_TEXT),
}));

ItemSearchTree<dstr_t> __english_desc_tree =
ItemSearchTree<dstr_t>(std::array<ItemSearchTree<dstr_t>, (int)item_ID::__nb_of_item__>({
	ItemSearchTree<dstr_t>({ "NULL", 0 }),
	ItemSearchTree<dstr_t>( // evolution stone
		std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into fire types.\n", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into water types.\n", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into grass types.\n", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into electric types.\n", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into ice types.\n", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ground
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t(" - Only for pawns.\n - Makes bug types evolve into rock types.\n", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t(" - Only for pawns.\n - Makes water types evolve into dragon types.\n", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into dark types.\n", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t(" - Only for pawns.\n - Makes bug types evolve into steel types.\n", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t(" - Only for pawns.\n - Makes normal types evolve into fairy types.\n", MEDIUM_SIZE_DESC_TEXT), // fairy
	})),
	std::array<ItemSearchTree<dstr_t>, 18>({
		dstr_t("", 0), // to normal
		dstr_t("", 0), // to fire
		dstr_t(" - Only for water/psychic pawns\n - Allow for promotion into a King.\n", MEDIUM_SIZE_DESC_TEXT), // to water
		dstr_t("", 0), // to grass
		dstr_t("", 0), // to electrik
		dstr_t("", 0), // to ice
		dstr_t("", 0), // to fighting
		dstr_t("", 0), // to poison
		dstr_t("", 0), // to ground
		dstr_t("", 0), // to flying
		dstr_t(" - Only for water/psychic pawns\n - Allow for promotion into a King.\n", MEDIUM_SIZE_DESC_TEXT), // to psychic
		dstr_t("", 0), // to bug
		dstr_t("", 0), // to rock
		dstr_t("", 0), // to ghost
		dstr_t("", 0), // to dragon
		dstr_t(" - Only for steel/dark pawns\n - Allow for promotion into a King.\n", MEDIUM_SIZE_DESC_TEXT), // to dark
		dstr_t(" - Only for steel/dark pawns\n - Allow for promotion into a King.\n", MEDIUM_SIZE_DESC_TEXT), // to steel
		dstr_t("", 0), // to fairy
	}),
	dstr_t({" - Empeche son porteur d'evoluer.\n", MEDIUM_SIZE_DESC_TEXT}),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t(" - Gives a normal resistance.\n", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Removes a fire weakness.\n", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Removes a water weakness.\n", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t(" - Removes a grass weakness.\n", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Removes a electric weakness.\n", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t(" - Removes an ice weakness.\n", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t(" - Removes a fighting weakness.\n", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t(" - Removes a poison weakness.\n", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t(" - Removes a ground weakness.\n", MEDIUM_SIZE_DESC_TEXT), // ground
		dstr_t(" - Removes a flying weakness.\n", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t(" - Removes a psychic weakness.\n", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t(" - Removes a bug weakness.\n", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t(" - Removes a rock weakness.\n", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t(" - Removes a ghost weakness.\n", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t(" - Removes a dragon weakness.\n", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t(" - Removes a dark weakness.\n", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t(" - Removes a steel weakness.\n", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t(" - Removes a fairy weakness.\n", MEDIUM_SIZE_DESC_TEXT), // fairy
	}),
	std::array<ItemSearchTree<dstr_t>, 9>({
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - One time use.\n - Gives fire immunity.\n", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - One time use.\n - Gives a water immunity.\n", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - One time use.\n - Gives an electric immunity.\n", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t(" - One time use.\n - Gives a ground immunity.\n", MEDIUM_SIZE_DESC_TEXT), // ground
#if false
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t("", MEDIUM_SIZE_DESC_TEXT), // fairy
#endif
	}),
	dstr_t(" - Its user won't be resisted by rock types.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Only for pawns.\n - Removes all weaknesses.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Only for normal type pieces\n - Usable once.\n - Your first attack will gain one effectiveness stage.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - A baguette.\n - En passant captures are super effective.\n", MEDIUM_SIZE_DESC_TEXT),
	std::array<ItemSearchTree<dstr_t>, 18>({ // resistance berry
		dstr_t(" - Enables its user to terastalize into a normal type.", MEDIUM_SIZE_DESC_TEXT), // normal
		dstr_t(" - Enables its user to terastalize into a fire type.", MEDIUM_SIZE_DESC_TEXT), // fire
		dstr_t(" - Enables its user to terastalize into a water type.", MEDIUM_SIZE_DESC_TEXT), // water
		dstr_t(" - Enables its user to terastalize into a grass type.", MEDIUM_SIZE_DESC_TEXT), // grass
		dstr_t(" - Enables its user to terastalize into a electric type.", MEDIUM_SIZE_DESC_TEXT), // electrik
		dstr_t(" - Enables its user to terastalize into an ice type.", MEDIUM_SIZE_DESC_TEXT), // ice
		dstr_t(" - Enables its user to terastalize into a fighting type.", MEDIUM_SIZE_DESC_TEXT), // fighting
		dstr_t(" - Enables its user to terastalize into a poison type.", MEDIUM_SIZE_DESC_TEXT), // poison
		dstr_t(" - Enables its user to terastalize into a ground type.", MEDIUM_SIZE_DESC_TEXT), // ground
		dstr_t(" - Enables its user to terastalize into a flying type.", MEDIUM_SIZE_DESC_TEXT), // flying
		dstr_t(" - Enables its user to terastalize into a psychic type.", MEDIUM_SIZE_DESC_TEXT), // psychic
		dstr_t(" - Enables its user to terastalize into a bug type.", MEDIUM_SIZE_DESC_TEXT), // bug
		dstr_t(" - Enables its user to terastalize into a rock type.", MEDIUM_SIZE_DESC_TEXT), // rock
		dstr_t(" - Enables its user to terastalize into a ghost type.", MEDIUM_SIZE_DESC_TEXT), // ghost
		dstr_t(" - Enables its user to terastalize into a dragon type.", MEDIUM_SIZE_DESC_TEXT), // dragon
		dstr_t(" - Enables its user to terastalize into a dark type.", MEDIUM_SIZE_DESC_TEXT), // dark
		dstr_t(" - Enables its user to terastalize into a steel type.", MEDIUM_SIZE_DESC_TEXT), // steel
		dstr_t(" - Enables its user to terastalize into a fairy type.", MEDIUM_SIZE_DESC_TEXT), // fairy
	}),
	dstr_t(" - Opposing bug types can't resist the attraction of honey, if they can reach honey they won't be able to move elsewhere.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - May kill the attacking piece when you are captured.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - When its user gets killed by an attack that resulted into a bonus move, the attacker gets send back to the square it started the turn, and the bonus move is skipped.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - May kill its user when it moves\n - Sticks to enemy pieces.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - May kill its user each time it attacks.\n - Increase greatly critical hit rate.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Each RNG roll affecting it's user are performed twice, only the best result is kept.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Increase slightly critical hit rate.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Increase critical hit rate for each consecutive move performed in a row.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Enables its user to crit when performing a non attacking move.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Usable once.\n - Prevents your first attack from missing.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Increases accuracy.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Prevents super effective moves from missing.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Slightly increases evasion.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Ignore opposing items when being attacked.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Ignore opposing items when attacking.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Usable once.\n - Enables for a single knight move.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Usable once.\n - Enables to perform a special move, such as castling or a double step, a second time.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Removes all immunities.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Prevents non capturing moves.\n - Greatly increases evasion.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - Removes any ground immunity.\n - Prevents castling.\n - Pawns can no longer perform a double step.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - May kill its holder, if he is not a poison type, when it moves.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t(" - May kill its holder, if he is not a fire type, when it moves.\n - \n - Might give an increase in critical hit rate for some pieces.\n", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Only for electric type pawns.\n - Greatly increases critical hit rate.\n", MEDIUM_SIZE_DESC_TEXT),
	dstr_t("NotInplementedError", MEDIUM_SIZE_DESC_TEXT),
	dstr_t("NotInplementedError", MEDIUM_SIZE_DESC_TEXT),

	dstr_t(" - Enables pawns to advance one less rank in order to promote.", MEDIUM_SIZE_DESC_TEXT),
}));

ItemSearchTree<dstr_t> const item_description_tree[NB_OF_LANGUAGE] = {
	__french_desc_tree,
	__english_desc_tree
};


dstr_t PokeItem::get_desc(LANGUAGE language) {
	return item_description_tree[(int)language][self];
}

void PokeItem::to_graveyard(Board& board, Piece* holder) {
	switch (id) {
	case item_ID::honey:
		board.honey_holder_mask &= ~get_square_mask(holder->in_limbo, holder->pos);
		break;
	default:
		return;
	}
}

int PokeItem::priority() const {
	return 0;
}

PokeItem item_table[ITEM_TABLE_LEN];

extern std::set<PokeItem> rng_dependant_items;
std::set<PokeItem> rng_dependant_items;

void* init_item_list() {
	int current = 0;
#define __CHECK_BOUNDS if (current >= ITEM_TABLE_LEN) throw
#define ADD(id, rng, ...) __CHECK_BOUNDS; item_table[current++] = PokeItem(id, __VA_ARGS__); if (rng) rng_dependant_items.insert(PokeItem(id, __VA_ARGS__))
	
#define END_LINE __CHECK_BOUNDS; item_table[current++] = PokeItem(item_ID::__newline_placeholder__);
#define ADD_ALL_TYPES(id, rng) \
iter_typing(t) { \
	ADD(id, rng, t); \
}
	ADD(item_ID::evolution_stone, false, fire, normal);
	ADD(item_ID::evolution_stone, false, water, normal);
	ADD(item_ID::evolution_stone, false, electric, normal);
	ADD(item_ID::evolution_stone, false, grass, normal);
	ADD(item_ID::evolution_stone, false, ice, normal);
	ADD(item_ID::evolution_stone, false, psychic, normal);
	ADD(item_ID::evolution_stone, false, dark, normal);
	ADD(item_ID::evolution_stone, false, fairy, normal);
	ADD(item_ID::evolution_stone, false, dragon, normal);
	ADD(item_ID::evolution_stone, false, steel, bug);
	ADD(item_ID::evolution_stone, false, rock, bug);
	ADD(item_ID::king_promotion, false, steel, dark);
	ADD(item_ID::king_promotion, false, psychic, water);
	ADD(item_ID::rare_candy, false);

	END_LINE;

	ADD_ALL_TYPES(item_ID::resistance_berry, false);

	ADD(item_ID::HDB, false, rock);
	ADD(item_ID::eviolite, false);
	ADD(item_ID::immunity, false, ground);
	ADD(item_ID::immunity, false, electric);
	ADD(item_ID::immunity, false, fire);

	END_LINE;

	ADD(item_ID::expert_belt, true);
	ADD(item_ID::blunder_policy, true);
	ADD(item_ID::bright_powder, true);
	ADD(item_ID::wide_lens, true);
	ADD(item_ID::scope_lens, true);
	ADD(item_ID::life_orb, true);
	ADD(item_ID::light_ball, true);
	ADD(item_ID::metronome, true);
	ADD(item_ID::quick_claw, true);

	END_LINE;

	ADD(item_ID::shed_shell, false);
	ADD(item_ID::leppa_berry, false);
	ADD(item_ID::assault_vest, true);
	ADD(item_ID::honey, false, bug);

	END_LINE;

	ADD(item_ID::protective_pads, false);
	ADD(item_ID::safety_googles, false);

	ADD(item_ID::rocky_helmet, true);
	ADD(item_ID::red_card, false);
	ADD(item_ID::sticky_barbs, true);
	ADD(item_ID::black_sludge, true, poison);
	ADD(item_ID::iron_ball, false);
	ADD(item_ID::baguette, false);

	END_LINE;

	ADD_ALL_TYPES(item_ID::tera_shard, false);

#undef ADD
	return NULL;
}

int number_of_drawed_terashard = (int)(Uint64)init_item_list(); // will always return NULL
int number_of_drawed_resistance_berry = 0;