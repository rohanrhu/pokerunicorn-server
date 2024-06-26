/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

/**
 * \defgroup random Chaos
 * \brief RNG things.
 */

/**
 * \addtogroup random
 * \ingroup random
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/random.h"

#include "../include/card.h"

int pkrsrv_random_range(int min, int max) {
    unsigned int seed;
    
    FILE* file = fopen("/dev/random", "r");
    fread(&seed, 4, 1, file);
    fclose(file);
    
    srand(seed);
    return min + (rand() % max - min);
}

void pkrsrv_random_bytes(unsigned char* array, int length) {
    FILE* file = fopen("/dev/random", "r");
    fread(array, length, 1, file);
    fclose(file);
}

/**
 * @}
 */