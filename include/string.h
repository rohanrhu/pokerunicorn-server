/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup string Strings
 * \brief Ref-counted string object and utilities.
 */

#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>

#include "ref.h"
#include "util.h"

/**
 * \ingroup string
 * \implements pkrsrv_ref_counted
 * Ref-counted string object
 */
struct pkrsrv_string {
    char* value;
    unsigned int size;
    unsigned int length;
    bool is_alloc_str;
    bool is_binary;
    PKRSRV_REF_COUNTEDIFY();
};
typedef struct pkrsrv_string pkrsrv_string_t;

/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_string_new();

/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_string_new__n(int length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t pkrsrv_string_from_cstr__copy(char* cstr, ssize_t length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_string_new_from_cstr__copy(char* cstr, ssize_t length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_string_new_from_binary__copy(unsigned char* data, size_t length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t pkrsrv_string_from_cstr(char* cstr, ssize_t length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t pkrsrv_string_from_binary(unsigned char* data, ssize_t length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_string_new_from_cstr(char* cstr, ssize_t length);
/**
 * \memberof pkrsrv_string
 */
pkrsrv_string_t* pkrsrv_string_format_new(const char* format, ...);

/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_free(pkrsrv_string_t* string);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_set_value(pkrsrv_string_t* string, char* value);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_set_value__n(pkrsrv_string_t* string, char* value, size_t length);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_set_value__n__copy(pkrsrv_string_t* string, char* value, size_t length);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_append__cstr__n(pkrsrv_string_t* string, char* value, ssize_t length);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_append__cstr(pkrsrv_string_t* string, char* value);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_append__int(pkrsrv_string_t* string, int value);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_append__uint(pkrsrv_string_t* string, unsigned int value);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_append__uint64(pkrsrv_string_t* string, uint64_t value);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_append(pkrsrv_string_t* string, pkrsrv_string_t* other);

/**
 * \memberof pkrsrv_string
 */
bool pkrsrv_string_compare(pkrsrv_string_t* string, pkrsrv_string_t* other);

/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_set(pkrsrv_string_t* string, pkrsrv_string_t* other);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_set__copy(pkrsrv_string_t* string, pkrsrv_string_t* other);
/**
 * \memberof pkrsrv_string
 */
void pkrsrv_string_move(pkrsrv_string_t* string, pkrsrv_string_t* other);