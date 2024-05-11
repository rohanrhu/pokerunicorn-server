/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "../include/util.h"

const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int base64_mod_table[] = {0, 2, 1};
const char base64_rev_table[128] = {
    ['A'] = 0, ['B'] = 1, ['C'] = 2, ['D'] = 3, ['E'] = 4, ['F'] = 5, ['G'] = 6, ['H'] = 7,
    ['I'] = 8, ['J'] = 9, ['K'] = 10, ['L'] = 11, ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15,
    ['Q'] = 16, ['R'] = 17, ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29, ['e'] = 30, ['f'] = 31,
    ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39,
    ['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
    ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55,
    ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59, ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63
};

void pkrsrv_util_verbose(const char* format, ...) {
    if (!is_verbose) {
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void pkrsrv_util_verbose_set(int p_is_verbose) {
    is_verbose = p_is_verbose;
}

int pkrsrv_util_msleep(long millis) {
    struct timespec ts;
    int result;

    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;

    result = nanosleep(&ts, &ts);

    return result;
}

uint64_t pkrsrv_util_get_time_msec() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    return current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
}

int pkrsrv_util_int2str(int number, char* target) {
    int current = number;
    int str_i = 0;

    char digits[11] = {'\0'};
    
    int length = 0;

    for (int i=9;; i--) {
        int modulo = current % 10;
        current = current / 10;

        digits[i] = '0' + modulo;
        length++;

        if (current < 1) {
            break;
        }
    }

    int j = 0;
    
    for (int i=9; i > 0; i--) {
        target[j++] = digits[i];
    }

    return length;
}

unsigned char* pkrsrv_util_base64_encode(unsigned char* data, int length) {
    int len_encoded = (length + 2) / 3 * 4;
    unsigned char* encoded = malloc(len_encoded + 1);
    if (encoded == NULL) return NULL;
    encoded[len_encoded] = '\0';

    for (int i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? data[i++] : 0;
        uint32_t octet_b = i < length ? data[i++] : 0;
        uint32_t octet_c = i < length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < base64_mod_table[length % 3]; i++)
        encoded[len_encoded - 1 - i] = '=';

    return encoded;
}

unsigned char* pkrsrv_util_base64_decode(unsigned char* data, int length) {
    int output_length = length / 4 * 3;
    if (data[length - 1] == '=') --output_length;
    if (data[length - 2] == '=') --output_length;

    char *output = malloc(output_length + 1);
    if (output == NULL) return NULL;

    for (int i = 0, j = 0; i < length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : base64_rev_table[(int)data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : base64_rev_table[(int)data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : base64_rev_table[(int)data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : base64_rev_table[(int)data[i++]];
        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < output_length) output[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < output_length) output[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < output_length) output[j++] = (triple >> 0 * 8) & 0xFF;
    }

    output[output_length] = '\0';
    return output;
}

void pkrsrv_util_assert_fail() {
    printf("pkrsrv_util_assert_fail()\n");
}