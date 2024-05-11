/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

#include <stdbool.h>

static char CARD_KINDS[] = {'H', 'C', 'D', 'S'};
static int CARDS[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const char* CARD_SYMBOLS[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};

typedef enum PKRSRV_CARD_HAND_RANK {
    PKRSRV_CARD_HAND_RANK_HIGH_CARD = 1000000,
    PKRSRV_CARD_HAND_RANK_PAIR = 2000000,
    PKRSRV_CARD_HAND_RANK_TWO_PAIR = 3000000,
    PKRSRV_CARD_HAND_RANK_THREE_KIND = 4000000,
    PKRSRV_CARD_HAND_RANK_STRAIGHT = 5000000,
    PKRSRV_CARD_HAND_RANK_FLUSH = 6000000,
    PKRSRV_CARD_HAND_RANK_FULL_HOUSE = 7000000,
    PKRSRV_CARD_HAND_RANK_QUAD = 8000000,
    PKRSRV_CARD_HAND_RANK_STRAIGHT_FLUSH = 9000000,
    PKRSRV_CARD_HAND_RANK_ROYAL_FLUSH = 10000000
} pkrsrv_card_hand_rank_t;

static int FIVES_INDEXES[] = {
    0, 1, 2, 3, 4, 
    0, 1, 2, 3, 5, 
    0, 1, 2, 3, 6, 
    0, 1, 2, 4, 5, 
    0, 1, 2, 4, 6, 
    0, 1, 2, 5, 6, 
    0, 1, 3, 4, 5, 
    0, 1, 3, 4, 6, 
    0, 1, 3, 5, 6, 
    0, 1, 4, 5, 6, 
    0, 2, 3, 4, 5, 
    0, 2, 3, 4, 6, 
    0, 2, 3, 5, 6, 
    0, 2, 4, 5, 6, 
    0, 3, 4, 5, 6, 
    1, 2, 3, 4, 5, 
    1, 2, 3, 4, 6, 
    1, 2, 3, 5, 6, 
    1, 2, 4, 5, 6, 
    1, 3, 4, 5, 6, 
    2, 3, 4, 5, 6
};

typedef struct pkrsrv_card pkrsrv_card_t;
typedef struct pkrsrv_card_check_result pkrsrv_card_check_result_t;
typedef struct pkrsrv_card_score_result pkrsrv_card_score_result_t;

struct pkrsrv_card {
    int index;
    int kind;
};

struct pkrsrv_card_check_result {
    bool is_exists;
    int rank;
    pkrsrv_card_t cards[5];
};

struct pkrsrv_card_score_result {
    bool is_high_card;
    bool is_pair;
    bool is_two_pair;
    bool is_three_kind;
    bool is_straight;
    bool is_flush;
    bool is_full_house;
    bool is_quad;
    bool is_straight_flush;
    bool is_royal_flush;
    pkrsrv_card_check_result_t check_result;
    pkrsrv_card_t cards[5];
};

pkrsrv_card_t pkrsrv_card_get_random(pkrsrv_card_t* history);

void pkrsrv_card_sort_sevens(pkrsrv_card_t* sevens);

void pkrsrv_card_get_fives(pkrsrv_card_t* sevens, pkrsrv_card_t* target);
pkrsrv_card_score_result_t pkrsrv_card_get_best_fives_score(pkrsrv_card_t* sevens);
pkrsrv_card_score_result_t pkrsrv_card_get_get_fives_score(pkrsrv_card_t* fives);

pkrsrv_card_check_result_t pkrsrv_card_check_high_card(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_pair(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_two_pair(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_three_kind(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_straight(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_flush(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_full_house(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_quad(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_straight_flush(pkrsrv_card_t* fives);
pkrsrv_card_check_result_t pkrsrv_card_check_royal_flush(pkrsrv_card_t* fives);