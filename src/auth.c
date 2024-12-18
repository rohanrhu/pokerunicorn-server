/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

#include "thirdparty/hiredis/hiredis.h"

#include "../include/ref.h"
#include "../include/random.h"
#include "../include/redis.h"
#include "../include/auth.h"

pkrsrv_auth_session_t* pkrsrv_auth_session_new(pkrsrv_account_t* account, pkrsrv_string_t* token) {
    PKRSRV_REF_COUNTED_USE(account);
    PKRSRV_REF_COUNTED_USE(token);

    pkrsrv_auth_session_t* session = malloc(sizeof(pkrsrv_auth_session_t));
    PKRSRV_REF_COUNTED_INIT(session, pkrsrv_auth_session_free);
    session->account = account;
    session->token = token;

    return session;
}

void pkrsrv_auth_session_free(pkrsrv_auth_session_t* session) {
    PKRSRV_REF_COUNTED_LEAVE(session->account);
    PKRSRV_REF_COUNTED_LEAVE(session->token);
    free(session);
}

pkrsrv_auth_session_t* pkrsrv_auth_session_create(pkrsrv_account_t* account) {
    pkrsrv_auth_session_t* session = pkrsrv_auth_session_new(account, pkrsrv_random_generate_token(32));

    pkrsrv_string_t* session_key = pkrsrv_string_format_new("session:%s", session->token->value);

    redisReply* reply = redisCommand(redis_connection, "SET %s %s", session_key->value, session->account->id_token->value);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Failed to store session in Redis: %s\n", reply ? reply->str : "No reply");
        pkrsrv_string_free(session_key);
        if (reply)
            freeReplyObject(reply);
        pkrsrv_auth_session_free(session);
        return NULL;
    }

    freeReplyObject(redisCommand(redis_connection, "EXPIRE %s %d", session_key->value, PKRSRV_AUTH_SESSION_EXPIRATION));

    pkrsrv_string_free(session_key);
    freeReplyObject(reply);

    return session;
}

bool pkrsrv_auth_session_destroy(pkrsrv_auth_session_t* session) {
    pkrsrv_string_t* session_key = pkrsrv_string_format_new("session:%s", session->token->value);

    redisReply* reply = redisCommand(redis_connection, "DEL %s", session_key->value);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Failed to delete session from Redis: %s\n", reply ? reply->str : "No reply");
        pkrsrv_string_free(session_key);
        if (reply)
            freeReplyObject(reply);
        return false;
    }

    pkrsrv_string_free(session_key);
    freeReplyObject(reply);

    pkrsrv_auth_session_free(session);

    return true;
}

pkrsrv_auth_session_t* pkrsrv_auth_session_getby_token(PGconn* pg_conn, pkrsrv_string_t* token) {
    pkrsrv_string_t* session_key = pkrsrv_string_format_new("session:%s", token->value);

    redisReply* reply = redisCommand(redis_connection, "GET %s", session_key->value);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Failed to get session from Redis: %s\n", reply ? reply->str : "No reply");
        pkrsrv_string_free(session_key);
        if (reply)
            freeReplyObject(reply);
        return NULL;
    }

    if (reply->type == REDIS_REPLY_NIL) {
        pkrsrv_string_free(session_key);
        freeReplyObject(reply);
        return NULL;
    }

    pkrsrv_account_t* account = pkrsrv_account_getby_id_token(pg_conn, reply->str);
    if (!account) {
        printf("Failed to get account by ID!\n");
        pkrsrv_string_free(session_key);
        freeReplyObject(reply);
        return NULL;
    }

    pkrsrv_string_free(session_key);
    freeReplyObject(reply);

    pkrsrv_auth_session_t* session = pkrsrv_auth_session_new(account, token);
    pkrsrv_auth_session_refresh(session);

    return session;
}

bool pkrsrv_auth_session_refresh(pkrsrv_auth_session_t* session) {
    pkrsrv_string_t* session_key = pkrsrv_string_format_new("session:%s", session->token->value);

    redisReply* reply = redisCommand(redis_connection, "EXPIRE %s %d", session_key->value, PKRSRV_AUTH_SESSION_EXPIRATION);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        printf("Failed to refresh session in Redis: %s\n", reply ? reply->str : "No reply");
        pkrsrv_string_free(session_key);
        if (reply)
            freeReplyObject(reply);
        return false;
    }

    pkrsrv_string_free(session_key);
    freeReplyObject(reply);

    return true;
}