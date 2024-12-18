/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

#include "string.h"

/**
 * \defgroup random Chaos
 * \brief RNG things.
 */

/**
 * \addtogroup random
 * \ingroup random
 */

int pkrsrv_random_range(int min, int max);
void pkrsrv_random_bytes(unsigned char* array, int length);

/**
 * \brief Generates a random token.
 * 
 * Generates a portable random token.
 * 
 * \param length The length of the token.
 */
pkrsrv_string_t* pkrsrv_random_generate_token(int length);

/**
 * @}
 */