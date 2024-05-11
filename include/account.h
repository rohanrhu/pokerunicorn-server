/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup account Accounts
 * \brief Account management and interactions.
 */

/**
 * \addtogroup account
 * \ingroup account
 * @{
 */

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

#include <libpq-fe.h>

#include "sugar.h"
#include "ref.h"
#include "string.h"

#define PKRSRV_ACCOUNT_SELECT_COLUMNS " id, id_token, name, avatar, balance, xmr_deposit_address, locked_balance, total_deposited, xmr_deposit_address_index, xmr_height "
#define PKRSRV_ACCOUNT_SELECT_COLUMNS_COUNT 11
#define PKRSRV_ACCOUNT_SELECT_COLUMNS_ARRAY { "id", "id_token", "name", "avatar", "balance", "xmr_deposit_address", "locked_balance", "total_deposited", "xmr_deposit_address_index", "xmr_height", NULL }

typedef struct pkrsrv_account pkrsrv_account_t;

/**
 * \implements pkrsrv_ref_counted
 * \brief Account object
 */
struct pkrsrv_account {
    PKRSRV_REF_COUNTEDIFY();
    uint64_t id;
    pkrsrv_string_t* id_token;
    pkrsrv_string_t* name;
    pkrsrv_string_t* avatar;
    pkrsrv_string_t* xmr_deposit_address;
    uint64_t xmr_deposit_address_index;
    uint64_t balance;
    uint64_t locked_balance;
    uint64_t total_deposited;
    uint64_t xmr_height;
    void* owner;
    void (*on_updated)(pkrsrv_account_t* account, void* param);
    void* on_updated_param;
};

typedef struct pkrsrv_account_new_params {
    uint64_t id;
    pkrsrv_string_t* id_token;
    pkrsrv_string_t* name;
    pkrsrv_string_t* avatar;
    pkrsrv_string_t* xmr_deposit_address;
    uint64_t xmr_deposit_address_index;
    uint64_t balance;
    uint64_t locked_balance;
    uint64_t total_deposited;
    uint64_t xmr_height;
} pkrsrv_account_new_params_t;

pkrsrv_account_t* pkrsrv_account_new(pkrsrv_account_new_params_t params);
void pkrsrv_account_free(pkrsrv_account_t* player);

/**
 * ! Safe account balance setter
 */
bool pkrsrv_account_set_balance(pkrsrv_account_t* account, uint64_t balance);

/**
 * ! Safe account balance getter
 */
void pkrsrv_account_get_balance(pkrsrv_account_t* account, uint64_t balance);

typedef struct pkrsrv_account_create_params {
    pkrsrv_string_t* id_token;
    pkrsrv_string_t* name;
    pkrsrv_string_t* avatar;
    pkrsrv_string_t* password;
    pkrsrv_string_t* xmr_deposit_address;
    uint64_t xmr_deposit_address_index;
    uint64_t balance;
    uint64_t locked_balance;
    uint64_t total_deposited;
    uint64_t xmr_height;
} pkrsrv_account_create_params_t;
typedef struct pkrsrv_account_create_result {
    pkrsrv_account_t* account;
    bool is_already_exists;
} pkrsrv_account_create_result_t;
pkrsrv_account_create_result_t pkrsrv_account_create(PGconn* pg_conn, pkrsrv_account_create_params_t params);

typedef struct pkrsrv_account_update_params {
    uint64_t id;
    pkrsrv_string_t* name;
    pkrsrv_string_t* avatar;
} pkrsrv_account_update_params_t;
typedef struct pkrsrv_account_update_result {
    bool is_ok;
    bool is_avatar_too_big;
} pkrsrv_account_update_result_t;
pkrsrv_account_update_result_t pkrsrv_account_update(PGconn* pg_conn, pkrsrv_account_update_params_t params);
bool pkrsrv_account_update_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t balance);
bool pkrsrv_account_add_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount);
bool pkrsrv_account_update_locked_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t locked_balance);
bool pkrsrv_account_remove_locked_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount);
bool pkrsrv_account_lock_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount);
bool pkrsrv_account_unlock_balance(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t amount);
bool pkrsrv_account_revert_locked_balanes(PGconn* pg_conn);

bool pkrsrv_account_update_xmr_height(PGconn* pg_conn, pkrsrv_account_t* account, uint64_t xmr_height);

typedef struct pkrsrv_account_getby_credentials_params {
    pkrsrv_string_t* id_token;
    pkrsrv_string_t* password;
} pkrsrv_account_getby_credentials_params_t;
pkrsrv_account_t* pkrsrv_account_getby_credentials(PGconn* pg_conn, pkrsrv_account_getby_credentials_params_t params);

pkrsrv_account_t* pkrsrv_account_getby_id(PGconn* pg_conn, uint64_t p_id);

void pkrsrv_account_fetch(PGconn* pg_conn, pkrsrv_account_t* p_account);

/**
 * \implements pkrsrv_ref_counted
 * \brief Account iterator object
 *
 * This struct represents an iterator for iterating over a collection of account objects.
 * It provides methods for creating, querying, and iterating over the accounts.
 */
typedef struct pkrsrv_account_iterator {
    PKRSRV_REF_COUNTEDIFY() /**< Macro for implementing reference counting */
    pkrsrv_account_t* current; /**< Pointer to the current account */
    int index; /**< Index of the current account */
    int count; /**< Total number of accounts */
    PGresult* query_result; /**< Result of the query */
} pkrsrv_account_iterator_t;

/**
 * \brief Creates a new account iterator.
 *
 * This function creates a new account iterator object.
 *
 * \return A pointer to the newly created account iterator.
 */
pkrsrv_account_iterator_t* pkrsrv_account_iterator_new();

/**
 * \brief Frees the memory occupied by an account iterator.
 *
 * This function frees the memory occupied by the given account iterator object.
 *
 * \param iterator The account iterator to be freed.
 */
void pkrsrv_account_iterator_free(pkrsrv_account_iterator_t* iterator);

/**
 * \brief Queries the database for account objects.
 *
 * This function queries the given PostgreSQL connection for account objects
 * using the specified query and parameters. It returns an account iterator
 * object that can be used to iterate over the queried accounts.
 *
 * \param pg_conn The PostgreSQL connection.
 * \param query The query string.
 * \param params An array of query parameters.
 * \param params_length The length of the params array.
 * \return A pointer to the account iterator object.
 */
pkrsrv_account_iterator_t* pkrsrv_account_iterator_query(PGconn* pg_conn, pkrsrv_string_t* query, char** params, int params_length);

/**
 * \brief Moves the iterator to the next account.
 *
 * This function moves the iterator to the next account in the collection.
 * It returns true if there is a next account, false otherwise.
 *
 * \param iterator The account iterator.
 * \return True if there is a next account, false otherwise.
 */
bool pkrsrv_account_iterator_next(pkrsrv_account_iterator_t* iterator);

/**
 * \example account_iterator_example.c
 *
 * This is an example code that demonstrates how to use the account iterator.
 *
 * \code{.c}
 * #include <stdio.h>
 * #include "account.h"
 *
 * int main() {
 *     // Create a new account iterator
 *     pkrsrv_account_iterator_t* iterator = pkrsrv_account_iterator_new();
 *
 *     // Query the database for account objects
 *     pkrsrv_account_iterator_query(pg_conn, query, params, params_length);
 *
 *     // Iterate over the accounts
 *     while (pkrsrv_account_iterator_next(iterator)) {
 *         // Get the current account
 *         pkrsrv_account_t* account = iterator->current;
 *
 *         // Print the account details
 *         printf("Account ID: %d\n", account->id);
 *         printf("Account Name: %s\n", account->name);
 *     }
 *
 *     // Free the memory occupied by the iterator
 *     pkrsrv_account_iterator_free(iterator);
 *
 *     return 0;
 * }
 * \endcode
 */

/**
 * @}
 */