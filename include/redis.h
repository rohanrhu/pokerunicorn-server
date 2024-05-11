/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

#include <stddef.h>

#include "thirdparty/hiredis/hiredis.h"

void pkrsrv_redis_init();
redisContext* pkrsrv_redis_connect(char* host, int port);

extern redisContext* redis_connection;