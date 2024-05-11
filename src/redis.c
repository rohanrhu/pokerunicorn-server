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

#include "thirdparty/hiredis/hiredis.h"

#include "../include/redis.h"
#include "../include/util.h"

redisContext* redis_connection = NULL;

void pkrsrv_redis_init() {
    redis_connection = NULL;
}

redisContext* pkrsrv_redis_connect(char* host, int port) {
    redisOptions redis_options = {0};
    REDIS_OPTIONS_SET_TCP(&redis_options, host, port);
    
    redis_connection = redisConnectWithOptions(&redis_options);
    if (!redis_connection || redis_connection->err) {
        printf("[Error] Redis connection failed\n");
        PKRSRV_UTIL_ASSERT(redis_connection && !redis_connection->err);
        return NULL;
    }

    pkrsrv_util_verbose("Redis connection is established!\n");

    return redis_connection;
}