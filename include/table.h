/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup table Tables
 * \brief Poker tables and interactions.
 */

/**
 * \addtogroup table
 * \ingroup table
 * @{
 */

#include <libpq-fe.h>

#include "sugar.h"
#include "uniqid.h"
#include "ref.h"
#include "string.h"

#define MAX_PLAYERS 10
#define ACTION_TIMEOUT 10000 /* Milliseconds */

/**
 * ! Free by ref counting
 */
typedef struct pkrsrv_table {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_uniqid_uuid_t id;
    uint64_t big_blind;
    uint64_t small_blind;
    int max_players;
    int action_timeout; /* Microseconds */
    pkrsrv_string_t* name;
    uint64_t enterance_min;
    uint64_t enterance_max;
    int players_count;
    int watchers_count;
} pkrsrv_table_t;

typedef struct pkrsrv_tables {
    PKRSRV_REF_COUNTEDIFY();
    int length;
    pkrsrv_table_t** tables;
} pkrsrv_table_list_t;

typedef struct {
    pkrsrv_uniqid_uuid_t id;
    uint64_t big_blind;
    uint64_t small_blind;
    int max_players;
    int action_timeout; /* Microseconds */
    pkrsrv_string_t* name;
    uint64_t enterance_min;
    uint64_t enterance_max;
    int players_count;
    int watchers_count;
} pkrsrv_table_new_params_t;

pkrsrv_table_t* pkrsrv_table_new(pkrsrv_table_new_params_t params);
void pkrsrv_table_free(pkrsrv_table_t* table);
pkrsrv_table_t* pkrsrv_table_get(PGconn* pg_conn, uint64_t p_id);
pkrsrv_table_t* pkrsrv_table_get__redis(uint64_t id);

typedef struct {
    int offset;
    int length;
} pkrsrv_table_get_list_params_t;
pkrsrv_table_list_t* pkrsrv_table_get_list(PGconn* pg_conn, pkrsrv_table_get_list_params_t params);
pkrsrv_table_list_t* pkrsrv_table_get_list__redis(pkrsrv_table_get_list_params_t params);
void pkrsrv_table_list_free(pkrsrv_table_list_t* list);

void pkrsrv_table_set_players_num(PGconn* pg_conn, pkrsrv_table_t* table, int num);
void pkrsrv_table_set_watchers_num(PGconn* pg_conn, pkrsrv_table_t* table, int num);
void pkrsrv_table_incr_players_num(PGconn* pg_conn, pkrsrv_table_t* table, int by);
void pkrsrv_table_incr_watchers_num(PGconn* pg_conn, pkrsrv_table_t* table, int by);

/**
 * @}
 */