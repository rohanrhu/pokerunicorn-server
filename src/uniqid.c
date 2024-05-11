/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "../include/uniqid.h"
#include "../include/util.h"
#include "../include/random.h"

pkrsrv_uniqid_uuid_t pkrsrv_uniqid_generate() {
    unsigned char seed[8];
    pkrsrv_random_bytes(seed, 7);
    seed[7] = 0;

    
    pkrsrv_uniqid_uuid_t uniqid;
    uniqid.scalar = (uint64_t) (*((uint64_t *) seed));
    
    return uniqid;
}

char* pkrsrv_uniqid_to_string(pkrsrv_uniqid_uuid_t uuid) {
    char* uuid_str = malloc(PKRSRV_UNIQID_STRING_SIZE);
    uuid_str[PKRSRV_UNIQID_STRING_LEN] = '\0';

    return uuid_str;
}

void pkrsrv_uniqid_string_free(char* uniqid) {
    free(uniqid);
}