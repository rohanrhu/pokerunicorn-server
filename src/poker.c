/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "../include/poker.h"

#include "../include/pkrsrv.h"
#include "../include/sugar.h"
#include "../include/util.h"
#include "../include/ref.h"
#include "../include/account.h"
#include "../include/card.h"
#include "../include/eventloop.h"

pkrsrv_poker_t* pkrsrv_poker_new(pkrsrv_poker_new_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.table);
    
    pkrsrv_poker_t* poker = malloc(sizeof(pkrsrv_poker_t));
    PKRSRV_REF_COUNTED_INIT(poker, pkrsrv_poker_free);
    poker->owner = params.owner;
    poker->eventloop = params.eventloop;
    poker->on_action = params.on_action;
    poker->process_latency = pkrsrv_process_latency;
    poker->table = params.table;
    poker->state = PKRSRV_POKER_STATE_CREATED;
    poker->playing_position = -1;
    poker->hand_kind = PKRSRV_POKER_HAND_KIND_PREFLOP;
    poker->synced_action_counter = 0;
    poker->last_raised_pos = -1;
    poker->last_pos_before_last_raise = -1;
    poker->is_sb_must_call = false;
    poker->winner = NULL;
    poker->scores_length = 0;
    poker->starter_players_num = 0;
    poker->is_playing_last = false;
    poker->pot_amount = 0;
    poker->current_raise = -1;
    poker->current_raise_total = -1;
    poker->current_bet = -1;
    poker->starting_time = 0;
    poker->action_time = 0;

    pkrsrv_poker_hand_t* hands[] = {
        &poker->hand_preflop,
        &poker->hand_flop,
        &poker->hand_turn,
        &poker->hand_river
    };

    for (int i=0; i < 4; i++) {
        pkrsrv_poker_hand_t* hand = hands[i];
        hand->actions = malloc(sizeof(pkrsrv_poker_actions_t));
        LIST_INIT(hand->actions);
        hand->is_raised = false;
        hand->cards[0] = (pkrsrv_card_t) {.index = -1, .kind = -1};
        hand->cards[1] = (pkrsrv_card_t) {.index = -1, .kind = -1};
        hand->cards[2] = (pkrsrv_card_t) {.index = -1, .kind = -1};
        hand->is_checked = false;
    }

    poker->hand_preflop.kind = PKRSRV_POKER_HAND_KIND_PREFLOP;
    poker->hand_flop.kind = PKRSRV_POKER_HAND_KIND_FLOP;
    poker->hand_turn.kind = PKRSRV_POKER_HAND_KIND_TURN;
    poker->hand_river.kind = PKRSRV_POKER_HAND_KIND_RIVER;
    
    poker->players = pkrsrv_poker_players_new((pkrsrv_poker_players_new_params_t) {
        .poker = poker
    });
    
    poker->sorted_players = malloc(poker->table->max_players * sizeof(pkrsrv_poker_player_t));

    for (int i=0; i < 52; i++) {
        poker->card_history[i] = (pkrsrv_card_t) {
            .index = -1,
            .kind = -1
        };
    }

    return poker;
}

void pkrsrv_poker_ready(pkrsrv_poker_t* poker) {
    poker->state = PKRSRV_POKER_STATE_W4P;
    poker->players->button_position = 0;
}

void pkrsrv_poker_starting(pkrsrv_poker_t* poker) {
    poker->state = PKRSRV_POKER_STATE_STARTING;
    poker->starting_time = pkrsrv_util_get_time_msec();
}

bool pkrsrv_poker_start(pkrsrv_poker_t* poker) {
    int playable_count = pkrsrv_poker_players_get_playable_count(poker);

    if (playable_count < 2) {
        return false;
    }

    poker->state = PKRSRV_POKER_STATE_SMALL_BLIND;
    poker->action_time = pkrsrv_util_get_time_msec();
    poker->proceed_time = pkrsrv_util_get_time_msec();

    for (int i = poker->players->button_position;; i++) {
        i = i % 10;

        pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_position(poker, i);
        
        if (!player || player->is_left) {
            continue;
        }

        poker->players->sb_player = player;
        
        break;
    }

    poker->players->button_position = poker->players->sb_player->position;
    
    for (int i = poker->players->button_position + 1;; i++) {
        i = i % 10;
        
        pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_position(poker, i);

        if (!player || player->is_left) {
            continue;
        }

        poker->players->bb_player = player;

        break;
    }

    poker->playing_position = poker->players->sb_player->position;
    
    poker->state = PKRSRV_POKER_STATE_SMALL_BLIND;
    
    {
        pkrsrv_poker_action_t params;
        params.account = *(poker->players->sb_player->account);
        params.kind = PKRSRV_POKER_ACTION_KIND_SMALL_BLIND;
        params.amount = poker->table->small_blind;

        pkrsrv_poker_do_action(poker, poker->players->sb_player, params);
    }

    poker->state = PKRSRV_POKER_STATE_BIG_BLIND;

    {
        pkrsrv_poker_action_t params;
        params.account = *(poker->players->bb_player->account);
        params.kind = PKRSRV_POKER_ACTION_KIND_BIG_BLIND;
        params.amount = poker->table->big_blind;

        pkrsrv_poker_do_action(poker, poker->players->bb_player, params);
    }

    poker->current_bet = poker->table->big_blind;
    poker->current_raise = poker->table->big_blind;
    poker->current_raise_total = poker->table->big_blind;

    poker->starter_players_num = pkrsrv_poker_players_get_playing_count(poker);

    pkrsrv_poker_deal(poker);

    poker->state = PKRSRV_POKER_STATE_PREFLOP;

    return true;
}

bool pkrsrv_poker_restart(pkrsrv_poker_t* poker) {
    poker->state = PKRSRV_POKER_STATE_W4P;
    poker->hand_kind = PKRSRV_POKER_HAND_KIND_PREFLOP;
    poker->synced_action_counter = 0;
    poker->last_raised_pos = -1;
    poker->last_pos_before_last_raise = -1;
    poker->is_sb_must_call = false;
    poker->winner = NULL;
    poker->scores_length = 0;
    poker->is_playing_last = false;
    poker->pot_amount = 0;
    poker->current_raise = -1;
    poker->current_raise_total = -1;
    poker->current_bet = -1;

    pkrsrv_poker_hand_t* hands[] = {
        &poker->hand_preflop,
        &poker->hand_flop,
        &poker->hand_turn,
        &poker->hand_river
    };

    for (int i=0; i < 4; i++) {
        pkrsrv_poker_hand_t* hand = hands[i];

        LIST_FOREACH(hand->actions, action)
            free(action);
        END_FOREACH

        free(hand->actions);
        
        hand->actions = malloc(sizeof(pkrsrv_poker_actions_t));
        LIST_INIT(hand->actions);
        hand->is_raised = false;
    }

    poker->hand_preflop.kind = PKRSRV_POKER_HAND_KIND_PREFLOP;
    poker->hand_flop.kind = PKRSRV_POKER_HAND_KIND_FLOP;
    poker->hand_turn.kind = PKRSRV_POKER_HAND_KIND_TURN;
    poker->hand_river.kind = PKRSRV_POKER_HAND_KIND_RIVER;

    for (int i=0; i < 52; i++) {
        poker->card_history[i] = (pkrsrv_card_t) {
            .index = -1,
            .kind = -1
        };
    }

    poker->players->sb_player = NULL;
    poker->players->bb_player = NULL;

    poker->players->button_position++;
    if (poker->players->button_position >= poker->players->length) {
        poker->players->button_position = 0;
    }

    LIST_FOREACH(poker->players, player)
        player->is_playing = true;
        player->is_dealt = false;
        player->is_folded = false;
        player->is_allin = false;
        player->bet = 0;
        player->cards[0] = (pkrsrv_card_t) {.index = -1, .kind = -1};
        player->cards[1] = (pkrsrv_card_t) {.index = -1, .kind = -1};
    END_FOREACH

    pkrsrv_poker_players_get_sorted(poker, poker->sorted_players);

    for (int i=0; i < poker->scores_length; i++) {
        PKRSRV_REF_COUNTED_LEAVE(poker->scores[i].player);
    }

    poker->scores_length = 0;

    return pkrsrv_poker_start(poker);
}

void pkrsrv_poker_free(pkrsrv_poker_t* poker) {
    PKRSRV_REF_COUNTED_LEAVE(poker->table);
    
    pkrsrv_poker_players_free(poker->players);
    free(poker->sorted_players);

    pkrsrv_poker_hand_t* hands[] = {
        &poker->hand_preflop,
        &poker->hand_flop,
        &poker->hand_turn,
        &poker->hand_river
    };

    for (int i = 0; i < sizeof(hands) / sizeof(pkrsrv_poker_hand_t*); i++) {
        pkrsrv_poker_hand_t* hand = hands[i];

        LIST_FOREACH(hand->actions, action)
            free(action);
        END_FOREACH
        
        free(hand->actions);
    }

    for (int i=0; i < poker->scores_length; i++) {
        PKRSRV_REF_COUNTED_LEAVE(poker->scores[i].player);
    }

    free(poker);
}

void pkrsrv_poker_deal(pkrsrv_poker_t* poker) {
    pkrsrv_poker_players_deal(poker);
}

pkrsrv_poker_hand_t* pkrsrv_poker_get_current_hand(pkrsrv_poker_t* poker) {
    if (poker->state == PKRSRV_POKER_STATE_FLOP) {
        return &poker->hand_flop;
    }
    if (poker->state == PKRSRV_POKER_STATE_TURN) {
        return &poker->hand_turn;
    }
    if (poker->state == PKRSRV_POKER_STATE_RIVER) {
        return &poker->hand_river;
    }
    if (poker->state <= PKRSRV_POKER_STATE_PREFLOP) {
        return &poker->hand_preflop;
    }

    return NULL;
}

pkrsrv_poker_actions_action_t* pkrsrv_poker_actions_action_new(pkrsrv_poker_actions_action_new_params_t params) {
    pkrsrv_poker_actions_action_t* action = malloc(sizeof(pkrsrv_poker_actions_action_t));
    LIST_ITEM_INIT(action);
    action->action = params.action;
    action->player = params.player;

    return action;
}

void pkrsrv_poker_actions_add(pkrsrv_poker_actions_t* actions, pkrsrv_poker_actions_action_t* action) {
    LIST_APPEND(actions, action);
}

bool pkrsrv_poker_player_set_balance(pkrsrv_poker_player_t* player, uint64_t amount) {
    player->balance = amount;
    return true;
}

bool pkrsrv_poker_player_remove_balance(pkrsrv_poker_player_t* player, uint64_t amount) {
    if (player->balance < amount) {
        return false;
    }

    uint64_t new_balance = player->balance - amount;

    pkrsrv_poker_player_set_balance(player, new_balance);

    return true;
}

bool pkrsrv_poker_player_add_balance(pkrsrv_poker_player_t* player, uint64_t amount) {
    player->balance += amount;
    return true;
}

int pkrsrv_poker_get_last_playing_position(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* action_player) {
    int actual_pos_to_play_last = -1;

    if (poker->last_raised_pos == -1) {
        actual_pos_to_play_last = poker->players->bb_player->position;
        goto RETURN;
    }

    int iterated = 0;

    for (int i=0;; i = (i+1) % poker->players->length) {
        if (iterated == poker->players->length) {
            break;
        }

        iterated++;

        pkrsrv_poker_player_t* player = poker->sorted_players[i];

        if ((player->position == poker->last_raised_pos) && player->is_playing && !player->is_left && !player->is_folded && !player->is_allin) {
            actual_pos_to_play_last = player->position;
            break;
        }

        if (player->is_playing && !player->is_left && !player->is_folded && !player->is_allin && (player->bet < poker->current_bet)) {
            actual_pos_to_play_last = player->position;
        }
    }

    RETURN:

    return actual_pos_to_play_last;
}

pkrsrv_poker_action_result_t pkrsrv_poker_do_action(
    pkrsrv_poker_t* poker,
    pkrsrv_poker_player_t* player,
    pkrsrv_poker_action_t action
) {
    pkrsrv_poker_action_result_t result = (pkrsrv_poker_action_result_t) {
        .is_done = true
    };

    if (poker->state > PKRSRV_POKER_STATE_RIVER) {
        // ! Unexpected Behavior
        result.is_done = false;
        goto RETURN;
    }

    if (!player->is_playing) {
        // ! Unexpected Behavior
        result.is_done = false;
        goto RETURN;
    }

    if (player->is_folded) {
        // ! Unexpected Behavior
        result.is_done = false;
        goto RETURN;
    }

    if (poker->state < PKRSRV_POKER_STATE_SMALL_BLIND) {
        // ! Unexpected Behavior
        result.is_done = false;
        goto RETURN;
    }

    if (player->position != poker->playing_position) {
        // ! Unexpected Behavior
        result.is_done = false;
        goto RETURN;
    }

    if (player->is_allin) {
        // ! Unexpected Behavior
        result.is_done = false;
        goto RETURN;
    }
    
    pkrsrv_poker_hand_t* hand = pkrsrv_poker_get_current_hand(poker);

    if (poker->state == PKRSRV_POKER_STATE_SMALL_BLIND) {
        if (player != poker->players->sb_player) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        if (action.kind != PKRSRV_POKER_ACTION_KIND_SMALL_BLIND) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }
        
        if (!pkrsrv_poker_player_remove_balance(player, poker->table->small_blind)) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }
        
        player->bet += poker->table->small_blind;
        poker->pot_amount += poker->table->small_blind;

        poker->is_sb_must_call = true;
    }

    if (poker->state == PKRSRV_POKER_STATE_BIG_BLIND) {
        if (player != poker->players->bb_player) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }
        
        if (action.kind != PKRSRV_POKER_ACTION_KIND_BIG_BLIND) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        if (!pkrsrv_poker_player_remove_balance(player, poker->table->big_blind)) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        player->bet += poker->table->big_blind;
        poker->pot_amount += poker->table->big_blind;

        hand->is_raised = true;
    }

    if (action.kind == PKRSRV_POKER_ACTION_KIND_RAISE) {
        uint64_t raise_to = action.amount;
        uint64_t to_bet = raise_to - player->bet;
        uint64_t raise_by = raise_to - poker->current_bet;
        uint64_t min_raise_to = poker->current_bet + poker->current_raise;

        if (player->balance < to_bet) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        bool is_allin = player->balance == to_bet;
        bool is_insufficient_allin = is_allin && (to_bet < poker->current_raise);

        if (!is_allin && (raise_by < poker->current_raise)) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        if (!is_allin && poker->is_sb_must_call && (poker->players->sb_player == player) && (raise_to < min_raise_to)) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        if (!pkrsrv_poker_player_remove_balance(player, to_bet)) {
            // ! Unexpected Behavior
            result.is_done = false;
            goto RETURN;
        }

        if (poker->is_sb_must_call && (poker->players->sb_player == player)) {
            poker->is_sb_must_call = false;
        }

        if (raise_to >= poker->current_bet) {
            if (raise_by >= poker->current_raise) {
                poker->current_raise = raise_by;
            }

            if (raise_to > poker->current_bet) {
                hand->is_raised = true;
                poker->current_bet += raise_by; 
            }

            poker->current_raise_total += raise_by;

            player->bet += to_bet;
            poker->pot_amount += to_bet;

            poker->last_raised_pos = player->position;
        }

        poker->synced_action_counter++;

        if (player == poker->players->sb_player) {
            poker->is_sb_must_call = false;
        }
    } else if (action.kind == PKRSRV_POKER_ACTION_KIND_FOLD) {
        player->is_folded = true;
    } else if (action.kind == PKRSRV_POKER_ACTION_KIND_CHECK) {
        if (poker->is_sb_must_call && (player->account->id == poker->players->sb_player->account->id)) {
            // * SB calls (BB - SB)
            
            if (player->balance < poker->table->small_blind) {
                // ! Unexpected Behavior
                result.is_done = false;
                goto RETURN;
            }

            if (!pkrsrv_poker_player_remove_balance(player, poker->table->small_blind)) {
                // ! Unexpected Behavior
                result.is_done = false;
                goto RETURN;
            }

            poker->pot_amount += poker->table->small_blind;
            player->bet += poker->table->small_blind;

            poker->is_sb_must_call = false;
        } else if (hand->is_raised) {
            // * Calling
            
            uint64_t to_bet = poker->current_bet - player->bet;
            
            if (!pkrsrv_poker_player_remove_balance(player, to_bet)) {
                // * Insufficient All-in
                player->is_allin = true;
                to_bet = player->balance;
                pkrsrv_poker_player_set_balance(player, 0);
            }
            
            poker->pot_amount += to_bet;
            player->bet += to_bet;
        } else {
            // * Checking
        }
    }

    if (player->balance <= 0) {
        player->is_allin = true;
    }

    if (poker->last_pos_before_last_raise == -1) {
        poker->last_pos_before_last_raise = player->position;
    }

    pkrsrv_poker_actions_action_t* hand_action = pkrsrv_poker_actions_action_new((pkrsrv_poker_actions_action_new_params_t) {
        .action = action,
        .player = player
    });
    
    pkrsrv_poker_actions_add(hand->actions, hand_action);

    int playing_players_count = pkrsrv_poker_players_get_playing_count(poker);
    int ingame_players_count = pkrsrv_poker_players_get_ingame_count(poker);

    if (playing_players_count == 0) {
        goto NEXT_PLAYER_CHECKED;
    }

    NEXT_PLAYER:

    poker->playing_position++;

    if (poker->playing_position > poker->sorted_players[poker->players->length-1]->position) {
        poker->playing_position = poker->sorted_players[0]->position;
    }
    
    pkrsrv_poker_player_t* next_player = pkrsrv_poker_players_getby_position_in_sorted(poker, poker->sorted_players, poker->playing_position);

    if (!next_player || !next_player->is_playing || next_player->is_folded || next_player->is_allin) {
        goto NEXT_PLAYER;
    }

    NEXT_PLAYER_CHECKED:

    if (poker->state == PKRSRV_POKER_STATE_PREFLOP) {
        if (action.kind == PKRSRV_POKER_ACTION_KIND_RAISE) {
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_CHECK) {
            poker->synced_action_counter++;

            int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);

            bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                  ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
            
            if (is_last_to_act) {
                poker->state = PKRSRV_POKER_STATE_FLOP;
                poker->synced_action_counter = 0;
                poker->last_raised_pos = -1;
                poker->last_pos_before_last_raise = -1;
                if (poker->starter_players_num == 2) {
                    poker->playing_position = poker->players->bb_player->position;
                }

                poker->hand_flop.cards[0] = pkrsrv_card_get_random(poker->card_history);
                poker->hand_flop.cards[1] = pkrsrv_card_get_random(poker->card_history);
                poker->hand_flop.cards[2] = pkrsrv_card_get_random(poker->card_history);
            }
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_FOLD) {
            if (ingame_players_count == 1) {
                poker->winner = NULL;

                LIST_FOREACH(poker->players, _player)
                    if (_player->is_playing && !_player->is_folded) {
                        poker->winner = _player;
                        break;
                    }
                END_FOREACH

                PKRSRV_UTIL_ASSERT(poker->winner != NULL);

                poker->state = PKRSRV_POKER_STATE_DONE;

                pkrsrv_poker_player_add_balance(poker->winner, poker->pot_amount);

                goto DO_ACTION_END;
            } else {
                int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
                
                bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                      ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
                
                if (is_last_to_act) {
                    poker->state = PKRSRV_POKER_STATE_FLOP;
                    poker->synced_action_counter = 0;
                    poker->last_raised_pos = -1;
                    poker->last_pos_before_last_raise = -1;
                    if (poker->starter_players_num == 2) {
                        poker->playing_position = poker->players->bb_player->position;
                    }

                    poker->hand_flop.cards[0] = pkrsrv_card_get_random(poker->card_history);
                    poker->hand_flop.cards[1] = pkrsrv_card_get_random(poker->card_history);
                    poker->hand_flop.cards[2] = pkrsrv_card_get_random(poker->card_history);
                }
            }
        }
    } else if (poker->state == PKRSRV_POKER_STATE_FLOP) {
        if (action.kind == PKRSRV_POKER_ACTION_KIND_RAISE) {
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_CHECK) {
            poker->synced_action_counter++;
            
            int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
            
            bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                  ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
            
            if (is_last_to_act) {
                poker->state = PKRSRV_POKER_STATE_TURN;
                poker->synced_action_counter = 0;
                poker->last_raised_pos = -1;
                poker->last_pos_before_last_raise = -1;
                if (poker->starter_players_num == 2) {
                    poker->playing_position = poker->players->bb_player->position;
                }

                poker->hand_turn.cards[0] = pkrsrv_card_get_random(poker->card_history);
            }
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_FOLD) {
            if (ingame_players_count == 1) {
                poker->winner = NULL;

                LIST_FOREACH(poker->players, _player)
                    if (_player->is_playing && !_player->is_folded) {
                        poker->winner = _player;
                        break;
                    }
                END_FOREACH

                PKRSRV_UTIL_ASSERT(poker->winner != NULL);

                poker->state = PKRSRV_POKER_STATE_DONE;

                pkrsrv_poker_player_add_balance(poker->winner, poker->pot_amount);

                goto DO_ACTION_END;
            } else {
                poker->synced_action_counter++;
                
                int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
                
                bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                      ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
                
                if (is_last_to_act) {
                    poker->state = PKRSRV_POKER_STATE_TURN;
                    poker->synced_action_counter = 0;
                    poker->last_raised_pos = -1;
                    poker->last_pos_before_last_raise = -1;
                    if (poker->starter_players_num == 2) {
                        poker->playing_position = poker->players->bb_player->position;
                    }

                    poker->hand_turn.cards[0] = pkrsrv_card_get_random(poker->card_history);
                }
            }
        }
    } else if (poker->state == PKRSRV_POKER_STATE_TURN) {
        if (action.kind == PKRSRV_POKER_ACTION_KIND_RAISE) {
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_CHECK) {
            poker->synced_action_counter++;
            
            int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
            
            bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                  ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
            
            if (is_last_to_act) {
                poker->state = PKRSRV_POKER_STATE_RIVER;
                poker->synced_action_counter = 0;
                poker->last_raised_pos = -1;
                poker->last_pos_before_last_raise = -1;
                if (poker->starter_players_num == 2) {
                    poker->playing_position = poker->players->bb_player->position;
                }

                poker->hand_river.cards[0] = pkrsrv_card_get_random(poker->card_history);
            }
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_FOLD) {
            if (ingame_players_count == 1) {
                poker->winner = NULL;

                LIST_FOREACH(poker->players, _player)
                    if (_player->is_playing && !_player->is_folded) {
                        poker->winner = _player;
                        break;
                    }
                END_FOREACH

                PKRSRV_UTIL_ASSERT(poker->winner != NULL);

                poker->state = PKRSRV_POKER_STATE_DONE;

                pkrsrv_poker_player_add_balance(poker->winner, poker->pot_amount);

                goto DO_ACTION_END;
            } else {
                poker->synced_action_counter++;
                
                int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
                
                bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                      ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
                
                if (is_last_to_act) {
                    poker->state = PKRSRV_POKER_STATE_RIVER;
                    poker->synced_action_counter = 0;
                    poker->last_raised_pos = -1;
                    poker->last_pos_before_last_raise = -1;
                    if (poker->starter_players_num == 2) {
                        poker->playing_position = poker->players->bb_player->position;
                    }
                    
                    poker->hand_river.cards[0] = pkrsrv_card_get_random(poker->card_history);
                }
            }
        }
    } else if (poker->state == PKRSRV_POKER_STATE_RIVER) {
        if (action.kind == PKRSRV_POKER_ACTION_KIND_RAISE) {
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_CHECK) {
            poker->synced_action_counter++;
            
            int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
            
            bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                  ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
            
            if (is_last_to_act) {
                poker->state = PKRSRV_POKER_STATE_DONE;
                poker->synced_action_counter = 0;
                poker->last_raised_pos = -1;
                poker->last_pos_before_last_raise = -1;
                if (poker->starter_players_num == 2) {
                    poker->playing_position = poker->players->bb_player->position;
                }
            }
        } else if (action.kind == PKRSRV_POKER_ACTION_KIND_FOLD) {
            if (ingame_players_count == 1) {
                poker->winner = NULL;

                LIST_FOREACH(poker->players, _player)
                    if (_player->is_playing && !_player->is_folded) {
                        poker->winner = _player;
                        break;
                    }
                END_FOREACH

                PKRSRV_UTIL_ASSERT(poker->winner != NULL);

                poker->state = PKRSRV_POKER_STATE_DONE;

                pkrsrv_poker_player_add_balance(poker->winner, poker->pot_amount);

                goto DO_ACTION_END;
            } else {
                int actual_pos_to_play_last = pkrsrv_poker_get_last_playing_position(poker, player);
                
                bool is_last_to_act = ((poker->last_raised_pos == -1) && (poker->synced_action_counter == playing_players_count)) ||
                                      ((poker->last_raised_pos != -1) && (player->position == actual_pos_to_play_last));
                
                if (is_last_to_act) {
                    poker->state = PKRSRV_POKER_STATE_DONE;
                    poker->synced_action_counter = 0;
                    poker->last_raised_pos = -1;
                    poker->last_pos_before_last_raise = -1;
                    if (poker->starter_players_num == 2) {
                        poker->playing_position = poker->players->bb_player->position;
                    }
                }
            }
        }

        if (poker->state == PKRSRV_POKER_STATE_DONE) {
            goto DONE;
        }
    }

    if (poker->is_playing_last || (playing_players_count == 0)) {
        if (poker->state == PKRSRV_POKER_STATE_PREFLOP) {
            poker->hand_flop.cards[0] = pkrsrv_card_get_random(poker->card_history);
            poker->hand_flop.cards[1] = pkrsrv_card_get_random(poker->card_history);
            poker->hand_flop.cards[2] = pkrsrv_card_get_random(poker->card_history);
            poker->hand_turn.cards[0] = pkrsrv_card_get_random(poker->card_history);
            poker->hand_river.cards[0] = pkrsrv_card_get_random(poker->card_history);
        } else if (poker->state == PKRSRV_POKER_STATE_FLOP) {
            poker->hand_turn.cards[0] = pkrsrv_card_get_random(poker->card_history);
            poker->hand_river.cards[0] = pkrsrv_card_get_random(poker->card_history);
        } else if (poker->state == PKRSRV_POKER_STATE_TURN) {
            poker->hand_river.cards[0] = pkrsrv_card_get_random(poker->card_history);
        }
        
        poker->state = PKRSRV_POKER_STATE_DONE;

        goto DONE;
    } else if (playing_players_count == 1) {
        poker->is_playing_last = true;
    }

    goto DO_ACTION_END;

    DONE: {
        struct {
            pkrsrv_poker_player_t* player;
            uint64_t bet;
            int rank;
        } ranks[poker->players->length];

        int ranks_length = 0;
        
        LIST_FOREACH(poker->players, _player)
            if (!_player->is_playing || _player->is_folded || _player->is_left) {
                continue;
            }

            ranks_length++;

            pkrsrv_card_t sevens[] = {
                _player->cards[0],
                _player->cards[1],
                poker->hand_flop.cards[0],
                poker->hand_flop.cards[1],
                poker->hand_flop.cards[2],
                poker->hand_turn.cards[0],
                poker->hand_river.cards[0],
            };

            pkrsrv_card_score_result_t score_result = pkrsrv_card_get_best_fives_score(sevens);

            ranks[_player_i].player = _player;
            ranks[_player_i].bet = _player->bet;
            ranks[_player_i].rank = score_result.check_result.rank;
        END_FOREACH

        for (int i=0; i < poker->players->length; i++) {
            for (int j=0; j < poker->players->length; j++) {
                if (ranks[j].rank < ranks[i].rank) {
                    __typeof__(*ranks) tmp = ranks[j];
                    ranks[j] = ranks[i];
                    ranks[i] = tmp;
                }
            }
        }
        
        poker->scores_length = 0;
        
        uint64_t pot_diff;
        pot_diff = poker->pot_amount;
        
        for (int i=0; i < poker->players->length; i++) {
            __typeof__(&(*ranks)) rank = &(ranks[i]);

            uint64_t bet_diff = poker->current_bet - rank->player->bet;

            pot_diff -= rank->player->bet;
            
            pkrsrv_poker_player_add_balance(rank->player, rank->bet);

            for (int j=i+1; j < ranks_length; j++) {
                __typeof__(&(*ranks)) other = &(ranks[j]);

                if (other->bet >= rank->player->bet) {
                    other->bet -= rank->bet;
                    pkrsrv_poker_player_add_balance(rank->player, rank->bet);
                } else {
                    other->bet = 0;
                    pkrsrv_poker_player_add_balance(rank->player, other->bet);
                }
            }
            
            PKRSRV_REF_COUNTED_USE(rank->player);
            
            poker->scores[i].player = rank->player;
            poker->scores[i].bet_diff = bet_diff;

            poker->scores_length++;

            if (pot_diff == 0) {
                break;
            }
        }
        
        poker->winner = ranks[0].player;
    }

    DO_ACTION_END:

    poker->action_time = pkrsrv_util_get_time_msec();

    pkrsrv_poker_on_action_params_t* task_params = malloc(sizeof(pkrsrv_poker_on_action_params_t));
    task_params->poker = poker;
    task_params->player = player;
    task_params->action = action;

    pkrsrv_eventloop_call(poker->eventloop, poker->on_action, task_params);

    RETURN:

    return result;
}

pkrsrv_poker_actions_action_t* pkrsrv_poker_get_last_action(pkrsrv_poker_t* poker) {
    pkrsrv_poker_hand_t* hand = pkrsrv_poker_get_current_hand(poker);
    if (!hand) {
        return NULL;
    }

    pkrsrv_poker_actions_action_t* action = hand->actions->terminal;

    return action;
}

void pkrsrv_poker_proceed(pkrsrv_poker_t* poker) {
    if (poker->players->length < 2) {
        poker->state = PKRSRV_POKER_STATE_W4P;
    }

    if ((poker->state == PKRSRV_POKER_STATE_W4P) ||
        (poker->state == PKRSRV_POKER_STATE_CREATED) ||
        (poker->state == PKRSRV_POKER_STATE_DONE)
    ) {
        goto END;
    }
    
    uint64_t current_time = pkrsrv_util_get_time_msec();
    uint64_t ms_elapsed = current_time - poker->action_time;
    
    pkrsrv_poker_hand_t* hand = pkrsrv_poker_get_current_hand(poker);
    pkrsrv_poker_actions_action_t* last_action = pkrsrv_poker_get_last_action(poker);
    pkrsrv_poker_player_t* playing_player = pkrsrv_poker_players_getby_position(poker, poker->playing_position);

    if ((ms_elapsed > poker->table->action_timeout) && (poker->state >= PKRSRV_POKER_STATE_SMALL_BLIND)) {
        pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_position(poker, poker->playing_position);

        pkrsrv_util_verbose("Action timeout! Folding... (Player: %llu, Elapsed: %llu)\n", player->account->id, ms_elapsed);

        pkrsrv_poker_do_action(poker, player, (pkrsrv_poker_action_t) {
            .account = *(player->account),
            .amount = 0,
            .kind = PKRSRV_POKER_ACTION_KIND_FOLD
        });
    }

    END:

    poker->proceed_time = pkrsrv_util_get_time_msec();
}

pkrsrv_poker_players_t* pkrsrv_poker_players_new(pkrsrv_poker_players_new_params_t params) {
    pkrsrv_poker_players_t* players = malloc(sizeof(pkrsrv_poker_players_t));
    LIST_INIT(players);
    players->sb_player = NULL;
    players->bb_player = NULL;
    players->button_position = 0;
    players->position_i = 0;
    players->is_dealt = false;
    
    return players;
}

void pkrsrv_poker_players_free(pkrsrv_poker_players_t* players) {
    LIST_FOREACH(players, curr)
        PKRSRV_REF_COUNTED_LEAVE(curr);
    END_FOREACH

    free(players);
}

bool pkrsrv_poker_players_add(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* player) {
    PKRSRV_UTIL_ASSERT(poker->players->length <= poker->table->max_players);
    
    if (poker->table->max_players < poker->players->length) {
        return false;
    }

    PKRSRV_REF_COUNTED_USE(player);
    
    player->is_playing = (poker->state < PKRSRV_POKER_STATE_SMALL_BLIND);
    
    LIST_APPEND(poker->players, player);

    if (!poker->players->sb_player) {
        poker->playing_position = player->position;
        poker->players->sb_player = player;
    } else if (!poker->players->bb_player) {
        poker->players->bb_player = player;
    }

    pkrsrv_poker_players_get_sorted(poker, poker->sorted_players);

    return true;
}

void pkrsrv_poker_players_remove(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* player) {
    LIST_REMOVE(poker->players, player);
    pkrsrv_poker_players_get_sorted(poker, poker->sorted_players);
    PKRSRV_REF_COUNTED_LEAVE(player);
}

pkrsrv_poker_player_t* pkrsrv_poker_players_addby_account(pkrsrv_poker_t* poker, pkrsrv_account_t* account, int position) {
    PKRSRV_UTIL_ASSERT(poker->players->length <= poker->table->max_players);

    if (poker->table->max_players < poker->players->length) {
        return NULL;
    }
    
    pkrsrv_poker_player_t* player = pkrsrv_poker_player_new((pkrsrv_poker_player_new_params_t) {
        .account = account
    });

    player->position = position % poker->table->max_players;

    if (poker->state <= PKRSRV_POKER_STATE_W4P) {
        player->is_playing = true;
    }
    
    pkrsrv_poker_players_add(poker, player);

    return player;
}

void pkrsrv_poker_players_get_sorted(pkrsrv_poker_t* poker, pkrsrv_poker_player_t** result) {
    pkrsrv_poker_player_t* player = (pkrsrv_poker_player_t *) poker->players->next;

    for (int i=0; i < poker->players->length; i++) {
        result[i] = player;
        player = player->next;
    }

    for (int i=0; i < poker->players->length; i++) {
        for (int j=0; j < poker->players->length; j++) {
            if (result[j]->position > result[i]->position) {
                pkrsrv_poker_player_t* tmp = result[i];
                result[i] = result[j];
                result[j] = tmp;
            }
        }
    }
}

pkrsrv_poker_player_t* pkrsrv_poker_players_getby_index(pkrsrv_poker_t* poker, int index) {
    LIST_FOREACH(poker->players, player)
        if (player_i == index) {
            return player;
        }
    END_FOREACH
    
    return NULL;
} 

pkrsrv_poker_player_t* pkrsrv_poker_players_getby_id(pkrsrv_poker_t* poker, uint64_t id) {
    LIST_FOREACH(poker->players, player)
        if (player->account->id == id) {
            return player;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_poker_player_t* pkrsrv_poker_players_getby_position(pkrsrv_poker_t* poker, int position) {
    LIST_FOREACH(poker->players, player)
        if (player->position == position) {
            return player;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_poker_player_t* pkrsrv_poker_players_getby_position_in_sorted(pkrsrv_poker_t* poker, pkrsrv_poker_player_t** sorted, int position) {
    for (int i=0; i < poker->players->length; i++) {
        if (sorted[i]->position == position) {
            return sorted[i];
        }
    }

    return NULL;
}

int pkrsrv_poker_players_get_playable_count(pkrsrv_poker_t* poker) {
    int count = 0;
    
    pkrsrv_poker_player_t* curr = (pkrsrv_poker_player_t *) poker->players;

    LIST_FOREACH(poker->players, player)
        if (!player->is_left) {
            count++;
        }
    END_FOREACH

    return count;
}

bool pkrsrv_poker_player_is_playing(pkrsrv_poker_player_t* player) {
    return player->is_playing && !player->is_folded && !player->is_allin;
}

int pkrsrv_poker_players_get_playing_count(pkrsrv_poker_t* poker) {
    int count = 0;
    
    LIST_FOREACH(poker->players, player)
        if (pkrsrv_poker_player_is_playing(player)) {
            count++;
        }
    END_FOREACH

    return count;
}

int pkrsrv_poker_players_get_ingame_count(pkrsrv_poker_t* poker) {
    int count = 0;
    
    LIST_FOREACH(poker->players, player)
        if (player->is_playing && !player->is_folded) {
            count++;
        }
    END_FOREACH

    return count;
}

int pkrsrv_poker_players_get_mustplay_count(pkrsrv_poker_t* poker) {
    int count = 0;
    
    LIST_FOREACH(poker->players, player)
        count += player->is_playing && !player->is_left && !player->is_folded && !player->is_allin && (player->bet < poker->current_bet);
    END_FOREACH

    return count;
}

void pkrsrv_poker_players_deal(pkrsrv_poker_t* poker) {
    LIST_FOREACH(poker->players, player)
        pkrsrv_poker_player_deal(poker, player);
    END_FOREACH

    poker->players->is_dealt = true;
}

pkrsrv_poker_player_t* pkrsrv_poker_player_new(pkrsrv_poker_player_new_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.account);
    
    pkrsrv_poker_player_t* player = malloc(sizeof(pkrsrv_poker_player_t));
    PKRSRV_REF_COUNTED_INIT(player, pkrsrv_poker_player_free);
    LIST_ITEM_INIT(player);
    player->is_playing = false;
    player->is_dealt = false;
    player->is_folded = false;
    player->is_left = false;
    player->is_allin = false;
    player->account = params.account;
    player->cards[0] = (pkrsrv_card_t) {.index = -1, .kind = -1};
    player->cards[1] = (pkrsrv_card_t) {.index = -1, .kind = -1};
    player->position = -1;
    player->enterance_balance = 0;
    player->balance = 0;
    player->bet = 0;
    player->owner = NULL;

    return player;
}

void pkrsrv_poker_player_free(pkrsrv_poker_player_t* player) {
    PKRSRV_REF_COUNTED_LEAVE(player->account);
    free(player);
}

void pkrsrv_poker_player_deal(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* player) {
    player->is_dealt = true;
    player->cards[0] = pkrsrv_card_get_random(poker->card_history);
    player->cards[1] = pkrsrv_card_get_random(poker->card_history);
}