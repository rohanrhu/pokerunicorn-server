/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#include "../include/pkrsrv.h"
#include "../include/server.h"

#include "../include/db.h"
#include "../include/pkrsrv.h"
#include "../include/util.h"
#include "../include/string.h"
#include "../include/websocket.h"
#include "../include/sugar.h"
#include "../include/poker.h"
#include "../include/eventloop.h"

#define WRITE_OR_CLOSE(write_f, args...) { \
    int written = write_f(args); \
    if (written <= 0) { \
        client_disconnected(client); \
        retval = false; \
        goto RETURN; \
    } \
}

#define READ_OR_CLOSE(read_f, args...) { \
    int readed = read_f(args); \
    if (readed <= 0) { \
        client_disconnected(client); \
        goto RETURN; \
    } \
}

/**
 * * Indexes match with pkrsrv_server_opcode_t (enum PKRSRV_SERVER_OPCODE)
 * * NULL-terminated
 */
static opcode_handler_t OPCODE_HANDLERS[] = {
    opcode_handler_nop, // NOP
    opcode_handler_meow,
    opcode_handler_ping,
    opcode_handler_nop, // PONG
    opcode_handler_login,
    opcode_handler_nop, // LOGIN_RES
    opcode_handler_signup,
    opcode_handler_nop, // SIGNUP_RES
    opcode_handler_get_account,
    opcode_handler_nop, // ACCOUNT
    opcode_handler_enter,
    opcode_handler_nop, // ENTER_RES
    opcode_handler_leave,
    opcode_handler_nop, // LEAVE_RES
    opcode_handler_join,
    opcode_handler_unjoin,
    opcode_handler_nop, // UNJOIN_RES
    opcode_handler_nop, // JOIN_RES
    opcode_handler_nop, // POKER_INFO
    opcode_handler_nop, // POKER_STATE
    opcode_handler_poker_action,
    opcode_handler_nop, // POKER_ACTION_REFLECTION
    opcode_handler_nop, // POKER_END
    opcode_handler_nop, // POKER_RESTARTED
    opcode_handler_json, // JSON
    opcode_handler_nop, // UNJOINED
    opcode_handler_get_tables,
    opcode_handler_nop, // TABLES
    opcode_handler_get_sessions,
    opcode_handler_nop, // SESSIONS
    opcode_handler_update_account,
    opcode_handler_nop, // UPDATE_ACCOUNT_RES
    opcode_handler_nop, // SERVER_INFO
    NULL
};

int pkrsrv_server_ssl_read(SSL* ssl, void* buffer, ssize_t length) {
    ssize_t total_received = 0;

    int received;

    LOOP:

    received = SSL_read(ssl, ((unsigned char *) buffer) + total_received, length - total_received);

    if (received <= 0) {
        return 0;
    }

    total_received += received;

    if (total_received < length) {
        goto LOOP;
    }

    return total_received;
}

int pkrsrv_server_ssl_write(SSL* ssl, void* buffer, ssize_t length) {
    return SSL_write(ssl, buffer, length);
}

int pkrsrv_server_net_read(pkrsrv_server_client_t* client, void* buffer, ssize_t length) {
    if (client->is_websocket) {
        return pkrsrv_websocket_read_payload(&(client->websocket), client->ssl, buffer, length);
    } else {
        return pkrsrv_server_ssl_read(client->ssl, buffer, length);
    }
}

int pkrsrv_server_net_write(pkrsrv_server_client_t* client, void* buffer, ssize_t length) {
    return pkrsrv_server_ssl_write(client->ssl, buffer, length);
}

static void opcode_handler_nop(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {}

static void opcode_handler_meow(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    pkrsrv_util_verbose("Received Meow: (Client#%d)\n", client->socket);
    
    on_client_meowed_params_t* task_params = malloc(sizeof(on_client_meowed_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_meowed, task_params);
}

static void opcode_handler_ping(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    pkrsrv_util_verbose("Received Ping: (Client#%d)\n", client->socket);
    pkrsrv_server_send_pong(client);
}

static void opcode_handler_login(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    pkrsrv_util_verbose("Received Login: (Client#%d)\n", client->socket);

    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    pkrsrv_server_packet_login_t login_packet;

    pkrsrv_server_packet_frame_login_t login;

    READ_OR_CLOSE(pkrsrv_server_net_read, client, &login, sizeof(pkrsrv_server_packet_frame_login_t));

    char* id_token = malloc(login.id_token_length + 1);
    id_token[login.id_token_length] = '\0';
    char* password = malloc(login.password_length + 1);
    password[login.password_length] = '\0';

    if (login.id_token_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, id_token, login.id_token_length);
    }

    if (login.password_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, password, login.password_length);
    }

    pkrsrv_util_verbose("Received Login:\n");
    pkrsrv_util_verbose("\tEmail: %s\n", id_token);
    pkrsrv_util_verbose("\tPassword: %s\n", password);

    login_packet.id_token = pkrsrv_string_new_from_cstr(id_token, login.id_token_length);
    login_packet.id_token->is_alloc_str = true;
    login_packet.password = pkrsrv_string_new_from_cstr(password, login.password_length);
    login_packet.password->is_alloc_str = true;

    on_client_login_params_t* task_params = malloc(sizeof(on_client_login_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->login = login_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_login, task_params);

    goto SUCCESS;

    RETURN:

    free(id_token);
    free(password);

    SUCCESS:
    return;
}

static void opcode_handler_signup(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    pkrsrv_util_verbose("Received Signup: (Client#%d)\n", client->socket);

    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    pkrsrv_server_packet_signup_t signup_packet;

    pkrsrv_server_packet_frame_signup_t signup;

    READ_OR_CLOSE(pkrsrv_server_net_read, client, &signup, sizeof(pkrsrv_server_packet_frame_signup_t));

    char* id_token = malloc(signup.id_token_length + 1);
    id_token[signup.id_token_length] = '\0';
    char* password = malloc(signup.password_length + 1);
    password[signup.password_length] = '\0';
    char* name = malloc(signup.name_length + 1);
    name[signup.name_length] = '\0';

    char* avatar = NULL;
    if (signup.avatar_length > 0) {
        avatar = malloc(signup.avatar_length + 1);
    }

    if (signup.id_token_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, id_token, signup.id_token_length);
    }

    if (signup.password_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, password, signup.password_length);
    }

    if (signup.name_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, name, signup.name_length);
    }
    
    if (signup.avatar_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, avatar, signup.avatar_length);
    }

    pkrsrv_util_verbose("Received signup:\n");
    pkrsrv_util_verbose("\tEmail: %s\n", id_token);
    pkrsrv_util_verbose("\tPassword: %s\n", password);
    pkrsrv_util_verbose("\tName: %s\n", name);

    signup_packet.id_token = pkrsrv_string_new_from_cstr(id_token, signup.id_token_length);
    
    signup_packet.password = pkrsrv_string_new_from_cstr(password, signup.password_length);
    
    signup_packet.name = pkrsrv_string_new_from_cstr(name, signup.name_length);

    if (avatar) {
        signup_packet.avatar = pkrsrv_string_new_from_cstr(avatar, signup.avatar_length);
    } else {
        signup_packet.avatar = pkrsrv_string_new();
    }
    
    on_client_signup_params_t* task_params = malloc(sizeof(on_client_signup_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->signup = signup_packet;
    
    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_signup, task_params);

    RETURN:
    return;
}

static void opcode_handler_get_account(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    on_client_get_account_params_t* task_params = malloc(sizeof(on_client_get_account_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_get_account, task_params);
}

static void opcode_handler_enter(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    
    pkrsrv_server_packet_frame_enter_t enter;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &enter, sizeof(pkrsrv_server_packet_frame_enter_t));

    pkrsrv_server_packet_enter_t enter_packet;
    enter_packet.table_id = enter.table_id;

    on_client_enter_params_t* task_params = malloc(sizeof(on_client_enter_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->enter = enter_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_enter, task_params);

    RETURN:
    return;
}

static void opcode_handler_leave(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    
    pkrsrv_server_packet_frame_leave_t leave;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &leave, sizeof(pkrsrv_server_packet_frame_leave_t));

    pkrsrv_server_packet_leave_t leave_packet;
    leave_packet.table_id = leave.table_id;

    on_client_leave_params_t* task_params = malloc(sizeof(on_client_leave_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->leave = leave_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_leave, task_params);

    RETURN:
    return;
}

static void opcode_handler_join(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    
    pkrsrv_server_packet_frame_join_t join;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &join, sizeof(pkrsrv_server_packet_frame_join_t));

    pkrsrv_server_packet_join_t join_packet;
    join_packet.table_id = join.table_id;
    join_packet.enterance_amount = join.enterance_amount;
    join_packet.position = join.position;

    on_client_join_params_t* task_params = malloc(sizeof(on_client_join_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->join = join_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_join, task_params);

    RETURN:
    return;
}

static void opcode_handler_unjoin(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    
    pkrsrv_server_packet_frame_unjoin_t unjoin;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &unjoin, sizeof(pkrsrv_server_packet_frame_unjoin_t));

    pkrsrv_server_packet_unjoin_t unjoin_packet;
    unjoin_packet.table_id = unjoin.table_id;

    on_client_unjoin_params_t* task_params = malloc(sizeof(on_client_unjoin_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->unjoin = unjoin_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_unjoin, task_params);

    RETURN:
    return;
}

static void opcode_handler_poker_action(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    int result;
    
    pkrsrv_server_packet_frame_poker_action_t action_frame;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &action_frame, sizeof(pkrsrv_server_packet_frame_poker_action_t));

    PKRSRV_UTIL_ASSERT(client->server->on_client_action);

    pkrsrv_server_packet_poker_action_t action_packet;
    action_packet.table_id = action_frame.table_id;
    action_packet.action_kind = action_frame.action_kind;
    action_packet.amount = action_frame.amount;

    on_client_action_params_t* task_params = malloc(sizeof(on_client_action_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->action = action_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_action, task_params);

    RETURN:
    return;
}

static void opcode_handler_get_tables(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    int result;
    
    pkrsrv_server_packet_frame_get_tables_t get_tables_frame;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &get_tables_frame, sizeof(pkrsrv_server_packet_frame_get_tables_t));

    PKRSRV_UTIL_ASSERT(client->server->on_client_get_tables);

    pkrsrv_server_packet_get_tables_t get_tables_packet;
    get_tables_packet.offset = get_tables_frame.offset;
    get_tables_packet.length = get_tables_frame.length;

    on_client_get_tables_params_t* task_params = malloc(sizeof(on_client_get_tables_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->get_tables = get_tables_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_get_tables, task_params);

    RETURN:
    return;
}

static void opcode_handler_get_sessions(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    int result;
    
    pkrsrv_server_packet_frame_get_tables_t get_sessions_frame;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &get_sessions_frame, sizeof(pkrsrv_server_packet_frame_get_tables_t));

    PKRSRV_UTIL_ASSERT(client->server->on_client_get_sessions);

    pkrsrv_server_packet_get_sessions_t get_sessions_packet;
    get_sessions_packet.offset = get_sessions_frame.offset;
    get_sessions_packet.length = get_sessions_frame.length;

    on_client_get_sessions_params_t* task_params = malloc(sizeof(on_client_get_sessions_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->get_sessions = get_sessions_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_get_sessions, task_params);

    RETURN:
    return;
}

static void opcode_handler_update_account(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    int result;
    
    pkrsrv_server_packet_frame_update_account_t update_account;
    READ_OR_CLOSE(pkrsrv_server_net_read, client, &update_account, sizeof(pkrsrv_server_packet_frame_update_account_t));

    char* name = malloc(update_account.name_length + 1);
    name[update_account.name_length] = '\0';

    char* avatar = NULL;
    if (update_account.avatar_length > 0) {
        avatar = malloc(update_account.avatar_length + 1);
    }

    if (update_account.name_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, name, update_account.name_length);
    }
    
    if (update_account.avatar_length > 0) {
        READ_OR_CLOSE(pkrsrv_server_net_read, client, avatar, update_account.avatar_length);
    }

    pkrsrv_util_verbose("Received Update Account:\n");
    pkrsrv_util_verbose("\tName: %:\n", name);
    pkrsrv_util_verbose("\tAvatar: [%d]:\n", update_account.avatar_length);

    PKRSRV_UTIL_ASSERT(client->server->on_client_update_account);

    pkrsrv_server_packet_update_account_t update_account_packet;
    
    update_account_packet.name = pkrsrv_string_new_from_cstr(name, update_account.name_length);

    if (avatar) {
        update_account_packet.avatar = pkrsrv_string_new_from_cstr(avatar, update_account.avatar_length);
    } else {
        update_account_packet.avatar = pkrsrv_string_new();
    }
    
    on_client_update_account_params_t* task_params = malloc(sizeof(on_client_update_account_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->update_account = update_account_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_update_account, task_params);

    RETURN:
    return;
}

static void opcode_handler_json(pkrsrv_server_client_t* client, pkrsrv_server_packet_frame_header_t req_header) {
    int result;
    
    pkrsrv_server_packet_json_t json_packet;
    json_packet.json = pkrsrv_string_new__n(req_header.length);
    PKRSRV_REF_COUNTED_USE(json_packet.json);

    READ_OR_CLOSE(pkrsrv_server_net_read, client, &(json_packet.json->value), sizeof(pkrsrv_server_packet_frame_poker_action_t));

    on_client_json_params_t* task_params = malloc(sizeof(on_client_json_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;
    task_params->json = json_packet;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_json, task_params);

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(json_packet.json);

    return;
}

bool pkrsrv_server_send_over_capacity(pkrsrv_server_client_t* client) {
    bool retval = true;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }

    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_OVER_CAPACITY;
    
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));
    
    return retval;
}

bool pkrsrv_server_send_pong(pkrsrv_server_client_t* client) {
    bool retval = true;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }

    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    
    header.opcode = PKRSRV_SERVER_OPCODE_PONG;
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));
    
    return retval;
}

bool pkrsrv_server_send_login_res(pkrsrv_server_send_login_res_params_t params) {
    bool retval = true;
    
    if (params.account) {
        PKRSRV_REF_COUNTED_USE(params.account);
    }
    
    pkrsrv_server_client_t* client = params.client;

    pthread_mutex_lock(&(client->write_mutex));

    pkrsrv_util_verbose("Sending Login Res: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_login_res_t);
        if (params.is_ok && params.is_logined) {
            length += sizeof(pkrsrv_server_packet_frame_login_res_account_t);
            length += params.account->id_token->length;
            length += params.account->name->length;
            length += params.account->avatar->length;
            length += params.account->xmr_deposit_address->length;
        }
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_LOGIN_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_login_res_t login_res;
    login_res.is_ok = params.is_ok;
    login_res.is_logined = params.is_logined;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &login_res, sizeof(pkrsrv_server_packet_frame_login_res_t));

    if (params.is_ok && params.is_logined) {
        pkrsrv_server_packet_frame_login_res_account_t login_res_account;
        login_res_account.xmr_deposit_address_length = params.account->xmr_deposit_address->length;
        login_res_account.id = params.account->id;
        login_res_account.id_token_length = params.account->id_token->length;
        login_res_account.name_length = params.account->name->length;
        login_res_account.avatar_length = params.account->avatar->length;
        login_res_account.balance = params.account->balance;

        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &login_res_account, sizeof(pkrsrv_server_packet_frame_login_res_account_t));
        if (login_res_account.xmr_deposit_address_length) {
            WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->xmr_deposit_address->value, login_res_account.xmr_deposit_address_length);
        }
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->id_token->value, login_res_account.id_token_length);
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->name->value, login_res_account.name_length);
        if (login_res_account.avatar_length) {
            WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->avatar->value, login_res_account.avatar_length);
        }
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    if (params.account) {
        PKRSRV_REF_COUNTED_LEAVE(params.account);
    }
    
    return retval;
}

bool pkrsrv_server_send_signup_res(pkrsrv_server_send_signup_res_params_t params) {
    bool retval = true;
    
    if (params.account) {
        PKRSRV_REF_COUNTED_USE(params.account);
    }
    
    pkrsrv_server_client_t* client = params.client;

    pthread_mutex_lock(&(client->write_mutex));

    pkrsrv_util_verbose("Sending Signup Res: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_signup_res_t);
        if (params.is_ok && params.is_logined) {
            length += sizeof(pkrsrv_server_packet_frame_signup_res_account_t);
            length += params.account->id_token->length;
            length += params.account->name->length;
            length += params.account->avatar->length;
            length += params.account->xmr_deposit_address->length;
        }
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_SIGNUP_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_signup_res_t signup_res;
    signup_res.is_ok = params.is_ok;
    signup_res.is_logined = params.is_logined;
    signup_res.status = params.status;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &signup_res, sizeof(pkrsrv_server_packet_frame_signup_res_t));

    if (params.is_ok && params.is_logined) {
        pkrsrv_server_packet_frame_signup_res_account_t signup_res_account;
        signup_res_account.id = params.account->id;
        signup_res_account.id_token_length = params.account->id_token->length;
        signup_res_account.name_length = params.account->name->length;
        signup_res_account.avatar_length = params.account->avatar->length;
        signup_res_account.balance = params.account->balance;
        signup_res_account.xmr_deposit_address_length = params.account->xmr_deposit_address->length;

        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &signup_res_account, sizeof(pkrsrv_server_packet_frame_signup_res_account_t));
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->id_token->value, signup_res_account.id_token_length);
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->name->value, signup_res_account.name_length);
        if (signup_res_account.avatar_length) {
            WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->avatar->value, signup_res_account.avatar_length);
        }
        if (signup_res_account.xmr_deposit_address_length) {
            WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->xmr_deposit_address->value, signup_res_account.xmr_deposit_address_length);
        }
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    if (params.account) {
        PKRSRV_REF_COUNTED_LEAVE(params.account);
    }

    return retval;
}

bool pkrsrv_server_send_account(pkrsrv_server_send_account_params_t params) {
    bool retval = true;
    
    PKRSRV_REF_COUNTED_USE(params.client);
    PKRSRV_REF_COUNTED_USE(params.account);
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    pkrsrv_util_verbose("Sending Account: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_account_t);
        length += params.account->xmr_deposit_address->length;
        length += params.account->id_token->length;
        length += params.account->name->length;
        length += params.account->avatar->length;
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_ACCOUNT;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_account_t account_frame;
    account_frame.xmr_deposit_address_length = params.account->xmr_deposit_address->length;
    account_frame.id_token_length = params.account->id_token->length;
    account_frame.name_length = params.account->name->length;
    account_frame.avatar_length = params.account->avatar->length;
    account_frame.id = params.account->id;
    account_frame.balance = params.account->balance;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &account_frame, sizeof(pkrsrv_server_packet_frame_account_t));
    if (account_frame.xmr_deposit_address_length) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->xmr_deposit_address->value, account_frame.xmr_deposit_address_length);
    }
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->id_token->value, account_frame.id_token_length);
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->name->value, account_frame.name_length);
    if (account_frame.avatar_length) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.account->avatar->value, account_frame.avatar_length);
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    PKRSRV_REF_COUNTED_LEAVE(params.client);
    PKRSRV_REF_COUNTED_LEAVE(params.account);

    return retval;
}

bool pkrsrv_server_send_enter_res(pkrsrv_server_send_enter_res_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));
    
    pkrsrv_util_verbose("Sending Enter Res: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_enter_res_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_ENTER_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_enter_res_t enter_res;
    enter_res.is_ok = params.is_ok;
    enter_res.table_id = params.table_id;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &enter_res, sizeof(pkrsrv_server_packet_frame_enter_res_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_leave_res(pkrsrv_server_send_leave_res_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));
    
    pkrsrv_util_verbose("Sending Leave Res: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_leave_res_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_LEAVE_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_leave_res_t leave_res;
    leave_res.table_id = params.table_id;
    leave_res.is_ok = params.is_ok;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &leave_res, sizeof(pkrsrv_server_packet_frame_leave_res_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_join_res(pkrsrv_server_send_join_res_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));
    
    pkrsrv_util_verbose("Sending Join Res: (Client: %d)\n", params.client->socket);
    
    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_join_res_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_JOIN_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_join_res_t join_res;
    join_res.is_ok = params.is_ok;
    join_res.table_id = params.table_id;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &join_res, sizeof(pkrsrv_server_packet_frame_join_res_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_unjoin_res(pkrsrv_server_send_unjoin_res_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));
    
    pkrsrv_util_verbose("Sending Unjoin Res: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_unjoin_res_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_UNJOIN_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_unjoin_res_t unjoin_res;
    unjoin_res.table_id = params.table_id;
    unjoin_res.is_ok = params.is_ok;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &unjoin_res, sizeof(pkrsrv_server_packet_frame_unjoin_res_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_poker_info(pkrsrv_server_send_poker_info_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_poker_info_t);
        length += params.poker->table->name->length;

        LIST_FOREACH(params.poker->players, player)
            length += sizeof(pkrsrv_server_packet_frame_poker_info_player_t);
            length += player->account->name->length;
            length += player->account->avatar->length;
        END_FOREACH
        
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Poker Info: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_POKER_INFO;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_poker_info_t poker_info;
    poker_info.name_length = params.poker->table->name->length;
    poker_info.players_length = params.poker->players->length;
    poker_info.table_id = params.poker->table->id.scalar;
    poker_info.max_players = params.poker->table->max_players;
    poker_info.action_timeout = params.poker->table->action_timeout;
    poker_info.small_blind = params.poker->table->small_blind;
    poker_info.big_blind = params.poker->table->big_blind;
    poker_info.enterance_min = params.poker->table->enterance_min;
    poker_info.enterance_max = params.poker->table->enterance_max;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &poker_info, sizeof(pkrsrv_server_packet_frame_poker_info_t));
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.poker->table->name->value, poker_info.name_length);

    LIST_FOREACH(params.poker->players, player)
        pkrsrv_server_send_poker_info_player(client, player);
    END_FOREACH

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_poker_info_player(pkrsrv_server_client_t* client, pkrsrv_poker_player_t* p_player) {
    bool retval = true;
    
    pkrsrv_server_packet_frame_poker_info_player_t player;
    player.name_length = p_player->account->name->length;
    player.avatar_length = p_player->account->avatar->length;
    player.id = p_player->account->id;
    player.position = p_player->position;
    player.is_playing = p_player->is_playing;
    player.is_dealt = p_player->is_dealt;
    player.is_allin = p_player->is_allin;
    player.is_folded = p_player->is_folded;
    player.is_left = p_player->is_left;
    player.balance = p_player->balance;
    
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &player, sizeof(pkrsrv_server_packet_frame_poker_info_player_t));
    if (player.name_length > 0) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, p_player->account->name->value, player.name_length);
    }
    if (player.avatar_length > 0) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, p_player->account->avatar->value, player.avatar_length);
    }

    RETURN:

    return retval;
}

bool pkrsrv_server_send_poker_state(pkrsrv_server_send_poker_state_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_poker_state_t);

        LIST_FOREACH(params.poker->players, player)
            length += sizeof(pkrsrv_server_packet_frame_poker_state_player_t);
            length += player->account->name->length;
        END_FOREACH
        
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Poker State: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_POKER_STATE;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_poker_state_t poker_state;
    poker_state.players_length = params.poker->players->length;
    poker_state.table_id = params.poker->table->id.scalar;
    poker_state.state = params.poker->state;

    poker_state.playing_position = params.poker->playing_position;

    poker_state.cards[0] = (0 | (params.poker->hand_flop.cards[0].index << 4)) |
                           (params.poker->hand_flop.cards[0].kind);
    poker_state.cards[1] = (0 | (params.poker->hand_flop.cards[1].index << 4)) |
                           (params.poker->hand_flop.cards[1].kind);
    poker_state.cards[2] = (0 | (params.poker->hand_flop.cards[2].index << 4)) |
                           (params.poker->hand_flop.cards[2].kind);
    
    poker_state.cards[3] = (0 | (params.poker->hand_turn.cards[0].index << 4)) |
                           (params.poker->hand_turn.cards[0].kind);

    poker_state.cards[4] = (0 | (params.poker->hand_river.cards[0].index << 4)) |
                           (params.poker->hand_river.cards[0].kind);

    poker_state.current_raise = params.poker->current_raise;
    poker_state.current_bet = params.poker->current_bet;
    poker_state.pot_amount = params.poker->pot_amount;
    poker_state.starting_time = params.poker->starting_time;

    if (params.player) {
        poker_state.is_playing = params.player->is_playing;
        poker_state.position = params.player->position;
        poker_state.balance = params.player->balance;
        poker_state.bet = params.player->bet;
        poker_state.is_dealt = (unsigned char) (params.player->is_dealt > 0);

        if (params.player->is_dealt) {
            poker_state.hand_cards[0] = (0 | (params.player->cards[0].index << 4)) |
                                        (params.player->cards[0].kind);
            poker_state.hand_cards[1] = (0 | (params.player->cards[1].index << 4)) |
                                        (params.player->cards[1].kind);
        } else {
            poker_state.hand_cards[0] = 0;
            poker_state.hand_cards[1] = 0;
        }
    } else {
        poker_state.position = 0;
        poker_state.is_playing = false;
        poker_state.is_dealt = false;
        poker_state.balance = 0;
        poker_state.bet = 0;
        poker_state.hand_cards[0] = 0;
        poker_state.hand_cards[1] = 0;
    }

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &poker_state, sizeof(pkrsrv_server_packet_frame_poker_state_t));

    LIST_FOREACH(params.poker->players, player)
        pkrsrv_server_send_poker_state_player(client, player);
    END_FOREACH

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_poker_state_player(pkrsrv_server_client_t* client, pkrsrv_poker_player_t* p_player) {
    bool retval = true;
    
    pkrsrv_server_packet_frame_poker_state_player_t player;
    player.id = p_player->account->id;
    player.name_length = p_player->account->name->length;
    player.position = p_player->position;
    player.is_playing = p_player->is_playing;
    player.is_dealt = p_player->is_dealt;
    player.is_folded = p_player->is_folded;
    player.is_allin = p_player->is_allin;
    player.is_left = p_player->is_left;
    player.balance = p_player->balance;
    
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &player, sizeof(pkrsrv_server_packet_frame_poker_state_player_t));
    if (player.name_length > 0) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, p_player->account->name->value, player.name_length);
    }

    RETURN:
    
    return retval;
}

bool pkrsrv_server_send_poker_action_reflection(pkrsrv_server_send_poker_action_reflection_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_poker_action_reflection_t);
        
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Action Reflection: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_POKER_ACTION_REFLECTION;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_poker_action_reflection_t action_reflection;
    action_reflection.table_id = params.table_id;
    action_reflection.account_id = params.account_id;
    action_reflection.action_kind = params.action_kind;
    action_reflection.amount = params.amount;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &action_reflection, sizeof(pkrsrv_server_packet_frame_poker_action_reflection_t));

    RETURN:
    
    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_poker_end(pkrsrv_server_send_poker_end_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_poker_end_t);
        length += sizeof(pkrsrv_server_packet_frame_poker_end_score_t) * params.poker->scores_length;
        
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Poker End: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_POKER_END;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_poker_end_t end;
    end.table_id = params.table_id;
    end.winner_account_id = params.winner_account_id;
    end.scores_length = params.poker->scores_length;
    end.earned_amount = params.earned_amount;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &end, sizeof(pkrsrv_server_packet_frame_poker_end_t));

    for (int i=0; i < end.scores_length; i++) {
        pkrsrv_poker_player_score_t* score = &(params.poker->scores[i]);

        pkrsrv_server_packet_frame_poker_end_score_t score_frame;
        score_frame.bet_diff_length = score->bet_diff;
        score_frame.account_id = score->player->account->id;

        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &score_frame, sizeof(pkrsrv_server_packet_frame_poker_end_score_t));
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_poker_restarted(pkrsrv_server_send_poker_restarted_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_poker_restarted_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Poker End: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_POKER_RESTARTED;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_poker_restarted_t restarted;
    restarted.table_id = params.table_id;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &restarted, sizeof(pkrsrv_server_packet_frame_poker_restarted_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_unjoined(pkrsrv_server_send_unjoined_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_unjoined_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Unjoined: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_UNJOINED;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_unjoined_t unjoined;
    unjoined.table_id = params.table_id;
    unjoined.reason = 0;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &unjoined, sizeof(pkrsrv_server_packet_frame_unjoined_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_tables(pkrsrv_server_send_tables_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_tables_t);
        
        for (int i=0; i < params.list->length; i++) {
            pkrsrv_table_t* table = params.list->tables[i];
            length += sizeof(pkrsrv_server_packet_frame_table_t);
            length += table->name->length;
        }
        
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Tables: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_TABLES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_tables_t tables;
    tables.offset = params.offset;
    tables.length = params.list->length;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &tables, sizeof(pkrsrv_server_packet_frame_tables_t));

    for (int i=0; i < params.list->length; i++) {
        pkrsrv_table_t* table = params.list->tables[i];

        pkrsrv_util_verbose("Sending Table#%llu: (Client: %d)\n", table->id.scalar, client->socket);

        pkrsrv_server_packet_frame_table_t table_frame;
        table_frame.name_length = table->name->length;
        table_frame.id = table->id.scalar;
        table_frame.max_players = table->max_players;
        table_frame.players_count = table->players_count;
        table_frame.watchers_count = table->watchers_count;
        table_frame.small_blind = table->small_blind;
        table_frame.big_blind = table->big_blind;

        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &table_frame, sizeof(pkrsrv_server_packet_frame_table_t));
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, table->name->value, table->name->length);
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_sessions(pkrsrv_server_send_sessions_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_sessions_t);

        for (int i = 0; i < params.pokers_length; i++) {
            pkrsrv_poker_t* poker = params.pokers[i];

            length += sizeof(pkrsrv_server_packet_frame_poker_info_t);
            length += poker->table->name->length;

            LIST_FOREACH(poker->players, player)
                length += sizeof(pkrsrv_server_packet_frame_poker_info_player_t);
                length += player->account->name->length;
                length += player->account->avatar->length;
            END_FOREACH
        }
        
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_util_verbose("Sending Sessions: (Client: %d)\n", client->socket);
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_SESSIONS;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_tables_t tables_frame;
    tables_frame.offset = params.offset;
    tables_frame.length = params.pokers_length;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &tables_frame, sizeof(pkrsrv_server_packet_frame_sessions_t));
    
    for (int i = 0; i < params.pokers_length; i++) {
        pkrsrv_poker_t* poker = params.pokers[i];
        
        pkrsrv_server_packet_frame_poker_info_t poker_info;
        poker_info.name_length = poker->table->name->length;
        poker_info.table_id = poker->table->id.scalar;
        poker_info.players_length = poker->players->length;
        poker_info.max_players = poker->table->max_players;
        poker_info.action_timeout = poker->table->action_timeout;
        poker_info.small_blind = poker->table->small_blind;
        poker_info.big_blind = poker->table->big_blind;
        poker_info.enterance_min = poker->table->enterance_min;
        poker_info.enterance_max = poker->table->enterance_max;

        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &poker_info, sizeof(pkrsrv_server_packet_frame_poker_info_t));
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, poker->table->name->value, poker->table->name->length);

        LIST_FOREACH(poker->players, player)
            pkrsrv_server_send_poker_info_player(client, player);
        END_FOREACH
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_update_account_res(pkrsrv_server_send_update_account_res_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));
    
    pkrsrv_util_verbose("Sending Update Account Res: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_update_account_res_t);
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT_RES;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_update_account_res_t update_account_res;
    update_account_res.is_ok = (uint8_t) params.is_ok;
    update_account_res.is_avatar_too_big = (uint8_t) params.is_avatar_too_big;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &update_account_res, sizeof(pkrsrv_server_packet_frame_update_account_res_t));

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_server_info(pkrsrv_server_send_server_info_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;

    pthread_mutex_lock(&(client->write_mutex));

    pkrsrv_util_verbose("Sending Server Info: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += sizeof(pkrsrv_server_packet_frame_server_info_t);
        length += params.version->length;
        length += params.revision->length;
        length += params.compiler->length;
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_SERVER_INFO;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));

    pkrsrv_server_packet_frame_server_info_t server_info;
    server_info.build_number = params.build_number;
    server_info.version_length = params.version->length;
    server_info.revision_length = params.revision->length;
    server_info.compiler_length = params.compiler->length;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &server_info, sizeof(pkrsrv_server_packet_frame_server_info_t));
    if (params.version->length) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.version->value, params.version->length);
    }
    if (params.revision->length) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.revision->value, params.revision->length);
    }
    if (params.compiler->length) {
        WRITE_OR_CLOSE(pkrsrv_server_net_write, client, params.compiler->value, params.compiler->length);
    }

    RETURN:

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

bool pkrsrv_server_send_json(pkrsrv_server_send_json_params_t params) {
    bool retval = true;
    
    pkrsrv_server_client_t* client = params.client;
    
    pthread_mutex_lock(&(client->write_mutex));
    
    PKRSRV_REF_COUNTED_USE(params.json);
    
    pkrsrv_util_verbose("Sending Poker End: (Client: %d)\n", client->socket);

    if (client->is_websocket) {
        ssize_t length = 0;
        length += sizeof(pkrsrv_server_packet_frame_header_t);
        length += params.json->length;
        WRITE_OR_CLOSE(pkrsrv_websocket_send_header, &(client->websocket), client->ssl, length);
    }
    
    pkrsrv_server_packet_frame_header_t header = {0};
    header.opcode = PKRSRV_SERVER_OPCODE_JSON;
    header.length = params.json->length;

    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &header, sizeof(pkrsrv_server_packet_frame_header_t));
    WRITE_OR_CLOSE(pkrsrv_server_net_write, client, &params.json->value, params.json->length);
    
    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(params.json);

    pthread_mutex_unlock(&(client->write_mutex));

    return retval;
}

pkrsrv_server_t* pkrsrv_server_new(pkrsrv_server_new_params_t params) {
    pkrsrv_server_t* server = malloc(sizeof(pkrsrv_server_t));
    PKRSRV_REF_COUNTED_INIT(server, pkrsrv_server_free);
    server->owner = params.owner;
    server->port = params.port;
    server->bind_address = params.bind_address;
    server->host_address = NULL;
    server->max_clients = params.max_clients;
    server->is_running = false;
    server->clients = pkrsrv_server_clients_new();
    
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&(server->mutex), &mutex_attr);

    server->eventloop = params.eventloop;
    PKRSRV_REF_COUNTED_USE(params.eventloop);

    server->on_client_connected = params.on_client_connected;
    server->on_client_disconnected = params.on_client_disconnected;
    server->on_client_meowed = params.on_client_meowed;
    server->on_client_enter = params.on_client_enter;
    server->on_client_leave = params.on_client_leave;
    server->on_client_join = params.on_client_join;
    server->on_client_unjoin = params.on_client_unjoin;
    server->on_client_login = params.on_client_login;
    server->on_client_get_account = params.on_client_get_account;
    server->on_client_signup = params.on_client_signup;
    server->on_client_action = params.on_client_action;
    server->on_client_get_tables = params.on_client_get_tables;
    server->on_client_get_sessions = params.on_client_get_sessions;
    server->on_client_update_account = params.on_client_update_account;

    server->ssl_ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_set_mode(server->ssl_ctx, SSL_MODE_AUTO_RETRY);
    if (!server->ssl_ctx) {
        perror("Failed to create SSL context!\n");
        exit(1);
    }

    SSL_CTX_set_min_proto_version(server->ssl_ctx, TLS1_1_VERSION);

    if (SSL_CTX_use_PrivateKey_file(server->ssl_ctx, pkrsrv_ssl_key_file, SSL_FILETYPE_PEM) != 1) {
        perror("Failed to load SSL key!\n");
        exit(1);
    }

    if (SSL_CTX_use_certificate_file(server->ssl_ctx, pkrsrv_ssl_cert_file, SSL_FILETYPE_PEM) != 1) {
        perror("Failed to load SSL certificate!\n");
        exit(1);
    }

    return server;
}

void pkrsrv_server_add_client(pkrsrv_server_t* server, pkrsrv_server_client_t* client) {
    PKRSRV_REF_COUNTED_USE(client);
    LIST_APPEND(server->clients, client);
}

void pkrsrv_server_add_client__ts(pkrsrv_server_t* server, pkrsrv_server_client_t* client) {
    PKRSRV_REF_COUNTED_USE(client);
    pthread_mutex_lock(&server->mutex);
    LIST_APPEND(server->clients, client);
    pthread_mutex_unlock(&server->mutex);
}

void pkrsrv_server_remove_client(pkrsrv_server_t* server, pkrsrv_server_client_t* client) {
    PKRSRV_REF_COUNTED_LEAVE(client);
    LIST_REMOVE(server->clients, client);
}

void pkrsrv_server_remove_client__ts(pkrsrv_server_t* server, pkrsrv_server_client_t* client) {
    PKRSRV_REF_COUNTED_LEAVE(client);
    pthread_mutex_lock(&server->mutex);
    LIST_REMOVE(server->clients, client);
    pthread_mutex_unlock(&server->mutex);
}

void pkrsrv_server_start(pkrsrv_server_t* server) {
    pthread_create(&server->thread, NULL, (void *) &pkrsrv_server_listen, (void *) server);
    pthread_detach(server->thread);

    server->is_running = true;
}

void pkrsrv_server_listen(pkrsrv_server_t* server) {
    struct sigaction sa = {0};
    sa.sa_handler = sigabrt_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGABRT, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);
    
    int server_socket;
    int client_socket;
    int client_addr_len;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Socket error");
        exit(1);
    }

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(server->port);

    if (server->bind_address) {
        serv_addr.sin_addr.s_addr = inet_addr(server->bind_address);
    }

    if (bind(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind error.");
        exit(1);
    }

    listen(server_socket, 10);
    client_addr_len = sizeof(cli_addr);

    pkrsrv_util_verbose("Server is listening from 0.0.0.0:%d\n", server->port);

    pthread_t client_thread;

    char str_addr[INET6_ADDRSTRLEN+1];

    pkrsrv_server_client_t* client = NULL;
    pkrsrv_server_client_t* current_client = NULL;

    for (;;) {
        client_socket = accept(server_socket, (struct sockaddr *) &cli_addr, (socklen_t *) (&client_addr_len));
        if (client_socket < 0) {
            printf("[Server] Accept error. Gonna retry in 2 seconds... (errno: %d)\n", errno);
            pkrsrv_util_msleep(2000);
            continue;
        }

        inet_ntop(AF_INET, (void*)&cli_addr.sin_addr, str_addr, INET_ADDRSTRLEN);
        
        pkrsrv_util_verbose("New client: %s\n", str_addr);

        SSL* ssl = SSL_new(server->ssl_ctx);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        SSL_set_fd(ssl, client_socket);

        if (SSL_accept(ssl) <= 0) {
            pkrsrv_util_verbose("SSL handshake error! (client#%s)\n", str_addr);
            
            ERR_print_errors_fp(stdout);
            close(client_socket);
            SSL_free(ssl);

            continue;
        }

        client = pkrsrv_server_client_new(server);
        client->server = server;
        client->is_alive = true;
        client->server_socket = server_socket;
        client->socket = client_socket;
        client->address = cli_addr.sin_addr.s_addr;
        client->ssl = ssl;

        current_client = client;

        pthread_mutex_lock(&server->mutex);

        pkrsrv_server_add_client(server, client);

        if (server->max_clients && (server->clients->length > server->max_clients)) {
            pkrsrv_util_verbose("Client limit reached, disconnecting the client... (client#%d)\n", client->socket);
            pthread_mutex_unlock(&server->mutex);
            pkrsrv_server_send_over_capacity(client);
            close(client_socket);
            pkrsrv_server_remove_client__ts(server, client);
            continue;
        }

        pthread_mutex_unlock(&server->mutex);

        PKRSRV_REF_COUNTED_USE(client);
        pthread_create(
            &client_thread,
            NULL,
            (void *) &client_handler,
            (void *) client
        );

        pthread_detach(client_thread);
    }
}

static void client_handler(pkrsrv_server_client_t* client) {
    signal(SIGPIPE, SIG_IGN);

    on_client_connected_params_t* task_params = malloc(sizeof(on_client_connected_params_t));
    task_params->owner = client->server->owner;
    task_params->client = client;

    pkrsrv_eventloop_call(client->server->eventloop, client->server->on_client_connected, task_params);

    receive_packet(client);
}

static void receive_packet(pkrsrv_server_client_t* client) {
    char str_addr[INET6_ADDRSTRLEN+1];
    inet_ntop(AF_INET, (void *) &client->address, str_addr, INET_ADDRSTRLEN);
    
    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};

    RECEIVE_FIRST_FRAME:

    result = pkrsrv_server_ssl_read(client->ssl, &header, sizeof(pkrsrv_server_packet_frame_header_t));
    if (!result) {
        client_disconnected(client);
        return;
    }

    char* header_str = (char *) (&header);

    client->is_websocket = (header_str[0] == 'G') &&
                           (header_str[1] == 'E') &&
                           (header_str[2] == 'T') &&
                           (header_str[3] == ' ') &&
                           (header_str[4] == '/');

    client->is_protocol_determined = true;
    
    if (!client->is_websocket) {
        goto CHECK_OPCODE;
    }

    pkrsrv_websocket_read_http_header(&(client->websocket), client->ssl);

    receive_websocket_packet(client);

    return;

    RECEIVE_FRAME:

    {
        int readed = pkrsrv_server_net_read(client, &header, sizeof(pkrsrv_server_packet_frame_header_t));
        if (readed <= 0) {
            client_disconnected(client);
            return;
        }
    };

    CHECK_OPCODE:

    if ((header.opcode <= PKRSRV_SERVER_OPCODE_NOP) || (header.opcode >= PKRSRV_SERVER_OPCODE_END)) {
        printf("Invalid opcode received, disconnecting the client... OPCODE: %d\n", header.opcode);
        client_disconnected(client);
        return;
    }

    (*(OPCODE_HANDLERS + header.opcode))(client, header);

    goto RECEIVE_FRAME;
}

static void receive_websocket_packet(pkrsrv_server_client_t* client) {
    ssize_t result;
    pkrsrv_server_packet_frame_header_t header = {0};
    
    RECEIVE_FRAME:
    
    result = pkrsrv_websocket_read_header(&(client->websocket), client->ssl);
    if (!result) {
        client_disconnected(client);
        return;
    }

    {
        int readed = pkrsrv_server_net_read(client, &header, sizeof(pkrsrv_server_packet_frame_header_t));
        if (readed <= 0) {
            client_disconnected(client);
            return;
        }
    }

    if ((header.opcode <= PKRSRV_SERVER_OPCODE_NOP) || (header.opcode >= PKRSRV_SERVER_OPCODE_END)) {
        printf("Invalid opcode received, disconnecting the client... OPCODE: %d\n", header.opcode);
        client_disconnected(client);
        return;
    }

    (*(OPCODE_HANDLERS + header.opcode))(client, header);

    goto RECEIVE_FRAME;
}

static void client_disconnected(pkrsrv_server_client_t* client) {
    pkrsrv_server_t* server = client->server;

    pthread_mutex_lock(&server->mutex);

    if (client->is_alive == false) {
        pthread_mutex_unlock(&server->mutex);
        return;
    }

    char str_addr[INET6_ADDRSTRLEN+1];
    inet_ntop(AF_INET, (void*)&client->address, str_addr, INET_ADDRSTRLEN);
    
    pkrsrv_util_verbose("Client is disconnecting (client#%d)\n", client->socket);

    client->is_alive = false;

    pkrsrv_server_remove_client(server, client);
    
    on_client_disconnected_params_t* task_params = malloc(sizeof(on_client_disconnected_params_t));
    task_params->owner = server->owner;
    task_params->client = client;

    pkrsrv_eventloop_call(server->eventloop, server->on_client_disconnected, task_params);

    pkrsrv_util_verbose("Client#%d disconnected: %s\n", client->socket, str_addr);
    
    PKRSRV_REF_COUNTED_LEAVE(client);

    pthread_mutex_unlock(&server->mutex);
}

static bool send_packet(pkrsrv_server_client_t* client, unsigned char* message, size_t message_size) {
    pkrsrv_server_packet_frame_header_t message_frame;
    message_frame.opcode = PKRSRV_SERVER_OPCODE_NOP;

    {
        int written = pkrsrv_server_net_write(client, &message_frame, sizeof(pkrsrv_server_packet_frame_header_t));
        if (written <= 0) {
            client_disconnected(client);
            return false;
        }
    }

    return true;
}

pkrsrv_server_clients_t* pkrsrv_server_clients_new() {
    pkrsrv_server_clients_t* clients = malloc(sizeof(pkrsrv_server_clients_t));
    LIST_INIT(clients);
    return clients;
}

pkrsrv_server_client_t* pkrsrv_server_client_new(pkrsrv_server_t* server) {
    pkrsrv_server_client_t* client = malloc(sizeof(pkrsrv_server_client_t));
    PKRSRV_REF_COUNTED_INIT(client, pkrsrv_server_client_free);
    LIST_ITEM_INIT(client);
    client->socket = 0;
    client->address = 0;
    client->is_alive = false;
    client->is_protocol_determined = false;
    client->is_websocket = false;
    pkrsrv_websocket_init(&(client->websocket));
    client->websocket.read = pkrsrv_server_ssl_read;
    client->websocket.write = pkrsrv_server_ssl_write;
    client->websocket.write_mutex = &(client->write_mutex);

    client->owner = NULL;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&(client->write_mutex), &mutex_attr);

    client->pg_conn = pkrsrv_db_connect(pkrsrv_postgres_host, pkrsrv_postgres_port, pkrsrv_postgres_username, pkrsrv_postgres_password, pkrsrv_postgres_db);
    if (!client->pg_conn) {
        printf("[Error] Failed to connect to the database for the new client!\n");
        pkrsrv_server_client_free(client);
        return NULL;
    }
    
    return client;
}

void pkrsrv_server_client_free(pkrsrv_server_client_t* client) {
    pkrsrv_util_verbose("Freeing client#%d...\n", client->socket);

    pthread_mutex_destroy(&client->write_mutex);

    if (client->pg_conn) {
        PQfinish(client->pg_conn);
    }

    SSL_free(client->ssl);
    free(client);
}

void pkrsrv_server_free(pkrsrv_server_t* server) {
    PKRSRV_REF_COUNTED_LEAVE(server->eventloop);
    
    pthread_mutex_destroy(&server->mutex);
    
    LIST_FOREACH(server->clients, client)
        PKRSRV_REF_COUNTED_LEAVE(client);
    END_FOREACH

    free(server->clients);
    free(server);
}

bool pkrsrv_server_send_binary(pkrsrv_server_client_t* client, unsigned char* data, size_t size) {
    return send_packet(client, data, size);
}

static void sigabrt_handler() {
    pkrsrv_util_verbose("SIGABRT received...");
}

static void sigpipe_handler(int signal) {
    pkrsrv_util_verbose("Broken pipe.\n");
}