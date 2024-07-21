/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <libpq-fe.h>

#include "thirdparty/hiredis/hiredis.h"

#include "../include/table.h"

#include "../include/pkrsrv.h"
#include "../include/util.h"
#include "../include/db.h"
#include "../include/ref.h"
#include "../include/string.h"
#include "../include/redis.h"

pkrsrv_table_t* pkrsrv_table_new(pkrsrv_table_new_params_t params) {
    pkrsrv_table_t* table = malloc(sizeof(pkrsrv_table_t));
    PKRSRV_REF_COUNTED_INIT(table, pkrsrv_table_free);
    table->id = params.id;
    table->name = params.name;
    table->small_blind = params.small_blind;
    table->big_blind = params.big_blind;
    table->max_players = params.max_players ? params.max_players: MAX_PLAYERS;
    table->action_timeout = params.action_timeout ? params.action_timeout: ACTION_TIMEOUT;
    table->players_count = params.players_count;
    table->watchers_count = params.watchers_count;
    table->enterance_min = params.enterance_min;
    table->enterance_max = params.enterance_max;

    PKRSRV_REF_COUNTED_USE(table->name);

    return table;
}

void pkrsrv_table_free(pkrsrv_table_t* table) {
    PKRSRV_REF_COUNTED_LEAVE(table->name);
    free(table);
}

pkrsrv_table_t* pkrsrv_table_get(PGconn* pg_conn, uint64_t p_id) {
    pkrsrv_table_t* table = NULL;
    
    PGresult* query_result;
    ExecStatusType query_result_status;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", p_id);
    
    const char* query_params[] = {id_str};

    query_result = PQexecParams(
        pg_conn,
        "select "
            "id, name, sb, bb, action_timeout, max_players, enterance_min, enterance_max, "
            "players_count, watchers_count, stake_min, stake_max "
        "from tables "
        "where "
            " (id = $1::integer) "
        "limit 1",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {id_str_len},
        (int[]) {0},
        0
    );

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_TUPLES_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
        
        goto RETURN;
    }

    int rows_length = PQntuples(query_result);

    if (!rows_length) {
        goto RETURN;
    }

    uint64_t id = atoi(PQgetvalue(query_result, 0, 0));
    pkrsrv_string_t* name = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 1), PQgetlength(query_result, 0, 1));
    int sb_len = PQgetlength(query_result, 0, 2);
    char* sb_str = PQgetvalue(query_result, 0, 2);
    int bb_len = PQgetlength(query_result, 0, 3);
    char* bb_str = PQgetvalue(query_result, 0, 3);
    int action_timeout = atoi(PQgetvalue(query_result, 0, 4));
    int max_players = atoi(PQgetvalue(query_result, 0, 5));
    int enterance_min_len = PQgetlength(query_result, 0, 6);
    char* enterance_min_str = PQgetvalue(query_result, 0, 6);
    int enterance_max_len = PQgetlength(query_result, 0, 7);
    char* enterance_max_str = PQgetvalue(query_result, 0, 7);
    int players_count = atoi(PQgetvalue(query_result, 0, 8));
    int watchers_count = atoi(PQgetvalue(query_result, 0, 9));
    int stake_min = atoi(PQgetvalue(query_result, 0, 10));
    int stake_max = atoi(PQgetvalue(query_result, 0, 11));

    pkrsrv_table_new_params_t params;
    params.id = (pkrsrv_uniqid_uuid_t) {.scalar = id};
    params.name = name;
    params.small_blind = atoll(sb_str);
    params.big_blind = atoll(bb_str);
    params.max_players = max_players;
    params.action_timeout  = action_timeout * 1000;
    params.enterance_min = atoll(enterance_min_str);
    params.enterance_max = atoll(enterance_max_str);
    params.players_count = players_count;
    params.watchers_count = watchers_count;

    table = pkrsrv_table_new(params);

    RETURN:

    PQclear(query_result);

    return table;
}

pkrsrv_table_t* pkrsrv_table_get__redis(uint64_t id) {
    pkrsrv_table_t* table = NULL;
    
    redisReply* result = redisCommand(
        redis_connection,
        "HMGET table:%llu sb bb name stake_min stake_max max_players action_timeout enterance_min enterance_max players_count watchers_count",
        id
    );

    if (!result || (result->type != REDIS_REPLY_ARRAY) || (result->elements != 11)) {
        goto RETURN;
    }

    redisReply* sb = result->element[0];
    redisReply* bb = result->element[1];
    redisReply* name = result->element[2];
    redisReply* stake_min = result->element[3];
    redisReply* stake_max = result->element[4];
    redisReply* max_players = result->element[5];
    redisReply* action_timeout = result->element[6];
    redisReply* enterance_min = result->element[7];
    redisReply* enterance_max = result->element[8];
    redisReply* players_count = result->element[9];
    redisReply* watchers_count = result->element[10];

    if (
        (sb->type == REDIS_REPLY_NIL) ||
        (bb->type == REDIS_REPLY_NIL) ||
        (name->type == REDIS_REPLY_NIL) ||
        (stake_min->type == REDIS_REPLY_NIL) ||
        (stake_max->type == REDIS_REPLY_NIL) ||
        (max_players->type == REDIS_REPLY_NIL) ||
        (action_timeout->type == REDIS_REPLY_NIL) ||
        (enterance_min->type == REDIS_REPLY_NIL) ||
        (enterance_max->type == REDIS_REPLY_NIL) ||
        (players_count->type == REDIS_REPLY_NIL) ||
        (watchers_count->type == REDIS_REPLY_NIL)
    ) {
        goto RETURN;
    }
    
    pkrsrv_table_new_params_t params;
    params.id = (pkrsrv_uniqid_uuid_t) {.scalar = id};
    params.name = pkrsrv_string_new_from_cstr__copy(name->str, name->len);
    params.small_blind = atoll(sb->str);
    params.big_blind = atoll(bb->str);
    params.max_players = atoi(max_players->str);
    params.action_timeout = atoi(action_timeout->str) * 1000;
    params.enterance_min = atoll(enterance_min->str);
    params.enterance_max = atoll(enterance_max->str);
    params.players_count = atoi(players_count->str);
    params.watchers_count = atoi(watchers_count->str);
    
    table = pkrsrv_table_new(params);

    RETURN:

    freeReplyObject(result);

    return table;
}

pkrsrv_table_list_t* pkrsrv_table_get_list(PGconn* pg_conn, pkrsrv_table_get_list_params_t params) {
    pkrsrv_table_list_t* list = malloc(sizeof(pkrsrv_table_list_t));
    PKRSRV_REF_COUNTED_INIT(list, pkrsrv_table_list_free);
    list->length = 0;
    list->tables = NULL;
    
    PGresult* query_result;
    ExecStatusType query_result_status;

    char length_str[20];
    int length_str_len = sprintf(length_str, "%d", params.length);

    char offset_str[20];
    int offset_str_len = sprintf(offset_str, "%d", params.offset);

    const char* query_params[] = {length_str, offset_str};

    query_result = PQexecParams(
        pg_conn,
        "select "
            "id, name, sb, bb, action_timeout, max_players, enterance_min, enterance_max, "
            "players_count, watchers_count, stake_min, stake_max "
        "from tables "
        "order by players_count desc, watchers_count desc "
        "limit $1::integer "
        "offset $2::integer ",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {length_str_len, offset_str_len},
        (int[]) {0, 0},
        0
    );

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_TUPLES_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
        
        goto RETURN;
    }

    int rows_length = PQntuples(query_result);

    if (!rows_length) {
        goto RETURN;
    }

    list->length = rows_length;
    list->tables = malloc(list->length * sizeof(pkrsrv_table_t*));

    for (int i=0; i < rows_length; i++) {
        uint64_t id = atoi(PQgetvalue(query_result, i, 0));
        pkrsrv_string_t* name = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, i, 1), PQgetlength(query_result, i, 1));
        int sb_len = PQgetlength(query_result, i, 2);
        char* sb_str = PQgetvalue(query_result, i, 2);
        int bb_len = PQgetlength(query_result, i, 3);
        char* bb_str = PQgetvalue(query_result, i, 3);
        int action_timeout = atoi(PQgetvalue(query_result, i, 4));
        int max_players = atoi(PQgetvalue(query_result, i, 5));
        int enterance_min_len = PQgetlength(query_result, i, 6);
        char* enterance_min_str = PQgetvalue(query_result, i, 6);
        int enterance_max_len = PQgetlength(query_result, i, 7);
        char* enterance_max_str = PQgetvalue(query_result, i, 7);
        int players_count = atoi(PQgetvalue(query_result, i, 8));
        int watchers_count = atoi(PQgetvalue(query_result, i, 9));
        int stake_min = atoi(PQgetvalue(query_result, i, 10));
        int stake_max = atoi(PQgetvalue(query_result, i, 11));

        pkrsrv_table_new_params_t params;
        params.id = (pkrsrv_uniqid_uuid_t) {.scalar = id};
        params.name = name;
        params.small_blind = atoll(sb_str);
        params.big_blind = atoll(bb_str);
        params.max_players = max_players;
        params.action_timeout  = action_timeout * 1000;
        params.enterance_min = atoll(enterance_min_str);
        params.enterance_max = atoll(enterance_max_str);
        params.players_count = players_count;
        params.watchers_count = watchers_count;

        pkrsrv_table_t* table = pkrsrv_table_new(params);
        PKRSRV_REF_COUNTED_USE(table);
        
        list->tables[i] = table;
    } 

    RETURN:

    PQclear(query_result);

    return list;
}

pkrsrv_table_list_t* pkrsrv_table_get_list__redis(pkrsrv_table_get_list_params_t params) {
    pkrsrv_table_list_t* list = malloc(sizeof(pkrsrv_table_list_t));
    PKRSRV_REF_COUNTED_INIT(list, pkrsrv_table_list_free);
    list->length = 0;
    list->tables = NULL;
    
    redisReply* result = redisCommand(
        redis_connection,
        "LRANGE tables %d %d",
        params.offset,
        params.length - 1
    );

    if (!result || (result->type == REDIS_REPLY_ERROR) || (result->elements == 0)) {
        return list;
    }

    list->length = result->elements;

    list->tables = malloc(list->length * sizeof(pkrsrv_table_t*));

    for (int i = 0; i < list->length; i++) {
        char* id_str = result->element[i]->str;
        uint64_t id = atoll(id_str);
        
        pkrsrv_table_t* table = pkrsrv_table_get__redis(id);
        PKRSRV_UTIL_ASSERT(table);
        if (!table) {
            continue;
        }
        
        PKRSRV_REF_COUNTED_USE(table);
        list->tables[i] = table;
    }

    return list;
}

void pkrsrv_table_list_free(pkrsrv_table_list_t* list) {
    for (int i = 0; i < list->length; i++) {
        PKRSRV_REF_COUNTED_LEAVE(list->tables[i]);
    }
    
    free(list->tables);
    free(list);
}

void pkrsrv_table_set_players_num(PGconn* pg_conn, pkrsrv_table_t* table, int num) {
    PKRSRV_REF_COUNTED_USE(table);
    
    table->players_count = num;
    
    PGresult* query_result;
    ExecStatusType query_result_status;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", table->id.scalar);
    
    char num_str[20];
    int num_str_len = sprintf(num_str, "%d", num);

    const char* query_params[] = {id_str, num_str};

    query_result = PQexecParams(
        pg_conn,
        "update tables "
        "set "
            "players_count = $2::integer "
        "where "
            "id = $1",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {id_str_len, num_str_len},
        (int[]) {0, 0},
        0
    );

    query_result_status = PQresultStatus(query_result);
    if (query_result_status != PGRES_COMMAND_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
    }
    PQclear(query_result);
    
    if (PKRSRV_USE_REDIS) {
        redisReply* result = redisCommand(
            redis_connection,
            "HSET table:%llu players_count %d",
            table->id.scalar,
            num
        );

        freeReplyObject(result);
    }

    PKRSRV_REF_COUNTED_LEAVE(table);
}

void pkrsrv_table_set_watchers_num(PGconn* pg_conn, pkrsrv_table_t* table, int num) {
    PKRSRV_REF_COUNTED_USE(table);
    
    table->watchers_count = num;
    
    PGresult* query_result;
    ExecStatusType query_result_status;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", table->id.scalar);

    char num_str[20];
    int num_str_len = sprintf(num_str, "%d", num);
    
    const char* query_params[] = {id_str, num_str};

    query_result = PQexecParams(
        pg_conn,
        "update tables "
        "set "
            "watchers_count = $2::integer "
        "where "
            "id = $1",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {id_str_len, num_str_len},
        (int[]) {0, 0},
        0
    );

    query_result_status = PQresultStatus(query_result);
    if (query_result_status != PGRES_COMMAND_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
    }
    PQclear(query_result);
    
    if (PKRSRV_USE_REDIS) {
        redisReply* result = redisCommand(
            redis_connection,
            "HSET table:%llu watchers_count %d",
            table->id.scalar,
            num
        );

        freeReplyObject(result);
    }

    PKRSRV_REF_COUNTED_LEAVE(table);
}

void pkrsrv_table_incr_players_num(PGconn* pg_conn, pkrsrv_table_t* table, int by) {
    PKRSRV_REF_COUNTED_USE(table);
    
    table->players_count += by;

    PGresult* query_result;
    ExecStatusType query_result_status;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", table->id.scalar);

    char by_str[20];
    int by_str_len = sprintf(by_str, "%d", by);
    
    const char* query_params[] = {id_str, by_str};

    query_result = PQexecParams(
        pg_conn,
        "update tables "
        "set "
            "players_count = players_count + $2::integer "
        "where "
            "id = $1",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {id_str_len, by_str_len},
        (int[]) {0, 0},
        0
    );

    query_result_status = PQresultStatus(query_result);
    if (query_result_status != PGRES_COMMAND_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
    }
    PQclear(query_result);
    
    if (PKRSRV_USE_REDIS) {
        redisReply* result = redisCommand(
            redis_connection,
            "HINCRBY table:%llu players_count %d",
            table->id.scalar,
            by
        );

        freeReplyObject(result);
    }

    PKRSRV_REF_COUNTED_LEAVE(table);
}

void pkrsrv_table_incr_watchers_num(PGconn* pg_conn, pkrsrv_table_t* table, int by) {
    PKRSRV_REF_COUNTED_USE(table);
    
    table->watchers_count += by;

    PGresult* query_result;
    ExecStatusType query_result_status;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", table->id.scalar);

    char by_str[20];
    int by_str_len = sprintf(by_str, "%d", by);

    const char* query_params[] = {id_str, by_str};

    query_result = PQexecParams(
        pg_conn,
        "update tables "
        "set "
            "watchers_count = watchers_count + $2::integer "
        "where "
            "id = $1",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {id_str_len, by_str_len},
        (int[]) {0, 0},
        0
    );

    query_result_status = PQresultStatus(query_result);
    if (query_result_status != PGRES_COMMAND_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
    }
    PQclear(query_result);
    
    if (PKRSRV_USE_REDIS) {
        redisReply* result = redisCommand(
            redis_connection,
            "HINCRBY table:%llu watchers_count %d",
            table->id.scalar,
            by
        );

        freeReplyObject(result);
    }

    PKRSRV_REF_COUNTED_LEAVE(table);
}