/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup server TCP & WebSocket Server (TLS)
 * \brief Server module can handle both TCP and WebSocket connections over TLS
 */

/**
 * \addtogroup server
 * \ingroup server
 * @{
 */

#include <stdbool.h>
#include <stdint.h>

#include <openssl/ssl.h>
#include <libpq-fe.h>

#include "ref.h"
#include "sugar.h"
#include "string.h"
#include "account.h"
#include "card.h"
#include "poker.h"
#include "websocket.h"
#include "eventloop.h"

typedef struct pkrsrv_server_packet_frame_header pkrsrv_server_packet_frame_header_t;
typedef struct pkrsrv_server_packet_frame_login pkrsrv_server_packet_frame_login_t;
typedef struct pkrsrv_server_packet_frame_login_res pkrsrv_server_packet_frame_login_res_t;
typedef struct pkrsrv_server_packet_frame_login_res_account pkrsrv_server_packet_frame_login_res_account_t;
typedef struct pkrsrv_server_packet_frame_signup pkrsrv_server_packet_frame_signup_t;
typedef struct pkrsrv_server_packet_frame_signup_res pkrsrv_server_packet_frame_signup_res_t;
typedef struct pkrsrv_server_packet_frame_signup_res_account pkrsrv_server_packet_frame_signup_res_account_t;
typedef struct pkrsrv_server_packet_frame_account pkrsrv_server_packet_frame_account_t;
typedef struct pkrsrv_server_packet_frame_enter pkrsrv_server_packet_frame_enter_t;
typedef struct pkrsrv_server_packet_frame_enter_res pkrsrv_server_packet_frame_enter_res_t;
typedef struct pkrsrv_server_packet_frame_leave pkrsrv_server_packet_frame_leave_t;
typedef struct pkrsrv_server_packet_frame_leave_res pkrsrv_server_packet_frame_leave_res_t;
typedef struct pkrsrv_server_packet_frame_join pkrsrv_server_packet_frame_join_t;
typedef struct pkrsrv_server_packet_frame_join_res pkrsrv_server_packet_frame_join_res_t;
typedef struct pkrsrv_server_packet_frame_unjoin pkrsrv_server_packet_frame_unjoin_t;
typedef struct pkrsrv_server_packet_frame_unjoin_res pkrsrv_server_packet_frame_unjoin_res_t;
typedef struct pkrsrv_server_packet_frame_poker_info pkrsrv_server_packet_frame_poker_info_t;
typedef struct pkrsrv_server_packet_frame_poker_info_player pkrsrv_server_packet_frame_poker_info_player_t;
typedef struct pkrsrv_server_packet_frame_poker_state pkrsrv_server_packet_frame_poker_state_t;
typedef struct pkrsrv_server_packet_frame_poker_state_player pkrsrv_server_packet_frame_poker_state_player_t;
typedef struct pkrsrv_server_packet_frame_poker_action pkrsrv_server_packet_frame_poker_action_t;
typedef struct pkrsrv_server_packet_frame_poker_action_reflection pkrsrv_server_packet_frame_poker_action_reflection_t;
typedef struct pkrsrv_server_packet_frame_poker_end pkrsrv_server_packet_frame_poker_end_t;
typedef struct pkrsrv_server_packet_frame_poker_end_score pkrsrv_server_packet_frame_poker_end_score_t;
typedef struct pkrsrv_server_packet_frame_poker_restarted pkrsrv_server_packet_frame_poker_restarted_t;
typedef struct pkrsrv_server_packet_frame_unjoined pkrsrv_server_packet_frame_unjoined_t;
typedef struct pkrsrv_server_packet_frame_get_tables pkrsrv_server_packet_frame_get_tables_t;
typedef struct pkrsrv_server_packet_frame_tables pkrsrv_server_packet_frame_tables_t;
typedef struct pkrsrv_server_packet_frame_get_sessions pkrsrv_server_packet_frame_get_sessions_t;
typedef struct pkrsrv_server_packet_frame_sessions pkrsrv_server_packet_frame_sessions_t;
typedef struct pkrsrv_server_packet_frame_table pkrsrv_server_packet_frame_table_t;
typedef struct pkrsrv_server_packet_frame_update_account pkrsrv_server_packet_frame_update_account_t;
typedef struct pkrsrv_server_packet_frame_update_account_res pkrsrv_server_packet_frame_update_account_res_t;
typedef struct pkrsrv_server_packet_frame_server_info pkrsrv_server_packet_frame_server_info_t;
typedef struct pkrsrv_server pkrsrv_server_t;
typedef struct pkrsrv_server_clients pkrsrv_server_clients_t;
typedef struct pkrsrv_server_client pkrsrv_server_client_t;
typedef uint32_t pkrsrv_server_opcode_t;

/**
 * Server opcodes
 */
enum PKRSRV_SERVER_OPCODE {
    PKRSRV_SERVER_OPCODE_NOP = 0,
    PKRSRV_SERVER_OPCODE_MEOW,
    PKRSRV_SERVER_OPCODE_PING,
    PKRSRV_SERVER_OPCODE_PONG,
    PKRSRV_SERVER_OPCODE_LOGIN,
    PKRSRV_SERVER_OPCODE_LOGIN_RES,
    PKRSRV_SERVER_OPCODE_SIGNUP,
    PKRSRV_SERVER_OPCODE_SIGNUP_RES,
    PKRSRV_SERVER_OPCODE_GET_ACCOUNT,
    PKRSRV_SERVER_OPCODE_ACCOUNT,
    PKRSRV_SERVER_OPCODE_ENTER,
    PKRSRV_SERVER_OPCODE_ENTER_RES,
    PKRSRV_SERVER_OPCODE_LEAVE,
    PKRSRV_SERVER_OPCODE_LEAVE_RES,
    PKRSRV_SERVER_OPCODE_JOIN,
    PKRSRV_SERVER_OPCODE_JOIN_RES,
    PKRSRV_SERVER_OPCODE_UNJOIN,
    PKRSRV_SERVER_OPCODE_UNJOIN_RES,
    PKRSRV_SERVER_OPCODE_POKER_INFO,
    PKRSRV_SERVER_OPCODE_POKER_STATE,
    PKRSRV_SERVER_OPCODE_POKER_ACTION,
    PKRSRV_SERVER_OPCODE_POKER_ACTION_REFLECTION,
    PKRSRV_SERVER_OPCODE_POKER_END,
    PKRSRV_SERVER_OPCODE_POKER_RESTARTED,
    PKRSRV_SERVER_OPCODE_JSON,
    PKRSRV_SERVER_OPCODE_UNJOINED,
    PKRSRV_SERVER_OPCODE_GET_TABLES,
    PKRSRV_SERVER_OPCODE_TABLES,
    PKRSRV_SERVER_OPCODE_GET_SESSIONS,
    PKRSRV_SERVER_OPCODE_SESSIONS,
    PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT,
    PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT_RES,
    PKRSRV_SERVER_OPCODE_SERVER_INFO,
    PKRSRV_SERVER_OPCODE_OVER_CAPACITY,
    PKRSRV_SERVER_OPCODE_END
};

typedef enum PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS {
    PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_OK = 0,
    PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ERROR,
    PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ALREADY_EXISTS
} pkrsrv_server_packet_signup_res_status_t;

typedef void (*opcode_handler_t)(
    pkrsrv_server_client_t* client,
    pkrsrv_server_packet_frame_header_t req_header
);

struct __attribute__((__packed__))
pkrsrv_server_packet_frame_header {
    uint32_t opcode;
    uint32_t length;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login {
    uint16_t id_token_length;
    uint16_t password_length;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login_res {
    uint8_t is_ok;
    uint8_t is_logined;
};
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login_res_account {
    uint16_t xmr_deposit_address_length;
    uint16_t id_token_length;
    uint16_t name_length;
    uint32_t avatar_length;
    uint64_t id;
    uint64_t balance;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_signup {
    uint16_t id_token_length;
    uint16_t password_length;
    uint16_t name_length;
    uint32_t avatar_length;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_signup_res {
    uint8_t is_ok;
    uint8_t is_logined;
    uint16_t status;
};
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_signup_res_account {
    uint16_t id_token_length;
    uint16_t name_length;
    uint32_t avatar_length;
    uint16_t xmr_deposit_address_length;
    uint64_t id;
    uint64_t balance;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_account {
    uint16_t xmr_deposit_address_length;
    uint16_t id_token_length;
    uint16_t name_length;
    uint32_t avatar_length;
    uint64_t id;
    uint64_t balance;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_enter {
    uint64_t table_id;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_enter_res {
    uint64_t table_id;
    uint8_t is_ok;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_leave {
    uint64_t table_id;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_leave_res {
    uint64_t table_id;
    uint8_t is_ok;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_join {
    uint64_t table_id;
    uint8_t position;
    uint64_t enterance_amount;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_join_res {
    uint64_t table_id;
    uint8_t is_ok;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_unjoin {
    uint64_t table_id;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_unjoin_res {
    uint64_t table_id;
    uint8_t is_ok;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_info {
    uint16_t name_length;
    uint16_t players_length;
    uint64_t table_id;
    uint16_t max_players;
    uint16_t action_timeout;
    uint64_t small_blind;
    uint64_t big_blind;
    uint64_t enterance_min;
    uint64_t enterance_max;
};

struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_info_player {
    uint16_t name_length;
    uint32_t avatar_length;
    uint64_t id;
    uint8_t position;
    uint8_t is_playing;
    uint8_t is_dealt;
    uint8_t is_allin;
    uint8_t is_folded;
    uint8_t is_left;
    uint64_t balance;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_state {
    uint16_t players_length;
    uint64_t table_id;
    uint16_t state;
    uint8_t is_playing;
    uint8_t cards[5];
    uint8_t position;
    uint8_t playing_position;
    uint8_t is_dealt;
    uint8_t hand_cards[2];
    uint64_t balance;
    uint64_t bet;
    uint64_t current_raise;
    uint64_t current_bet;
    uint64_t pot_amount;
    uint64_t starting_time;
};

struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_state_player {
    uint16_t name_length;
    uint64_t id;
    uint8_t position;
    uint8_t is_playing;
    uint8_t is_dealt;
    uint8_t is_allin;
    uint8_t is_folded;
    uint8_t is_left;
    uint64_t balance;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_action {
    uint64_t table_id;
    uint16_t action_kind;
    uint64_t amount;
};

/**
 * * direction: SERVER_TO_CLIENT
 * * Broadcasted to all players in the session by server on poker actions
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_action_reflection {
    uint64_t table_id;
    uint64_t account_id;
    uint16_t action_kind;
    uint64_t amount;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_end {
    uint64_t table_id;
    uint64_t winner_account_id;
    uint8_t scores_length;
    uint64_t earned_amount;
};

struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_end_score {
    uint64_t bet_diff_length;
    uint64_t account_id;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_restarted {
    uint64_t table_id;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_unjoined {
    uint64_t table_id;
    uint32_t reason;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_get_sessions {
    uint16_t offset;
    uint16_t length;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_sessions {
    uint16_t offset;
    uint16_t length;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_get_tables {
    uint16_t offset;
    uint16_t length;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_tables {
    uint16_t offset;
    uint16_t length;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_table {
    uint16_t name_length;
    uint64_t id;
    uint16_t max_players;
    uint16_t players_count;
    uint16_t watchers_count;
    uint64_t small_blind;
    uint64_t big_blind;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_server_info {
    uint64_t build_number;
    uint16_t version_length;
    uint16_t revision_length;
    uint16_t compiler_length;
};

/**
 * * direction: CLIENT_TO_SERVER
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_update_account {
    uint16_t name_length;
    uint32_t avatar_length;
};

/**
 * * direction: SERVER_TO_CLIENT
 */
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_update_account_res {
    uint8_t is_ok;
    uint8_t is_avatar_too_big;
};

// * <API event types>

typedef struct pkrsrv_server_packet_login {
    pkrsrv_string_t* id_token;
    pkrsrv_string_t* password;
} pkrsrv_server_packet_login_t;

typedef struct pkrsrv_server_packet_signup {
    pkrsrv_string_t* id_token;
    pkrsrv_string_t* password;
    pkrsrv_string_t* name;
    uint32_t avatar_length;
    pkrsrv_string_t* avatar;
} pkrsrv_server_packet_signup_t;

typedef struct pkrsrv_server_packet_enter {
    uint64_t table_id;
} pkrsrv_server_packet_enter_t;

typedef struct pkrsrv_server_packet_leave {
    uint64_t table_id;
} pkrsrv_server_packet_leave_t;

typedef struct pkrsrv_server_packet_join {
    uint64_t table_id;
    uint64_t enterance_amount;
    uint8_t position;
} pkrsrv_server_packet_join_t;

typedef struct pkrsrv_server_packet_unjoin {
    uint64_t table_id;
} pkrsrv_server_packet_unjoin_t;

typedef struct pkrsrv_server_packet_poker_action {
    uint64_t table_id;
    uint16_t action_kind;
    uint64_t amount;
} pkrsrv_server_packet_poker_action_t;

typedef struct pkrsrv_server_packet_get_tables {
    uint16_t offset;
    uint16_t length;
} pkrsrv_server_packet_get_tables_t;

typedef struct pkrsrv_server_packet_get_sessions {
    uint16_t offset;
    uint16_t length;
} pkrsrv_server_packet_get_sessions_t;

typedef struct pkrsrv_server_packet_json {
    pkrsrv_string_t* json;
} pkrsrv_server_packet_json_t;

typedef struct pkrsrv_server_packet_update_account {
    pkrsrv_string_t* name;
    pkrsrv_string_t* avatar;
} pkrsrv_server_packet_update_account_t;

// * </API event types>

typedef struct on_client_connected_params {
    void* owner;
    pkrsrv_server_client_t* client;
} on_client_connected_params_t;
typedef struct on_client_disconnected_params {
    void* owner;
    pkrsrv_server_client_t* client;
} on_client_disconnected_params_t;
typedef struct on_client_meowed_params {
    void* owner;
    pkrsrv_server_client_t* client;
} on_client_meowed_params_t;
typedef struct on_client_enter_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_enter_t enter;
} on_client_enter_params_t;
typedef struct on_client_leave_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_leave_t leave;
} on_client_leave_params_t;
typedef struct on_client_join_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_join_t join;
} on_client_join_params_t;
typedef struct on_client_unjoin_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_unjoin_t unjoin;
} on_client_unjoin_params_t;
typedef struct on_client_login_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_login_t login;
} on_client_login_params_t;
typedef struct on_client_signup_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_signup_t signup;
} on_client_signup_params_t;
typedef struct on_client_get_account_params {
    void* owner;
    pkrsrv_server_client_t* client;
} on_client_get_account_params_t;
typedef struct on_client_action_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_poker_action_t action;
} on_client_action_params_t;
typedef struct on_client_get_tables_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_get_tables_t get_tables;
} on_client_get_tables_params_t;
typedef struct on_client_get_sessions_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_get_sessions_t get_sessions;
} on_client_get_sessions_params_t;
typedef struct on_client_update_account_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_update_account_t update_account;
} on_client_update_account_params_t;
typedef struct on_client_json_params {
    void* owner;
    pkrsrv_server_client_t* client;
    pkrsrv_server_packet_json_t json;
} on_client_json_params_t;

/**
 * \implements pkrsrv_ref_counted
 * Server object
 */
struct pkrsrv_server {
    PKRSRV_REF_COUNTEDIFY();
    void *owner;
    pkrsrv_eventloop_t* eventloop;
    int port;
    char* bind_address;
    char* host_address;
    int max_clients;
    pkrsrv_server_clients_t* clients;
    SSL_CTX* ssl_ctx;
    void (*on_client_connected)(pkrsrv_eventloop_task_t* task);
    void (*on_client_disconnected)(pkrsrv_eventloop_task_t* task);
    void (*on_client_meowed)(pkrsrv_eventloop_task_t* task);
    void (*on_client_login)(pkrsrv_eventloop_task_t* task);
    void (*on_client_signup)(pkrsrv_eventloop_task_t* task);
    void (*on_client_get_account)(pkrsrv_eventloop_task_t* task);
    void (*on_client_enter)(pkrsrv_eventloop_task_t* task);
    void (*on_client_leave)(pkrsrv_eventloop_task_t* task);
    void (*on_client_join)(pkrsrv_eventloop_task_t* task);
    void (*on_client_unjoin)(pkrsrv_eventloop_task_t* task);
    void (*on_client_action)(pkrsrv_eventloop_task_t* task);
    void (*on_client_get_tables)(pkrsrv_eventloop_task_t* task);
    void (*on_client_get_sessions)(pkrsrv_eventloop_task_t* task);
    void (*on_client_update_account)(pkrsrv_eventloop_task_t* task);
    void (*on_client_json)(pkrsrv_eventloop_task_t* task);
    pthread_mutex_t mutex;
    pthread_t thread;
    bool is_running;
};

struct pkrsrv_server_clients {
    pkrsrv_server_client_t* prev;
    pkrsrv_server_client_t* next;
    pkrsrv_server_client_t* terminal;
    int length;
};

/**
 * \implements pkrsrv_ref_counted
 * Client object
 */
struct pkrsrv_server_client {
    PKRSRV_REF_COUNTEDIFY();
    ITEMIFY(pkrsrv_server_client_t*);
    pkrsrv_server_t* server;
    PGconn* pg_conn;
    SSL* ssl;
    bool is_alive;
    bool is_protocol_determined;
    bool is_websocket;
    pkrsrv_websocket_t websocket;
    pthread_mutex_t write_mutex;
    int socket;
    int server_socket;
    int address;
    void* owner;
};

int pkrsrv_server_ssl_read(SSL* ssl, void* buffer, ssize_t length);
int pkrsrv_server_ssl_write(SSL* ssl, void* buffer, ssize_t length);

int pkrsrv_server_net_read(pkrsrv_server_client_t* client, void* buffer, ssize_t length);
int pkrsrv_server_net_write(pkrsrv_server_client_t* client, void* buffer, ssize_t length);

struct pkrsrv_server_new_params {
    void* owner;
    int port;
    char* bind_address;
    pkrsrv_eventloop_t* eventloop;
    int max_clients;
    void (*on_client_connected)(pkrsrv_eventloop_task_t* task);
    void (*on_client_disconnected)(pkrsrv_eventloop_task_t* task);
    void (*on_client_meowed)(pkrsrv_eventloop_task_t* task);
    void (*on_client_login)(pkrsrv_eventloop_task_t* task);
    void (*on_client_signup)(pkrsrv_eventloop_task_t* task);
    void (*on_client_get_account)(pkrsrv_eventloop_task_t* task);
    void (*on_client_enter)(pkrsrv_eventloop_task_t* task);
    void (*on_client_leave)(pkrsrv_eventloop_task_t* task);
    void (*on_client_join)(pkrsrv_eventloop_task_t* task);
    void (*on_client_unjoin)(pkrsrv_eventloop_task_t* task);
    void (*on_client_action)(pkrsrv_eventloop_task_t* task);
    void (*on_client_get_tables)(pkrsrv_eventloop_task_t* task);
    void (*on_client_get_sessions)(pkrsrv_eventloop_task_t* task);
    void (*on_client_update_account)(pkrsrv_eventloop_task_t* task);
    void (*on_client_json)(pkrsrv_eventloop_task_t* task);
};
typedef struct pkrsrv_server_new_params pkrsrv_server_new_params_t;

/**
 * \memberof pkrsrv_server
 */
pkrsrv_server_t* pkrsrv_server_new(pkrsrv_server_new_params_t params);
/**
 * \memberof pkrsrv_server
 */
void pkrsrv_server_start(pkrsrv_server_t* server);
/**
 * \memberof pkrsrv_server
 */
void pkrsrv_server_listen(pkrsrv_server_t* server);
/**
 * \memberof pkrsrv_server
 */
void pkrsrv_server_free(pkrsrv_server_t* server);

pkrsrv_server_clients_t* pkrsrv_server_clients_new();
void pkrsrv_server_add_client(pkrsrv_server_t* server, pkrsrv_server_client_t* client);
void pkrsrv_server_add_client__ts(pkrsrv_server_t* server, pkrsrv_server_client_t* client);
void pkrsrv_server_remove_client(pkrsrv_server_t* server, pkrsrv_server_client_t* client);
void pkrsrv_server_remove_client__ts(pkrsrv_server_t* server, pkrsrv_server_client_t* client);
pkrsrv_server_client_t* pkrsrv_server_client_new(pkrsrv_server_t* server);
void pkrsrv_server_client_free(pkrsrv_server_client_t* client);

static void client_handler(pkrsrv_server_client_t* client);
static void receive_packet(pkrsrv_server_client_t* client);
static void receive_websocket_packet(pkrsrv_server_client_t* client);
static bool send_packet(pkrsrv_server_client_t* client, uint8_t* message, size_t message_size);
static void client_disconnected(pkrsrv_server_client_t* client);

bool pkrsrv_server_send_binary(pkrsrv_server_client_t* client, uint8_t* data, size_t size);

static void opcode_handler_nop(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_meow(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_ping(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_login(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_signup(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_get_account(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_enter(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_leave(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_join(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_unjoin(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_poker_action(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_get_tables(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_get_sessions(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_update_account(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);
static void opcode_handler_json(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header);

bool pkrsrv_server_send_over_capacity(pkrsrv_server_client_t* client);
bool pkrsrv_server_send_pong(pkrsrv_server_client_t* client);

typedef struct {
    pkrsrv_server_client_t* client;
    uint8_t is_ok;
    uint8_t is_logined;
    pkrsrv_account_t* account;
} pkrsrv_server_send_login_res_params_t;
bool pkrsrv_server_send_login_res(pkrsrv_server_send_login_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint8_t is_ok;
    uint8_t is_logined;
    uint16_t status;
    pkrsrv_account_t* account;
} pkrsrv_server_send_signup_res_params_t;
bool pkrsrv_server_send_signup_res(pkrsrv_server_send_signup_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    pkrsrv_account_t* account;
} pkrsrv_server_send_account_params_t;
bool pkrsrv_server_send_account(pkrsrv_server_send_account_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    uint8_t is_ok;
} pkrsrv_server_send_enter_res_params_t;
bool pkrsrv_server_send_enter_res(pkrsrv_server_send_enter_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    uint8_t is_ok;
} pkrsrv_server_send_leave_res_params_t;
bool pkrsrv_server_send_leave_res(pkrsrv_server_send_leave_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    uint8_t is_ok;
} pkrsrv_server_send_join_res_params_t;
bool pkrsrv_server_send_join_res(pkrsrv_server_send_join_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    uint8_t is_ok;
} pkrsrv_server_send_unjoin_res_params_t;
bool pkrsrv_server_send_unjoin_res(pkrsrv_server_send_unjoin_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    pkrsrv_poker_t* poker;
} pkrsrv_server_send_poker_info_params_t;
bool pkrsrv_server_send_poker_info(pkrsrv_server_send_poker_info_params_t params);
bool pkrsrv_server_send_poker_info_player(pkrsrv_server_client_t* p_client, pkrsrv_poker_player_t* p_player);

typedef struct {
    pkrsrv_server_client_t* client;
    pkrsrv_poker_t* poker;
    pkrsrv_poker_player_t* player;
} pkrsrv_server_send_poker_state_params_t;
bool pkrsrv_server_send_poker_state(pkrsrv_server_send_poker_state_params_t params);
bool pkrsrv_server_send_poker_state_player(pkrsrv_server_client_t* p_client, pkrsrv_poker_player_t* p_player);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    uint64_t account_id;
    uint16_t action_kind;
    uint64_t amount;
} pkrsrv_server_send_poker_action_reflection_params_t;
bool pkrsrv_server_send_poker_action_reflection(pkrsrv_server_send_poker_action_reflection_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    
    uint64_t winner_account_id;
    uint64_t earned_amount;

    pkrsrv_poker_t* poker;
} pkrsrv_server_send_poker_end_params_t;
bool pkrsrv_server_send_poker_end(pkrsrv_server_send_poker_end_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
} pkrsrv_server_send_poker_restarted_params_t;
bool pkrsrv_server_send_poker_restarted(pkrsrv_server_send_poker_restarted_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    uint32_t reason;
} pkrsrv_server_send_unjoined_params_t;
bool pkrsrv_server_send_unjoined(pkrsrv_server_send_unjoined_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint16_t offset;
    pkrsrv_table_list_t* list;
} pkrsrv_server_send_tables_params_t;
bool pkrsrv_server_send_tables(pkrsrv_server_send_tables_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint16_t offset;
    int pokers_length;
    pkrsrv_poker_t** pokers;
} pkrsrv_server_send_sessions_params_t;
bool pkrsrv_server_send_sessions(pkrsrv_server_send_sessions_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    bool is_ok;
    bool is_avatar_too_big;
} pkrsrv_server_send_update_account_res_params_t;
bool pkrsrv_server_send_update_account_res(pkrsrv_server_send_update_account_res_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    uint64_t build_number;
    pkrsrv_string_t* version;
    pkrsrv_string_t* revision;
    pkrsrv_string_t* compiler;
} pkrsrv_server_send_server_info_params_t;
bool pkrsrv_server_send_server_info(pkrsrv_server_send_server_info_params_t params);

typedef struct {
    pkrsrv_server_client_t* client;
    pkrsrv_string_t* json;
} pkrsrv_server_send_json_params_t;
bool pkrsrv_server_send_json(pkrsrv_server_send_json_params_t params);

static void sigabrt_handler();
static void sigpipe_handler(int signal);

/**
 * @}
 */