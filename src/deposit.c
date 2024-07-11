/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <libpq-fe.h>

#include "../thirdparty/jsonic/jsonic.h"

#include "../include/pkrsrv.h"
#include "../include/deposit.h"
#include "../include/util.h"
#include "../include/string.h"
#include "../include/db.h"
#include "../include/account.h"

pkrsrv_deposit_monero_address_t* pkrsrv_deposit_monero_address_new(pkrsrv_string_t* address, int account_index, int index, int payment_id) {
    PKRSRV_REF_COUNTED_USE(address);
    
    pkrsrv_deposit_monero_address_t* monero_address = malloc(sizeof(pkrsrv_deposit_monero_address_t));
    PKRSRV_REF_COUNTED_INIT(monero_address, pkrsrv_deposit_monero_address_free);
    monero_address->address = address;
    monero_address->index = index;
    monero_address->payment_id = payment_id;

    return monero_address;
}

void pkrsrv_deposit_monero_address_free(pkrsrv_deposit_monero_address_t* monero_address) {
    PKRSRV_REF_COUNTED_LEAVE(monero_address->address);
    free(monero_address);
}

pkrsrv_deposit_service_t* pkrsrv_deposit_service_new() {
    pkrsrv_deposit_service_t* service = malloc(sizeof(pkrsrv_deposit_service_t));
    service->is_running = false;
    service->pg_conn = pkrsrv_db_connect(pkrsrv_postgres_host, pkrsrv_postgres_port, pkrsrv_postgres_username, pkrsrv_postgres_password, pkrsrv_postgres_db);
    if (!service->pg_conn) {
        printf("[Deposit] [Error] Failed to connect to the database for the new session!\n");
        free(service);
        return NULL;
    }
    
    return service;
}

void pkrsrv_deposit_service_free(pkrsrv_deposit_service_t* service) {
    if (service->pg_conn) {
        PQfinish(service->pg_conn);
    }
    free(service);
}

void pkrsrv_deposit_service_start(pkrsrv_deposit_service_t* service) {
    pkrsrv_util_verbose("[Deposit] Starting deposit service...\n");

    service->is_running = true;

    pthread_create(&service->thread, NULL, (void* (*)(void*)) pkrsrv_deposit_service_thread_f, service);
}

void pkrsrv_deposit_service_stop(pkrsrv_deposit_service_t* service) {
    pkrsrv_util_verbose("[Deposit] Stopping deposit service...\n");
    service->is_running = false;
}

void pkrsrv_deposit_service_thread_f(pkrsrv_deposit_service_t* service) {
    pkrsrv_string_t* main_address = pkrsrv_deposit_monero_get_main_address();
    pkrsrv_util_verbose("Main Address: %s\n", main_address->value);
    
    uint64_t balance, unlocked_balance;
    bool result = pkrsrv_deposit_monero_get_balance(&balance, &unlocked_balance);

    pkrsrv_util_verbose("[Deposit] Balance: %llu\n", balance);
    pkrsrv_util_verbose("[Deposit] Unlocked Balance: %llu\n", unlocked_balance);
    
    pkrsrv_util_verbose("[Deposit] Deposit service started.\n");
    
    while (service->is_running) {
        curl_global_init(CURL_GLOBAL_ALL);
        
        uint64_t height = pkrsrv_deposit_monero_get_height();
        
        pkrsrv_string_t* query = pkrsrv_string_format_new(
            "SELECT %s "
            "FROM accounts "
            "WHERE (xmr_deposit_address IS NOT NULL) AND (xmr_deposit_address != '')",
            PKRSRV_ACCOUNT_SELECT_COLUMNS
        );
        
        pkrsrv_account_iterator_t* iterator = pkrsrv_account_iterator_query(service->pg_conn, query, NULL, 0);
        
        while (iterator && pkrsrv_account_iterator_next(iterator)) {
            pkrsrv_account_t* account = iterator->current;
            PKRSRV_REF_COUNTED_USE(account);

            pkrsrv_util_verbose("[Deposit] Checking deposits for account %s\n", account->id_token->value);

            pkrsrv_deposit_monero_tx_list_t* txs = pkrsrv_deposit_monero_get_txs(0, account->xmr_deposit_address_index, account->xmr_height);
            
            if (!txs) {
                pkrsrv_util_verbose("[Deposit] Couldn't get transactions for account %s. Skipping...\n", account->id_token->value);
                PKRSRV_REF_COUNTED_LEAVE(account);
                continue;
            }

            PKRSRV_REF_COUNTED_USE(txs);

            LIST_FOREACH(txs, tx)
                if (!pkrsrv_string_compare(tx->address, account->xmr_deposit_address)) {
                    continue;
                }
            
                pkrsrv_util_verbose("[Deposit] Found transaction (TX: %s, height: %llu)\n", tx->txid->value, tx->height);

                if (tx->height < account->xmr_height) {
                    pkrsrv_util_verbose("[Deposit] Skipping transaction %s (height: %llu)\n", tx->txid->value, tx->height);
                    continue;
                }

                pkrsrv_util_verbose("[Deposit] Processing transaction %s\n", tx->txid->value);

                pkrsrv_deposit_monero_deposit_t* existing = pkrsrv_deposit_monero_deposit_getby_txid(service->pg_conn, tx->txid);

                if (existing && (existing->status == PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_CONFIRMED)) {
                    pkrsrv_util_verbose("[Deposit] Deposit for transaction %s already exists\n", tx->txid->value);
                    continue;
                } else if (existing && (existing->status == PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_PENDING)) {
                    PKRSRV_REF_COUNTED_USE(existing);

                    pkrsrv_util_verbose("[Deposit] Deposit for transaction %s is pending\n", tx->txid->value);
                    
                    if (tx->is_confirmed) {
                        bool result;
                        
                        pkrsrv_db_transaction_begin(service->pg_conn);
                        
                        result = pkrsrv_deposit_monero_deposit_update_status(service->pg_conn, existing, PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_CONFIRMED);
                        if (!result) {
                            pkrsrv_db_transaction_rollback(service->pg_conn);
                            pkrsrv_util_verbose("[Deposit] Couldn't update deposit status to confirmed for transaction %s\n", tx->txid->value);
                            PKRSRV_REF_COUNTED_LEAVE(existing);
                            continue;
                        }
                        pkrsrv_account_fetch(service->pg_conn, account);
                        uint64_t new_balance = account->balance + tx->amount;
                        
                        result = pkrsrv_account_update_balance(service->pg_conn, account, new_balance);
                        if (!result) {
                            pkrsrv_db_transaction_rollback(service->pg_conn);
                            pkrsrv_util_verbose("[Deposit] Couldn't update account balance for account %s\n", account->id_token->value);
                            PKRSRV_REF_COUNTED_LEAVE(existing);
                            continue;
                        }

                        result = pkrsrv_account_update_xmr_height(service->pg_conn, account, tx->height - 1);
                        if (!result) {
                            pkrsrv_db_transaction_rollback(service->pg_conn);
                            pkrsrv_util_verbose("[Deposit] Couldn't update account XMR height for account %s\n", account->id_token->value);
                            PKRSRV_REF_COUNTED_LEAVE(existing);
                            continue;
                        }

                        pkrsrv_db_transaction_commit(service->pg_conn);
                        pkrsrv_util_verbose("[Deposit] Updated deposit status to confirmed for transaction %s\n", tx->txid->value);
                    }

                    PKRSRV_REF_COUNTED_LEAVE(existing);

                    continue;
                }
                
                if (existing) {
                    PKRSRV_REF_COUNTED_LEAVE(existing);
                    continue;
                }
                
                pkrsrv_deposit_monero_deposit_status_t status = tx->is_confirmed
                                                                ? PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_CONFIRMED
                                                                : PKRSRV_DEPOSIT_MONERO_DEPOSIT_STATUS_PENDING;

                bool result;

                pkrsrv_db_transaction_begin(service->pg_conn);

                pkrsrv_account_fetch(service->pg_conn, account);
                uint64_t new_balance = account->balance + tx->amount;
                
                result = pkrsrv_account_update_balance(service->pg_conn, account, new_balance);
                if (!result) {
                    pkrsrv_db_transaction_rollback(service->pg_conn);
                    pkrsrv_util_verbose("[Deposit] Couldn't update account balance for account %s\n", account->id_token->value);
                    continue;
                }

                pkrsrv_deposit_monero_deposit_create_params_t params;
                params.account = account;
                params.txid = tx->txid;
                params.to_address = account->xmr_deposit_address;
                params.status = status;
                params.timestamp = tx->timestamp;
                params.amount = tx->amount;

                pkrsrv_deposit_monero_deposit_t* deposit = pkrsrv_deposit_monero_deposit_create(service->pg_conn, params);
                if (!deposit) {
                    pkrsrv_util_verbose("[Deposit] Couldn't create deposit for transaction %s\n", tx->txid->value);
                    pkrsrv_db_transaction_rollback(service->pg_conn);
                    continue;
                }

                result = pkrsrv_account_update_xmr_height(service->pg_conn, account, tx->height - 1);
                if (!result) {
                    pkrsrv_db_transaction_rollback(service->pg_conn);
                    pkrsrv_util_verbose("[Deposit] Couldn't update account XMR height for account %s\n", account->id_token->value);
                    continue;
                }

                pkrsrv_db_transaction_commit(service->pg_conn);
            
                pkrsrv_util_verbose("[Deposit] Created deposit#%llu for transaction %s\n", deposit->id, tx->txid->value);
            END_FOREACH

            if (account->xmr_height < (height - 1)) {
                pkrsrv_account_update_xmr_height(service->pg_conn, account, height - 1);
            }

            PKRSRV_REF_COUNTED_LEAVE(txs);
            PKRSRV_REF_COUNTED_LEAVE(account);
        }

        if (iterator) {
            pkrsrv_account_iterator_free(iterator);
        }

        pkrsrv_string_free(query);

        curl_global_cleanup();
        
        pkrsrv_util_msleep(PKRSRV_DEPOSIT_SERVICE_DELAY);
    }

    pkrsrv_string_free(main_address);

    pkrsrv_util_verbose("Deposit service stopped.\n");
}

size_t curl_write_f(void* contents, size_t size, size_t nmemb, void* userp) {
    pkrsrv_string_t* response = (pkrsrv_string_t*) userp;
    size_t chunk_size = size * nmemb;
    pkrsrv_string_append__cstr__n(response, contents, chunk_size);
    
    return chunk_size;
}

pkrsrv_string_t* pkrsrv_deposit_monero_wallet_rpc_call(char* method, char* params) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    PKRSRV_UTIL_ASSERT(curl);
    if (!curl) {
        printf("curl_easy_init() failed\n");
        return NULL;
    }

    pkrsrv_string_t* response = pkrsrv_string_new();

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_f);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_URL, "http://monero-wallet-rpc:18082/json_rpc");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    pkrsrv_string_t* query = pkrsrv_string_format_new("{\"jsonrpc\":\"2.0\",\"id\":\"0\",\"method\":\"%s\",\"params\":%s}", method, params);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query->value);

    res = curl_easy_perform(curl);
    PKRSRV_UTIL_ASSERT(res == CURLE_OK);
    if (res != CURLE_OK) {
        printf("[Deposit] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        pkrsrv_string_free(response);
        pkrsrv_string_free(query);
        return NULL;
    }

    curl_easy_cleanup(curl);
    pkrsrv_string_free(query);

    return response;
}

bool pkrsrv_deposit_monero_wallet_open(char* wallet_name, char* password) {
    pkrsrv_string_t* params = pkrsrv_string_format_new("{\"filename\":\"%s\",\"password\":\"%s\"}", wallet_name, password);
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("open_wallet", params->value);
    pkrsrv_string_free(params);
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Couldn't open Monero wallet!\n");
        return false;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to open Monero wallet!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("Monero Wallet RPC Error:\n");
        printf("  Error Code: %s\n", error_code_json->val);
        printf("  Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return false;
    }

    jsonic_free(&error_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return true;
}

bool pkrsrv_deposit_monero_wallet_close() {
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("close_wallet", "{}");
    PKRSRV_UTIL_ASSERT(response);
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Couldn't close Monero wallet!\n");
        return false;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to open Monero wallet!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return false;
    }

    jsonic_free(&error_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return true;
}

bool pkrsrv_deposit_monero_wallet_create(char* wallet_name, char* password) {
    pkrsrv_string_t* params = pkrsrv_string_format_new("{\"filename\":\"%s\",\"password\":\"%s\",\"language\":\"English\"}", wallet_name, password);
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("create_wallet", params->value);
    PKRSRV_UTIL_ASSERT(response);
    pkrsrv_string_free(params);
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Couldn't create Monero wallet!\n");
        return false;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to open Monero wallet!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&root_json);
        jsonic_free(&error_json);
        pkrsrv_string_free(response);
        return false;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");

    jsonic_free(&result_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return true;
}

pkrsrv_string_t* pkrsrv_deposit_monero_get_main_address() {
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("getaddress", "{}");
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Monero wallet RPC call failed\n");
        return NULL;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to get main XMR address!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&root_json);
        jsonic_free(&error_json);
        return NULL;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");
    jsonic_node_t* address_json = jsonic_object_get(response->value, result_json, "address");

    pkrsrv_string_t* address = pkrsrv_string_new_from_cstr__copy(address_json->val, address_json->len);

    jsonic_free(&result_json);
    jsonic_free(&address_json);
    jsonic_free(&root_json);

    pkrsrv_string_free(response);
    
    return address;
}

bool pkrsrv_deposit_monero_get_balance(uint64_t* balance, uint64_t* unlocked_balance) {
    *balance = 0;
    *unlocked_balance = 0;
    
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("getbalance", "{}");
    if (response == NULL) {
        pkrsrv_util_verbose("[Deposit] Monero wallet RPC call failed\n");
        return false;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to get XMR balance!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return false;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");
    jsonic_node_t* balance_json = jsonic_object_get(response->value, result_json, "balance");
    jsonic_node_t* unlocked_balance_json = jsonic_object_get(response->value, result_json, "unlocked_balance");

    *balance = atoll(balance_json->val);
    *unlocked_balance = atoll(unlocked_balance_json->val);

    jsonic_free(&result_json);
    jsonic_free(&balance_json);
    jsonic_free(&unlocked_balance_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return true;
}

bool pkrsrv_deposit_monero_get_address_balance(char* address, uint64_t* balance, uint64_t* unlocked_balance) {
    *balance = 0;
    *unlocked_balance = 0;
    
    pkrsrv_string_t* params = pkrsrv_string_format_new("{\"address\":\"%s\"}", address);
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("getaddressbalance", params->value);
    pkrsrv_string_free(params);
    if (response == NULL) {
        pkrsrv_util_verbose("[Deposit] Monero wallet RPC call failed\n");
        return false;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to get XMR address balance!\n");
        
        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return false;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");
    jsonic_node_t* balance_json = jsonic_object_get(response->value, result_json, "balance");
    jsonic_node_t* unlocked_balance_json = jsonic_object_get(response->value, result_json, "unlocked_balance");

    *balance = atoll(balance_json->val);
    *unlocked_balance = atoll(unlocked_balance_json->val);

    jsonic_free(&result_json);
    jsonic_free(&balance_json);
    jsonic_free(&unlocked_balance_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return true;
}

pkrsrv_deposit_monero_address_t* pkrsrv_deposit_monero_create_address(int account, int index) {
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("create_address", "{}");
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Monero wallet RPC call failed\n");
        return NULL;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to create XMR address!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return false;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");
    jsonic_node_t* address_json = jsonic_object_get(response->value, result_json, "address");

    pkrsrv_string_t* address = pkrsrv_string_new_from_cstr__copy(address_json->val, address_json->len);

    jsonic_free(&result_json);
    jsonic_free(&root_json);
    jsonic_free(&address_json);

    pkrsrv_deposit_monero_address_t* monero_address = pkrsrv_deposit_monero_address_new(address, 0, index, 0);
    pkrsrv_string_free(response);

    return monero_address;
}

pkrsrv_deposit_monero_tx_t* pkrsrv_deposit_monero_tx_new(pkrsrv_deposit_monero_tx_new_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.txid);
    PKRSRV_REF_COUNTED_USE(params.address);
    
    pkrsrv_deposit_monero_tx_t* tx = malloc(sizeof(pkrsrv_deposit_monero_tx_t));
    PKRSRV_REF_COUNTED_INIT(tx, pkrsrv_deposit_monero_tx_free);
    LIST_ITEM_INIT(tx);
    tx->txid = params.txid;
    tx->address = params.address;
    tx->amount = params.amount;
    tx->fee = params.fee;
    tx->unlock_time = params.unlock_time;
    tx->timestamp = params.timestamp;
    tx->height = params.height;
    tx->is_confirmed = params.is_confirmed;

    return tx;
}

void pkrsrv_deposit_monero_tx_free(pkrsrv_deposit_monero_tx_t* tx) {
    PKRSRV_REF_COUNTED_LEAVE(tx->txid);
    PKRSRV_REF_COUNTED_LEAVE(tx->address);

    free(tx);
}

pkrsrv_deposit_monero_tx_list_t* pkrsrv_deposit_monero_tx_list_new() {
    pkrsrv_deposit_monero_tx_list_t* list = malloc(sizeof(pkrsrv_deposit_monero_tx_list_t));
    PKRSRV_REF_COUNTED_INIT(list, pkrsrv_deposit_monero_tx_list_free);
    LIST_INIT(list);
    return list;
}

void pkrsrv_deposit_monero_tx_list_add(pkrsrv_deposit_monero_tx_list_t* list, pkrsrv_deposit_monero_tx_t* tx) {
    PKRSRV_REF_COUNTED_USE(tx);
    LIST_APPEND(list, tx);
}

void pkrsrv_deposit_monero_tx_list_remove(pkrsrv_deposit_monero_tx_list_t* list, pkrsrv_deposit_monero_tx_t* tx) {
    LIST_REMOVE(list, tx);
    PKRSRV_REF_COUNTED_LEAVE(tx);
}

void pkrsrv_deposit_monero_tx_list_free(pkrsrv_deposit_monero_tx_list_t* list) {
    LIST_FOREACH(list, tx)
        PKRSRV_REF_COUNTED_LEAVE(tx);
    END_FOREACH

    free(list);
}

uint64_t pkrsrv_deposit_monero_get_height() {
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("getheight", "{}");
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Monero wallet RPC call failed\n");
        return 0;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to get XMR height!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return 0;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");
    jsonic_node_t* height_json = jsonic_object_get(response->value, result_json, "height");

    uint64_t height = atoll(height_json->val);

    jsonic_free(&height_json);
    jsonic_free(&result_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return height;
}

pkrsrv_deposit_monero_tx_list_t* pkrsrv_deposit_monero_get_txs(int account_index, int address_index, int min_height) {
    pkrsrv_string_t* params = pkrsrv_string_format_new(
        "{"
            "\"in\": true, "
            "\"account_index\": %d, "
            "\"subaddress_indices\": [%d],"
            "\"filter_by_height\": true,"
            "\"min_height\": %d"
        "}",
        account_index,
        address_index,
        min_height
    );
    pkrsrv_string_t* response = pkrsrv_deposit_monero_wallet_rpc_call("get_transfers", params->value);
    pkrsrv_string_free(params);
    if (!response) {
        pkrsrv_util_verbose("[Deposit] Monero wallet RPC call failed\n");
        return NULL;
    }

    jsonic_node_t* root_json = jsonic_get_root(response->value);
    jsonic_node_t* error_json = jsonic_object_get(response->value, root_json, "error");
    if (error_json->type != JSONIC_NONE) {
        pkrsrv_util_verbose("[Deposit] Failed to get XMR transactions!\n");

        jsonic_node_t* error_code_json = jsonic_object_get(response->value, error_json, "code");
        jsonic_node_t* error_message_json = jsonic_object_get(response->value, error_json, "message");

        printf("[Deposit] Monero Wallet RPC Error:\n");
        printf("[Deposit]   Error Code: %s\n", error_code_json->val);
        printf("[Deposit]   Error Message: %s\n", error_message_json->val);
        
        jsonic_free(&error_code_json);
        jsonic_free(&error_message_json);
        
        jsonic_free(&error_json);
        jsonic_free(&root_json);
        pkrsrv_string_free(response);
        return NULL;
    }
    jsonic_free(&error_json);
    jsonic_node_t* result_json = jsonic_object_get(response->value, root_json, "result");
    jsonic_node_t* in_json = jsonic_object_get(response->value, result_json, "in");

    pkrsrv_deposit_monero_tx_list_t* txs = pkrsrv_deposit_monero_tx_list_new();

    jsonic_node_t* tx_json = NULL;
    for (;;) {
        tx_json = jsonic_array_iter_free(response->value, in_json, tx_json, 0);
        if (tx_json->type == JSONIC_NONE) break;

        jsonic_node_t* txid_json = jsonic_object_get(response->value, tx_json, "txid");
        jsonic_node_t* address_json = jsonic_object_get(response->value, tx_json, "address");
        jsonic_node_t* amount_json = jsonic_object_get(response->value, tx_json, "amount");
        jsonic_node_t* fee_json = jsonic_object_get(response->value, tx_json, "fee");
        jsonic_node_t* unlock_time_json = jsonic_object_get(response->value, tx_json, "unlock_time");
        jsonic_node_t* timestamp_json = jsonic_object_get(response->value, tx_json, "timestamp");
        jsonic_node_t* height_json = jsonic_object_get(response->value, tx_json, "height");
        jsonic_node_t* confirmations_json = jsonic_object_get(response->value, tx_json, "confirmations");

        uint64_t amount = atoll(amount_json->val);
        uint64_t fee = atoll(fee_json->val);

        int confirmations = atoi(confirmations_json->val);

        pkrsrv_deposit_monero_tx_new_params_t params;
        params.txid = pkrsrv_string_new_from_cstr__copy(txid_json->val, txid_json->len);
        params.address = pkrsrv_string_new_from_cstr__copy(address_json->val, address_json->len);
        params.amount = amount;
        params.fee = fee;
        params.unlock_time = atoi(unlock_time_json->val);
        params.timestamp = atoi(timestamp_json->val);
        params.height = atoi(height_json->val);
        params.is_confirmed = confirmations >= PKRSRV_DEPOSIT_MONERO_MIN_CONFIRMATIONS;

        pkrsrv_deposit_monero_tx_t* tx = pkrsrv_deposit_monero_tx_new(params);
        pkrsrv_deposit_monero_tx_list_add(txs, tx);

        jsonic_free(&txid_json);
        jsonic_free(&address_json);
        jsonic_free(&amount_json);
        jsonic_free(&fee_json);
        jsonic_free(&unlock_time_json);
        jsonic_free(&timestamp_json);
        jsonic_free(&height_json);
        jsonic_free(&confirmations_json);
    }

    jsonic_free(&tx_json);
    jsonic_free(&in_json);
    jsonic_free(&result_json);
    jsonic_free(&root_json);
    pkrsrv_string_free(response);

    return txs;
}

pkrsrv_deposit_monero_deposit_t* pkrsrv_deposit_monero_deposit_new(pkrsrv_deposit_monero_deposit_new_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.account);
    PKRSRV_REF_COUNTED_USE(params.txid);
    PKRSRV_REF_COUNTED_USE(params.to_address);
    
    pkrsrv_deposit_monero_deposit_t* deposit = malloc(sizeof(pkrsrv_deposit_monero_deposit_t));
    PKRSRV_REF_COUNTED_INIT(deposit, pkrsrv_deposit_monero_deposit_free);
    deposit->id = params.id;
    deposit->account = params.account;
    deposit->txid = params.txid;
    deposit->timestamp = params.timestamp;
    deposit->amount = params.amount;
    deposit->to_address = params.to_address;
    deposit->status = params.status;

    return deposit;
}

void pkrsrv_deposit_monero_deposit_free(pkrsrv_deposit_monero_deposit_t* deposit) {
    PKRSRV_REF_COUNTED_LEAVE(deposit->account);
    PKRSRV_REF_COUNTED_LEAVE(deposit->txid);
    PKRSRV_REF_COUNTED_LEAVE(deposit->to_address);
    
    free(deposit);
}

pkrsrv_deposit_monero_deposit_t* pkrsrv_deposit_monero_deposit_getby_txid(PGconn* pg_conn, pkrsrv_string_t* p_txid) {
    PKRSRV_REF_COUNTED_USE(p_txid);

    pkrsrv_deposit_monero_deposit_t* deposit = NULL;

    PGresult* query_result;
    ExecStatusType query_status;

    pkrsrv_string_t* query = pkrsrv_string_format_new(
        "SELECT id, txid, account, status, amount, to_address, timestamp "
        "FROM monero_deposits "
        "WHERE txid = $1 "
        "LIMIT 1 "
    );

    query_result = PQexecParams(
        pg_conn,
        query->value,
        1,
        NULL,
        (const char* const[]) {p_txid->value},
        NULL,
        NULL,
        0
    );

    query_status = PQresultStatus(query_result);

    if (query_status != PGRES_TUPLES_OK) {
        pkrsrv_util_verbose("[Deposit] Couldn't get deposit by txid\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        PKRSRV_REF_COUNTED_LEAVE(p_txid);
        
        pkrsrv_string_free(query);
        PQclear(query_result);

        return NULL;
    }

    if (PQntuples(query_result) == 0) {
        pkrsrv_util_verbose("[Deposit] Deposit not found by TXID %s\n", p_txid->value);
        
        PKRSRV_REF_COUNTED_LEAVE(p_txid);
        
        pkrsrv_string_free(query);
        PQclear(query_result);

        return NULL;
    }

    char* id_str = PQgetvalue(query_result, 0, 0);
    char* txid_str = PQgetvalue(query_result, 0, 1);
    int txid_len = strlen(txid_str);
    char* account_str = PQgetvalue(query_result, 0, 2);
    int account_id = atoi(account_str);
    char* status_str = PQgetvalue(query_result, 0, 3);
    char* amount_str = PQgetvalue(query_result, 0, 4);
    char* to_address_str = PQgetvalue(query_result, 0, 5);
    char* timestamp_str = PQgetvalue(query_result, 0, 6);

    pkrsrv_account_t* account = pkrsrv_account_getby_id(pg_conn, account_id);

    uint64_t id = atoll(id_str);
    pkrsrv_string_t* txid = pkrsrv_string_new_from_cstr__copy(txid_str, txid_len);
    pkrsrv_string_t* to_address = pkrsrv_string_new_from_cstr__copy(to_address_str, strlen(to_address_str));
    pkrsrv_deposit_monero_deposit_status_t status = atoi(status_str);
    uint64_t amount = atoll(amount_str);
    uint64_t timestamp = atoll(timestamp_str);
    
    pkrsrv_deposit_monero_deposit_new_params_t params;
    params.id = id;
    params.account = account;
    params.txid = txid;
    params.timestamp = timestamp;
    params.amount = amount;
    params.to_address = to_address;
    params.status = status;

    deposit = pkrsrv_deposit_monero_deposit_new(params);

    PQclear(query_result);
    pkrsrv_string_free(query);

    PKRSRV_REF_COUNTED_LEAVE(p_txid);
    
    return deposit;
}

pkrsrv_deposit_monero_deposit_list_t* pkrsrv_deposit_monero_deposit_list_new() {
    pkrsrv_deposit_monero_deposit_list_t* list = malloc(sizeof(pkrsrv_deposit_monero_deposit_list_t));
    PKRSRV_REF_COUNTED_INIT(list, pkrsrv_deposit_monero_deposit_list_free);
    LIST_INIT(list);
    return list;
}

void pkrsrv_deposit_monero_deposit_list_add(pkrsrv_deposit_monero_deposit_list_t* list, pkrsrv_deposit_monero_deposit_t* deposit) {
    PKRSRV_REF_COUNTED_USE(deposit);
    LIST_APPEND(list, deposit);
}

void pkrsrv_deposit_monero_deposit_list_remove(pkrsrv_deposit_monero_deposit_list_t* list, pkrsrv_deposit_monero_deposit_t* deposit) {
    LIST_REMOVE(list, deposit);
    PKRSRV_REF_COUNTED_LEAVE(deposit);
}

void pkrsrv_deposit_monero_deposit_list_free(pkrsrv_deposit_monero_deposit_list_t* list) {
    LIST_FOREACH(list, deposit)
        PKRSRV_REF_COUNTED_LEAVE(deposit);
    END_FOREACH

    free(list);
}

pkrsrv_deposit_monero_deposit_list_t* pkrsrv_deposit_monero_get_deposits(PGconn* pg_conn, pkrsrv_deposit_monero_get_deposits_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.account);
    
    pkrsrv_deposit_monero_deposit_list_t* deposits = pkrsrv_deposit_monero_deposit_list_new();

    PGresult* query_result;
    ExecStatusType query_status;

    pkrsrv_string_t* query = pkrsrv_string_format_new(
        "SELECT id, txid, account, status, amount, to_address, timestamp "
        "FROM monero_deposits "
        "WHERE "
            "(account = $1) and "
            "((status = $2) or $2 = 0) and"
            "((timestamp >= $3) or ($3 = 0)) and"
            "((height >= $4) or ($4 = 0)) "
    );

    if (params.offset > 0) {
        pkrsrv_string_append__cstr(query, "OFFSET ");
        pkrsrv_string_append__int(query, params.offset);
    }

    if (params.limit > 0) {
        pkrsrv_string_append__cstr(query, "LIMIT ");
        pkrsrv_string_append__int(query, params.limit);
    }

    query_result = PQexecParams(
        pg_conn,
        query->value,
        1,
        NULL,
        (const char* const[]) {params.account->id_token->value},
        NULL,
        NULL,
        0
    );

    query_status = PQresultStatus(query_result);

    if (query_status != PGRES_TUPLES_OK) {
        pkrsrv_util_verbose("[Deposit] Couldn't get deposits\n");

        PKRSRV_REF_COUNTED_LEAVE(params.account);
        
        pkrsrv_string_free(query);
        PQclear(query_result);

        return NULL;
    }

    for (int i = 0; i < PQntuples(query_result); i++) {
        char* id_str = PQgetvalue(query_result, i, 0);
        char* txid_str = PQgetvalue(query_result, i, 1);
        int txid_len = strlen(txid_str);
        char* account_str = PQgetvalue(query_result, i, 2);
        int account_id = atoi(account_str);
        char* status_str = PQgetvalue(query_result, i, 3);
        char* amount_str = PQgetvalue(query_result, i, 4);
        char* to_address_str = PQgetvalue(query_result, i, 5);
        char* timestamp_str = PQgetvalue(query_result, i, 6);

        pkrsrv_account_t* account = pkrsrv_account_getby_id(pg_conn, account_id);

        uint64_t id = atoll(id_str);
        pkrsrv_string_t* txid = pkrsrv_string_new_from_cstr__copy(txid_str, strlen(txid_str));
        pkrsrv_string_t* to_address = pkrsrv_string_new_from_cstr__copy(to_address_str, strlen(to_address_str));
        pkrsrv_deposit_monero_deposit_status_t status = atoi(status_str);
        uint64_t amount = atoll(amount_str);
        uint64_t timestamp = atoll(timestamp_str);

        pkrsrv_deposit_monero_deposit_new_params_t params;
        params.id = id;
        params.account = account;
        params.txid = txid;
        params.timestamp = timestamp;
        params.amount = amount;
        params.to_address = to_address;
        params.status = status;

        pkrsrv_deposit_monero_deposit_t* deposit = pkrsrv_deposit_monero_deposit_new(params);
        pkrsrv_deposit_monero_deposit_list_add(deposits, deposit);
    }

    PQclear(query_result);
    pkrsrv_string_free(query);

    PKRSRV_REF_COUNTED_LEAVE(params.account);

    return deposits;
}

pkrsrv_deposit_monero_deposit_t* pkrsrv_deposit_monero_deposit_create(PGconn* pg_conn, pkrsrv_deposit_monero_deposit_create_params_t params) {
    PKRSRV_REF_COUNTED_USE(params.account);
    PKRSRV_REF_COUNTED_USE(params.txid);
    PKRSRV_REF_COUNTED_USE(params.to_address);
    
    PGresult* query_result;
    ExecStatusType query_status;

    pkrsrv_string_t* query = pkrsrv_string_format_new(
        "INSERT INTO monero_deposits (txid, account, status, amount, to_address, timestamp) "
        "VALUES ($1, $2, $3, $4, $5, $6) "
        "RETURNING id"
    );

    char status_str[4];
    sprintf(status_str, "%d", params.status);
    char timestamp_str[20];
    sprintf(timestamp_str, "%llu", params.timestamp);
    char account_id_str[20];
    sprintf(account_id_str, "%llu", params.account->id);
    char amount_str[21];
    sprintf(amount_str, "%llu", params.amount);

    const char* query_params[] = {
        params.txid->value,
        (char *) account_id_str,
        (char *) status_str,
        amount_str,
        params.to_address->value,
        (char *) timestamp_str
    };

    query_result = PQexecParams(
        pg_conn,
        query->value,
        6,
        NULL,
        query_params,
        NULL,
        NULL,
        0
    );

    query_status = PQresultStatus(query_result);

    pkrsrv_string_free(query);

    if (query_status != PGRES_TUPLES_OK) {
        pkrsrv_util_verbose("[Deposit] Couldn't create deposit\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        PKRSRV_REF_COUNTED_LEAVE(params.account);
        PKRSRV_REF_COUNTED_LEAVE(params.txid);
        PKRSRV_REF_COUNTED_LEAVE(params.to_address);
        
        PQclear(query_result);

        return NULL;
    }

    char* id_str = PQgetvalue(query_result, 0, 0);
    uint64_t id = atoll(id_str);

    pkrsrv_deposit_monero_deposit_new_params_t deposit_params;
    deposit_params.id = id;
    deposit_params.account = params.account;
    deposit_params.txid = params.txid;
    deposit_params.timestamp = params.timestamp;
    deposit_params.amount = params.amount;
    deposit_params.to_address = params.to_address;
    deposit_params.status = params.status;

    pkrsrv_deposit_monero_deposit_t* deposit = pkrsrv_deposit_monero_deposit_new(deposit_params);

    PQclear(query_result);

    PKRSRV_REF_COUNTED_LEAVE(params.account);
    PKRSRV_REF_COUNTED_LEAVE(params.txid);
    PKRSRV_REF_COUNTED_LEAVE(params.to_address);
    
    return deposit;
}

bool pkrsrv_deposit_monero_deposit_update_status(PGconn* pg_conn, pkrsrv_deposit_monero_deposit_t* deposit, pkrsrv_deposit_monero_deposit_status_t status) {
    PKRSRV_REF_COUNTED_USE(deposit);
    
    PGresult* query_result;
    ExecStatusType query_status;

    pkrsrv_string_t* query = pkrsrv_string_format_new(
        "UPDATE monero_deposits "
        "SET status = $1 "
        "WHERE id = $2"
    );

    pkrsrv_string_t* id_str = pkrsrv_string_format_new("%llu", deposit->id);
    pkrsrv_string_t* status_str = pkrsrv_string_format_new("%d", status);

    query_result = PQexecParams(
        pg_conn,
        query->value,
        2,
        NULL,
        (const char* const[]) {
            status_str->value,
            id_str->value
        },
        NULL,
        NULL,
        0
    );

    query_status = PQresultStatus(query_result);

    if (query_status != PGRES_COMMAND_OK) {
        pkrsrv_util_verbose("[Deposit] Couldn't update deposit status\n");
        printf("\tPostgreSQL Error: %s\n", PQresultErrorMessage(query_result));

        pkrsrv_string_free(id_str);
        pkrsrv_string_free(status_str);

        PKRSRV_REF_COUNTED_LEAVE(deposit);
        
        pkrsrv_string_free(query);
        PQclear(query_result);

        return false;
    }

    pkrsrv_string_free(id_str);
    pkrsrv_string_free(status_str);

    PQclear(query_result);
    pkrsrv_string_free(query);

    PKRSRV_REF_COUNTED_LEAVE(deposit);
    
    return true;
}