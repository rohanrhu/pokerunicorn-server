/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdio.h>

#include <openssl/opensslv.h>

#include <libpq-fe.h>

#include "thirdparty/hiredis/hiredis.h"

#include "../thirdparty/lua/lua.h"
#include "../thirdparty/lua/lualib.h"
#include "../thirdparty/lua/lauxlib.h"

#include "../include/pkrsrv.h"

#include "../include/sugar.h"
#include "../include/arg.h"
#include "../include/string.h"
#include "../include/table.h"
#include "../include/account.h"
#include "../include/poker.h"
#include "../include/card.h"
#include "../include/lobby.h"

#include "../include/redis.h"
#include "../include/db.h"

#include "../include/deposit.h"

#include "../tests/rsa/rsa.h"

int __pkrsrv_version__[] = PKRSRV_VERSION;
char* __pkrsrv_version_string__ = PKRSRV_VERSION_STRING;
char* __pkrsrv_version_stability__ = PKRSRV_VERSION_STABILITY;
char* __pkrsrv_revision__ = PKRSRV_REVISION;
char* __pkrsrv_compiler__ = PKRSRV_COMPILER_INFO;
int __pkrsrv_util_verbose__ = PKRSRV_VERBOSE;
char* __pkrsrv_build_date__ = PKRSRV_BUILD_DATE;

uint64_t pkrsrv_build_number = 0;
pkrsrv_string_t* pkrsrv_version = NULL;
pkrsrv_string_t* pkrsrv_revision = NULL;
pkrsrv_string_t* pkrsrv_compiler = NULL;

int pkrsrv_port = PKRSRV_PORT;
char* pkrsrv_bind_address = PKRSRV_BIND_ADDRESS;
int pkrsrv_max_clients = PKRSRV_MAX_CLIENTS;

char* pkrsrv_redis_host = PKRSRV_REDIS_HOST;
int pkrsrv_redis_port = PKRSRV_REDIS_PORT;

char* pkrsrv_postgres_host = PKRSRV_POSTGRES_HOST;
int pkrsrv_postgres_port = PKRSRV_POSTGRES_PORT;
char* pkrsrv_postgres_username = PKRSRV_POSTGRES_USERNAME;
char* pkrsrv_postgres_password = PKRSRV_POSTGRES_PASSWORD;
char* pkrsrv_postgres_db = PKRSRV_POSTGRES_DB;

char* pkrsrv_ssl_key_file = PKRSRV_SSL_KEY_FILE;
char* pkrsrv_ssl_cert_file = PKRSRV_SSL_CERT_FILE;

int pkrsrv_process_latency = PKRSRV_PROCESS_LATENCY;

char* pkrsrv_monero_wallet_filename = PKRSRV_MONERO_WALLET_FILENAME;
char* pkrsrv_monero_wallet_password = PKRSRV_MONERO_WALLET_PASSWORD;

PGconn* pkrsrv_db_conn = NULL;

int main(int argc, char* argv[]) {
    pkrsrv_build_number = PKRSRV_BUILD;
    pkrsrv_version = pkrsrv_string_new_from_cstr(__pkrsrv_version_string__, strlen(__pkrsrv_version_string__));
    pkrsrv_revision = pkrsrv_string_new_from_cstr(__pkrsrv_revision__, strlen(__pkrsrv_revision__));
    pkrsrv_compiler = pkrsrv_string_new_from_cstr(__pkrsrv_compiler__, strlen(__pkrsrv_compiler__));
    
    pkrsrv_arg_t* arg;
    arg = pkrsrv_arg_init(argv, argc, (void **) arguments);
    pkrsrv_arg_handle(arg);

    printf("Meowing Poker Server... (%s, Build: %d, Revision: %s)\n", PKRSRV_VERSION_STRING, PKRSRV_BUILD, PKRSRV_REVISION);
    printf("Build Info: %s\n", PKRSRV_COMPILER_INFO);

    if (PKRSRV_USE_REDIS) {
        pkrsrv_redis_init();
        pkrsrv_redis_connect(pkrsrv_redis_host, pkrsrv_redis_port);
    }

    INIT_DB:

    pkrsrv_db_init();
    pkrsrv_db_conn = pkrsrv_db_connect(pkrsrv_postgres_host, pkrsrv_postgres_port, pkrsrv_postgres_username, pkrsrv_postgres_password, pkrsrv_postgres_db);

    printf("Recovering locked balances...\n");

    pkrsrv_account_revert_locked_balanes(pkrsrv_db_conn);

    pkrsrv_deposit_service_t* deposit_service = pkrsrv_deposit_service_new();

    pkrsrv_string_t* main_address;
    bool result;

    CHECK_WALLET:

    main_address = pkrsrv_deposit_monero_get_main_address();

    if (!main_address) {
        printf("Couldn't reach to monero-wallet-rpc. Trying to open wallet...\n");
        goto OPEN_WALLET;
    }

    pkrsrv_string_free(main_address);

    OPEN_WALLET:

    result = pkrsrv_deposit_monero_wallet_open(pkrsrv_monero_wallet_filename, pkrsrv_monero_wallet_password);
    if (!result) {
        printf("Monero wallet is not found. Creating a new wallet...\n");
        goto CREATE_WALLET;
    }

    goto START_DEPOSIT;

    CREATE_WALLET:

    result = pkrsrv_deposit_monero_wallet_create(pkrsrv_monero_wallet_filename, pkrsrv_monero_wallet_password);
    if (!result) {
        printf("Monero wallet creation failed! Re-trying in 5 seconds...\n");
        pkrsrv_util_msleep(5000);
        goto CHECK_WALLET;
    }

    goto OPEN_WALLET;

    START_DEPOSIT:
    
    pkrsrv_deposit_service_start(deposit_service);

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    pkrsrv_lobby_t* lobby = pkrsrv_lobby_new((pkrsrv_lobby_new_params_t) {
        .port = pkrsrv_port,
        .bind_address = pkrsrv_bind_address,
        .process_latency = pkrsrv_process_latency,
        .max_clients = pkrsrv_max_clients
    });

    PKRSRV_REF_COUNTED_USE(lobby);

    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);

    char *code = "print('[Lua] Lua scripting is enabled!')";

    if (luaL_loadstring(lua, code) == LUA_OK) {
        if (lua_pcall(lua, 0, 0, 0) == LUA_OK) {
            lua_pop(lua, lua_gettop(lua));
        }
    }

    lua_close(lua);
    
    pkrsrv_lobby_run(lobby);

    PKRSRV_REF_COUNTED_LEAVE(lobby);

    lua_close(lua);
    
    return 0;
}

void arg_handler_max_clients(char* value) {
    pkrsrv_max_clients = atoi(value);
}

void arg_handler_port(char* value) {
    pkrsrv_port = atoi(value);
}

void arg_handler_bind(char* value) {
    pkrsrv_bind_address = value;
}

void arg_handler_redis_host(char* value) {
    pkrsrv_redis_host = value;
}

void arg_handler_redis_port(char* value) {
    pkrsrv_redis_port = atoi(value);
}

void arg_handler_postgres_host(char* value) {
    pkrsrv_postgres_host = value;
}

void arg_handler_postgres_port(char* value) {
    pkrsrv_postgres_port = atoi(value);
}

void arg_handler_postgres_username(char* value) {
    pkrsrv_postgres_username = value;
}

void arg_handler_postgres_password(char* value) {
    pkrsrv_postgres_password = value;
}

void arg_handler_postgres_db(char* value) {
    pkrsrv_postgres_db = value;
}

void arg_handler_ssl_key_file(char* value) {
    pkrsrv_ssl_key_file = value;
}

void arg_handler_ssl_cert_file(char* value) {
    pkrsrv_ssl_cert_file = value;
}

void arg_handler_process_latency(char* value) {
    pkrsrv_process_latency = atoi(value);
}

void arg_handler_monero_wallet_filename(char* value) {
    pkrsrv_monero_wallet_filename = value;
}

void arg_handler_monero_wallet_password(char* value) {
    pkrsrv_monero_wallet_password = value;
}

void arg_handler_help(char* value) {
    printf("PKRSRV (%s) - Meowing Poker Game-Server\n\n", PKRSRV_VERSION_STRING);
    printf("Usage: pkrsrv [option(=value), ...]? command?\n\n");
    printf("Options:\n");
    printf("  --help, -h:\t\t\t\tShows this help text.\n");
    printf("  --version, -v:\t\t\tShows PKRSRV versions.\n");
    printf("  --max-clients:\t\t\tSets maximum clients. (Default: %d)\n", PKRSRV_MAX_CLIENTS);
    printf("  --tests:\t\t\t\tRuns all tests.\n");
    printf("  --port, -p:\t\t\t\tSets server port. (Default: %d)\n", PKRSRV_PORT);
    printf("  --bind, -b:\t\t\t\tSets server bind address. (Default: %s)\n", PKRSRV_BIND_ADDRESS);
    printf("  --redis-host, -rh:\t\t\tSets Redis host. (Default: %s)\n", PKRSRV_REDIS_HOST);
    printf("  --redis-port, -rp:\t\t\tSets Redis port. (Default: %d)\n", PKRSRV_REDIS_PORT);
    printf("  --postgres-host, -ph:\t\t\tSets Postgres host. (Default: %s)\n", PKRSRV_POSTGRES_HOST);
    printf("  --postgres-port, -pp:\t\t\tSets Postgres port. (Default: %d)\n", PKRSRV_POSTGRES_PORT);
    printf("  --postgres-username, -pp:\t\tSets Postgres username. (Default: %s)\n", PKRSRV_POSTGRES_USERNAME);
    printf("  --postgres-password, -ps:\t\tSets Postgres password. (Default: %s)\n", PKRSRV_POSTGRES_PASSWORD);
    printf("  --postgres-db, -rp:\t\t\tSets Postgres database name. (Default: %s)\n", PKRSRV_POSTGRES_DB);
    printf("  --ssl-key-file, -sk:\t\t\tSpecifies SSL private key file path. (Default: %s)\n", PKRSRV_SSL_KEY_FILE);
    printf("  --ssl-cert-file, -sc:\t\t\tSpecifies SSL private certificate file path. (Default: %s)\n", PKRSRV_SSL_CERT_FILE);
    printf("  --process-latency, -pl:\t\tSets process latency.. (Default: %d)\n", PKRSRV_PROCESS_LATENCY);
    printf("  --monero-wallet-filename, -mwf:\tSets Monero wallet filename. (Default: %s)\n", PKRSRV_MONERO_WALLET_FILENAME);
    printf("  --monero-wallet-password, -mwp:\tSets Monero wallet password. (Default: %s)\n", PKRSRV_MONERO_WALLET_PASSWORD);
    exit(0);
}

void arg_handler_version(char* value) {
    printf("PokerServer %s, build %d (built at %s)\n\n", PKRSRV_VERSION_STRING, PKRSRV_BUILD, PKRSRV_BUILD_DATE);
    
    printf("Build Flags:\n");
    printf("  VERBOSE:\t\t\t%d\n", PKRSRV_VERBOSE);
    printf("  VERSION:\t\t\t%s\n", PKRSRV_VERSION_STRING);
    printf("  MAX_CLIENTS:\t\t%d\n", PKRSRV_MAX_CLIENTS);
    printf("  BUILD:\t\t\t%d\n", PKRSRV_BUILD);
    printf("  REVISION:\t\t\t%s\n", PKRSRV_REVISION);
    printf("  COMPILER_INFO:\t\t%s\n", PKRSRV_COMPILER_INFO);
    printf("  BUILD_DATE:\t\t\t%s\n", PKRSRV_BUILD_DATE);
    printf("  REDIS_HOST:\t\t%s\n", PKRSRV_REDIS_HOST);
    printf("  REDIS_PORT:\t\t%d\n", PKRSRV_REDIS_PORT);
    printf("  POSTGRES_HOST:\t%s\n", PKRSRV_POSTGRES_HOST);
    printf("  POSTGRES_PORT:\t%d\n", PKRSRV_POSTGRES_PORT);
    printf("  POSTGRES_USERNAME:\t%s\n", PKRSRV_POSTGRES_USERNAME);
    printf("  POSTGRES_PASSWORD:\t%s\n", PKRSRV_POSTGRES_PASSWORD);
    printf("  POSTGRES_DB:\t\t%s\n", PKRSRV_POSTGRES_DB);
    printf("  SSL_KEY_FILE:\t\t%s\n", PKRSRV_SSL_KEY_FILE);
    printf("  SSL_CERT_FILE:\t%s\n", PKRSRV_SSL_CERT_FILE);
    printf("  PROCESS_LATENCY:\t%d\n", PKRSRV_PROCESS_LATENCY);
    
    printf("\nThird-Party Libraries:\n");

    printf("\nOpenSSL:\n");
    printf("  OPENSSL_VERSION_TEXT:\t%s\n", OPENSSL_VERSION_TEXT);

    printf("libpq:\n");
    printf("  PQlibVersion():\t\t%d\n", PQlibVersion());

    printf("Hiredis::\n");
    printf("  HIREDIS_SONAME:\t\t%s\n", TOSTRING(HIREDIS_SONAME));

    printf("Lua:\n");
    printf("  LUA_RELEASE:\t\t\t%s\n", LUA_RELEASE);
    
    exit(0);
}

void arg_handler_tests(char* value) {
    printf("Running tests...\n");

    bool result;

    result = test_rsa();

    if (result) {
        printf("RSA test passed!\n");
    } else {
        printf("RSA test failed!\n");
    }
    
    exit(0);
}