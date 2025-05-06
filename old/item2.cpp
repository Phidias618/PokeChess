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

int PokeItem::usefulness_tier(Piece* piece) {
	switch (id) {
	case item_ID::evolution_stone:
		return piece->Class == Pawn::cls and piece->base_type == old_type;
	case item_ID::king_promotion:
		return (piece->Class == Pawn::cls) ? (piece->type == typeless) + 2 * (piece->type == t1 or piece->type == t2) : 0;
	case item_ID::everstone:
		return (piece->Class == Pawn::cls) ? -1 : 0;
	case item_ID::resistance_berry: {
		effectiveness const e = get_effectiveness(type, piece->type, piece->board.with_reversed_typechart);
		int res = (int)(e == super_effective or (t1 == normal and e == neutral));
		return res - number_of_drawed_resistance_berry;
	}
	case item_ID::immunity:
		return get_effectiveness(type, piece->type, piece->board.with_reversed_typechart) == super_effective ? 2 : 1;
	case item_ID::HDB:
		return get_effectiveness(piece->type, type, piece->board.with_reversed_typechart) == not_very_effective ? 2 : 1;
	case item_ID::eviolite:
		return 2 * (piece->Class == Pawn::cls);
	case item_ID::normal_gem:
		return 2 * (piece->type == type);
	case item_ID::baguette:
		return 2 * (piece->Class == Pawn::cls);
	case item_ID::tera_shard:
		return 2 * (piece->type != type) - number_of_drawed_terashard;
	case item_ID::honey:
		return (get_effectiveness(type, piece->type, piece->board.with_reversed_typechart) <= not_very_effective) ? 2 : 1;
	case item_ID::rocky_helmet:
		return 2;
	case item_ID::red_card:
		return 2;
	case item_ID::sticky_barbs:
		return 2 - (piece->Class != Pawn::cls) - 2 * (piece->Class == King::cls);
	case item_ID::life_orb:
		return 2;
	case item_ID::loaded_dice:
		return 2;
	case item_ID::scope_lens:
		return (piece->board.with_ability and piece->ability_no_guard) ? 0 : 2;
	case item_ID::metronome:
		return 2;
	case item_ID::quick_claw:
		return 2;
	case item_ID::blunder_policy:
		return (piece->board.with_ability and piece->ability_no_guard) ? 0 : 2;
	case item_ID::wide_lens:
		return 2;
	case item_ID::expert_belt: {
		int counter = 0;
		iter_typing(t) {
			if (get_effectiveness(piece->type, t, piece->board.with_reversed_typechart) == super_effective)
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
		return (piece->Class == Knight::cls) ? 0 : 2;
	case item_ID::leppa_berry:
		return 2 * (piece->Class == King::cls or piece->Class == Pawn::cls);
	case item_ID::ring_target: {
		int counter = 0;
		iter_typing(t) {
			if (get_effectiveness(t, piece->type, piece->board.with_reversed_typechart) == immune) {
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
			return (piece->board.with_ability and piece->ability_guts) ? 2 : -1;
		}
	case item_ID::light_ball:
		return (piece->Class == Pawn::cls) ? (piece->type == typeless) + 2 * (piece->type == electric) : 0;
	case item_ID::deep_sea_scale:
	case item_ID::deep_sea_tooth:
		return (piece->Class == Pawn::cls) ? (piece->type == typeless) + 2 * (piece->type == water) : 0;
	default:
		return 0;
	}
}

void PokeItem::draw(Surface dest, SDL_Rect* rect, size s, anchor a) {
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

void PokeItem::attack_modifier(move_data& data) {
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

void PokeItem::defense_modifier(move_data& data) {
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
		if (data.defender->Class == Pawn::cls) {
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

void PokeItem::accuracy_modifier(move_data& data) {
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

void PokeItem::crit_modifier(move_data& data) {
	Piece* const holder = data.attacker;
	switch (id) {
	case item_ID::life_orb:
		data.crit_rate *= 2;
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
		data.crit_rate * 1.3;
		break;
	case item_ID::metronome: {
		for (move_data const& d : holder->board.move_historic) {
			if (d.attacker->color == holder->color) {
				if (d.attacker == holder && d.defender != NULL) {
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
		if (holder->board.with_ability and holder->ability_guts) {
			data.crit_rate *= 2;
		}
		break;
	case item_ID::light_ball: {
		if (holder->type == electric and (holder->Class == Pawn::cls or holder->evolved))
			data.crit_rate *= 2;
		break;
	}
	case item_ID::deep_sea_tooth:
		if (holder->type == water and (holder->Class == Pawn::cls))
			data.crit_rate *= 2;
		break;
	default:
		return;
	}
}


void PokeItem::evasion_modifier(move_data & data) {
	Piece* const holder = data.defender;

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
		data.miss_rate = 1 - data.miss_rate;
		data.miss_rate *= 0.825;
		data.miss_rate = 1 - data.miss_rate;
		break;
	default:
		return;
	}
}

void PokeItem::adjust_reachable(Piece* const holder) {
	switch (id) {
	case item_ID::shed_shell: {
		Uint64 const old = holder->reachable_mask;
		Knight knight = *dynacast_unsafe<Knight>(holder);
		knight.square = holder->square;
		knight.set_reachable();
		knight.reachable_mask &= ~holder->board.presence_mask;
		holder->item_reachable_mask = knight.reachable_mask & ~old;
		holder->reachable_mask = knight.reachable_mask | old;
		break;
	}
	case item_ID::leppa_berry: {
		if (not holder->has_already_move)
			return;
		holder->has_already_move = false;
		Uint64 old_mask = holder->reachable_mask;
		holder->set_reachable();
		holder->item_reachable_mask = holder->reachable_mask & ~old_mask;
		holder->has_already_move = true;
		break;
	}
	case item_ID::assault_vest:
		holder->reachable_mask &= holder->board.presence_mask;
		break;
	case item_ID::iron_ball:
		if (holder->Class == Pawn::cls) {
			int direction = holder->color == white ? 1 : -1;
			short Y = holder->y + 2 * direction;
			if (0 <= 0 and Y <= 7) {
				holder->reachable_mask &= ~(0b1111'1111ull << (Y << 3));
			}
		}
		else if (holder->Class == King::cls) {
			if (not holder->has_already_move) {
				holder->has_already_move = true;
				holder->set_reachable();
				holder->has_already_move = false;
			}
		}
		break;
	default:
		return;
	}
}


move_data PokeItem::move_to(Piece* holder, Square& target) {
	move_data data;
	
	switch (id) {
	case item_ID::shed_shell:
		data = holder->base_move_to(target);
		used = true;
		break;
	case item_ID::leppa_berry:
		holder->has_already_move = false;
		data = holder->base_move_to(target);
		data.was_piece_first_move = false;
		holder->has_already_move = true;
		holder->set_item(NO_ITEM);
		break;
	default:
		throw;
	}
	return data;
}

void PokeItem::move_with(Piece* holder, Square& begin_square, Square& end_square) {
	switch (id) {
	case item_ID::honey:
		holder->board.honey_holder_mask &= ~begin_square.get_mask();
		holder->board.honey_holder_mask |= end_square.get_mask();
		break;
	case item_ID::metronome:

		break;
	default:
		return;
	}
}

void PokeItem::after_move_effects(move_data& data) {
	Piece* const holder = data.attacker;
#if ENABLE_SAFETY_CHECKS
	if (holder->item != self)
		throw;
#endif
	switch (id) {
	case item_ID::everstone:
		if (data.promotion) {
			data.promotion = false;
			data.interrupt_move = false;
		}
		break;
	case item_ID::normal_gem:
		if (not (data.do_miss or data.do_crit or data.defender == NULL or data.cancel)) {
			holder->set_item(PokeItem());
		}
		break;
	case item_ID::honey:
		if (not data.cancel) {
			holder->board.honey_holder_mask &= ~data.begin_square->get_mask();
			if (not data.suicide) {
				holder->board.honey_holder_mask |= holder->square->get_mask();
			}
		}
		break;
	case item_ID::sticky_barbs:
		if ((not data.suicide) and (((double)game.RNG() / (double)game.RNG.max()) < 0.125)) {
			holder->square->to_graveyard();
			used = true;
			data.suicide = true;
			data.move_again = false;
		}
		break;
	case item_ID::life_orb:
		if (data.defender != NULL and not data.suicide && ((double)game.RNG() / (double)game.RNG.max()) <= 0.1) {
			holder->square->to_graveyard();
			used = true;
			data.suicide = true;
		}
		break;
	case item_ID::quick_claw:
		if (data.defender == NULL and not data.move_again) {
			if ((double)game.RNG() / (double)game.RNG.max() <= data.crit_rate) {
				data.do_crit = data.move_again = true;
			}
		}
		break;
	case item_ID::black_sludge:
	case item_ID::flame_orb:
		if (holder->type != type and ((double)game.RNG() / (double)game.RNG.max()) < 0.125) {
			holder->square->to_graveyard();
			used = true;
			data.suicide = true;
		}
		break;
	default:
		return;
	}
}


void PokeItem::revenge(move_data& data) {
	Piece* const holder = data.defender;
#if ENABLE_SAFETY_CHECKS
	if (holder->item != self)
		throw;
#endif

	switch (id) {
	case item_ID::resistance_berry: {
		if (holder->is_in_graveyard and data.attacker->type == type and (data.matchup == not_very_effective or (type == normal and data.matchup == neutral)))
			used = true;
		break;
	}
	case item_ID::immunity:
		if ((not holder->is_in_graveyard) and data.attacker->type == type) {
			used = true;
		}
		break;
	case item_ID::rocky_helmet:
		if (holder->is_in_graveyard and not data.suicide and ((double)game.RNG() / (double)game.RNG.max()) < 1.0 / 6.0) {
			data.attacker->square->to_graveyard();
			used = true;
			data.suicide = true;
		}
		break;
	case item_ID::red_card:
		if (holder->is_in_graveyard and data.move_again and not data.suicide) {
			data.move_again = false;
			used = true;
			game.board.in_bonus_move = false;

			data.attacker->set_square(data.begin_square);
		}
		break;
	case item_ID::sticky_barbs:
		if (holder->is_in_graveyard) {
			used = false;
			data.attacker->set_item(self);
			holder->set_item(NO_ITEM);
		}
		break;
	default:
		return;
	}
}

void PokeItem::promote(Piece* new_holder) {
	switch (id) {
	case item_ID::evolution_stone:
		if (new_holder->type == old_type) {
			new_holder->type = new_type;
			new_holder->set_item(PokeItem());
		}
		break;
	case item_ID::king_promotion:
		if (new_holder->Class == King::cls) {
			new_holder->set_item(PokeItem());
		}
		break;
	case item_ID::light_ball:
		if (new_holder->type == electric) {
			new_holder->set_pokeicon(PokemonIcon(27 + 2 * new_holder->Class->id + new_holder->color, 1 + electric));
		}
		break;
	case item_ID::deep_sea_scale:
		break;
	case item_ID::deep_sea_tooth:
		break;
	default:
		return;
	}
}


bool PokeItem::prepare_promotion(Piece* holder, Uint8* avaible) {
	switch (id) {
	case item_ID::king_promotion:
		if (holder->base_type == t1 or holder->base_type == t2) {
			*avaible = 0b00011111;
			// 1-th bit for king
			// 2-th bit for queen
			// 3-th bit for bishop
			// 4-th bit for knight
			// 5-th bit for rook
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


void PokeItem::add_cosmetic(move_data const& data) {
	switch (id) {
	case item_ID::immunity:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.defender->Class->name[(int)game.language]);
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
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to Rcky Hlmt");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::red_card:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nwas sent back.");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::sticky_barbs:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to StckyBrbs");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::life_orb:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to life orb");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::shed_shell:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nfled successfuly");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::black_sludge:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to\nBlack Sludge");
			game.add_textbox(buffer);
		}
		break;
	case item_ID::flame_orb:
		if (used) {
			char buffer[256] = "";
			strcat_s(buffer, data.attacker->Class->name[(int)game.language]);
			strcat_s(buffer, "\nDied to Flame Orb");
			game.add_textbox(buffer);
		}
		break;
	default:
		return;
	}
}


void PokeItem::update_status(move_data const& data) {
	switch (id) {
	case item_ID::resistance_berry:
	case item_ID::immunity:
	case item_ID::red_card:
		if (used) {
			used = false;
			data.defender->set_item(NO_ITEM);
		}
		break;
	case item_ID::shed_shell:
		if (used) {
			used = false;
			data.attacker->set_item(NO_ITEM);
		}
		break;
	}
}

void PokeItem::select_holder(Piece* holder) {
	switch (id) {
	case item_ID::tera_shard:
		if ((holder->color == white and game.board.white_tera) or (holder->color == black and game.board.black_tera))
			game.buttons->add(new TeraButton(9, 10.5, holder, type));
		break;
	default:
		return;
	}
}


void PokeItem::change_holder(Piece* previous_holder, Piece* new_holder) {
	switch (id) {
	case item_ID::evolution_stone:
		
		break;
	case item_ID::king_promotion:

		break;
	case item_ID::everstone:
		break;
	case item_ID::resistance_berry:
		number_of_drawed_resistance_berry++;
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
		number_of_drawed_terashard++;
		break;
	case item_ID::honey:
		if (previous_holder != NULL)
			previous_holder->board.honey_holder_mask &= ~previous_holder->square->get_mask();
		if (new_holder != NULL)
			new_holder->board.honey_holder_mask |= new_holder->square->get_mask();
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
	}
}

void PokeItem::sync_holder(Piece* holder) {
	switch (id) {
	case item_ID::evolution_stone:
		if (holder->base_type == old_type)
			holder->set_pokeicon(PokemonIcon(21 + holder->color, 1 + new_type));
		break;
	case item_ID::king_promotion:
		if (holder->Class == Pawn::cls and (holder->type == t1 or holder->type == t2))
			holder->set_pokeicon(PokemonIcon(25 + holder->color, 1 + t1));
		break;
	case item_ID::light_ball:
		if (holder->type == electric and holder->Class == Pawn::cls) {
			holder->set_pokeicon(PokemonIcon(25 + holder->color, 1 + electric));
		}
		break;
	case item_ID::deep_sea_scale:
	case item_ID::deep_sea_tooth:

		break;
	default:
		return;
	}
}

void PokeItem::to_graveyard(Piece* holder) {
	switch (id) {
	case item_ID::honey:
		holder->board.honey_holder_mask &= ~holder->square->get_mask();
		break;
	default:
		return;
	}
}

int PokeItem::priority() {
	return 0;
}

PokeItem item_table[ITEM_TABLE_LEN];

extern std::set<PokeItem> rng_dependant_items;
std::set<PokeItem> rng_dependant_items;

void* init_item_list() {
	int current = 0;
#define __CHECK_BOUNDS if (current+1 >= ITEM_TABLE_LEN) throw
#define ADD(id, rng, ...) item_table[current++] = PokeItem(id, __VA_ARGS__); __CHECK_BOUNDS; if (rng) rng_dependant_items.insert(PokeItem(id, __VA_ARGS__))
	
#define END_LINE item_table[current++] = PokeItem(item_ID::__newline_placeholder__); __CHECK_BOUNDS
#define ADD_ALL_TYPES(id, rng) \
iter_typing(t) { \
	ADD(id, rng, t); \
}
	ADD(item_ID::evolution_stone, false, normal, fire);
	ADD(item_ID::evolution_stone, false, normal, water);
	ADD(item_ID::evolution_stone, false, normal, electric);
	ADD(item_ID::evolution_stone, false, normal, grass);
	ADD(item_ID::evolution_stone, false, normal, ice);
	ADD(item_ID::evolution_stone, false, normal, psychic);
	ADD(item_ID::evolution_stone, false, normal, dark);
	ADD(item_ID::evolution_stone, false, normal, fairy);
	ADD(item_ID::evolution_stone, false, water, dragon);
	ADD(item_ID::evolution_stone, false, bug, steel);
	ADD(item_ID::evolution_stone, false, bug, rock);
	ADD(item_ID::king_promotion, false, steel, dark);
	ADD(item_ID::king_promotion, false, psychic, water);

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