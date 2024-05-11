/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../include/trie.h"

pkrsrv_trie__unicode_t* pkrsrv_trie_init__unicode() {
    pkrsrv_trie__unicode_t* pkrsrv_trie = malloc(sizeof(pkrsrv_trie__unicode_t));
    pkrsrv_trie->root = pkrsrv_trie_node_init__unicode();

    return pkrsrv_trie;
}

pkrsrv_trie_node__unicode_t* pkrsrv_trie_set__unicode(pkrsrv_trie__unicode_t* trie, char* key, void* value) {
    pkrsrv_trie_node__unicode_t* current = trie->root;
    
    int i = 0;
    unsigned char c;
    pkrsrv_trie_node__unicode_t* latest;

    ITERATE:
    
    for (; (c = key[i]) != '\0'; i++) {
        latest = current->map[c];
        
        if (!latest) {
            break;
        }

        current = latest;
    }
    
    i++;

    if (c == '\0') {
        goto END;
    }

    current->map[c] = pkrsrv_trie_node_init__unicode();
    current->map[c]->parent = current;
    current->len++;

    if (c != '\0') {
        current = current->map[c];
        goto ITERATE;
    }
    
    END:

    current->value = value;

    return current;
}

void pkrsrv_trie_unset__unicode(pkrsrv_trie__unicode_t* trie, char* key) {
    pkrsrv_trie_node__unicode_t* current;
    pkrsrv_trie_node__unicode_t* to_remove;

    current = pkrsrv_trie_node_key__unicode(trie->root, key);

    if (!current) {
        return;
    }

    to_remove = current;
    current = current->parent;

    to_remove->value = NULL;
    to_remove->parent->len--;
    free(to_remove);

    ITERATE:
    
    if (!current) {
        goto END;
    }

    if (current->len == 0 && !current->value) {
        to_remove = current;
        current = current->parent;
        to_remove->parent->len--;
        free(to_remove);

        goto ITERATE;
    }

    END:;
}

pkrsrv_trie_node__unicode_t* pkrsrv_trie_get__unicode(pkrsrv_trie__unicode_t* trie, char* key) {
    pkrsrv_trie_node__unicode_t* node = pkrsrv_trie_node_key__unicode(trie->root, key);
    return node;
}

pkrsrv_trie_node__unicode_t* pkrsrv_trie_node_init__unicode() {
    pkrsrv_trie_node__unicode_t* node = malloc(sizeof(pkrsrv_trie_node__unicode_t));
    memset(node, 0, sizeof(pkrsrv_trie_node__unicode_t));

    return node;
}

pkrsrv_trie_node__unicode_t* pkrsrv_trie_node_key__unicode(pkrsrv_trie_node__unicode_t* root, char* key) {
    pkrsrv_trie_node__unicode_t* current = root;
    
    pkrsrv_trie_node__unicode_t* result = NULL;
    
    for (int i=0; key[i] != '\0'; i++) {
        current = current->map[(unsigned char) key[i]];
        
        if (!current && key[i] != '\0') {
            return NULL;
        }
        
        if (!current) {
            break;
        }

        result = current;
    }
    
    return result;
}

void pkrsrv_trie_free__unicode(pkrsrv_trie__unicode_t* trie) {
    pkrsrv_trie_node_free__unicode(trie->root);
    free(trie);
}

static void free_node__unicode(pkrsrv_trie_node__unicode_t* node) {
    for (int i=0; i < 256; i++) {
        free_node__unicode(node->map[i]);
    }
    
    node->value = NULL;
    free(node);
}

void pkrsrv_trie_node_free__unicode(pkrsrv_trie_node__unicode_t* node) {
    free_node__unicode(node);
}

pkrsrv_trie__ascii_t* pkrsrv_trie_init__ascii() {
    pkrsrv_trie__ascii_t* pkrsrv_trie = malloc(sizeof(pkrsrv_trie__ascii_t));
    pkrsrv_trie->root = pkrsrv_trie_node_init__ascii();

    return pkrsrv_trie;
}

pkrsrv_trie_node__ascii_t* pkrsrv_trie_set__ascii(pkrsrv_trie__ascii_t* trie, char* key, void* value) {
    pkrsrv_trie_node__ascii_t* current = trie->root;
    
    int i = 0;
    unsigned char c;
    pkrsrv_trie_node__ascii_t* latest;

    ITERATE:
    
    for (; (c = key[i]) != '\0'; i++) {
        latest = current->map[c];
        
        if (!latest) {
            break;
        }

        current = latest;
    }
    
    i++;

    if (c == '\0') {
        goto END;
    }

    current->map[c] = pkrsrv_trie_node_init__ascii();
    current->map[c]->parent = current;
    current->len++;

    if (c != '\0') {
        current = current->map[c];
        goto ITERATE;
    }
    
    END:

    current->value = value;

    return current;
}

void pkrsrv_trie_unset__ascii(pkrsrv_trie__ascii_t* trie, char* key) {
    pkrsrv_trie_node__ascii_t* current;
    pkrsrv_trie_node__ascii_t* to_remove;

    current = pkrsrv_trie_node_key__ascii(trie->root, key);

    if (!current) {
        return;
    }

    to_remove = current;
    current = current->parent;

    to_remove->value = NULL;
    to_remove->parent->len--;
    free(to_remove);

    ITERATE:
    
    if (!current) {
        goto END;
    }

    if (current->len == 0 && !current->value) {
        to_remove = current;
        current = current->parent;
        to_remove->parent->len--;
        free(to_remove);

        goto ITERATE;
    }

    END:;
}

pkrsrv_trie_node__ascii_t* pkrsrv_trie_get__ascii(pkrsrv_trie__ascii_t* trie, char* key) {
    pkrsrv_trie_node__ascii_t* node = pkrsrv_trie_node_key__ascii(trie->root, key);
    return node;
}

pkrsrv_trie_node__ascii_t* pkrsrv_trie_node_init__ascii() {
    pkrsrv_trie_node__ascii_t* node = malloc(sizeof(pkrsrv_trie_node__ascii_t));
    memset(node, 0, sizeof(pkrsrv_trie_node__ascii_t));

    return node;
}

pkrsrv_trie_node__ascii_t* pkrsrv_trie_node_key__ascii(pkrsrv_trie_node__ascii_t* root, char* key) {
    pkrsrv_trie_node__ascii_t* current = root;
    
    pkrsrv_trie_node__ascii_t* result = NULL;
    
    for (int i=0; key[i] != '\0'; i++) {
        current = current->map[(unsigned char) key[i]];
        
        if (!current && key[i] != '\0') {
            return NULL;
        }
        
        if (!current) {
            break;
        }

        result = current;
    }
    
    return result;
}

void pkrsrv_trie_free__ascii(pkrsrv_trie__ascii_t* trie) {
    pkrsrv_trie_node_free__ascii(trie->root);
    free(trie);
}

static void free_node__ascii(pkrsrv_trie_node__ascii_t* node) {
    for (int i=0; i < 128; i++) {
        free_node__ascii(node->map[i]);
    }
    
    node->value = NULL;
    free(node);
}

void pkrsrv_trie_node_free__ascii(pkrsrv_trie_node__ascii_t* node) {
    free_node__ascii(node);
}

pkrsrv_trie__index_t* pkrsrv_trie_init__index() {
    pkrsrv_trie__index_t* pkrsrv_trie = malloc(sizeof(pkrsrv_trie__index_t));
    pkrsrv_trie->root = pkrsrv_trie_node_init__index();

    return pkrsrv_trie;
}

pkrsrv_trie_node__index_t* pkrsrv_trie_set__index(pkrsrv_trie__index_t* trie, char* key, void* value) {
    pkrsrv_trie_node__index_t* current = trie->root;
    
    int i = 0;
    unsigned char c;
    pkrsrv_trie_node__index_t* latest;

    ITERATE:
    
    for (; (c = key[i]) != '\0'; i++) {
        latest = current->map[c];
        
        if (!latest) {
            break;
        }

        current = latest;
    }
    
    i++;

    if (c == '\0') {
        goto END;
    }

    current->map[c] = pkrsrv_trie_node_init__index();
    current->map[c]->parent = current;
    current->len++;

    if (c != '\0') {
        current = current->map[c];
        goto ITERATE;
    }
    
    END:

    current->value = value;

    return current;
}

void pkrsrv_trie_unset__index(pkrsrv_trie__index_t* trie, char* key) {
    pkrsrv_trie_node__index_t* current;
    pkrsrv_trie_node__index_t* to_remove;

    current = pkrsrv_trie_node_key__index(trie->root, key);

    if (!current) {
        return;
    }

    to_remove = current;
    current = current->parent;

    to_remove->value = NULL;
    to_remove->parent->len--;
    free(to_remove);

    ITERATE:
    
    if (!current) {
        goto END;
    }

    if (current->len == 0 && !current->value) {
        to_remove = current;
        current = current->parent;
        to_remove->parent->len--;
        free(to_remove);

        goto ITERATE;
    }

    END:;
}

pkrsrv_trie_node__index_t* pkrsrv_trie_get__index(pkrsrv_trie__index_t* trie, char* key) {
    pkrsrv_trie_node__index_t* node = pkrsrv_trie_node_key__index(trie->root, key);
    return node;
}

pkrsrv_trie_node__index_t* pkrsrv_trie_node_init__index() {
    pkrsrv_trie_node__index_t* node = malloc(sizeof(pkrsrv_trie_node__index_t));
    memset(node, 0, sizeof(pkrsrv_trie_node__index_t));

    return node;
}

pkrsrv_trie_node__index_t* pkrsrv_trie_node_key__index(pkrsrv_trie_node__index_t* root, char* key) {
    pkrsrv_trie_node__index_t* current = root;
    
    pkrsrv_trie_node__index_t* result = NULL;
    
    for (int i=0; key[i] != '\0'; i++) {
        current = current->map[(unsigned char) key[i]];
        
        if (!current && key[i] != '\0') {
            return NULL;
        }
        
        if (!current) {
            break;
        }

        result = current;
    }
    
    return result;
}

void pkrsrv_trie_free__index(pkrsrv_trie__index_t* trie) {
    pkrsrv_trie_node_free__index(trie->root);
    free(trie);
}

static void free_node__index(pkrsrv_trie_node__index_t* node) {
    for (int i=0; i < 10; i++) {
        free_node__index(node->map[i]);
    }
    
    node->value = NULL;
    free(node);
}

void pkrsrv_trie_node_free__index(pkrsrv_trie_node__index_t* node) {
    free_node__index(node);
}