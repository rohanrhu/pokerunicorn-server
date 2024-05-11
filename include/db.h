/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup db Database
 * \brief Database connection and operations.
 */

/**
 * \addtogroup db
 * \ingroup db
 * @{
 */

#include <stdbool.h>

#include <libpq-fe.h>

extern PGconn* db_connection;

void pkrsrv_db_init();
PGconn* pkrsrv_db_connect(char* host, int port, char* username, char* password, char* db_name);

bool pkrsrv_db_transaction_begin(PGconn* pg_conn);
bool pkrsrv_db_transaction_commit(PGconn* pg_conn);
bool pkrsrv_db_transaction_rollback(PGconn* pg_conn);

/**
 * @}
 */