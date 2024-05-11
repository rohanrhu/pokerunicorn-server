/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup util Utilities
 * \brief Utility functions and macros.
 */

/**
 * \addtogroup util
 * \ingroup util
 * @{
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>

static bool is_verbose = true;

/**
 * When an assertion fails, this function is called.
 * It is good always to have a breakpoint here.
 */
void pkrsrv_util_assert_fail();

/**
 * Use this macro to assert a condition.
 */
#define PKRSRV_UTIL_ASSERT(condition) \
    { \
        if (!(condition)) { \
            printf("Assertion failed: %s\n", #condition); \
            printf("\tat: %s:%d\n", __FILE__, __LINE__); \
            \
            void* callstack[128]; \
            int frames = backtrace(callstack, sizeof(callstack) / sizeof(void*)); \
            char** lines = backtrace_symbols(callstack, frames); \
            \
            if (lines) { \
                printf("Call Stack:\n"); \
                \
                for (int i = 0; i < frames; i++) { \
                    printf("%s\n", lines[i]); \
                } \
            } \
            pkrsrv_util_assert_fail(); \
        }; \
    }

void pkrsrv_util_verbose(const char* format, ...);
void pkrsrv_util_verbose_set(int p_is_verbose);
int pkrsrv_util_msleep(long millis);
uint64_t pkrsrv_util_get_time_msec();
int pkrsrv_util_int2str(int number, char* target);
unsigned char* pkrsrv_util_base64_encode(unsigned char* data, int length);
unsigned char* pkrsrv_util_base64_decode(unsigned char* data, int length);

/**
 * @}
 */