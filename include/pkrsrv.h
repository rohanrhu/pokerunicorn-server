/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

#include <stddef.h>

#include <libpq-fe.h>
#include "thirdparty/hiredis/hiredis.h"

#include "arg.h"
#include "string.h"

#define PKRSRV_STR_HELPER(x) #x
#define PKRSRV_STR(x) PKRSRV_STR_HELPER(x)

#ifdef __GNUC__
    #define PKRSRV_COMPILER_INFO "GCC " PKRSRV_STR(__GNUC__) "." PKRSRV_STR(__GNUC_MINOR__) "." PKRSRV_STR(__GNUC_PATCHLEVEL__)
#elif defined(__clang__)
    #define PKRSRV_COMPILER_INFO "Clang " PKRSRV_STR(__clang_major__) "." PKRSRV_STR(__clang_minor__) "." PKRSRV_STR(__clang_patchlevel__)
#elif defined(_MSC_VER)
    #define PKRSRV_COMPILER_INFO "MSVC " PKRSRV_STR(_MSC_VER)
#else
    #define PKRSRV_COMPILER_INFO "Unknown Compiler"
#endif

#define PKRSRV_VERSION {0, 10, 0}
#define PKRSRV_VERSION_STABILITY "git"
#define PKRSRV_VERSION_STRING "v0.10.0-git"

#ifndef PKRSRV_VERBOSE
#define PKRSRV_VERBOSE 1
#endif

#ifndef PKRSRV_BUILD
#define PKRSRV_BUILD 0
#endif

#ifndef PKRSRV_REVISION
#define PKRSRV_REVISION "__no_rev__"
#endif

#ifndef PKRSRV_BUILD_DATE
#define PKRSRV_BUILD_DATE ""
#endif

#ifndef PKRSRV_PORT
#define PKRSRV_PORT 5560
#endif

#ifndef PKRSRV_BIND_ADDRESS
#define PKRSRV_BIND_ADDRESS "0.0.0.0"
#endif

#ifndef PKRSRV_REDIS_HOST
#define PKRSRV_REDIS_HOST "redis"
#endif

#ifndef PKRSRV_REDIS_PORT
#define PKRSRV_REDIS_PORT 6379
#endif

#ifndef PKRSRV_POSTGRES_HOST
#define PKRSRV_POSTGRES_HOST "postgres"
#endif

#ifndef PKRSRV_POSTGRES_PORT
#define PKRSRV_POSTGRES_PORT 5432
#endif

#ifndef PKRSRV_POSTGRES_USERNAME
#define PKRSRV_POSTGRES_USERNAME "meowingcat"
#endif

#ifndef PKRSRV_POSTGRES_PASSWORD
#define PKRSRV_POSTGRES_PASSWORD "meow"
#endif

#ifndef PKRSRV_POSTGRES_DB
#define PKRSRV_POSTGRES_DB "poker"
#endif

#ifndef PKRSRV_SSL_KEY_FILE
#define PKRSRV_SSL_KEY_FILE "ssl/test.key"
#endif

#ifndef PKRSRV_SSL_CERT_FILE
#define PKRSRV_SSL_CERT_FILE "ssl/test.crt"
#endif

#ifndef PKRSRV_AVATAR_MAX_SIZE
#define PKRSRV_AVATAR_MAX_SIZE 1000000 // 1 MB
#endif

#ifndef PKRSRV_PROCESS_LATENCY
#define PKRSRV_PROCESS_LATENCY 1000000 // 1 second
#endif

#ifndef PKRSRV_MONERO_WALLET_FILENAME
#define PKRSRV_MONERO_WALLET_FILENAME "test-wallet"
#endif

#ifndef PKRSRV_MONERO_WALLET_PASSWORD
#define PKRSRV_MONERO_WALLET_PASSWORD "meow"
#endif

#ifndef PKRSRV_MAX_CLIENTS
#define PKRSRV_MAX_CLIENTS 200
#endif

#ifndef PKRSRV_USE_REDIS
#define PKRSRV_USE_REDIS 0
#endif

extern int __pkrsrv_version__[];
extern char* __pkrsrv_version_string__;
extern char* __pkrsrv_version_stability__;
extern char* __pkrsrv_revision__;
extern char* __pkrsrv_compiler__;
extern int __pkrsrv_util_verbose__;
extern char* __pkrsrv_build_date__;

extern uint64_t pkrsrv_build_number;
extern pkrsrv_string_t* pkrsrv_version;
extern pkrsrv_string_t* pkrsrv_revision;
extern pkrsrv_string_t* pkrsrv_compiler;

extern char* pkrsrv_redis_host;
extern int pkrsrv_redis_port;

extern char* pkrsrv_postgres_host;
extern int pkrsrv_postgres_port;
extern char* pkrsrv_postgres_username;
extern char* pkrsrv_postgres_password;
extern char* pkrsrv_postgres_db;

extern char* pkrsrv_ssl_key_file;
extern char* pkrsrv_ssl_cert_file;

extern int pkrsrv_process_latency;

extern char* monero_wallet_filename;
extern char* monero_wallet_password;

extern PGconn* pkrsrv_db_conn;

void arg_handler_help(char* value);
void arg_handler_version(char* value);
void arg_handler_max_clients(char* value);
void arg_handler_tests(char* value);
void arg_handler_port(char* value);
void arg_handler_bind(char* value);
void arg_handler_redis_host(char* value);
void arg_handler_redis_port(char* value);
void arg_handler_postgres_host(char* value);
void arg_handler_postgres_port(char* value);
void arg_handler_postgres_username(char* value);
void arg_handler_postgres_password(char* value);
void arg_handler_postgres_db(char* value);
void arg_handler_ssl_key_file(char* value);
void arg_handler_ssl_cert_file(char* value);
void arg_handler_process_latency(char* value);
void arg_handler_monero_wallet_filename(char* value);
void arg_handler_monero_wallet_password(char* value);

static void* arguments[] = {
    "--help", "-h", PKRSRV_ARG_WITHOUT_VALUE, arg_handler_help,
    "--version", "-v", PKRSRV_ARG_WITHOUT_VALUE, arg_handler_version,
    "--max-clients", NULL, PKRSRV_ARG_WITH_VALUE, arg_handler_max_clients,
    "--tests", NULL, PKRSRV_ARG_WITHOUT_VALUE, arg_handler_tests,
    "--port", "-p", PKRSRV_ARG_WITH_VALUE, arg_handler_port,
    "--bind", "-b", PKRSRV_ARG_WITH_VALUE, arg_handler_bind,
    "--redis-host", "-rh", PKRSRV_ARG_WITH_VALUE, arg_handler_redis_host,
    "--redis-port", "-rp", PKRSRV_ARG_WITH_VALUE, arg_handler_redis_port,
    "--postgres-host", "-ph", PKRSRV_ARG_WITH_VALUE, arg_handler_postgres_host,
    "--postgres-port", "-pp", PKRSRV_ARG_WITH_VALUE, arg_handler_postgres_port,
    "--postgres-username", "-pu", PKRSRV_ARG_WITH_VALUE, arg_handler_postgres_username,
    "--postgres-password", "-ps", PKRSRV_ARG_WITH_VALUE, arg_handler_postgres_password,
    "--postgres-db", "-pp", PKRSRV_ARG_WITH_VALUE, arg_handler_postgres_db,
    "--ssl-key-file", "-sk", PKRSRV_ARG_WITH_VALUE, arg_handler_ssl_key_file,
    "--ssl-cert-file", "-sc", PKRSRV_ARG_WITH_VALUE, arg_handler_ssl_cert_file,
    "--process-latency", "-pl", PKRSRV_ARG_WITH_VALUE, arg_handler_process_latency,
    "--monero-wallet-filename", "-mwf", PKRSRV_ARG_WITH_VALUE, arg_handler_monero_wallet_filename,
    "--monero-wallet-password", "-mwp", PKRSRV_ARG_WITH_VALUE, arg_handler_monero_wallet_password,
    NULL
};