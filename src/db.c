/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <libpq-fe.h>

#include "../include/db.h"
#include "../include/util.h"

PGconn* db_connection = NULL;

void pkrsrv_db_init() {
    db_connection = NULL;
}

PGconn* pkrsrv_db_connect(char* host, int port, char* username, char* password, char* db_name) {
    char connection_string[200];
    sprintf(connection_string, "postgresql://%s:%s@%s?port=%d&dbname=%s", username, password, host, port, db_name);
    
    db_connection = PQconnectdb((char *) connection_string);

    if (PQstatus(db_connection) != CONNECTION_OK) {
        printf("[Error] Postgres connection failed:\n");
        printf("  %s\n", PQerrorMessage(db_connection));
        exit(1);
    }

    pkrsrv_util_verbose("Postgres connection is established!\n");
    
    return db_connection;
}

bool pkrsrv_db_transaction_begin(PGconn* pg_conn) {
    PGresult* result = PQexec(pg_conn, "BEGIN");
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        printf("[Error] Postgres transaction begin failed:\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
        PQclear(result);
        return false;
    }
    PQclear(result);
    return true;
}

bool pkrsrv_db_transaction_commit(PGconn* pg_conn) {
    PGresult* result = PQexec(pg_conn, "COMMIT");
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        printf("[Error] Postgres transaction commit failed:\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
        PQclear(result);
        return false;
    }
    PQclear(result);
    return true;
}

bool pkrsrv_db_transaction_rollback(PGconn* pg_conn) {
    PGresult* result = PQexec(pg_conn, "ROLLBACK");
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        printf("[Error] Postgres transaction rollback failed:\n");
        printf("  %s\n", PQerrorMessage(pg_conn));
        PQclear(result);
        return false;
    }
    PQclear(result);
    return true;
}