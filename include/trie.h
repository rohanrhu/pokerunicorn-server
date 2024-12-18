/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup trie Tries
 * \brief Unicode, ASCII, Index tries. (Planned to be used for efficient storage of objects.)
 */

/**
 * \addtogroup trie
 * \ingroup trie
 * @{
 */

typedef enum pkrsrv_TRIE_TYPE pkrsrv_trie_type_t;
enum pkrsrv_TRIE_TYPE {
    pkrsrv_TRIE_TYPE_UNICODE = 1,
    pkrsrv_TRIE_TYPE_ASCII,
    pkrsrv_TRIE_TYPE_INDEX
};

typedef struct pkrsrv_trie_node__unicode pkrsrv_trie_node__unicode_t;
struct pkrsrv_trie_node__unicode {
    pkrsrv_trie_node__unicode_t* map[256];
    pkrsrv_trie_node__unicode_t* parent;
    void* value;
    int len;
};

typedef struct pkrsrv_trie_node__ascii pkrsrv_trie_node__ascii_t;
struct pkrsrv_trie_node__ascii {
    pkrsrv_trie_node__ascii_t* map[128];
    pkrsrv_trie_node__ascii_t* parent;
    void* value;
    int len;
    char index;
};

typedef struct pkrsrv_trie_node__index pkrsrv_trie_node__index_t;
struct pkrsrv_trie_node__index {
    pkrsrv_trie_node__index_t* map[10];
    pkrsrv_trie_node__index_t* parent;
    void* value;
    int len;
};

typedef struct pkrsrv_trie__unicode pkrsrv_trie__unicode_t;
struct pkrsrv_trie__unicode {
    pkrsrv_trie_node__unicode_t* root;
};

typedef struct pkrsrv_trie__ascii pkrsrv_trie__ascii_t;
struct pkrsrv_trie__ascii {
    pkrsrv_trie_node__ascii_t* root;
};

typedef struct pkrsrv_trie__index pkrsrv_trie__index_t;
struct pkrsrv_trie__index {
    pkrsrv_trie_node__index_t* root;
};

pkrsrv_trie__unicode_t* pkrsrv_trie_new__unicode();
void pkrsrv_trie_free__unicode(pkrsrv_trie__unicode_t* pkrsrv_trie);
pkrsrv_trie_node__unicode_t* pkrsrv_trie_set__unicode(pkrsrv_trie__unicode_t* pkrsrv_trie, char* key, void* value);
void pkrsrv_trie_unset__unicode(pkrsrv_trie__unicode_t* pkrsrv_trie, char* key);
pkrsrv_trie_node__unicode_t* pkrsrv_trie_get__unicode(pkrsrv_trie__unicode_t* pkrsrv_trie, char* key);

pkrsrv_trie_node__unicode_t* pkrsrv_trie_node_new__unicode();
void pkrsrv_trie_node_free__unicode(pkrsrv_trie_node__unicode_t* node);
pkrsrv_trie_node__unicode_t* pkrsrv_trie_node_key__unicode(pkrsrv_trie_node__unicode_t* root, char* key);

pkrsrv_trie__ascii_t* pkrsrv_trie_new__ascii();
void pkrsrv_trie_free__ascii(pkrsrv_trie__ascii_t* pkrsrv_trie);
pkrsrv_trie_node__ascii_t* pkrsrv_trie_set__ascii(pkrsrv_trie__ascii_t* pkrsrv_trie, char* key, void* value);
void pkrsrv_trie_unset__ascii(pkrsrv_trie__ascii_t* pkrsrv_trie, char* key);
pkrsrv_trie_node__ascii_t* pkrsrv_trie_get__ascii(pkrsrv_trie__ascii_t* pkrsrv_trie, char* key);

pkrsrv_trie_node__ascii_t* pkrsrv_trie_node_new__ascii();
void pkrsrv_trie_node_free__ascii(pkrsrv_trie_node__ascii_t* node);
pkrsrv_trie_node__ascii_t* pkrsrv_trie_node_key__ascii(pkrsrv_trie_node__ascii_t* root, char* key);

pkrsrv_trie__index_t* pkrsrv_trie_new__index();
void pkrsrv_trie_free__index(pkrsrv_trie__index_t* pkrsrv_trie);
pkrsrv_trie_node__index_t* pkrsrv_trie_set__index(pkrsrv_trie__index_t* pkrsrv_trie, char* key, void* value);
void pkrsrv_trie_unset__index(pkrsrv_trie__index_t* pkrsrv_trie, char* key);
pkrsrv_trie_node__index_t* pkrsrv_trie_get__index(pkrsrv_trie__index_t* pkrsrv_trie, char* key);

pkrsrv_trie_node__index_t* pkrsrv_trie_node_new__index();
void pkrsrv_trie_node_free__index(pkrsrv_trie_node__index_t* node);
pkrsrv_trie_node__index_t* pkrsrv_trie_node_key__index(pkrsrv_trie_node__index_t* root, char* key);

/**
 * @}
 */