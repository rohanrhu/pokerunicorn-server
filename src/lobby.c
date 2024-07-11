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
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <search.h>
#include <assert.h>

#include <libpq-fe.h>

#include "../thirdparty/jsonic/jsonic.h"

#include "../include/pkrsrv.h"
#include "../include/lobby.h"

#include "../include/util.h"
#include "../include/uniqid.h"
#include "../include/ref.h"
#include "../include/db.h"
#include "../include/table.h"
#include "../include/account.h"
#include "../include/poker.h"
#include "../include/server.h"
#include "../include/eventloop.h"
#include "../include/mime.h"
#include "../include/sugar.h"
#include "../include/string.h"
#include "../include/deposit.h"

void on_account_updated__async(pkrsrv_eventloop_task_t* task) {
    on_account_updated_params_t* task_params = (on_account_updated_params_t *) task->params;
    pkrsrv_account_t* account = task_params->account;
    pkrsrv_lobby_client_t* lobby_client = task_params->lobby_client;

    pkrsrv_util_verbose("Sending account to the owner client due to Account#%llu is updated...\n", account->id);

    pkrsrv_server_send_account((pkrsrv_server_send_account_params_t) {
        .client = lobby_client->client,
        .account = account
    });

    PKRSRV_REF_COUNTED_LEAVE(account);
    PKRSRV_REF_COUNTED_LEAVE(lobby_client);
}

void on_account_updated(pkrsrv_account_t* account, pkrsrv_lobby_client_t* lobby_client) {
    PKRSRV_REF_COUNTED_USE(account);
    PKRSRV_REF_COUNTED_USE(lobby_client);
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) lobby_client->lobby;
    
    pkrsrv_util_verbose("Account#%llu is updated.\n", account->id);
    
    on_account_updated_params_t* task_params = malloc(sizeof(on_account_updated_params_t));
    task_params->account = account;
    task_params->lobby_client = lobby_client;
    pkrsrv_eventloop_call(lobby->eventloop, on_account_updated__async, task_params);
}

pkrsrv_lobby_t* pkrsrv_lobby_new(pkrsrv_lobby_new_params_t params) {
    pkrsrv_lobby_t* lobby = malloc(sizeof(pkrsrv_lobby_t));
    PKRSRV_REF_COUNTED_INIT(lobby, pkrsrv_lobby_free);
    
    pthread_mutex_init(&lobby->mutex, NULL);

    lobby->eventloop = pkrsrv_eventloop_new();
    PKRSRV_REF_COUNTED_USE(lobby->eventloop);
    
    lobby->sessions = malloc(sizeof(pkrsrv_lobby_sessions_t));
    LIST_INIT(lobby->sessions);

    lobby->server = pkrsrv_server_new((pkrsrv_server_new_params_t) {
        .owner = lobby,
        .eventloop = lobby->eventloop,
        .port = params.port,
        .bind_address = params.bind_address,
        .max_clients = params.max_clients,
        .on_client_connected = on_client_connected,
        .on_client_disconnected = on_client_disconnected,
        .on_client_meowed = on_client_meowed,
        .on_client_enter = on_client_enter,
        .on_client_leave = on_client_leave,
        .on_client_join = on_client_join,
        .on_client_unjoin = on_client_unjoin,
        .on_client_login = on_client_login,
        .on_client_signup = on_client_signup,
        .on_client_get_account = on_client_get_account,
        .on_client_action = on_client_action,
        .on_client_get_tables = on_client_get_tables,
        .on_client_get_sessions = on_client_get_sessions,
        .on_client_update_account = on_client_update_account,
        .on_client_json = on_client_json
    });
    
    PKRSRV_REF_COUNTED_USE(lobby->server);

    return lobby;
}

void pkrsrv_lobby_free(pkrsrv_lobby_t* lobby) {
    pthread_mutex_destroy(&lobby->mutex);
    
    PKRSRV_REF_COUNTED_LEAVE(lobby->eventloop);
    PKRSRV_REF_COUNTED_LEAVE(lobby->server);

    LIST_FOREACH(lobby->sessions, session)
        pkrsrv_lobby_sessions_remove(session->lobby->sessions, session);
    END_FOREACH

    free(lobby->sessions);
    free(lobby);
}

void pkrsrv_lobby_run(pkrsrv_lobby_t* lobby) {
    pkrsrv_server_start(lobby->server);
    pkrsrv_eventloop_run(lobby->eventloop);
}

pkrsrv_lobby_session_t* pkrsrv_lobby_session_getby_id(pkrsrv_lobby_t* lobby, uint64_t id) {
    LIST_FOREACH(lobby->sessions, session)
        PKRSRV_UTIL_ASSERT(session->poker);
        if (session->id.scalar == id) {
            return session;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_lobby_session_t* pkrsrv_lobby_session_getby_table_id(pkrsrv_lobby_t* lobby, uint64_t id) {
    LIST_FOREACH(lobby->sessions, session)
        PKRSRV_UTIL_ASSERT(session->poker);
        if (session->poker->table->id.scalar == id) {
            return session;
        }
    END_FOREACH

    return NULL;
}

void pkrsrv_lobby_session_start(pkrsrv_lobby_session_t* session) {
    pkrsrv_util_verbose("Starting session thread...\n");

    PKRSRV_REF_COUNTED_USE(session);
    session->is_running = true;

    pkrsrv_lobby_session_proceed__async_params_t* task_params = malloc(sizeof(pkrsrv_lobby_session_proceed__async_params_t));
    task_params->session = session;
    session->proceed_task = pkrsrv_eventloop_task_new(session->lobby->eventloop, pkrsrv_lobby_session_proceed__async, task_params);
    pkrsrv_eventloop_task_call(session->lobby->eventloop, session->proceed_task);
}

void pkrsrv_lobby_session_stop(pkrsrv_lobby_session_t* session) {
    pkrsrv_util_verbose("Stopping session#%llu...\n", session->id.scalar);

    session->is_running = false;

    if (session->poker_start_task) {
        pkrsrv_eventloop_task_cancel(session->poker_start_task);
        PKRSRV_REF_COUNTED_LEAVE(session->poker_start_task);
        session->poker_start_task = NULL;
    }

    if (session->poker_restart_task) {
        pkrsrv_eventloop_task_cancel(session->poker_restart_task);
        PKRSRV_REF_COUNTED_LEAVE(session->poker_restart_task);
        session->poker_restart_task = NULL;
    }

    if (!session->proceed_task) {
        pkrsrv_lobby_session_proceed__async_params_t* task_params = malloc(sizeof(pkrsrv_lobby_session_proceed__async_params_t));
        task_params->session = session;
        session->proceed_task = pkrsrv_eventloop_task_new(session->lobby->eventloop, pkrsrv_lobby_session_proceed__async, task_params);
        pkrsrv_eventloop_task_call(session->lobby->eventloop, session->proceed_task);
    }
}

void pkrsrv_lobby_session_proceed__async(pkrsrv_eventloop_task_t* task) {
    pkrsrv_lobby_session_proceed__async_params_t* task_params = (pkrsrv_lobby_session_proceed__async_params_t *) task->params;
    pkrsrv_lobby_session_t* session = task_params->session;

    session->proceed_task = NULL;

    if (!session->is_running) {
        pkrsrv_util_verbose("Session#%llu is stopped.\n", session->id.scalar);
        PKRSRV_REF_COUNTED_LEAVE(session);
        return;
    }
    
    if (session->poker->state == PKRSRV_POKER_STATE_DONE) {
        LIST_FOREACH(session->clients, session_client)
            pkrsrv_server_send_poker_end_params_t poker_end_params;
            poker_end_params.client = session_client->client->client;
            poker_end_params.table_id = session->poker->table->id.scalar;
            poker_end_params.poker = session->poker;

            if (session->poker->winner) {
                poker_end_params.winner_account_id = session->poker->winner->account->id;
                poker_end_params.earned_amount = session->poker->pot_amount;
            } else {
                poker_end_params.winner_account_id = 0;
                poker_end_params.earned_amount = 0;
            }
            
            pkrsrv_server_send_poker_end(poker_end_params);
        
            pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_id(session->poker, session_client->client->account->id);

            if (player && (player->balance < session->poker->table->enterance_min)) {
                pkrsrv_lobby_client_unjoin_session(session_client->client, session);
            }
        END_FOREACH

        LIST_FOREACH(session->poker->players, player)
            if (player->is_left) {
                pkrsrv_poker_players_remove(session->poker, player);
            }
        END_FOREACH

        reorder_updated_session(session, session->poker->players->length);
        pkrsrv_lobby_broadcast_sessions(session->lobby);

        pkrsrv_poker_starting(session->poker);

        LIST_FOREACH(session->clients, session_client)
            pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(session->poker, session_client->client->account->id);
            
            pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
                .client = session_client->client->client,
                .poker = session->poker,
                .player = receiver_player
            });
        END_FOREACH
        
        pkrsrv_lobby_session_poker_restart__async_params_t* task_params = malloc(sizeof(pkrsrv_lobby_session_poker_restart__async_params_t));
        task_params->session = session;
        session->poker_restart_task = pkrsrv_eventloop_task_new(session->lobby->eventloop, pkrsrv_lobby_session_poker_restart__async, task_params);
        PKRSRV_REF_COUNTED_USE(session->poker_restart_task);
        pkrsrv_eventloop_task_call_after(session->lobby->eventloop, session->poker_restart_task, 10000);
    } else {
        pkrsrv_lobby_session_proceed__async_params_t* task_params = malloc(sizeof(pkrsrv_lobby_session_proceed__async_params_t));
        task_params->session = session;
        session->proceed_task = pkrsrv_eventloop_task_new(session->lobby->eventloop, pkrsrv_lobby_session_proceed__async, task_params);
        pkrsrv_eventloop_task_call_after(session->lobby->eventloop, session->proceed_task, 1000);
    }

    pkrsrv_poker_proceed(session->poker);
}

void pkrsrv_lobby_session_poker_start__async(pkrsrv_eventloop_task_t* task) {
    pkrsrv_lobby_session_poker_start__async_params_t* task_params = (pkrsrv_lobby_session_poker_start__async_params_t *) task->params;
    pkrsrv_lobby_session_t* session = task_params->session;

    PKRSRV_REF_COUNTED_LEAVE(session->poker_start_task);
    session->poker_start_task = NULL;

    bool is_started = pkrsrv_poker_start(session->poker);

    if (!is_started) {
        return;
    }
    
    LIST_FOREACH(session->clients, session_client)
        pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(session->poker, session_client->client->account->id);
        
        pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
            .client = session_client->client->client,
            .poker = session->poker,
            .player = receiver_player
        });
    END_FOREACH
}

void pkrsrv_lobby_session_poker_restart__async(pkrsrv_eventloop_task_t* task) {
    pkrsrv_lobby_session_poker_restart__async_params_t* task_params = (pkrsrv_lobby_session_poker_restart__async_params_t *) task->params;
    pkrsrv_lobby_session_t* session = task_params->session;

    PKRSRV_REF_COUNTED_LEAVE(session->poker_restart_task);
    session->poker_restart_task = NULL;
    
    pkrsrv_poker_restart(session->poker);

    LIST_FOREACH(session->clients, session_client)
        pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(session->poker, session_client->client->account->id);

        pkrsrv_server_send_poker_restarted((pkrsrv_server_send_poker_restarted_params_t) {
            .client = session_client->client->client,
            .table_id = session->poker->table->id.scalar
        });

        pkrsrv_server_send_poker_info((pkrsrv_server_send_poker_info_params_t) {
            .client = session_client->client->client,
            .poker = session->poker
        });

        pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
            .client = session_client->client->client,
            .poker = session->poker,
            .player = receiver_player
        });
    END_FOREACH

    pkrsrv_lobby_session_proceed__async_params_t* proceed_task_params = malloc(sizeof(pkrsrv_lobby_session_proceed__async_params_t));
    proceed_task_params->session = session;
    session->proceed_task = pkrsrv_eventloop_task_new(session->lobby->eventloop, pkrsrv_lobby_session_proceed__async, proceed_task_params);
    pkrsrv_eventloop_task_call_after(session->lobby->eventloop, session->proceed_task, 1000);
}

static void on_client_login(pkrsrv_eventloop_task_t* task) {
    on_client_login_params_t* task_params = (on_client_login_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_login_t login = task_params->login;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    PKRSRV_REF_COUNTED_USE(login.id_token);
    PKRSRV_REF_COUNTED_USE(login.password);
    
    pkrsrv_poker_t* pokers[lobby->sessions->length];
    memset(pokers, 0, sizeof(void*) * lobby->sessions->length);
    
    pkrsrv_util_verbose("session.on_client_login(%d)\n", client->socket);

    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_account_t* account = pkrsrv_account_getby_credentials(client->pg_conn, (pkrsrv_account_getby_credentials_params_t) {
        .id_token = login.id_token,
        .password = login.password
    });

    if (!account) {
        pkrsrv_server_send_login_res((pkrsrv_server_send_login_res_params_t) {
            .client = client,
            .is_ok = 1,
            .is_logined = 0,
            .account = NULL
        });

        goto RETURN;
    }

    account->owner = lobby;
    account->on_updated = (void (*)(pkrsrv_account_t*, void*)) on_account_updated;
    account->on_updated_param = lobby_client;

    pkrsrv_lobby_client_set_account(lobby_client, account);

    bool result = pkrsrv_server_send_login_res((pkrsrv_server_send_login_res_params_t) {
        .client = client,
        .is_ok = 1,
        .is_logined = 1,
        .account = lobby_client->account
    });
    if (!result) { 
        goto RETURN;
    }
    
    LIST_FOREACH(lobby->sessions, session)
        pokers[session_i] = session->poker;
    END_FOREACH

    pkrsrv_server_send_sessions((pkrsrv_server_send_sessions_params_t) {
        .client = client,
        .offset = 0,
        .pokers_length = lobby->sessions->length,
        .pokers = pokers
    });

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(login.id_token);
    PKRSRV_REF_COUNTED_LEAVE(login.password);
}

static void on_client_signup(pkrsrv_eventloop_task_t* task) {
    on_client_signup_params_t* task_params = (on_client_signup_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_signup_t signup = task_params->signup;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;

    PKRSRV_REF_COUNTED_USE(signup.avatar);
    PKRSRV_REF_COUNTED_USE(signup.id_token);
    PKRSRV_REF_COUNTED_USE(signup.password);
    PKRSRV_REF_COUNTED_USE(signup.name);
    
    pkrsrv_poker_t* pokers[lobby->sessions->length];
    memset(pokers, 0, sizeof(void*) * lobby->sessions->length);
    
    pkrsrv_util_verbose("session.on_client_signup(%d)\n", client->socket);

    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    uint64_t balance = 10000000000000;

    pkrsrv_account_create_params_t params;
    params.id_token = signup.id_token;
    params.name = signup.name;
    params.avatar = signup.avatar;
    params.password = signup.password;
    params.balance = balance;
    params.xmr_deposit_address = NULL;
    
    pkrsrv_account_create_result_t result = pkrsrv_account_create(client->pg_conn, params);

    uint16_t signup_res_status = PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_OK;

    if (!result.account) {
        signup_res_status = PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ERROR;
        if (result.is_already_exists) {
            signup_res_status = PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ALREADY_EXISTS;
        }

        pkrsrv_server_send_signup_res((pkrsrv_server_send_signup_res_params_t) {
            .client = client,
            .is_ok = 0,
            .is_logined = 0,
            .account = NULL,
            .status = signup_res_status
        });

        goto RETURN;
    }

    result.account->owner = lobby;
    result.account->on_updated = (void (*)(pkrsrv_account_t*, void*)) on_account_updated;
    result.account->on_updated_param = lobby_client;
    
    pkrsrv_lobby_client_set_account(lobby_client, result.account);

    bool is_logined = true;

    pkrsrv_server_send_signup_res((pkrsrv_server_send_signup_res_params_t) {
        .client = client,
        .is_ok = 1,
        .is_logined = (uint8_t) is_logined,
        .account = lobby_client->account,
        .status = signup_res_status
    });

    if (is_logined) {
        LIST_FOREACH(lobby->sessions, session)
            pokers[session_i] = session->poker;
        END_FOREACH

        pkrsrv_server_send_sessions((pkrsrv_server_send_sessions_params_t) {
            .client = client,
            .offset = 0,
            .pokers_length = lobby->sessions->length,
            .pokers = pokers
        });
    }
    
    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(signup.avatar);
    PKRSRV_REF_COUNTED_LEAVE(signup.id_token);
    PKRSRV_REF_COUNTED_LEAVE(signup.password);
    PKRSRV_REF_COUNTED_LEAVE(signup.name);
}

static void on_client_get_account(pkrsrv_eventloop_task_t* task) {
    on_client_get_account_params_t* task_params = (on_client_get_account_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;

    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    if (!lobby_client->account) {
        /**
         * ! Unexpected Behavior
         */
        pkrsrv_util_verbose("Unexpected Behavior: Client sent GET_ACCOUNT before logging in!\n");
        
        return;
    }

    pkrsrv_util_verbose("session.on_client_get_account(%d)\n", client->socket);

    pkrsrv_account_fetch(client->pg_conn, lobby_client->account);

    pkrsrv_server_send_account((pkrsrv_server_send_account_params_t) {
        .client = client,
        .account = lobby_client->account
    });
}

static void on_client_enter(pkrsrv_eventloop_task_t* task) {
    on_client_enter_params_t* task_params = (on_client_enter_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_enter_t enter = task_params->enter;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;
    
    if (!lobby_client->account) {
        /**
         * ! Unexpected Behavior
         */
        pkrsrv_util_verbose("Unexpected Behavior: Enter before login!\n");
        
        pkrsrv_server_send_enter_res((pkrsrv_server_send_enter_res_params_t) {
            .client = client,
            .table_id = enter.table_id,
            .is_ok = 0
        });

        return;
    }

    pkrsrv_util_verbose("Player (socket#%d) is entering to table#%d\n", client->socket, enter.table_id);

    pkrsrv_lobby_session_t* session = pkrsrv_lobby_session_getby_table_id(lobby, enter.table_id);
    
    if (!session) {
        pkrsrv_util_verbose("There is no a session for table#%d, creating a new one...\n", enter.table_id);

        pkrsrv_table_t* table = pkrsrv_table_get(client->pg_conn, enter.table_id);
        
        if (!table) {
            /**
             * ! Unexpected Behavior
             */
            pkrsrv_util_verbose("Table#%llu is not found!\n", enter.table_id);
            
            pkrsrv_server_send_enter_res((pkrsrv_server_send_enter_res_params_t) {
                .client = client,
                .table_id = enter.table_id,
                .is_ok = 0
            });

            return;
        }
        
        table->players_count = 0;
        table->watchers_count = 0;
         
        session = pkrsrv_lobby_session_new((pkrsrv_lobby_session_new_params_t) {
            .lobby = lobby_client->lobby,
            .table = table
        });

        pkrsrv_lobby_sessions_add(lobby->sessions, session);
        pkrsrv_lobby_session_start(session);
    }

    bool is_entered = pkrsrv_lobby_client_enter_session(lobby_client, session);
    
    pkrsrv_server_send_enter_res((pkrsrv_server_send_enter_res_params_t) {
        .client = client,
        .table_id = session->poker->table->id.scalar,
        .is_ok = is_entered
    });

    if (is_entered) {
        pkrsrv_server_send_poker_info((pkrsrv_server_send_poker_info_params_t) {
            .client = client,
            .poker = session->poker
        });
    }
}

static void on_client_leave(pkrsrv_eventloop_task_t* task) {
    on_client_leave_params_t* task_params = (on_client_leave_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_leave_t leave = task_params->leave;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;
    
    pkrsrv_lobby_client_session_t* to_leave = pkrsrv_lobby_client_session_getby_table_id(lobby_client, leave.table_id);

    if (!to_leave) {
        pkrsrv_util_verbose( "Client#%d requested to leave session (table#%llu), but not entered!:\n", client->socket, leave.table_id );
        return;
    }

    pkrsrv_util_verbose("Client#%d requested to leave session (session#%llu, table: %s), leaving sessions...\n",
                        client->socket, to_leave->session->id.scalar, to_leave->session->poker->table->name->value);
    pkrsrv_util_verbose("\tLeaving session(table#%llu)...\n", to_leave->session->poker->table->id.scalar);

    pkrsrv_lobby_client_leave_session(lobby_client, to_leave);

    pkrsrv_server_send_leave_res((pkrsrv_server_send_leave_res_params_t) {
        .client = client,
        .is_ok = 1,
        .table_id = leave.table_id
    });
}

static void on_client_join(pkrsrv_eventloop_task_t* task) {
    on_client_join_params_t* task_params = (on_client_join_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_join_t join = task_params->join;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;

    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    if (!lobby_client->account) {
        /**
         * ! Unexpected Behavior
         */
        pkrsrv_util_verbose("Unexpected Behavior: Join before login!\n");
        
        pkrsrv_server_send_join_res((pkrsrv_server_send_join_res_params_t) {
            .client = client,
            .table_id = join.table_id,
            .is_ok = 0
        });

        return;
    }

    pkrsrv_account_fetch(client->pg_conn, lobby_client->account);

    if (lobby_client->account->balance < join.enterance_amount) {
        /**
         * ! Unexpected Behavior
         */
        pkrsrv_util_verbose("Unexpected Behavior: Insufficient join!\n");

        pkrsrv_server_send_join_res((pkrsrv_server_send_join_res_params_t) {
            .client = client,
            .table_id = join.table_id,
            .is_ok = 0
        });

        return;
    }

    pkrsrv_util_verbose("Player (socket#%d) is joining to table#%d\n", client->socket, join.table_id);

    pkrsrv_lobby_session_t* session = pkrsrv_lobby_session_getby_table_id(lobby, join.table_id);
    
    if (!session) {
        /**
         * ! Unexpected Behavior
         */
        pkrsrv_util_verbose("Unexpected Behavior: Joining session#%d doesn't exist!\n", join.table_id);
        
        pkrsrv_server_send_join_res((pkrsrv_server_send_join_res_params_t) {
            .client = client,
            .table_id = join.table_id,
            .is_ok = 0
        });

        return;
    }


    bool is_joined = pkrsrv_lobby_client_join_session(lobby_client, session, join.enterance_amount, join.position);

    pkrsrv_server_send_join_res((pkrsrv_server_send_join_res_params_t) {
        .client = client,
        .table_id = session->poker->table->id.scalar,
        .is_ok = is_joined
    });


    if (is_joined) {
        pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_id(session->poker, lobby_client->account->id);

        pkrsrv_server_send_poker_info((pkrsrv_server_send_poker_info_params_t) {
            .client = client,
            .poker = session->poker
        });

        LIST_FOREACH(session->clients, session_client)
            pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(session->poker, session_client->client->account->id);

            pkrsrv_server_send_poker_info((pkrsrv_server_send_poker_info_params_t) {
                .client = session_client->client->client,
                .poker = session->poker
            });
            
            pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
                .client = session_client->client->client,
                .poker = session->poker,
                .player = receiver_player
            });
        END_FOREACH
    }
}

static void on_client_unjoin(pkrsrv_eventloop_task_t* task) {
    on_client_unjoin_params_t* task_params = (on_client_unjoin_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_unjoin_t unjoin = task_params->unjoin;

    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_lobby_client_session_t* to_unjoin = pkrsrv_lobby_client_session_getby_table_id(lobby_client, unjoin.table_id);

    pkrsrv_util_verbose(
        "Client#%d requested to unjoin session (session#%llu, table: %s), unjoining session...\n",
        client->socket,
        to_unjoin->session->id.scalar,
        to_unjoin->session->poker->table->name->value
    );

    pkrsrv_util_verbose("\tUnjoining session(table#%llu)...\n", to_unjoin->session->poker->table->id.scalar);

    pkrsrv_lobby_session_t* session = to_unjoin->session;

    bool result = pkrsrv_lobby_client_unjoin_session(lobby_client, to_unjoin->session);

    pkrsrv_server_send_unjoin_res((pkrsrv_server_send_unjoin_res_params_t) {
        .client = client,
        .is_ok = result,
        .table_id = unjoin.table_id
    });
}

static void on_client_action(pkrsrv_eventloop_task_t* task) {
    on_client_action_params_t* task_params = (on_client_action_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_poker_action_t action = task_params->action;

    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_lobby_client_session_t* playing_session = pkrsrv_lobby_client_session_getby_table_id(lobby_client, action.table_id);

    if (!playing_session) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Unexpected Behavior: (client#%d) Session#%llu not found!\n", client->socket, action.table_id);
        
        goto RETURN;
    }

    pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_id(playing_session->session->poker, lobby_client->account->id);

    if (!player) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Unexpected Behavior: (client#%d) Session#%llu player doesn't exist in poker machine!!\n", client->socket, action.table_id);

        goto RETURN;
    }

    pkrsrv_util_verbose("Action received from client#%d: %d\n", client->socket, action.action_kind);

    pkrsrv_poker_action_t poker_action;
    poker_action.account = *lobby_client->account;
    poker_action.kind = action.action_kind;
    poker_action.amount = action.amount;

    pkrsrv_poker_action_result_t result = pkrsrv_poker_do_action(playing_session->session->poker, player, poker_action);
    
    if (!result.is_done) {
        // ! Action is not done!
        pkrsrv_util_verbose("Client#%d: Action is not done!\n", client->socket);
    }

    RETURN:

    return;
}

static void on_client_get_tables(pkrsrv_eventloop_task_t* task) {
    on_client_get_tables_params_t* task_params = (on_client_get_tables_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_get_tables_t get_tables = task_params->get_tables;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_util_verbose("Table list requested by client#%d, sending tables...\n", client->socket);

    pkrsrv_table_list_t* list = pkrsrv_table_get_list(client->pg_conn, (pkrsrv_table_get_list_params_t) {
        .offset = get_tables.offset,
        .length = get_tables.length
    });

    pkrsrv_server_send_tables((pkrsrv_server_send_tables_params_t) {
        .client = client,
        .list = list
    });

    pkrsrv_table_list_free(list);
}

static void on_client_get_sessions(pkrsrv_eventloop_task_t* task) {
    on_client_get_sessions_params_t* task_params = (on_client_get_sessions_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_get_sessions_t get_sessions = task_params->get_sessions;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;
    
    pkrsrv_util_verbose("Sessions list requested by client#%d, sending sessions...\n", client->socket);

    pkrsrv_poker_t* pokers[lobby->sessions->length];
    memset(pokers, 0, sizeof(void*) * lobby->sessions->length);
    
    LIST_FOREACH(lobby->sessions, session)
        pokers[session_i] = session->poker;
    END_FOREACH

    pkrsrv_server_send_sessions((pkrsrv_server_send_sessions_params_t) {
        .client = client,
        .offset = get_sessions.offset,
        .pokers_length = lobby->sessions->length,
        .pokers = pokers
    });
}

static void on_client_update_account(pkrsrv_eventloop_task_t* task) {
    on_client_update_account_params_t* task_params = (on_client_update_account_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_update_account_t update_account = task_params->update_account;

    PKRSRV_REF_COUNTED_USE(update_account.name);
    PKRSRV_REF_COUNTED_USE(update_account.avatar);
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_util_verbose("Client#%d, updating their account...\n", client->socket);

    if (!lobby_client->account) {
        /**
         * ! Unexpected Behavior
         */
        pkrsrv_util_verbose("Unexpected Behavior: Account update before login!\n");
        
        pkrsrv_server_send_update_account_res((pkrsrv_server_send_update_account_res_params_t) {
            .client = client,
            .is_ok = false,
            .is_avatar_too_big = false
        });

        goto RETURN;
    }

    pkrsrv_account_update_result_t result = pkrsrv_account_update(client->pg_conn, (pkrsrv_account_update_params_t) {
        .id = lobby_client->account->id,
        .name = update_account.name,
        .avatar = update_account.avatar
    });

    if (result.is_ok) {
        update_account.avatar->is_alloc_str = false;
        pkrsrv_string_set_value(lobby_client->account->avatar, update_account.avatar->value);
    }

    pkrsrv_server_send_update_account_res((pkrsrv_server_send_update_account_res_params_t) {
        .client = client,
        .is_ok = result.is_ok,
        .is_avatar_too_big = result.is_avatar_too_big
    });

    pkrsrv_account_fetch(client->pg_conn, lobby_client->account);

    pkrsrv_server_send_account((pkrsrv_server_send_account_params_t) {
        .client = client,
        .account = lobby_client->account
    });

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(update_account.name);
    PKRSRV_REF_COUNTED_LEAVE(update_account.avatar);
}

static void on_client_json(pkrsrv_eventloop_task_t* task) {
    on_client_json_params_t* task_params = (on_client_json_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    pkrsrv_server_packet_json_t json_packet = task_params->json;

    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    PKRSRV_REF_COUNTED_USE(json_packet.json);

    printf("\nJSON Received:\n%s\n\n", json_packet.json->value);

    PKRSRV_REF_COUNTED_LEAVE(json_packet.json);
}

static void on_client_connected(pkrsrv_eventloop_task_t* task) {
    on_client_connected_params_t* task_params = (on_client_connected_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;
    
    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;

    pkrsrv_lobby_client_t* lobby_client = pkrsrv_lobby_client_new(lobby, client);
    PKRSRV_REF_COUNTED_USE(lobby_client);
    client->owner = lobby_client;
}

static void on_client_disconnected(pkrsrv_eventloop_task_t* task) {
    on_client_disconnected_params_t* task_params = (on_client_disconnected_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;

    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;
    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_util_verbose("Client#%d disconnected, leaving sessions:\n", client->socket);

    LIST_FOREACH(lobby_client->sessions, to_leave)
        pkrsrv_util_verbose("\tLeaving session(table#%llu)...\n", to_leave->session->poker->table->id.scalar);
        pkrsrv_lobby_client_leave_session(lobby_client, to_leave);
    END_FOREACH

    PKRSRV_REF_COUNTED_LEAVE(lobby_client);
}

static void on_client_meowed(pkrsrv_eventloop_task_t* task) {
    on_client_meowed_params_t* task_params = (on_client_meowed_params_t *) task->params;

    void* owner = task_params->owner;
    pkrsrv_server_client_t* client = task_params->client;

    pkrsrv_lobby_t* lobby = (pkrsrv_lobby_t *) owner;

    pkrsrv_lobby_client_t* lobby_client = (pkrsrv_lobby_client_t *) client->owner;

    pkrsrv_server_send_server_info_params_t server_info;
    server_info.client = client;
    server_info.build_number = pkrsrv_build_number;
    server_info.version = pkrsrv_version;
    server_info.revision = pkrsrv_revision;
    server_info.compiler = pkrsrv_compiler;
    
    pkrsrv_server_send_server_info(server_info);
}

pkrsrv_lobby_client_t* pkrsrv_lobby_client_new(pkrsrv_lobby_t* lobby, pkrsrv_server_client_t* client) {
    PKRSRV_REF_COUNTED_USE(client);

    pkrsrv_lobby_client_t* lobby_client = malloc(sizeof(pkrsrv_lobby_client_t));
    PKRSRV_REF_COUNTED_INIT(lobby_client, pkrsrv_lobby_client_free);
    lobby_client->lobby = lobby;
    lobby_client->client = client;
    lobby_client->account = NULL;
    
    lobby_client->sessions = malloc(sizeof(pkrsrv_lobby_client_sessions_t));
    LIST_INIT(lobby_client->sessions);

    return lobby_client;
}

void pkrsrv_lobby_client_free(pkrsrv_lobby_client_t* lobby_client) {
    pkrsrv_util_verbose("Freeing lobby client object... (Client: socket#%d)\n", lobby_client->client->socket);
    
    if (lobby_client->account) {
        PKRSRV_REF_COUNTED_LEAVE(lobby_client->account);
    }

    PKRSRV_REF_COUNTED_LEAVE(lobby_client->client);

    LIST_FOREACH(lobby_client->sessions, session)
        pkrsrv_lobby_client_sessions_remove(lobby_client->sessions, session);
    END_FOREACH

    free(lobby_client->sessions);
    free(lobby_client);
}

void pkrsrv_lobby_client_set_account(pkrsrv_lobby_client_t* lobby_client, pkrsrv_account_t* account) {
    PKRSRV_REF_COUNTED_USE(account);
    lobby_client->account = account;
}

pkrsrv_lobby_session_t* pkrsrv_lobby_session_new(pkrsrv_lobby_session_new_params_t params) {
    pkrsrv_lobby_session_t* session = malloc(sizeof(pkrsrv_lobby_session_t));
    PKRSRV_REF_COUNTED_INIT(session, pkrsrv_lobby_session_free);
    LIST_ITEM_INIT(session);
    session->lobby = params.lobby;

    session->pg_conn = pkrsrv_db_connect(pkrsrv_postgres_host, pkrsrv_postgres_port, pkrsrv_postgres_username, pkrsrv_postgres_password, pkrsrv_postgres_db);
    if (!session->pg_conn) {
        printf("[Error] Failed to connect to the database for the new session!\n");
        free(session);
        return NULL;
    }

    session->clients = malloc(sizeof(pkrsrv_lobby_session_clients_t));
    LIST_INIT(session->clients);
    session->is_running = false;
    session->id = pkrsrv_uniqid_generate();
    session->process_latency = params.lobby->process_latency;
    session->proceed_task = NULL;
    session->poker_start_task = NULL;
    session->poker_restart_task = NULL;
    session->joined_clients_count = 0;
    pthread_mutex_init(&session->mutex, NULL);

    session->poker = pkrsrv_poker_new((pkrsrv_poker_new_params_t) {
        .owner = session,
        .table = params.table,
        .on_action = on_poker_action,
        .eventloop = params.lobby->eventloop
    });
    PKRSRV_REF_COUNTED_USE(session->poker);

    return session;
}

void pkrsrv_lobby_session_free(pkrsrv_lobby_session_t* session) {
    pkrsrv_util_verbose("Freeing session object... (Session: %llu)\n", session->id.scalar);

    pthread_mutex_destroy(&session->mutex);

    if (session->pg_conn) {
        PQfinish(session->pg_conn);
    }

    LIST_FOREACH(session->clients, client)
        PKRSRV_REF_COUNTED_LEAVE(client);
    END_FOREACH

    PKRSRV_REF_COUNTED_LEAVE(session->poker);

    free(session->clients);
    free(session);
}

void pkrsrv_lobby_sessions_add(pkrsrv_lobby_sessions_t* sessions, pkrsrv_lobby_session_t* session) {
    PKRSRV_REF_COUNTED_USE(session);
    LIST_APPEND(sessions, session);
}

void pkrsrv_lobby_sessions_remove(pkrsrv_lobby_sessions_t* sessions, pkrsrv_lobby_session_t* session) {
    LIST_REMOVE(sessions, session);
    PKRSRV_REF_COUNTED_LEAVE(session);
}

pkrsrv_lobby_session_client_t* pkrsrv_lobby_session_client_new(pkrsrv_lobby_client_t* lobby_client) {
    PKRSRV_REF_COUNTED_USE(lobby_client);

    pkrsrv_lobby_session_client_t* session_client = malloc(sizeof(pkrsrv_lobby_session_client_t));
    PKRSRV_REF_COUNTED_INIT(session_client, pkrsrv_lobby_session_client_free);
    LIST_ITEM_INIT(session_client);
    session_client->client = lobby_client;
    session_client->is_joined = false;

    return session_client;
}

void pkrsrv_lobby_session_client_free(pkrsrv_lobby_session_client_t* session_client) {
    pkrsrv_util_verbose("Freeing lobby session client object... (Client: socket#%d)\n", session_client->client->client->socket);
    PKRSRV_REF_COUNTED_LEAVE(session_client->client);
    free(session_client);
}

void pkrsrv_lobby_session_clients_add(pkrsrv_lobby_session_clients_t* session_clients, pkrsrv_lobby_session_client_t* session_client) {
    PKRSRV_REF_COUNTED_USE(session_client);
    LIST_APPEND(session_clients, session_client);
}

void pkrsrv_lobby_session_clients_remove(pkrsrv_lobby_session_clients_t* session_clients, pkrsrv_lobby_session_client_t* session_client) {
    LIST_REMOVE(session_clients, session_client);
    PKRSRV_REF_COUNTED_LEAVE(session_client);
}

pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_session_new(pkrsrv_lobby_session_t* session) {
    PKRSRV_REF_COUNTED_USE(session);

    pkrsrv_lobby_client_session_t* client_session = malloc(sizeof(pkrsrv_lobby_client_session_t));
    PKRSRV_REF_COUNTED_INIT(client_session, pkrsrv_lobby_client_session_free);
    LIST_ITEM_INIT(client_session);    
    client_session->is_joined = false;
    client_session->session = session;

    return client_session;
}

void pkrsrv_lobby_client_session_free(pkrsrv_lobby_client_session_t* client_session) {
    pkrsrv_util_verbose("Freeing lobby client session object... (Session: %llu)\n", client_session->session->id.scalar);
    PKRSRV_REF_COUNTED_LEAVE(client_session->session);
    free(client_session);
}

void pkrsrv_lobby_client_sessions_add(pkrsrv_lobby_client_sessions_t* client_sessions, pkrsrv_lobby_client_session_t* client_session) {
    PKRSRV_REF_COUNTED_USE(client_session);
    LIST_APPEND(client_sessions, client_session);
}

void pkrsrv_lobby_client_sessions_remove(pkrsrv_lobby_client_sessions_t* client_sessions, pkrsrv_lobby_client_session_t* client_session) {
    LIST_REMOVE(client_sessions, client_session);
    PKRSRV_REF_COUNTED_LEAVE(client_session);
}

bool pkrsrv_lobby_client_enter_session(pkrsrv_lobby_client_t* lobby_client, pkrsrv_lobby_session_t* session) {
    bool retval = true;

    if (!lobby_client->account) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Client (Socket#%d) is trying to join table#%llu but not logged in...", lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }

    pkrsrv_lobby_session_client_t* existing_session_client = pkrsrv_lobby_session_client_getby_account_id(session, lobby_client->account->id);
    
    if (existing_session_client) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Client (Account#%llu, Socket#%d) is trying to enter table#%llu more than one...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }

    LIST_FOREACH(session->poker->players, player)
        if (player->account->id == lobby_client->account->id) {
            // ! Unexpected Behavior
            pkrsrv_util_verbose("Client (Account#%llu, Socket#%d) is trying to enter table#%llu more than one...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
            retval = false;
            goto RETURN;
        }
    END_FOREACH

    PKRSRV_REF_COUNTED_USE(session);

    pkrsrv_lobby_session_client_t* session_client = pkrsrv_lobby_session_client_new(lobby_client);
    pkrsrv_lobby_session_clients_add(session->clients, session_client);
    
    pkrsrv_lobby_client_session_t* client_session = pkrsrv_lobby_client_session_new(session);
    pkrsrv_lobby_client_sessions_add(lobby_client->sessions, client_session);

    pkrsrv_table_set_watchers_num(lobby_client->client->pg_conn, session->poker->table, session->poker->table->watchers_count + 1);

    RETURN:

    return retval;
}

bool pkrsrv_lobby_client_join_session(pkrsrv_lobby_client_t* lobby_client, pkrsrv_lobby_session_t* session, uint64_t enterance_amount, int position) {
    bool retval = true;

    if (!lobby_client->account) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Client (Socket#%d) is trying to join table#%llu but not logged in...", lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }
    
    pkrsrv_lobby_session_client_t* session_client = pkrsrv_lobby_session_client_getby_account_id(session, lobby_client->account->id);

    if (!session_client) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Client (Account#%llu, Socket#%d) is trying to join table#%llu more than one...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }

    pkrsrv_lobby_client_session_t* client_session = pkrsrv_lobby_client_session_getby_session_id(lobby_client, session->id.scalar);

    PKRSRV_UTIL_ASSERT(client_session);
    if (!client_session) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Unexpected Behavior: Client (Account#%llu, Socket#%d) is trying to join table#%llu but not entered...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }
    
    if (client_session->is_joined) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Unexpected Behavior: Client (Account#%llu, Socket#%d) is trying to join table#%llu more than one...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }

    session_client->is_joined = true;
    client_session->is_joined = true;
    session->joined_clients_count++;

    int old_players_length = session->poker->players->length;
    pkrsrv_poker_player_t* player = pkrsrv_poker_players_addby_account(session->poker, lobby_client->account, position);
    PKRSRV_UTIL_ASSERT(player);
    reorder_updated_session(session, old_players_length);
    pkrsrv_lobby_broadcast_sessions(lobby_client->lobby);

    pkrsrv_account_lock_balance(lobby_client->client->pg_conn, lobby_client->account, enterance_amount);

    player->enterance_balance = enterance_amount;
    player->balance = enterance_amount;
    
    pkrsrv_table_set_players_num(lobby_client->client->pg_conn, session->poker->table, session->poker->table->players_count + 1);

    if ((session->poker->players->length == 2) && (session->poker->state == PKRSRV_POKER_STATE_W4P)) {
        pkrsrv_poker_starting(session->poker);
        
        pkrsrv_lobby_session_poker_start__async_params_t* task_params = malloc(sizeof(pkrsrv_lobby_session_poker_start__async_params_t));
        task_params->session = session;
        session->poker_start_task = pkrsrv_eventloop_task_new(session->lobby->eventloop, pkrsrv_lobby_session_poker_start__async, task_params);
        PKRSRV_REF_COUNTED_USE(session->poker_start_task);
        pkrsrv_eventloop_task_call_after(session->lobby->eventloop, session->poker_start_task, 10000);
    }

    RETURN:

    return retval;
}

bool pkrsrv_lobby_client_unjoin_session(pkrsrv_lobby_client_t* lobby_client, pkrsrv_lobby_session_t* session) {
    bool retval = true;
    
    pkrsrv_lobby_session_client_t* session_client = pkrsrv_lobby_session_client_getby_socket(session, lobby_client->client->socket);

    if (!session_client) {
        // ! Unexpected Behavior
        pkrsrv_util_verbose("Client (Account#%llu, Socket#%d) is trying to unjoin table#%llu but session client doesn't exist...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);

        retval = false;
        goto RETURN;
    }

    pkrsrv_lobby_client_session_t* client_session = pkrsrv_lobby_client_session_getby_session_id(lobby_client, session->id.scalar);

    if (!client_session->is_joined) {
        pkrsrv_util_verbose("Client (Account#%llu, Socket#%d) is trying to unjoin table#%llu but already not joined...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }

    session_client->is_joined = false;
    client_session->is_joined = false;
    session->joined_clients_count--;

    pkrsrv_poker_player_t* player = pkrsrv_poker_players_getby_id(session->poker, lobby_client->account->id);

    if (!player) {
        pkrsrv_util_verbose("Client (Account#%llu, Socket#%d) is trying to unjoin table#%llu but player doesn't exist in poker machine...", lobby_client->account->id, lobby_client->client->socket, session->poker->table->id);
        retval = false;
        goto RETURN;
    }

    player->is_left = true;

    pkrsrv_account_fetch(lobby_client->client->pg_conn, lobby_client->account);
    
    bool result;

    pkrsrv_db_transaction_begin(lobby_client->client->pg_conn);

    result = pkrsrv_account_remove_locked_balance(lobby_client->client->pg_conn, lobby_client->account, player->enterance_balance);
    PKRSRV_UTIL_ASSERT(result);
    result = result && pkrsrv_account_add_balance(lobby_client->client->pg_conn, lobby_client->account, player->balance);
    PKRSRV_UTIL_ASSERT(result);

    if (result) {
        pkrsrv_db_transaction_commit(lobby_client->client->pg_conn);
    } else {
        pkrsrv_db_transaction_rollback(lobby_client->client->pg_conn);
    }

    if ((session->poker->state <= PKRSRV_POKER_STATE_STARTING) || (session->poker->state == PKRSRV_POKER_STATE_DONE)) {
        if (session->poker->state == PKRSRV_POKER_STATE_DONE) {
            PKRSRV_REF_COUNTED_USE(player);
        }
        int old_players_length = session->poker->players->length;
        pkrsrv_poker_players_remove(session->poker, player);
        reorder_updated_session(session, old_players_length);
        pkrsrv_lobby_broadcast_sessions(session->lobby);

        if ((session->poker->players->length == 1) && (session->poker->state <= PKRSRV_POKER_STATE_STARTING)) {
            session->poker->state = PKRSRV_POKER_STATE_W4P;
        }
    }

    pkrsrv_server_send_unjoined_params_t send_unjoined_params;
    send_unjoined_params.client = lobby_client->client;
    send_unjoined_params.table_id = session->poker->table->id.scalar;
    send_unjoined_params.reason = 0;

    pkrsrv_server_send_unjoined(send_unjoined_params);

    LIST_FOREACH(session->clients, session_client)
        pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(session->poker, session_client->client->account->id);

        pkrsrv_server_send_poker_info((pkrsrv_server_send_poker_info_params_t) {
            .client = session_client->client->client,
            .poker = session->poker
        });

        pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
            .client = session_client->client->client,
            .poker = session->poker,
            .player = receiver_player
        });
    END_FOREACH

    pkrsrv_table_set_players_num(lobby_client->client->pg_conn, session->poker->table, session->poker->table->players_count - 1);

    if ((session->joined_clients_count == 1) && session->poker_start_task) {
        PKRSRV_REF_COUNTED_LEAVE(session->poker_start_task);
        pkrsrv_eventloop_task_cancel(session->poker_start_task);
        session->poker_start_task = NULL;
    }
    if ((session->joined_clients_count == 1) && session->poker_restart_task) {
        PKRSRV_REF_COUNTED_LEAVE(session->poker_restart_task);
        pkrsrv_eventloop_task_cancel(session->poker_restart_task);
        session->poker_restart_task = NULL;
    }
    
    RETURN:
    
    return retval;
}

bool pkrsrv_lobby_client_leave_session(pkrsrv_lobby_client_t* lobby_client, pkrsrv_lobby_client_session_t* client_session) {
    pkrsrv_util_verbose("Client#%d is leaving session#%llu... pkrsrv_lobby_client_leave_session(lobby_client#%p, client_session#%p); session = %p;\n", lobby_client->client->socket, client_session->session->id.scalar, lobby_client, client_session, client_session->session);

    pkrsrv_lobby_t* lobby = client_session->session->lobby;
    pkrsrv_lobby_session_t* session = client_session->session;
    
    if (client_session->is_joined) {
        pkrsrv_lobby_client_unjoin_session(lobby_client, client_session->session);
    }
    
    pkrsrv_lobby_session_client_t* session_client = pkrsrv_lobby_session_client_getby_socket(client_session->session, lobby_client->client->socket);
    PKRSRV_UTIL_ASSERT(session_client);

    PKRSRV_REF_COUNTED_USE(client_session);
    PKRSRV_REF_COUNTED_USE(session_client);
    
    pkrsrv_lobby_session_clients_remove(client_session->session->clients, session_client);
    pkrsrv_lobby_client_sessions_remove(lobby_client->sessions, client_session);

    if (client_session->session->clients->length == 0) {
        pkrsrv_lobby_session_stop(client_session->session);
        pkrsrv_lobby_sessions_remove(lobby_client->lobby->sessions, client_session->session);
        pkrsrv_lobby_broadcast_sessions(lobby);
    }

    LIST_FOREACH(client_session->session->clients, session_client)
        pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(client_session->session->poker, session_client->client->account->id);

        pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
            .client = session_client->client->client,
            .poker = client_session->session->poker,
            .player = receiver_player
        });
    END_FOREACH

    pkrsrv_table_set_watchers_num(lobby_client->client->pg_conn, client_session->session->poker->table, client_session->session->poker->table->watchers_count - 1);

    PKRSRV_REF_COUNTED_LEAVE(session);
    PKRSRV_REF_COUNTED_LEAVE(client_session);
    PKRSRV_REF_COUNTED_LEAVE(session_client);

    return true;
}

pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_get_session_by_id(pkrsrv_lobby_client_t* client, uint64_t id) {
    LIST_FOREACH(client->sessions, client_session)
        if (client_session->session->id.scalar == id) {
            return client_session;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_session_getby_table_id(pkrsrv_lobby_client_t* client, uint64_t id) {
    LIST_FOREACH(client->sessions, client_session)
        if (client_session->session->poker->table->id.scalar == id) {
            return client_session;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_lobby_session_client_t* pkrsrv_lobby_session_client_getby_socket(pkrsrv_lobby_session_t* session, int socket) {
    LIST_FOREACH(session->clients, session_client)
        if (session_client->client->client->socket == socket) {
            return session_client;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_lobby_session_client_t* pkrsrv_lobby_session_client_getby_account_id(pkrsrv_lobby_session_t* session, uint64_t account_id) {
    LIST_FOREACH(session->clients, session_client)
        pkrsrv_server_client_t* client = session_client->client->client;
        pkrsrv_lobby_client_t* lobby_client = client->owner;
        
        if (lobby_client->account->id == account_id) {
            return session_client;
        }
    END_FOREACH

    return NULL;
}

pkrsrv_lobby_client_session_t* pkrsrv_lobby_client_session_getby_session_id(pkrsrv_lobby_client_t* lobby_client, uint64_t session_id) {
    LIST_FOREACH(lobby_client->sessions, client_session)
        if (client_session->session->id.scalar == session_id) {
            return client_session;
        }
    END_FOREACH

    return NULL;
}

static void on_poker_action(pkrsrv_eventloop_task_t* task) {
    pkrsrv_poker_on_action_params_t* task_params = (pkrsrv_poker_on_action_params_t *) task->params;
    pkrsrv_poker_t* poker = task_params->poker;
    pkrsrv_poker_player_t* player = task_params->player;
    pkrsrv_poker_action_t action = task_params->action;

    pkrsrv_lobby_session_t* session = (pkrsrv_lobby_session_t *) poker->owner;
    
    LIST_FOREACH(session->clients, session_client)
        PKRSRV_UTIL_ASSERT(session_client->client->account);
        
        pkrsrv_server_send_poker_action_reflection_params_t params;
        params.client = session_client->client->client;
        params.table_id = poker->table->id.scalar;
        params.account_id = action.account.id;
        params.action_kind = action.kind;
        params.amount = action.amount;
        
        pkrsrv_server_send_poker_action_reflection(params);

        pkrsrv_poker_player_t* receiver_player = pkrsrv_poker_players_getby_id(poker, session_client->client->account->id);
        
        pkrsrv_server_send_poker_state((pkrsrv_server_send_poker_state_params_t) {
            .client = session_client->client->client,
            .poker = poker,
            .player = receiver_player
        });
    END_FOREACH

    pkrsrv_poker_player_t* playing_player = pkrsrv_poker_players_getby_position(poker, poker->playing_position);
    
    if (playing_player->is_left && playing_player->is_playing) {
        pkrsrv_poker_action_t action;
        action.account = *player->account;
        action.kind = PKRSRV_POKER_ACTION_KIND_FOLD;
        action.amount = 0;

        pkrsrv_poker_do_action(poker, playing_player, action);
    }
}

bool reorder_updated_session(pkrsrv_lobby_session_t* updated, int old_length) {
    bool retval = true;
    
    PKRSRV_REF_COUNTED_USE(updated);
    
    if (updated->poker->players->length == old_length) {
        retval = false;
        goto RETURN;
    }

    pkrsrv_lobby_t* lobby = updated->lobby;

    pkrsrv_lobby_session_t* to_place = NULL;
    pkrsrv_lobby_session_t* loop_from = NULL;
    
    if ((updated->poker->players->length < old_length) && (updated != lobby->sessions->terminal)) {
        loop_from = updated;
        LIST_REMOVE(lobby->sessions, updated);
    } else if ((updated->poker->players->length > old_length) && (updated != lobby->sessions->next)) {
        loop_from = (pkrsrv_lobby_session_t *) lobby->sessions;
        LIST_REMOVE(lobby->sessions, updated);
    } else {
        retval = false;
        goto RETURN;
    }

    LIST_FOREACH(loop_from, to_compare)
        if (to_compare->poker->players->length <= updated->poker->players->length) {
            to_place = to_compare;
            break;
        }
    END_FOREACH

    if (to_place) {
        if (to_place->prev) {
            to_place->prev->next = updated;
        }
        updated->prev = to_place->prev;
        updated->next = to_place;
        to_place->prev = updated;
        if (lobby->sessions->next == to_place) {
            lobby->sessions->next = updated;
        }
    } else {
        updated->prev = lobby->sessions->terminal;
        updated->next = NULL;
        if (lobby->sessions->terminal) {
            lobby->sessions->terminal->next = updated;
        }
        lobby->sessions->terminal = updated;
        if (!lobby->sessions->next) {
            lobby->sessions->next = updated;
        }
    }
    lobby->sessions->length++;

    RETURN:

    PKRSRV_REF_COUNTED_LEAVE(updated);

    return true;
}

void pkrsrv_lobby_broadcast_sessions(pkrsrv_lobby_t* lobby) {
    pkrsrv_server_client_t* client = NULL;

    LIST_FOREACH(lobby->server->clients, client)
        if (!client->is_protocol_determined) {
            continue;
        }

        pkrsrv_poker_t* pokers[lobby->sessions->length];
        memset(pokers, 0, sizeof(void*) * lobby->sessions->length);
    
        LIST_FOREACH(lobby->sessions, session)
            pokers[session_i] = session->poker;
        END_FOREACH

        pkrsrv_server_send_sessions((pkrsrv_server_send_sessions_params_t) {
            .client = client,
            .offset = 0,
            .pokers_length = lobby->sessions->length,
            .pokers = pokers
        });
    END_FOREACH
}