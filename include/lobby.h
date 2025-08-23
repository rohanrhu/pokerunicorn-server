/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup lobby Lobby
 * \brief Lobby and session management and interactions.
 */

/**
 * \addtogroup lobby
 * \ingroup lobby
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <libpq-fe.h>

#include "ref.h"
#include "sugar.h"
#include "table.h"
#include "account.h"
#include "poker.h"
#include "server.h"
#include "eventloop.h"
#include "trie.h"
#include "auth.h"
#include "../include/mime.h"

/**
 * \brief Lobby disconnection tolerance in milliseconds
 * 
 * If a client is disconnected from the lobby, it will be removed from the lobby after this time. Default is 20 seconds.
 * 
 * \note This avoids the client being removed from the lobby immediately after disconnection.
 */
#define PKRSRV_LOBBY_DISCONNECTION_TOLERANCE 20000

typedef struct pkrsrv_lobby pkrsrv_lobby_t;
typedef struct pkrsrv_lobby_sessions pkrsrv_lobby_sessions_t;
typedef struct pkrsrv_lobby_session pkrsrv_lobby_session_t;
typedef struct pkrsrv_lobby_session_clients pkrsrv_lobby_session_clients_t;
typedef struct pkrsrv_lobby_session_client pkrsrv_lobby_session_client_t;
typedef struct pkrsrv_lobby_client pkrsrv_lobby_client_t;
typedef struct pkrsrv_lobby_client_sessions pkrsrv_lobby_client_sessions_t;
typedef struct pkrsrv_lobby_client_session pkrsrv_lobby_client_session_t;

/**
 * \implements pkrsrv_ref_counted
 * \brief Lobby object
 */
struct pkrsrv_lobby {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_server_t* server;
    pthread_mutex_t mutex;
    int process_latency;
    pkrsrv_lobby_sessions_t* sessions;
    pkrsrv_eventloop_t* eventloop;
    pkrsrv_trie__ascii_t* authorized_clients;
};

/**
 * \brief Lobby sessions list structure
 */
struct pkrsrv_lobby_sessions {
    PKRSRV_REF_COUNTEDIFY();
    LISTIFY(pkrsrv_lobby_session_t*);
};

/**
 * \memberof pkrsrv_lobby_sessions
 * \brief Creates a new lobby sessions list
 */
pkrsrv_lobby_sessions_t* pkrsrv_lobby_sessions_new();
/**
 * \memberof pkrsrv_lobby_sessions
 * \brief Frees the lobby sessions list (stops & releases all sessions)
 */
void pkrsrv_lobby_sessions_free(pkrsrv_lobby_sessions_t* sessions);

/**
 * \implements pkrsrv_ref_counted
 * \brief Session object
 */
struct pkrsrv_lobby_session {
    PKRSRV_REF_COUNTEDIFY();
    ITEMIFY(pkrsrv_lobby_session_t*);
    PGconn* pg_conn;
    pkrsrv_lobby_t* lobby;
    pkrsrv_uniqid_uuid_t id;
    int process_latency;
    pkrsrv_poker_t* poker;
    pthread_mutex_t mutex;
    bool is_running;
    pkrsrv_eventloop_task_t* proceed_task;
    pkrsrv_eventloop_task_t* poker_start_task;
    pkrsrv_eventloop_task_t* poker_restart_task;
    pkrsrv_lobby_session_clients_t* clients;
    int joined_clients_count;
};

/**
 * \brief Session clients list structure
 */
struct pkrsrv_lobby_session_clients {
    LISTIFY(pkrsrv_lobby_session_client_t*);
};

/**
 * \implements pkrsrv_ref_counted
 * \brief Session client object
 */
struct pkrsrv_lobby_session_client {
    ITEMIFY(pkrsrv_lobby_session_client_t*);
    pkrsrv_lobby_client_t* client;
    bool is_joined;
    PKRSRV_REF_COUNTEDIFY();
};

/**
 * \implements pkrsrv_ref_counted
 * \brief Lobby client session object
 */
struct pkrsrv_lobby_client_session {
    ITEMIFY(pkrsrv_lobby_client_session_t*);
    pkrsrv_lobby_session_t* session;
    bool is_joined;
    PKRSRV_REF_COUNTEDIFY();
};

/**
 * \brief Lobby client sessions list structure
 */
struct pkrsrv_lobby_client_sessions {
    LISTIFY(pkrsrv_lobby_client_session_t*);
    PKRSRV_REF_COUNTEDIFY();
};

pkrsrv_lobby_client_sessions_t *pkrsrv_lobby_client_sessions_new();
void pkrsrv_lobby_client_sessions_free(pkrsrv_lobby_client_sessions_t *sessions);

/**
 * \implements pkrsrv_ref_counted
 * \brief Lobby client object
 */
struct pkrsrv_lobby_client {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_lobby_t* lobby;
    pkrsrv_server_client_t* client;
    pkrsrv_lobby_client_sessions_t* sessions;
    pkrsrv_account_t* account;
    pkrsrv_auth_session_t* auth_session;
    bool is_tolerant_disconnected;
    bool is_disconnected;
    bool is_connected;
    bool is_revived;
    pkrsrv_eventloop_task_t* tolerated_disconnected_task;
};

/**
 * \brief Parameters of `on_account_updated__async`
 */
struct on_account_updated_params {
    pkrsrv_account_t* account;
    pkrsrv_lobby_client_t* lobby_client;
};
typedef struct on_account_updated_params on_account_updated_params_t;
/**
 * \brief Used for `pkrsrv_account_t::on_updated`
 */
void on_account_updated__async(pkrsrv_eventloop_task_t* task);

/**
 * \brief Parameters of `pkrsrv_lobby_new`
 */
struct pkrsrv_lobby_new_params {
    int port;
    char* bind_address;
    int process_latency;
    int max_clients;
};
typedef struct pkrsrv_lobby_new_params pkrsrv_lobby_new_params_t;
/**
 * \brief Creates a new lobby object
 */
pkrsrv_lobby_t* pkrsrv_lobby_new(pkrsrv_lobby_new_params_t params);
/**
 * \brief Frees a lobby object
 */
void pkrsrv_lobby_free(pkrsrv_lobby_t* lobby);
/**
 * \brief Runs the lobby
 */
void pkrsrv_lobby_run(pkrsrv_lobby_t* lobby);

/**
 * \brief Creates a new lobby client object
 */
pkrsrv_lobby_client_t* pkrsrv_lobby_client_new(pkrsrv_lobby_t* lobby, pkrsrv_server_client_t* client);
/**
 * \brief Sets the account of a lobby client
 */
void pkrsrv_lobby_client_set_account(pkrsrv_lobby_client_t* lobby_client, pkrsrv_account_t* account);
/**
 * \brief Frees a lobby client object
 */
void pkrsrv_lobby_client_free(pkrsrv_lobby_client_t* lobby_client);

/**
 * \memberof pkrsrv_lobby_session
 * \brief Parameters of `pkrsrv_lobby_session_new`
 */
struct pkrsrv_lobby_session_new_params {
    pkrsrv_auth_session_t* auth_session;
    pkrsrv_lobby_t* lobby;
    pkrsrv_table_t* table;
};
typedef struct pkrsrv_lobby_session_new_params pkrsrv_lobby_session_new_params_t;
/**
 * \memberof pkrsrv_lobby_session
 * \brief Creates a new session object
 */
pkrsrv_lobby_session_t* pkrsrv_lobby_session_new(pkrsrv_lobby_session_new_params_t params);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Frees a session object
 */
void pkrsrv_lobby_session_free(pkrsrv_lobby_session_t* session);

/**
 * \memberof pkrsrv_lobby_sessions
 * \brief Adds a session to the lobby
 */
void pkrsrv_lobby_sessions_add(pkrsrv_lobby_sessions_t* sessions, pkrsrv_lobby_session_t* session);
/**
 * \memberof pkrsrv_lobby_sessions
 * \brief Removes a session from the lobby
 */
void pkrsrv_lobby_sessions_remove(pkrsrv_lobby_sessions_t* sessions, pkrsrv_lobby_session_t* session);

/**
 * \memberof pkrsrv_lobby_session
 * \brief Starts the session
 */
void pkrsrv_lobby_session_start(pkrsrv_lobby_session_t* session);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Stops the session
 */
void pkrsrv_lobby_session_stop(pkrsrv_lobby_session_t* session);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Proceeds the session
 */
void pkrsrv_lobby_session_free(pkrsrv_lobby_session_t* p_session);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Gives the session by its ID
 */
pkrsrv_lobby_session_t* pkrsrv_lobby_session_getby_id(pkrsrv_lobby_t* lobby, uint64_t id);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Gives the session by its table ID
 */
pkrsrv_lobby_session_t* pkrsrv_lobby_session_getby_table_id(pkrsrv_lobby_t* lobby, uint64_t id);

/**
 * \memberof pkrsrv_lobby_session
 * \brief Parameters of `pkrsrv_lobby_session_proceed__async`
 */
struct pkrsrv_lobby_session_proceed__async_params {
    pkrsrv_lobby_session_t* session;
};
typedef struct pkrsrv_lobby_session_proceed__async_params pkrsrv_lobby_session_proceed__async_params_t;
/**
 * \memberof pkrsrv_lobby_session
 * \brief Proceeds the session's game one step to future
 */
void pkrsrv_lobby_session_proceed__async(pkrsrv_eventloop_task_t* task);

/**
 * \memberof pkrsrv_lobby_session
 * \brief Parameters of `pkrsrv_lobby_session_poker_start__async`
 */
struct pkrsrv_lobby_session_poker_start__async_params {
    pkrsrv_lobby_session_t* session;
};
typedef struct pkrsrv_lobby_session_poker_start__async_params pkrsrv_lobby_session_poker_start__async_params_t;
/**
 * \memberof pkrsrv_lobby_session
 * \brief Starts the poker game of the session
 */
void pkrsrv_lobby_session_poker_start__async(pkrsrv_eventloop_task_t* task);

/**
 * \memberof pkrsrv_lobby_session
 * \brief Parameters of `pkrsrv_lobby_session_poker_restart__async`
 */
struct pkrsrv_lobby_session_poker_restart__async_params {
    pkrsrv_lobby_session_t* session;
};
typedef struct pkrsrv_lobby_session_poker_restart__async_params pkrsrv_lobby_session_poker_restart__async_params_t;
/**
 * \memberof pkrsrv_lobby_session
 * \brief Restarts the poker game of the session
 */
void pkrsrv_lobby_session_poker_restart__async(pkrsrv_eventloop_task_t* task);

/**
 * \memberof pkrsrv_lobby_session_client
 * \brief Creates a new session client object
 */
pkrsrv_lobby_session_client_t* pkrsrv_lobby_session_client_new(pkrsrv_lobby_client_t* lobby_client);
/**
 * \memberof pkrsrv_lobby_session_client
 * \brief Frees a session client object
 */
void pkrsrv_lobby_session_client_free(pkrsrv_lobby_session_client_t* session_client);
/**
 * \memberof pkrsrv_lobby_session_clients
 * \brief Adds a session client to the session
 */
void pkrsrv_lobby_session_clients_add(pkrsrv_lobby_session_clients_t* session_clients, pkrsrv_lobby_session_client_t* session_client);
/**
 * \memberof pkrsrv_lobby_session_clients
 * \brief Removes a session client from the session
 */
void pkrsrv_lobby_session_clients_remove(pkrsrv_lobby_session_clients_t* session_clients, pkrsrv_lobby_session_client_t* session_client);

/**
 * \memberof pkrsrv_lobby_client_session
 * \brief Creates a new lobby client session object
 */
pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_session_new(pkrsrv_lobby_session_t* session);
/**
 * \memberof pkrsrv_lobby_client_session
 * \brief Frees a lobby client session object
 */
void pkrsrv_lobby_client_session_free(pkrsrv_lobby_client_session_t* joined_session);
/**
 * \memberof pkrsrv_lobby_client_sessions
 * \brief Adds a lobby client session to the lobby client
 */
void pkrsrv_lobby_client_sessions_add(pkrsrv_lobby_client_sessions_t* client_sessions, pkrsrv_lobby_client_session_t* client_session);
/**
 * \memberof pkrsrv_lobby_client_sessions
 * \brief Removes a lobby client session from the lobby client
 */
void pkrsrv_lobby_client_sessions_remove(pkrsrv_lobby_client_sessions_t* client_sessions, pkrsrv_lobby_client_session_t* client_session);

/**
 * \memberof pkrsrv_lobby_client
 * \brief Makes a lobby enter a session
 */
bool pkrsrv_lobby_client_enter_session(pkrsrv_lobby_client_t* client, pkrsrv_lobby_session_t* session);
/**
 * \memberof pkrsrv_lobby_client
 * \brief Makes a lobby leave a session
 */
bool pkrsrv_lobby_client_leave_session(pkrsrv_lobby_client_t* client, pkrsrv_lobby_client_session_t* client_session);
/**
 * \memberof pkrsrv_lobby_client
 * \param client `pkrsrv_lobby_client``*` The client
 * \param session `pkrsrv_lobby_session``*` The session
 * \param enterance_amount `uint64_t` The amount of money to enter the session
 * \return Is the join was successful?
 * \brief Makes a lobby join a session
 */
bool pkrsrv_lobby_client_join_session(pkrsrv_lobby_client_t* client, pkrsrv_lobby_session_t* session, uint64_t enterance_amount, int position);
/**
 * \memberof pkrsrv_lobby_client
 * \brief Makes a lobby unjoin a session
 * \return true
 */
bool pkrsrv_lobby_client_unjoin_session(pkrsrv_lobby_client_t* client, pkrsrv_lobby_session_t* session);

/**
 * \memberof pkrsrv_lobby_client
 * \brief Gives the session of a lobby client by its ID
 */
pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_get_session_by_id(pkrsrv_lobby_client_t* client, uint64_t id);
/**
 * \memberof pkrsrv_lobby_client
 * \brief Gİies the session of a lobby client by its table ID
 */
pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_session_getby_table_id(pkrsrv_lobby_client_t* client, uint64_t id);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Gives the client of a session by its socket
 * \param session `pkrsrv_lobby_session``*` The session
 * \param socket `int` OS socket handler
 * \return The client
 */
pkrsrv_lobby_session_client_t* pkrsrv_lobby_session_client_getby_socket(pkrsrv_lobby_session_t* session, int socket);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Gives the client of a session by its account ID
 * \param session `pkrsrv_lobby_session``*` The session
 * \param account_id `uint64_t` The account ID
 * \return The client
 */
pkrsrv_lobby_session_client_t* pkrsrv_lobby_session_client_getby_account_id(pkrsrv_lobby_session_t* session, uint64_t account_id);
/**
 * \memberof pkrsrv_lobby_session
 * \brief Gives the client of a session by its account
 * \param session `pkrsrv_lobby_session``*` The session
 * \param account `pkrsrv_account``*` The account
 * \return The client
 */
pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_session_getby_session_id(pkrsrv_lobby_client_t* lobby_client, uint64_t session_id);

static void on_client_connected(pkrsrv_eventloop_task_t* task);
static void on_client_disconnected(pkrsrv_eventloop_task_t* task);
static void on_client_disconnected__tolerated(pkrsrv_eventloop_task_t* task);
static void on_client_meowed(pkrsrv_eventloop_task_t* task);
static void on_client_login(pkrsrv_eventloop_task_t* task);
static void on_client_signup(pkrsrv_eventloop_task_t* task);
static void on_client_auth_session(pkrsrv_eventloop_task_t* task);
static void on_client_get_account(pkrsrv_eventloop_task_t* task);
static void on_client_enter(pkrsrv_eventloop_task_t* task);
static void on_client_leave(pkrsrv_eventloop_task_t* task);
static void on_client_join(pkrsrv_eventloop_task_t* task);
static void on_client_unjoin(pkrsrv_eventloop_task_t* task);
static void on_client_action(pkrsrv_eventloop_task_t* task);
static void on_client_get_tables(pkrsrv_eventloop_task_t* task);
static void on_client_get_sessions(pkrsrv_eventloop_task_t* task);
static void on_client_update_account(pkrsrv_eventloop_task_t* task);
static void on_client_json(pkrsrv_eventloop_task_t* task);

static void on_poker_action(pkrsrv_eventloop_task_t* task);

/**
 * \brief Efficient session reorderer
 * \param updated `pkrsrv_lobby_session_t``*` The just-updated session
   \param old_length `int` Old number of total sessions before `updated` got updated
 */
bool reorder_updated_session(pkrsrv_lobby_session_t* updated, int old_length);

/**
 * \brief Broadcasts the sessions of the lobby to all clients
 */
void pkrsrv_lobby_broadcast_sessions(pkrsrv_lobby_t* lobby);

/**
 * @}
 */