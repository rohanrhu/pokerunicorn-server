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
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#include <libpq-fe.h>

#include "../include/ref.h"
#include "../include/util.h"
#include "../include/account.h"
#include "../include/string.h"
#include "../include/db.h"
#include "../include/deposit.h"

pkrsrv_account_t* pkrsrv_account_new(pkrsrv_account_new_params_t params) {
    pkrsrv_account_t* account = malloc(sizeof(pkrsrv_account_t));
    PKRSRV_REF_COUNTED_INIT(account, pkrsrv_account_free);

    PKRSRV_REF_COUNTED_USE(params.id_token);
    PKRSRV_REF_COUNTED_USE(params.name);
    PKRSRV_REF_COUNTED_USE(params.avatar);
    PKRSRV_REF_COUNTED_USE(params.xmr_deposit_address);
    
    account->id = params.id;
    account->id_token = params.id_token;
    account->name = params.name;
    account->avatar = params.avatar;
    account->xmr_deposit_address = params.xmr_deposit_address;
    account->xmr_deposit_address_index = params.xmr_deposit_address_index;
    account->xmr_height = params.xmr_height;
    account->balance =params.balance;
    account->locked_balance =params.locked_balance;
    account->total_deposited =params.total_deposited;
    account->owner = NULL;
    account->on_updated = NULL;
    account->on_updated_param = NULL;

    return account;
}

void pkrsrv_account_free(pkrsrv_account_t* account) {
    pkrsrv_util_verbose("Freeing account#%llu object...\n", account->id);
    
    PKRSRV_REF_COUNTED_LEAVE(account->id_token);
    PKRSRV_REF_COUNTED_LEAVE(account->name);
    PKRSRV_REF_COUNTED_LEAVE(account->avatar);
    PKRSRV_REF_COUNTED_LEAVE(account->xmr_deposit_address);
    
    free(account);
}

bool pkrsrv_account_set_balance(pkrsrv_account_t* account, uint64_t balance) {
    account->balance = balance;
    return true;
}

void pkrsrv_account_get_balance(pkrsrv_account_t* account, uint64_t balance) {
    balance = account->balance;
}

pkrsrv_account_create_result_t pkrsrv_account_create(PGconn* pg_conn, pkrsrv_account_create_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.id_token);
    PKRSRV_REF_COUNTED_USE(params.password);
    PKRSRV_REF_COUNTED_USE(params.name);
    PKRSRV_REF_COUNTED_USE(params.avatar);
    if (params.xmr_deposit_address) {
        PKRSRV_REF_COUNTED_USE(params.xmr_deposit_address);
    }
    
    pkrsrv_account_create_result_t result;
    result.account = NULL;
    result.is_already_exists = false;
    
    pkrsrv_account_t* account = NULL;
    
    char balance_str[21];
    sprintf(balance_str, "%llu", params.balance);

    unsigned char password_md5[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, params.password->value, params.password->length);
    MD5_Final(password_md5, &ctx);

    PGresult* query_result;
    ExecStatusType query_result_status;

    PQexec(pg_conn, "BEGIN");
    
    {
        const char* query_params[] = {
            params.id_token->value,
            params.name->value,
            params.avatar->value,
            password_md5,
            balance_str,
            params.xmr_deposit_address ? params.xmr_deposit_address->value: ""
        };
        
        query_result = PQexecParams(
            pg_conn,
            "INSERT INTO accounts "
                "(id_token, name, avatar, password, balance, xmr_deposit_address) "
            "values ($1, $2, $3, $4, $5, $6) "
            "returning id, xmr_deposit_address_index",
            sizeof(query_params) / sizeof(*query_params),
            NULL,
            query_params,
            (int[]) {0, 0, params.avatar->length, MD5_DIGEST_LENGTH, 0, params.xmr_deposit_address ? params.xmr_deposit_address->length: 0},
            (int[]) {0, 0, 1, 1, 0, 0},
            0
        );
    }

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_TUPLES_OK) {
        PQclear(query_result);

        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        result.is_already_exists = true;

        PQexec(pg_conn, "ROLLBACK");
        
        goto RETURN;
    }

    char* id_str = PQgetvalue(query_result, 0, 0);
    uint64_t id = atoi(id_str);
    int id_str_length = PQgetlength(query_result, 0, 0);
    id_str = strdup(id_str);
    char* xmr_index_str = PQgetvalue(query_result, 0, 1);
    int xmr_index = atoi(xmr_index_str);

    PQclear(query_result);

    pkrsrv_deposit_monero_address_t* xmr_deposit_address = NULL;

    if (!params.xmr_deposit_address) {
        xmr_deposit_address = pkrsrv_deposit_monero_create_address(0, xmr_index);

        if (!xmr_deposit_address) {
            printf("[Error] [Monero] Failed to create Monero deposit address!\n");
            PQexec(pg_conn, "ROLLBACK");
            free(id_str);
            goto RETURN;
        }

        PKRSRV_REF_COUNTED_USE(xmr_deposit_address);

        {
            const char* query_params[] = {
                xmr_deposit_address->address->value,
                id_str
            };
            
            query_result = PQexecParams(
                pg_conn,
                "UPDATE accounts SET "
                    "xmr_deposit_address = $1 "
                "where "
                    "id = $2 ",
                sizeof(query_params) / sizeof(*query_params),
                NULL,
                query_params,
                (int[]) {xmr_deposit_address->address->length, id_str_length},
                (int[]) {0, 0},
                0
            );
        }

        free(id_str);

        query_result_status = PQresultStatus(query_result);

        if (query_result_status != PGRES_COMMAND_OK) {
            PQclear(query_result);

            printf("[Error] [DB] Query failed!\n");
            printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

            PQexec(pg_conn, "ROLLBACK");

            pkrsrv_deposit_monero_address_free(xmr_deposit_address);
            
            goto RETURN;
        }
    } else {
        free(id_str);
    }

    PQexec(pg_conn, "COMMIT");

    pkrsrv_account_new_params_t new_params;
    new_params.id = id;
    new_params.id_token = params.id_token;
    new_params.name = params.name;
    new_params.avatar = params.avatar;
    new_params.balance = params.balance;
    if (xmr_deposit_address) {
        new_params.xmr_deposit_address = xmr_deposit_address->address;
    } else {
        new_params.xmr_deposit_address = params.xmr_deposit_address;
    }
    new_params.total_deposited = 0;
    new_params.locked_balance = 0;

    account = pkrsrv_account_new(new_params);

    result.account = account;

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(params.id_token);
    PKRSRV_REF_COUNTED_LEAVE(params.password);
    PKRSRV_REF_COUNTED_LEAVE(params.name);
    PKRSRV_REF_COUNTED_LEAVE(params.avatar);
    PKRSRV_REF_COUNTED_LEAVE(xmr_deposit_address);
    if (params.xmr_deposit_address) {
        PKRSRV_REF_COUNTED_LEAVE(params.xmr_deposit_address);
    }

    return result;
}

bool pkrsrv_account_update_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t balance) {
    PKRSRV_REF_COUNTED_USE(account);

    char balance_str[21];
    sprintf(balance_str, "%llu", balance);

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", account->id);

    PGresult* query_result;
    ExecStatusType query_result_status;

    {
        const char* query_params[] = {
            (char *) id_str
        };

        query_result = PQexecParams(
            pg_conn,
            "SELECT "
                "* "
            "FROM accounts "
            "WHERE "
                "id = $1 "
            "FOR UPDATE",
            sizeof(query_params) / sizeof(*query_params),
            NULL,
            query_params,
            (int[]) {id_str_len},
            (int[]) {0},
            0
        );
    }

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_TUPLES_OK) {
        PQclear(query_result);
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        PKRSRV_REF_COUNTED_LEAVE(account);
        
        return false;
    }

    PQclear(query_result);

    {
        const char* query_params[] = {
            balance_str,
            (char *) id_str
        };
        
        query_result = PQexecParams(
            pg_conn,
            "UPDATE accounts SET "
                "balance = $1 "
            "where "
                "id = $2 ",
            sizeof(query_params) / sizeof(*query_params),
            NULL,
            query_params,
            (int[]) {0, id_str_len},
            (int[]) {0, 0},
            0
        );
    }

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_COMMAND_OK) {
        PQclear(query_result);
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        PKRSRV_REF_COUNTED_LEAVE(account);
        
        return false;
    }

    pkrsrv_account_set_balance(account, balance);

    if (account->on_updated) {
        account->on_updated(account, account->on_updated_param);
    }

    PQclear(query_result);

    PKRSRV_REF_COUNTED_LEAVE(account);
    
    return true;
}

bool pkrsrv_account_add_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount) {
    bool retval = true;
    
    PKRSRV_REF_COUNTED_USE(account);

    uint64_t new_balance = account->balance;
    new_balance += amount;
    PKRSRV_UTIL_ASSERT(new_balance >= 0);

    if (!pkrsrv_account_update_balance(pg_conn, account, new_balance)) {
        goto RETURN;
    }

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(account);
    
    return retval;
}

bool pkrsrv_account_update_locked_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t locked_balance) {
    PKRSRV_REF_COUNTED_USE(account);

    char locked_balance_str[21];
    sprintf(locked_balance_str, "%llu", locked_balance);

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", account->id);

    PGresult* query_result;
    ExecStatusType query_result_status;

    {
        const char* query_params[] = {
            locked_balance_str,
            (char *) id_str
        };
        
        query_result = PQexecParams(
            pg_conn,
            "UPDATE accounts SET "
                "locked_balance = $1 "
            "where "
                "id = $2 ",
            sizeof(query_params) / sizeof(*query_params),
            NULL,
            query_params,
            (int[]) {0, id_str_len},
            (int[]) {0, 0},
            0
        );
    }

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_COMMAND_OK) {
        PQclear(query_result);
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        PKRSRV_REF_COUNTED_LEAVE(account);
        
        return false;
    }

    PQclear(query_result);

    PKRSRV_REF_COUNTED_LEAVE(account);
    
    return true;
}

bool pkrsrv_account_remove_locked_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount) {
    bool retval = true;
    
    PKRSRV_REF_COUNTED_USE(account);

    uint64_t new_locked_balance = account->locked_balance;

    if (new_locked_balance < amount) {
        PKRSRV_REF_COUNTED_LEAVE(account);
        return false;
    }

    new_locked_balance -= amount;

    PKRSRV_UTIL_ASSERT(new_locked_balance >= 0);

    if (!pkrsrv_account_update_locked_balance(pg_conn, account, new_locked_balance)) {
        goto RETURN;
    }

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(account);
    
    return retval;
}

bool pkrsrv_account_lock_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount) {
    bool retval = true;
    
    PKRSRV_REF_COUNTED_USE(account);

    uint64_t balance = account->balance;
    uint64_t locked_balance = account->locked_balance;

    if (balance < amount) {
        PKRSRV_REF_COUNTED_LEAVE(account);
        return false;
    }

    balance -= amount;
    locked_balance += amount;

    PKRSRV_UTIL_ASSERT(balance >= 0);
    PKRSRV_UTIL_ASSERT(locked_balance >= 0);

    if (!pkrsrv_account_update_balance(pg_conn, account, balance)) {
        goto RETURN;
    }

    if (!pkrsrv_account_update_locked_balance(pg_conn, account, locked_balance)) {
        goto RETURN;
    }

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(account);
    
    return retval;
}

bool pkrsrv_account_unlock_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount) {
    bool retval = true;
    
    PKRSRV_REF_COUNTED_USE(account);

    uint64_t balance = account->balance;
    uint64_t locked_balance = account->locked_balance;

    if (locked_balance < amount) {
        PKRSRV_REF_COUNTED_LEAVE(account);
        return false;
    }

    balance += amount;
    locked_balance -= amount;

    PKRSRV_UTIL_ASSERT(balance >= 0);
    PKRSRV_UTIL_ASSERT(locked_balance >= 0);

    if (!pkrsrv_account_update_balance(pg_conn, account, balance)) {
        goto RETURN;
    }

    if (!pkrsrv_account_update_locked_balance(pg_conn, account, locked_balance)) {
        goto RETURN;
    }

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(account);
    
    return retval;
}

bool pkrsrv_account_revert_locked_balanes(PGconn* pg_conn) {
    pkrsrv_util_verbose("Reverting locked balances...\n");
    
    PGresult* query_result;
    ExecStatusType query_result_status;

    int reverted_count = 0;

    query_result = PQexec(pg_conn, "UPDATE accounts SET balance = balance + locked_balance, locked_balance = 0 WHERE locked_balance > 0");

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_COMMAND_OK) {
        PQclear(query_result);
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));
        printf("\tReverting locked balances failed!\n");
        
        return false;
    }

    reverted_count = atoi(PQcmdTuples(query_result));

    pkrsrv_util_verbose("Reverted %d locked balances.\n", reverted_count);

    PQclear(query_result);

    return true;
}

bool pkrsrv_account_update_xmr_height(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t xmr_height) {
    PKRSRV_REF_COUNTED_USE(account);
    
    char xmr_height_str[20];
    int xmr_height_str_len = sprintf(xmr_height_str, "%llu", xmr_height);

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", account->id);

    PGresult* query_result;
    ExecStatusType query_result_status;

    {
        const char* query_params[] = {
            xmr_height_str,
            (char *) id_str
        };
        
        query_result = PQexecParams(
            pg_conn,
            "UPDATE accounts SET "
                "xmr_height = $1 "
            "where "
                "id = $2 ",
            sizeof(query_params) / sizeof(*query_params),
            NULL,
            query_params,
            (int[]) {xmr_height_str_len, id_str_len},
            (int[]) {0, 0},
            0
        );
    }

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_COMMAND_OK) {
        PQclear(query_result);
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));
        
        PKRSRV_REF_COUNTED_LEAVE(account);
        
        return false;
    }

    PQclear(query_result);

    PKRSRV_REF_COUNTED_LEAVE(account);

    return true;
}

pkrsrv_account_update_result_t pkrsrv_account_update(PGconn* pg_conn, pkrsrv_account_update_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.name);
    PKRSRV_REF_COUNTED_USE(params.avatar);
    
    pkrsrv_account_update_result_t result;
    result.is_ok = true;
    result.is_avatar_too_big = false;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", params.id);

    PGresult* query_result;
    ExecStatusType query_result_status;
    
    const char* query_params[] = {
        params.name->value,
        params.avatar->value,
        id_str
    };
    
    query_result = PQexecParams(
        pg_conn,
        "UPDATE accounts SET "
            " name = $1, "
            " avatar = CASE WHEN $2::bytea = ''::bytea THEN avatar ELSE $2::bytea END "
        "where "
            "id = $3 ",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {0, params.avatar->length, id_str_len},
        (int[]) {0, 1, 0},
        0
    );

    query_result_status = PQresultStatus(query_result);

     if (query_result_status != PGRES_COMMAND_OK) {
        PQclear(query_result);
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        result.is_ok = false;
        
        goto RETURN;
    }

    PQclear(query_result);

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(params.name);
    PKRSRV_REF_COUNTED_LEAVE(params.avatar);

    return result;
}

pkrsrv_account_t* pkrsrv_account_getby_credentials(PGconn* pg_conn, pkrsrv_account_getby_credentials_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.id_token);
    PKRSRV_REF_COUNTED_USE(params.password);
    
    pkrsrv_account_t* account = NULL;

    unsigned char password_md5[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, params.password->value, params.password->length);
    MD5_Final(password_md5, &ctx);

    PGresult* query_result;
    ExecStatusType query_result_status;

    const char* query_params[] = {
        params.id_token->value,
        password_md5,
    };

    query_result = PQexecParams(
        pg_conn,
        "select "
            PKRSRV_ACCOUNT_SELECT_COLUMNS
        "from accounts "
        "where "
            " (id_token = $1) and "
            " (password = $2)"
        "limit 1",
        sizeof(query_params) / sizeof(*query_params),
        NULL,
        query_params,
        (int[]) {0, MD5_DIGEST_LENGTH},
        (int[]) {0, 1},
        0
    );

    query_result_status = PQresultStatus(query_result);

    if (query_result_status != PGRES_TUPLES_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));
        
        goto RETURN;
    }

    int rows_length = PQntuples(query_result);

    if (!rows_length) {
        goto RETURN;
    }

    char* id_str = PQgetvalue(query_result, 0, 0);
    uint64_t id = atoi(id_str);

    pkrsrv_string_t* id_token = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 1), PQgetlength(query_result, 0, 1));
    pkrsrv_string_t* name = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 2), PQgetlength(query_result, 0, 2));
    char* avatar_str = PQgetvalue(query_result, 0, 3);
    size_t avatar_length = 0;
    unsigned char* avatar_data = PQunescapeBytea((unsigned char *) avatar_str, &avatar_length);
    pkrsrv_string_t* avatar = pkrsrv_string_new_from_binary__copy(avatar_data, avatar_length);
    PQfreemem(avatar_data);
    char* balance_str = PQgetvalue(query_result, 0, 4);
    pkrsrv_string_t* xmr_deposit_address = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 5), PQgetlength(query_result, 0, 5));
    char* locked_balance_str = PQgetvalue(query_result, 0, 6);
    char* total_deposited_str = PQgetvalue(query_result, 0, 7);
    char* xmr_index_str = PQgetvalue(query_result, 0, 8);
    int xmr_index = atoi(xmr_index_str);
    char* xmr_height_str = PQgetvalue(query_result, 0, 9);
    uint64_t xmr_height = atoll(xmr_height_str);

    pkrsrv_account_new_params_t new_params;
    new_params.id = id;
    new_params.id_token = id_token;
    new_params.name = name;
    new_params.avatar = avatar;
    new_params.xmr_deposit_address = xmr_deposit_address;
    new_params.xmr_deposit_address_index = xmr_index;
    new_params.xmr_height = xmr_height;
    new_params.balance = atoll(balance_str);
    new_params.locked_balance = atoll(locked_balance_str);
    new_params.total_deposited = atoll(total_deposited_str);

    account = pkrsrv_account_new(new_params);

    RETURN:

    PQclear(query_result);

    PKRSRV_REF_COUNTED_LEAVE(params.id_token);
    PKRSRV_REF_COUNTED_LEAVE(params.password);

    return account;
}

pkrsrv_account_t* pkrsrv_account_getby_id(PGconn* pg_conn, uint64_t p_id) {
    pkrsrv_account_t* account = NULL;

    char id_str[20];
    int id_str_len = sprintf(id_str, "%llu", p_id);

    PGresult* query_result;
    ExecStatusType query_result_status;

    const char* query_params[] = {
        (char *) &id_str,
    };

    query_result = PQexecParams(
        pg_conn,
        "select "
            PKRSRV_ACCOUNT_SELECT_COLUMNS
        "from accounts "
        "where "
            " (id = $1) "
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
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));
        
        goto RETURN;
    }

    int rows_length = PQntuples(query_result);

    if (!rows_length) {
        goto RETURN;
    }

    uint64_t id = p_id;

    pkrsrv_string_t* id_token = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 1), PQgetlength(query_result, 0, 1));
    pkrsrv_string_t* name = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 2), PQgetlength(query_result, 0, 2));
    char* avatar_str = PQgetvalue(query_result, 0, 3);
    size_t avatar_length = 0;
    unsigned char* avatar_data = PQunescapeBytea((unsigned char *) avatar_str, &avatar_length);
    pkrsrv_string_t* avatar = pkrsrv_string_new_from_binary__copy(avatar_data, avatar_length);
    PQfreemem(avatar_data);
    char* balance_str = PQgetvalue(query_result, 0, 4);
    pkrsrv_string_t* xmr_deposit_address = pkrsrv_string_new_from_cstr__copy(PQgetvalue(query_result, 0, 5), PQgetlength(query_result, 0, 5));
    char* locked_balance_str = PQgetvalue(query_result, 0, 6);
    char* total_deposited_str = PQgetvalue(query_result, 0, 7);
    char* xmr_index_str = PQgetvalue(query_result, 0, 8);
    int xmr_index = atoi(xmr_index_str);
    char* xmr_height_str = PQgetvalue(query_result, 0, 9);
    uint64_t xmr_height = atoll(xmr_height_str);

    pkrsrv_account_new_params_t new_params;
    new_params.id = id;
    new_params.id_token = id_token;
    new_params.name = name;
    new_params.avatar = avatar;
    new_params.xmr_deposit_address = xmr_deposit_address;
    new_params.xmr_deposit_address_index = xmr_index;
    new_params.xmr_height = xmr_height;
    new_params.balance = atoll(balance_str);
    new_params.locked_balance = atoll(locked_balance_str);
    new_params.total_deposited = atoll(total_deposited_str);

    account = pkrsrv_account_new(new_params);
    
    RETURN:

    PQclear(query_result);

    return account;
}

void pkrsrv_account_fetch(PGconn* pg_conn, pkrsrv_account_t* p_account) {
    pkrsrv_account_t* account = pkrsrv_account_getby_id(pg_conn, p_account->id);
    PKRSRV_UTIL_ASSERT(account);
    
    pkrsrv_string_move(p_account->xmr_deposit_address, account->xmr_deposit_address);
    pkrsrv_string_move(p_account->name, account->name);
    pkrsrv_string_move(p_account->avatar, account->avatar);

    p_account->balance = account->balance;
    p_account->locked_balance = account->locked_balance;
    p_account->total_deposited = account->total_deposited;
    p_account->xmr_height = account->xmr_height;
    p_account->xmr_deposit_address_index = account->xmr_deposit_address_index;
    
    pkrsrv_account_free(account);
}

pkrsrv_account_iterator_t* pkrsrv_account_iterator_new() {
    pkrsrv_account_iterator_t* iterator = malloc(sizeof(pkrsrv_account_iterator_t));
    PKRSRV_REF_COUNTED_INIT(iterator, pkrsrv_account_iterator_free);

    iterator->current = NULL;
    iterator->index = -1;
    iterator->count = 0;
    
    return iterator;
}

void pkrsrv_account_iterator_free(pkrsrv_account_iterator_t* iterator) {
    if (PQresultStatus(iterator->query_result) == PGRES_TUPLES_OK) {
        PQclear(iterator->query_result);
    }
    free(iterator);
}

pkrsrv_account_iterator_t* pkrsrv_account_iterator_query(PGconn* pg_conn, pkrsrv_string_t* query, char** params, int params_length) {
    pkrsrv_account_iterator_t* iterator = pkrsrv_account_iterator_new();

    iterator->query_result = PQexecParams(
        pg_conn,
        query->value,
        params_length,
        NULL,
        (const char**) params,
        NULL,
        NULL,
        0
    );

    ExecStatusType status = PQresultStatus(iterator->query_result);

    if (status != PGRES_TUPLES_OK) {
        printf("[Error] [DB] Query failed!\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(iterator->query_result));
        pkrsrv_account_iterator_free(iterator);
        return NULL;
    }

    iterator->count = PQntuples(iterator->query_result);

    return iterator;
}

bool pkrsrv_account_iterator_next(pkrsrv_account_iterator_t* iterator) {
    pkrsrv_account_t* account = NULL;

    iterator->index++;

    if (iterator->index >= iterator->count) {
        goto RETURN;
    }

    char* id_str = PQgetvalue(iterator->query_result, iterator->index, 0);
    uint64_t id = atoi(id_str);
    pkrsrv_string_t* id_token = pkrsrv_string_new_from_cstr__copy(PQgetvalue(iterator->query_result, iterator->index, 1), PQgetlength(iterator->query_result, iterator->index, 1));
    pkrsrv_string_t* name = pkrsrv_string_new_from_cstr__copy(PQgetvalue(iterator->query_result, iterator->index, 2), PQgetlength(iterator->query_result, iterator->index, 2));
    char* avatar_str = PQgetvalue(iterator->query_result, iterator->index, 3);
    size_t avatar_length = 0;
    unsigned char* avatar_data = PQunescapeBytea((unsigned char *) avatar_str, &avatar_length);
    pkrsrv_string_t* avatar = pkrsrv_string_new_from_binary__copy(avatar_data, avatar_length);
    PQfreemem(avatar_data);
    char* balance_str = PQgetvalue(iterator->query_result, iterator->index, 4);
    pkrsrv_string_t* xmr_deposit_address = pkrsrv_string_new_from_cstr__copy(PQgetvalue(iterator->query_result, iterator->index, 5), PQgetlength(iterator->query_result, iterator->index, 5));
    char* locked_balance_str = PQgetvalue(iterator->query_result, iterator->index, 6);
    char* total_deposited_str = PQgetvalue(iterator->query_result, iterator->index, 7);
    char* xmr_index_str = PQgetvalue(iterator->query_result, iterator->index, 8);
    int xmr_index = atoi(xmr_index_str);
    char* xmr_height_str = PQgetvalue(iterator->query_result, iterator->index, 9);
    uint64_t xmr_height = atoll(xmr_height_str);

    pkrsrv_account_new_params_t new_params;
    new_params.id = id;
    new_params.id_token = id_token;
    new_params.name = name;
    new_params.avatar = avatar;
    new_params.xmr_deposit_address = xmr_deposit_address;
    new_params.xmr_deposit_address_index = xmr_index;
    new_params.xmr_height = xmr_height;
    new_params.balance = atoll(balance_str);
    new_params.locked_balance = atoll(locked_balance_str);
    new_params.total_deposited = atoll(total_deposited_str);

    account = pkrsrv_account_new(new_params);

    RETURN:

    iterator->current = account;

    return iterator->current != NULL;
}