/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

enum {
    PKRSRV_ARGUMENT_FULL,
    PKRSRV_ARGUMENT_SHORT,
    PKRSRV_ARGUMENT_IS_EXPECT_VALUE,
    PKRSRV_ARGUMENT_FUNCTION
};

#define PKRSRV_ARG_WITH_VALUE (void*) 1
#define PKRSRV_ARG_WITHOUT_VALUE (void*) 0

struct pkrsrv_arg {
    char** argv;
    int argc;
    void** arguments;
    char* command;
};
typedef struct pkrsrv_arg pkrsrv_arg_t;

typedef void (*pkrsrv_arg_function_t)(char*);

pkrsrv_arg_t* pkrsrv_arg_init(char** argv, int argc, void** arguments);
void pkrsrv_arg_handle(pkrsrv_arg_t* arg);