/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "../include/random.h"
#include "../include/util.h"

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

pkrsrv_string_t* pkrsrv_random_generate_token(int length) {
    unsigned char* random_bytes = malloc(length);
    pkrsrv_random_bytes(random_bytes, length);
    
    unsigned char* encoded = pkrsrv_util_base64_encode(random_bytes, length);
    int encoded_length = strlen((char *) encoded);
    
    pkrsrv_string_t* token = pkrsrv_string_new__n(encoded_length);
    memcpy(token->value, encoded, encoded_length);
    token->length = encoded_length;
    token->value[encoded_length] = '\0';
    
    free(random_bytes);
    free(encoded);

    return token;
}