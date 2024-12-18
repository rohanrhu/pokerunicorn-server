/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup auth Authentication & Session
 * \brief Authentication and session management.
 */

#include <stdbool.h>
#include <libpq-fe.h>

#include "sugar.h"
#include "ref.h"
#include "string.h"
#include "account.h"

/**
 * \addtogroup auth
 * \ingroup auth
 * @{
 */

/**
 * \brief Default session expiration time in seconds
 */
#define PKRSRV_AUTH_SESSION_EXPIRATION 3600 // 1 hour

typedef struct pkrsrv_auth_session pkrsrv_auth_session_t;

/**
 * \implements pkrsrv_ref_counted
 * \brief Authentication session object
 */
struct pkrsrv_auth_session {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_string_t* token;
    pkrsrv_account_t* account;
};

/**
 * \brief Creates a new authentication session.
 *
 * This function creates a new authentication session object.
 *
 * \param params The session creation parameters.
 * \return A pointer to the newly created authentication session.
 */
pkrsrv_auth_session_t* pkrsrv_auth_session_new(pkrsrv_account_t* account, pkrsrv_string_t* token);

/**
 * \brief Frees the memory occupied by an authentication session.
 *
 * This function frees the memory occupied by the given authentication session object.
 *
 * \param session The authentication session to be freed.
 */
void pkrsrv_auth_session_free(pkrsrv_auth_session_t* session);

/**
 * \brief Creates a new authentication session.
 * 
 * This function creates a new authentication session object.
 * 
 * \param params The session creation parameters.
 */
pkrsrv_auth_session_t* pkrsrv_auth_session_create(pkrsrv_account_t* account);

/**
 * \brief Destroys an authentication session.
 * 
 * This function destroys an authentication session object.
 * 
 * \param session The session to be destroyed.
 * \return True if the session is destroyed successfully, false otherwise.
 */
bool pkrsrv_auth_session_destroy(pkrsrv_auth_session_t* session);

/**
 * \brief Returns the session by given token.
 * 
 * This function returns the session object by the given token.
 * 
 * \param token The token of the session.
 * \return The session object.
 */
pkrsrv_auth_session_t* pkrsrv_auth_session_getby_token(PGconn* pg_conn, pkrsrv_string_t* token);

/**
 * \brief Refreshes the session expiration time.
 * 
 * This function refreshes the expiration time of the given session.
 * 
 * \param session The session to be refreshed.
 */
bool pkrsrv_auth_session_refresh(pkrsrv_auth_session_t* session);

/**
 * @}
 */