/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup deposit Depositing
 * \brief Deposit management and interactions.
 */

/**
 * \addtogroup deposit
 * \ingroup deposit
 * @{
 */

#include <pthread.h>
#include <stdbool.h>

#include <libpq-fe.h>

#include "sugar.h"
#include "string.h"
#include "ref.h"

#include "../include/account.h"

#define PKRSRV_DEPOSIT_SERVICE_DELAY 10000
#define PKRSRV_DEPOSIT_MONERO_MIN_CONFIRMATIONS 1

typedef struct pkrsrv_deposit_service pkrsrv_deposit_service_t;
typedef struct pkrsrv_deposit_monero_address pkrsrv_deposit_monero_address_t;
typedef struct pkrsrv_deposit_monero_tx pkrsrv_deposit_monero_tx_t;
typedef struct pkrsrv_deposit_monero_tx_list pkrsrv_deposit_monero_tx_list_t;
typedef struct pkrsrv_deposit_monero_deposit pkrsrv_deposit_monero_deposit_t;
typedef struct pkrsrv_deposit_monero_deposit_list pkrsrv_deposit_monero_deposit_list_t;

typedef enum pkrsrv_deposit_monero_deposit_status {
    PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_NONE = 0,
    PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_PENDING,
    PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_CONFIRMED,
    PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_FAILED
} pkrsrv_deposit_monero_deposit_status_t;

/**
 * \implements pkrsrv_ref_counted
 * \brief Deposit service object
 */
struct pkrsrv_deposit_service {
    PKRSRV_REF_COUNTEDIFY();
    pthread_t thread;
    bool is_running;
    PGconn* pg_conn;
};

/**
 * \implements pkrsrv_ref_counted
 * \brief Monero address object
 */
struct pkrsrv_deposit_monero_address {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_string_t* address;
    int account_index;
    int index;
    int payment_id;
};

/**
 * \memberof pkrsrv_deposit_monero_address
 * \brief Creates a new Monero address object
 */
pkrsrv_deposit_monero_address_t* pkrsrv_deposit_monero_address_new(pkrsrv_string_t* address, int account_index, int index, int payment_id);
/**
 * \memberof pkrsrv_deposit_monero_address
 * \brief Frees a Monero address object
 */
void pkrsrv_deposit_monero_address_free(pkrsrv_deposit_monero_address_t* address);
/**
 * \memberof pkrsrv_deposit_monero_address
 * \brief Creates a new Monero address
 */
pkrsrv_deposit_monero_address_t* pkrsrv_deposit_monero_create_address(int account_index, int index);

/**
 * \memberof pkrsrv_deposit_service
 * \brief Creates a new deposit service object
 */
pkrsrv_deposit_service_t* pkrsrv_deposit_service_new();
/**
 * \memberof pkrsrv_deposit_service
 * \brief Frees a deposit service object
 */
void pkrsrv_deposit_service_free(pkrsrv_deposit_service_t* service);
/**
 * \memberof pkrsrv_deposit_service
 * \brief Starts the deposit service
 */
void pkrsrv_deposit_service_start(pkrsrv_deposit_service_t* service);
/**
 * \memberof pkrsrv_deposit_service
 * \brief Stops the deposit service
 */
void pkrsrv_deposit_service_stop(pkrsrv_deposit_service_t* service);
/**
 * \memberof pkrsrv_deposit_service
 * \brief Deposit service thread function
 */
void pkrsrv_deposit_service_thread_f(pkrsrv_deposit_service_t* service);

/**
 * \brief Makes a Monero wallet RPC call
 * \param method Method to call (C-str)
 * \param params Parameters to pass (C-str)
 * \return Response pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_deposit_monero_wallet_rpc_call(char* method, char* params);

/**
 * \brief Initializes the deposit service
 */
bool pkrsrv_deposit_monero_wallet_open(char* wallet_name, char* password);
/**
 * \brief Initializes the deposit service
 */
bool pkrsrv_deposit_monero_wallet_close();
/**
 * \brief Initializes the deposit service
 */
bool pkrsrv_deposit_monero_wallet_create(char* wallet_name, char* password);
/**
 * \brief Initializes the deposit service
 */
pkrsrv_string_t* pkrsrv_deposit_monero_get_main_address();
/**
 * \brief Initializes the deposit service
 */
bool pkrsrv_deposit_monero_get_balance(uint64_t* balance, uint64_t* unlocked_balance);
/**
 * \brief Initializes the deposit service
 */
bool pkrsrv_deposit_monero_get_address_balance(char* address, uint64_t* balance, uint64_t* unlocked_balance);

/**
 * \implements pkrsrv_ref_counted
 * \brief Initializes the deposit service
 */
struct pkrsrv_deposit_monero_tx {
    PKRSRV_REF_COUNTEDIFY();
    ITEMIFY(pkrsrv_deposit_monero_tx_t*);
    pkrsrv_string_t* txid;
    pkrsrv_string_t* address;
    uint64_t amount;
    uint64_t fee;
    uint64_t unlock_time;
    uint64_t timestamp;
    uint64_t height;
    bool is_confirmed;
};

/**
 * \memberof pkrsrv_deposit_monero_tx
 * \brief Creates a new Monero transaction object
 */
struct pkrsrv_deposit_monero_tx_list {
    PKRSRV_REF_COUNTEDIFY();
    LISTIFY(pkrsrv_deposit_monero_tx_t*);
};

/**
 * \memberof pkrsrv_deposit_monero_tx
 */
struct pkrsrv_deposit_monero_tx_new_params {
    pkrsrv_string_t* txid;
    pkrsrv_string_t* address;
    uint64_t amount;
    uint64_t fee;
    uint64_t unlock_time;
    uint64_t timestamp;
    uint64_t height;
    bool is_confirmed;
};
typedef struct pkrsrv_deposit_monero_tx_new_params pkrsrv_deposit_monero_tx_new_params_t;

/**
 * \memberof pkrsrv_deposit_monero_tx
 * \brief Creates a new Monero transaction object
 */
pkrsrv_deposit_monero_tx_t* pkrsrv_deposit_monero_tx_new(pkrsrv_deposit_monero_tx_new_params_t params);
/**
 * \memberof pkrsrv_deposit_monero_tx
 * \brief Frees a Monero transaction object
 */
void pkrsrv_deposit_monero_tx_free(pkrsrv_deposit_monero_tx_t* tx);

/**
 * \memberof pkrsrv_deposit_monero_tx_list
 * \brief Creates a new Monero transaction list object
 */
pkrsrv_deposit_monero_tx_list_t* pkrsrv_deposit_monero_tx_list_new();
/**
 * \memberof pkrsrv_deposit_monero_tx_list
 * \brief Frees a Monero transaction list object
 */
void pkrsrv_deposit_monero_tx_list_free(pkrsrv_deposit_monero_tx_list_t* list);
/**
 * \memberof pkrsrv_deposit_monero_tx_list
 * \brief Adds a Monero transaction to the list
 */
void pkrsrv_deposit_monero_tx_list_add(pkrsrv_deposit_monero_tx_list_t* list, pkrsrv_deposit_monero_tx_t* tx);
/**
 * \memberof pkrsrv_deposit_monero_tx_list
 * \brief Removes a Monero transaction from the list
 */
void pkrsrv_deposit_monero_tx_list_remove(pkrsrv_deposit_monero_tx_list_t* list, pkrsrv_deposit_monero_tx_t* tx);
/**
 * \memberof pkrsrv_deposit_monero_tx_list
 * \brief Gets a Monero transaction from the list by its transaction ID
 */
pkrsrv_deposit_monero_tx_t* pkrsrv_deposit_monero_tx_list_get_by_txid(pkrsrv_deposit_monero_tx_list_t* list, pkrsrv_string_t* txid);

/**
 * \brief Initializes the deposit service
 */
uint64_t pkrsrv_deposit_monero_get_height();
/**
 * \memberof pkrsrv_deposit_monero_tx_list
 * \brief Initializes the deposit service
 */
pkrsrv_deposit_monero_tx_list_t* pkrsrv_deposit_monero_get_txs(int account_index, int address_index, int min_height);

/**
 * \implements pkrsrv_ref_counted
 * \brief Monero deposit object
 */
struct pkrsrv_deposit_monero_deposit {
    PKRSRV_REF_COUNTEDIFY();
    ITEMIFY(pkrsrv_deposit_monero_deposit_t*);
    uint64_t id;
    pkrsrv_account_t* account;
    pkrsrv_string_t* txid;
    uint64_t timestamp;
    uint64_t amount;
    pkrsrv_string_t* to_address;
    pkrsrv_deposit_monero_deposit_status_t status;
};

/**
 * \memberof pkrsrv_deposit_monero_deposit
 */
typedef struct pkrsrv_deposit_monero_deposit_new_params {
    uint64_t id;
    pkrsrv_account_t* account;
    pkrsrv_string_t* txid;
    uint64_t timestamp;
    uint64_t amount;
    pkrsrv_string_t* to_address;
    pkrsrv_deposit_monero_deposit_status_t status;
} pkrsrv_deposit_monero_deposit_new_params_t;
/**
 * \memberof pkrsrv_deposit_monero_deposit
 * \brief Creates a new Monero deposit object
 */
pkrsrv_deposit_monero_deposit_t* pkrsrv_deposit_monero_deposit_new(pkrsrv_deposit_monero_deposit_new_params_t params);
/**
 * \memberof pkrsrv_deposit_monero_deposit
 * \brief Frees a Monero deposit object
 */
void pkrsrv_deposit_monero_deposit_free(pkrsrv_deposit_monero_deposit_t* deposit);
/**
 * \memberof pkrsrv_deposit_monero_deposit
 * \brief Gets a Monero deposit object by its transaction ID
 */
pkrsrv_deposit_monero_deposit_t* pkrsrv_deposit_monero_deposit_getby_txid(PGconn* pg_conn, pkrsrv_string_t* p_txid);

/**
 * \implements pkrsrv_ref_counted
 * \brief Monero deposit list object
 */
struct pkrsrv_deposit_monero_deposit_list {
    PKRSRV_REF_COUNTEDIFY();
    LISTIFY(pkrsrv_deposit_monero_deposit_t*);
};

/**
 * \memberof pkrsrv_deposit_monero_deposit_list
 * \brief Creates a new Monero deposit list object
 */
pkrsrv_deposit_monero_deposit_list_t* pkrsrv_deposit_monero_deposit_list_new();
/**
 * \memberof pkrsrv_deposit_monero_deposit_list
 * \brief Frees a Monero deposit list object
 */
void pkrsrv_deposit_monero_deposit_list_free(pkrsrv_deposit_monero_deposit_list_t* list);
/**
 * \memberof pkrsrv_deposit_monero_deposit_list
 * \brief Adds a Monero deposit to the list
 */
void pkrsrv_deposit_monero_deposit_list_add(pkrsrv_deposit_monero_deposit_list_t* list, pkrsrv_deposit_monero_deposit_t* deposit);
/**
 * \memberof pkrsrv_deposit_monero_deposit_list
 * \brief Removes a Monero deposit from the list
 */
void pkrsrv_deposit_monero_deposit_list_remove(pkrsrv_deposit_monero_deposit_list_t* list, pkrsrv_deposit_monero_deposit_t* deposit);

/**
 * \memberof pkrsrv_deposit_monero_deposit_list
 */
struct pkrsrv_deposit_monero_get_deposits_param {
    pkrsrv_account_t* account;
    pkrsrv_deposit_monero_deposit_status_t status;
    int min_height;
    int max_height;
    int offset;
    int limit;
};
typedef struct pkrsrv_deposit_monero_get_deposits_param pkrsrv_deposit_monero_get_deposits_params_t;

/**
 * \memberof pkrsrv_deposit_monero_deposit_list
 * \brief Retrieves deposits from the database
 */
pkrsrv_deposit_monero_deposit_list_t* pkrsrv_deposit_monero_get_deposits(PGconn* pg_conn, pkrsrv_deposit_monero_get_deposits_params_t params);

/**
 * \memberof pkrsrv_deposit_monero_deposit
 */
struct pkrsrv_deposit_monero_deposit_create_params {
    pkrsrv_account_t* account;
    pkrsrv_string_t* txid;
    uint64_t timestamp;
    uint64_t amount;
    pkrsrv_string_t* to_address;
    pkrsrv_deposit_monero_deposit_status_t status;
};
typedef struct pkrsrv_deposit_monero_deposit_create_params pkrsrv_deposit_monero_deposit_create_params_t;
/**
 * \memberof pkrsrv_deposit_monero_deposit
 * \brief Creates a deposit on the database
 */
pkrsrv_deposit_monero_deposit_t* pkrsrv_deposit_monero_deposit_create(PGconn* pg_conn, pkrsrv_deposit_monero_deposit_create_params_t params);

/**
 * \memberof pkrsrv_deposit_monero_deposit
 * \brief Updates the status of a deposit
 */
bool pkrsrv_deposit_monero_deposit_update_status(PGconn* pg_conn, pkrsrv_deposit_monero_deposit_t* deposit, pkrsrv_deposit_monero_deposit_status_t status);

/**
 * @}
 */