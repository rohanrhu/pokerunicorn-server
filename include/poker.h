/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup poker Texas Hold'em
 * \brief Poker game logic.
 */

/**
 * \addtogroup poker
 * \ingroup poker
 * @{
 */

#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>

#include "../include/ref.h"
#include "../include/table.h"
#include "../include/account.h"
#include "../include/card.h"
#include "../include/eventloop.h"
#include "../include/sugar.h"

typedef enum PKRSRV_POKER_STATE {
    PKRSRV_POKER_STATE_CREATED,
    PKRSRV_POKER_STATE_W4P,
    PKRSRV_POKER_STATE_STARTING,
    PKRSRV_POKER_STATE_SMALL_BLIND,
    PKRSRV_POKER_STATE_BIG_BLIND,
    PKRSRV_POKER_STATE_PREFLOP,
    PKRSRV_POKER_STATE_FLOP,
    PKRSRV_POKER_STATE_TURN,
    PKRSRV_POKER_STATE_RIVER,
    PKRSRV_POKER_STATE_DONE
} pkrsrv_poker_state_t;

typedef enum PKRSRV_POKER_HAND_KIND {
    PKRSRV_POKER_HAND_KIND_PREFLOP,
    PKRSRV_POKER_HAND_KIND_FLOP,
    PKRSRV_POKER_HAND_KIND_TURN,
    PKRSRV_POKER_HAND_KIND_RIVER
} pkrsrv_poker_hand_kind_t;

typedef enum PKRSRV_POKER_ACTION_KIND {
    PKRSRV_POKER_ACTION_KIND_SMALL_BLIND,
    PKRSRV_POKER_ACTION_KIND_BIG_BLIND,
    PKRSRV_POKER_ACTION_KIND_RAISE,
    PKRSRV_POKER_ACTION_KIND_CHECK,
    PKRSRV_POKER_ACTION_KIND_FOLD
} pkrsrv_poker_actions_action_kind_t;

typedef struct pkrsrv_poker pkrsrv_poker_t;
typedef struct pkrsrv_poker_players pkrsrv_poker_players_t;
typedef struct pkrsrv_poker_player pkrsrv_poker_player_t;
typedef struct pkrsrv_poker_player_score pkrsrv_poker_player_score_t;
typedef struct pkrsrv_poker_hand pkrsrv_poker_hand_t;
typedef struct pkrsrv_poker_action pkrsrv_poker_action_t;
typedef struct pkrsrv_poker_action_result pkrsrv_poker_action_result_t;
typedef struct pkrsrv_poker_actions pkrsrv_poker_actions_t;
typedef struct pkrsrv_poker_actions_action pkrsrv_poker_actions_action_t;
typedef struct pkrsrv_poker_on_action_params pkrsrv_poker_on_action_params_t;
typedef void (*pkrsrv_poker_on_action_t)(pkrsrv_eventloop_task_t* task);

/**
 * ! Stored in (pkrsrv_poker_t *)->actions* (linked-list)
 * * Members need to get serialized:
 *     * actions
 */
struct pkrsrv_poker_hand {
    pkrsrv_poker_hand_kind_t kind;
    pkrsrv_card_t cards[3];
    bool is_checked;
    pkrsrv_poker_actions_t* actions;
    bool is_raised;
};

struct pkrsrv_poker_player_score {
    pkrsrv_poker_player_t* player;
    uint64_t bet_diff;
};

/**!
 * ! Gets serialized to save every state to database
 * * Members need to get serialized:
 *     * table
 *     * players
 */
struct pkrsrv_poker {
    PKRSRV_REF_COUNTEDIFY();
    
    void* owner;
    pkrsrv_eventloop_t* eventloop;
    
    // * Game Information
    int process_latency; // * Microseconds
    pkrsrv_table_t* table;

    // * State Information
    pkrsrv_poker_players_t* players;
    pkrsrv_poker_player_t** sorted_players;
    pkrsrv_poker_state_t state;
    pkrsrv_poker_hand_t hand_preflop;
    pkrsrv_poker_hand_t hand_flop;
    pkrsrv_poker_hand_t hand_turn;
    pkrsrv_poker_hand_t hand_river;
    pkrsrv_poker_hand_kind_t hand_kind;
    uint64_t action_time; // * Microseconds
    uint64_t proceed_time; // * Microseconds
    uint64_t starting_time; // * Microseconds

    pkrsrv_card_t card_history[52];

    uint64_t current_raise;
    uint64_t current_raise_total;
    uint64_t current_bet;

    int button_position;
    int playing_position;
    uint64_t pot_amount;

    int starter_players_num;
    int last_raised_pos;
    int last_pos_before_last_raise;
    int synced_action_counter;
    
    bool is_playing_last;
    bool is_sb_must_call;

    pkrsrv_poker_on_action_t on_action;
    
    pkrsrv_poker_player_t* winner;
    pkrsrv_card_score_result_t winner_score_result;
    
    pkrsrv_poker_player_score_t scores[10];
    int scores_length;
};

struct pkrsrv_poker_players {
    LISTIFY(pkrsrv_poker_player_t*);
    pkrsrv_poker_player_t* sb_player;
    pkrsrv_poker_player_t* bb_player;
    int button_position;
    int position_i;
    bool is_dealt;
};

struct pkrsrv_poker_player {
    ITEMIFY(pkrsrv_poker_player_t*);
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_account_t* account;
    pkrsrv_card_t cards[2];
    bool is_playing;
    bool is_dealt;
    bool is_folded;
    bool is_left;
    bool is_allin;
    int position;
    uint64_t enterance_balance;
    uint64_t balance;
    uint64_t bet;
    void* owner;
};

typedef struct {
    pkrsrv_poker_t* poker;
} pkrsrv_poker_players_new_params_t;

typedef struct {
    pkrsrv_poker_players_t* players;
    pkrsrv_account_t* account;
} pkrsrv_poker_player_new_params_t;

/**
 * ! Serializable, client builds and sends to do action, server interprets and adds to the circle
 */
struct pkrsrv_poker_action {
    pkrsrv_account_t account;
    pkrsrv_poker_actions_action_kind_t kind;
    uint64_t amount;
};

struct pkrsrv_poker_action_result {
    bool is_done;
};

/**
 * ! To-serialize by poker machine to save states in database
 */
struct pkrsrv_poker_actions {
    LISTIFY(pkrsrv_poker_actions_action_t*);
};

struct pkrsrv_poker_actions_action {
    ITEMIFY(pkrsrv_poker_actions_action_t*);
    pkrsrv_poker_action_t action;
    pkrsrv_poker_player_t* player;
};

typedef struct pkrsrv_poker_actions_action_new_params {
    pkrsrv_poker_action_t action;
    pkrsrv_poker_player_t* player;
} pkrsrv_poker_actions_action_new_params_t;

typedef struct pkrsrv_poker_new_params {
    void* owner;
    pkrsrv_table_t* table;
    pkrsrv_poker_on_action_t on_action;
    pkrsrv_eventloop_t* eventloop;
} pkrsrv_poker_new_params_t;
pkrsrv_poker_t* pkrsrv_poker_new(pkrsrv_poker_new_params_t params);

struct pkrsrv_poker_on_action_params {
    pkrsrv_poker_t* poker;
    pkrsrv_poker_player_t* player;
    pkrsrv_poker_action_t action;
};

void pkrsrv_poker_free(pkrsrv_poker_t *poker);

/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_players_t* pkrsrv_poker_players_new(pkrsrv_poker_players_new_params_t params);
/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_players_free(pkrsrv_poker_players_t *players);
/**
 * \memberof pkrsrv_poker
 */
bool pkrsrv_poker_players_add(pkrsrv_poker_t *poker, pkrsrv_poker_player_t *player);
/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_players_remove(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* player);
/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_players_get_sorted(pkrsrv_poker_t* poker, pkrsrv_poker_player_t** result);
/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_player_t* pkrsrv_poker_players_getby_index(pkrsrv_poker_t* poker, int index);
/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_player_t* pkrsrv_poker_players_getby_id(pkrsrv_poker_t* poker, uint64_t id);
/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_player_t* pkrsrv_poker_players_getby_position(pkrsrv_poker_t* poker, int position);
/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_player_t* pkrsrv_poker_players_getby_position_in_sorted(pkrsrv_poker_t* poker, pkrsrv_poker_player_t** sorted, int position);
/**
 * \memberof pkrsrv_poker
 */
int pkrsrv_poker_players_get_playable_count(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 */
bool pkrsrv_poker_player_is_playing(pkrsrv_poker_player_t* player);
/**
 * \memberof pkrsrv_poker
 */
int pkrsrv_poker_players_get_playing_count(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 */
int pkrsrv_poker_players_get_ingame_count(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 */
int pkrsrv_poker_players_get_mustplay_count(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 */
bool pkrsrv_poker_player_set_balance(pkrsrv_poker_player_t* player, uint64_t amount);
/**
 * \memberof pkrsrv_poker
 */
bool pkrsrv_poker_player_remove_balance(pkrsrv_poker_player_t* player, uint64_t amount);
/**
 * \memberof pkrsrv_poker
 */
bool pkrsrv_poker_player_add_balance(pkrsrv_poker_player_t* player, uint64_t amount);

/**
 * \memberof pkrsrv_poker
 * Deals the cards to the all playing players
 */
void pkrsrv_poker_players_deal(pkrsrv_poker_t* poker);

/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_player_t* pkrsrv_poker_player_new(pkrsrv_poker_player_new_params_t params);
/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_player_free(pkrsrv_poker_player_t* player);
/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_player_t* pkrsrv_poker_players_addby_account(pkrsrv_poker_t* poker, pkrsrv_account_t* account, int position);
/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_player_deal(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* player);

/**
 * \memberof pkrsrv_poker
 * Deals the cards to the all playing players
 */
void pkrsrv_poker_deal(pkrsrv_poker_t* poker);

pkrsrv_poker_hand_t* pkrsrv_poker_get_current_hand(pkrsrv_poker_t* poker);
pkrsrv_poker_actions_action_t* pkrsrv_poker_get_last_action(pkrsrv_poker_t* poker);

/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_ready(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 * Sets the poker state to starting
 */
void pkrsrv_poker_starting(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 * Starts the game, does SB and BB actions
 */
bool pkrsrv_poker_start(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 */
void pkrsrv_poker_proceed(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 */
bool pkrsrv_poker_restart(pkrsrv_poker_t* poker);
/**
 * \memberof pkrsrv_poker
 * Gives the actual player who will act last in the current round
 * if a raise doesn't happen
 */
int pkrsrv_poker_get_last_playing_position(pkrsrv_poker_t* poker, pkrsrv_poker_player_t* action_player);

/**
 * \memberof pkrsrv_poker
 */
pkrsrv_poker_action_result_t pkrsrv_poker_do_action(
    pkrsrv_poker_t* poker,
    pkrsrv_poker_player_t* player,
    pkrsrv_poker_action_t action
);

pkrsrv_poker_actions_action_t* pkrsrv_poker_actions_action_new(pkrsrv_poker_actions_action_new_params_t params);
void pkrsrv_poker_actions_add(pkrsrv_poker_actions_t* actions, pkrsrv_poker_actions_action_t* action);

/**
 * @}
 */