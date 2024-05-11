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
#include <string.h>

#include "../include/arg.h"

pkrsrv_arg_t* pkrsrv_arg_init(char** argv, int argc, void** arguments) {
    pkrsrv_arg_t* arg = malloc(sizeof(pkrsrv_arg_t));

    arg->argv = argv;
    arg->argc = argc;
    arg->arguments = arguments;
    arg->command = NULL;

    return arg;
}

void pkrsrv_arg_handle(pkrsrv_arg_t* arg) {
    char* _taken;
    int _taken_i;
    
    pkrsrv_arg_function_t expector_f = NULL;
    char* expector_s = NULL;

    int _argument_i;
    char* _full;
    char* _short;
    void* _is_expect_value;
    pkrsrv_arg_function_t _function;

    int is_found;
    
    for (_taken_i=1; _taken_i < arg->argc; _taken_i++) {
        _taken = arg->argv[_taken_i];

        if (expector_f) {
            expector_f(_taken);
            expector_f = NULL;
            expector_s = NULL;
            continue;
        }

        is_found = 0;

        for (_argument_i=0;; _argument_i++) {
            _full = arg->arguments[_argument_i*4 + PKRSRV_ARGUMENT_FULL];
            if (_full == NULL) break;
            _short = arg->arguments[_argument_i*4 + PKRSRV_ARGUMENT_SHORT];
            _is_expect_value = arg->arguments[_argument_i*4 + PKRSRV_ARGUMENT_IS_EXPECT_VALUE];
            _function = arg->arguments[_argument_i*4 + PKRSRV_ARGUMENT_FUNCTION];

            if (_short && (strcmp(_taken, _short) == 0)) {
                if (_is_expect_value == NULL) _function(NULL);
                else {
                    expector_f = _function;
                    expector_s = _short;
                }
                
                break;
            }

            if ((_is_expect_value == NULL) && (strcmp(_taken, _full) == 0)) {
                _function(NULL);
                continue;
            }
            
            if (_is_expect_value == NULL) {
                continue;
            }
            
            if (strncmp(_taken, _full, strlen(_full)) != 0) {
                continue;
            }

            int _taken_len = strlen(_taken);
            int _full_len = strlen(_full);

            if ((_taken[_full_len] != '=') || (_taken[_full_len+1] == '\0')) {
                continue;
            }

            _function(_taken+_full_len+1);

            is_found = 1;
        }

        if (!is_found && !expector_f && _taken[0] == '-') {
            printf("Invalid option: %s\n", _taken);
            exit(0);
        } else {
            arg->command = _taken;
        }
    }

    if (expector_f) {
        printf("Missing value for: %s\n", expector_s);
        exit(0);
    }
}