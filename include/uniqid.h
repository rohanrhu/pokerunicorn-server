/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup uniqid Identifiers
 * \brief Unique ID generation and manipulation.
 */

/**
 * \addtogroup uniqid
 * \ingroup uniqid
 * @{
 */

#include <stdint.h>

typedef struct  {
    uint64_t scalar;
    char string[10];
} pkrsrv_uniqid_uuid_t;

/**
 * * Null-Terminated (20 + '\0')
 */
#define PKRSRV_UNIQID_STRING_LEN 20
#define PKRSRV_UNIQID_STRING_SIZE 21

pkrsrv_uniqid_uuid_t pkrsrv_uniqid_generate();
char* pkrsrv_uniqid_to_string(pkrsrv_uniqid_uuid_t uuid);
void pkrsrv_uniqid_string_free(char* uniqid);

static uint64_t pkrsrv_uniqid_uuid_i = 0;

/**
 * @}
 */