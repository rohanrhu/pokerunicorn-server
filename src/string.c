/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../include/string.h"

pkrsrv_string_t* pkrsrv_string_new() {
    pkrsrv_string_t* string = malloc(sizeof(pkrsrv_string_t));
    PKRSRV_REF_COUNTED_INIT(string, pkrsrv_string_free);
    string->length = 0;
    string->size = string->length + 1;
    string->is_alloc_str = true;
    string->is_binary = false;
    string->value = malloc(sizeof(1));
    *string->value = '\0';
    
    return string;
}

pkrsrv_string_t* pkrsrv_string_new__n(int length) {
    pkrsrv_string_t* string = malloc(sizeof(pkrsrv_string_t));
    PKRSRV_REF_COUNTED_INIT(string, pkrsrv_string_free);
    string->length = 0;
    string->size = string->length + 1;
    string->is_alloc_str = true;
    string->is_binary = false;
    string->value = malloc(length+1);
    string->value[length] = '\0';
    
    return string;
}

pkrsrv_string_t pkrsrv_string_from_cstr__copy(char* cstr, ssize_t length) {
    pkrsrv_string_t string;
    string.length = length;
    string.size = string.length + 1;
    string.is_alloc_str = true;
    string.is_binary = false;
    string.value = strndup(cstr, length);

    string.value[length] = '\0';
    
    return string;
}

pkrsrv_string_t* pkrsrv_string_new_from_binary__copy(unsigned char* data, size_t length) {
    pkrsrv_string_t* string = malloc(sizeof(pkrsrv_string_t));
    PKRSRV_REF_COUNTED_INIT(string, pkrsrv_string_free);
    string->length = length;
    string->size = string->length;
    string->is_alloc_str = true;
    string->is_binary = true;
    string->value = malloc(length);
    memcpy(string->value, data, length);
    
    return string;
}

pkrsrv_string_t* pkrsrv_string_new_from_cstr__copy(char* cstr, ssize_t length) {
    pkrsrv_string_t* string = malloc(sizeof(pkrsrv_string_t));
    PKRSRV_REF_COUNTED_INIT(string, pkrsrv_string_free);
    string->length = length;
    string->size = string->length + 1;
    string->is_alloc_str = true;
    string->is_binary = false;
    string->value = malloc(length+1);
    memcpy(string->value, cstr, length);
    string->value[length] = '\0';
    
    return string;
}

pkrsrv_string_t pkrsrv_string_from_cstr(char* cstr, ssize_t length) {
    pkrsrv_string_t string;
    string.length = length;
    string.size = string.length + 1;
    string.is_alloc_str = false;
    string.is_binary = false;
    string.value = cstr;
    string.value[length] = '\0';
    
    return string;
}

pkrsrv_string_t pkrsrv_string_from_binary(unsigned char* data, ssize_t length) {
    pkrsrv_string_t string;
    string.length = length;
    string.size = string.length + 1;

    string.is_alloc_str = false;
    string.value = data;
    
    return string;
}

pkrsrv_string_t* pkrsrv_string_new_from_cstr(char* cstr, ssize_t length) {
    pkrsrv_string_t* string = malloc(sizeof(pkrsrv_string_t));
    PKRSRV_REF_COUNTED_INIT(string, pkrsrv_string_free);
    string->length = length;
    string->size = string->length + 1;
    string->is_alloc_str = false;
    string->is_binary = false;
    string->value = cstr;
    
    return string;
}

pkrsrv_string_t* pkrsrv_string_format_new(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* buffer;
    int length = vasprintf(&buffer, format, args);
    if (length < 0) {
        return NULL;
    }
    va_end(args);

    pkrsrv_string_t* string = pkrsrv_string_new_from_cstr(buffer, length);
    string->is_alloc_str = true;

    return string;
}

void pkrsrv_string_free(pkrsrv_string_t* string) {
    if (string->is_alloc_str) {
        free(string->value);
    }
    free(string);
}

void pkrsrv_string_set_value(pkrsrv_string_t* string, char* value) {
    void* to_free = string->value;
    string->value = value;
    if (to_free && string->is_alloc_str) {
        free(to_free);
    }

    size_t length = strlen(string->value);
    string->length = length;
    string->size = string->length + 1;
    string->value[string->length] = '\0';
}

void pkrsrv_string_set_value__n(pkrsrv_string_t* string, char* value, size_t length) {
    void* to_free = string->value;
    string->value =  value;
    if (to_free && string->is_alloc_str) {
        free(to_free);
    }
    
    string->length = length;
    if (string->is_binary) {
        string->size = string->length;
    } else {
        string->size = string->length + 1;
        string->value[length] = '\0';
    }
}

void pkrsrv_string_set_value__n__copy(pkrsrv_string_t* string, char* value, size_t length) {
    void* to_free = string->value;
    string->value = malloc(length);
    memcpy(string->value, value, length);
    if (to_free && string->is_alloc_str) {
        free(to_free);
    }
    
    string->length = length;
    if (string->is_binary) {
        string->size = string->length;
    } else {
        string->size = string->length + 1;
        string->value[length] = '\0';
    }
}

void pkrsrv_string_append__cstr__n(pkrsrv_string_t* string, char* value, ssize_t length) {
    if ((string->length + length + 1) > string->size) {
        string->size = string->length + length + 1;
        string->value = realloc(string->value, string->size);
    }
    memcpy(string->value + string->length, value, length);
    string->length += length;
    string->value[string->length] = '\0';
}

void pkrsrv_string_append__cstr(pkrsrv_string_t* string, char* value) {
    size_t length = strlen(value);
    pkrsrv_string_append__cstr__n(string, value, length);
}

void pkrsrv_string_append__int(pkrsrv_string_t* string, int value) {
    char buffer[32];
    int length = snprintf(buffer, 32, "%d", value);
    pkrsrv_string_append__cstr__n(string, buffer, length);
}

void pkrsrv_string_append__uint(pkrsrv_string_t* string, unsigned int value) {
    char buffer[32];
    int length = snprintf(buffer, 32, "%u", value);
    pkrsrv_string_append__cstr__n(string, buffer, length);
}

void pkrsrv_string_append__uint64(pkrsrv_string_t* string, uint64_t value) {
    char buffer[32];
    int length = snprintf(buffer, 32, "%llu", value);
    pkrsrv_string_append__cstr__n(string, buffer, length);
}

void pkrsrv_string_append(pkrsrv_string_t* string, pkrsrv_string_t* other) {
    pkrsrv_string_append__cstr__n(string, other->value, other->length);
}

bool pkrsrv_string_compare(pkrsrv_string_t* string, pkrsrv_string_t* other) {
    if (string->length != other->length) {
        return false;
    }

    return memcmp(string->value, other->value, string->length) == 0;
}

void pkrsrv_string_set(pkrsrv_string_t* string, pkrsrv_string_t* other) {
    pkrsrv_string_set_value__n(string, other->value, other->length);
}

void pkrsrv_string_set__copy(pkrsrv_string_t* string, pkrsrv_string_t* other) {
    pkrsrv_string_set_value__n(string, other->value, other->length);
}

void pkrsrv_string_move(pkrsrv_string_t* string, pkrsrv_string_t* other) {
    if (string->is_alloc_str) {
        free(string->value);
    }
    
    string->is_alloc_str = other->is_alloc_str;
    other->is_alloc_str = false;
    string->value = other->value;
    string->length = other->length;
    string->size = other->size;
    string->is_binary = other->is_binary;
}