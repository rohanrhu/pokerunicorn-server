/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup sugar Sugars
 * \brief Cute macros
 */

/**
 * @{
 * \ingroup sugar
 */

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define VARCAT(left, right) left##right

#define LIST_FOREACH(list, node) { \
    int VARCAT(node, _i) = -1; \
    __typeof__(list->next) _##node = NULL; \
    __typeof__(list->next) _next_##node = list->next; \
    while (_next_##node) { \
        VARCAT(node, _i)++; \
        __typeof__(list->next) node = _next_##node; \
        _next_##node = (_next_##node)->next;

#define END_FOREACH \
    }}

#define LISTIFY(type) \
    type next; \
    type prev; \
    type terminal; \
    int length;

#define ITEMIFY(type) \
    type next; \
    type prev;

#define LIST_INIT(list) \
    list->prev = NULL; \
    list->next = NULL; \
    list->terminal = NULL; \
    list->length = 0;

#define LIST_ITEM_INIT(list) \
    list->prev = NULL; \
    list->next = NULL;

#define LIST_APPEND(list, node) \
    if (!list->next) { \
        list->next = node; \
    } \
    \
    node->prev = list->terminal; \
    if (list->terminal) { \
        list->terminal->next = node; \
    } \
    list->terminal = node; \
    \
    list->length++;

#define LIST_PREPEND(list, node) \
    node->prev = NULL; \
    if (!list->next) { \
        list->next = node; \
        list->terminal = node; \
    } else { \
        node->next = list->next; \
        list->next->prev = node; \
        list->next = node; \
    } \
    list->length++;

#define LIST_REMOVE(list, node) \
    if (list->terminal == node) { \
        list->terminal = node->next \
                       ? node->next \
                       : node->prev; \
    } \
    if (list->next == node) { \
        list->next = node->next; \
    } \
    \
    if (node->next) { \
        node->next->prev = node->prev; \
    } \
    \
    if (node->prev) { \
        node->prev->next = node->next; \
    } \
    list->length--;

#define LIST_SORT(list, a, b, cmp_expr) { \
    __typeof__(list->next) array[list->length]; \
    LIST_FOREACH(list, node) \
        array[node_i] = node; \
    END_FOREACH \
    for (int i = 0; i < list->length; i++) { \
        for (int j = i + 1; j < list->length; j++) { \
            if (cmp_expr) { \
                __typeof__(list->next) tmp = array[i]; \
                array[i] = array[j]; \
                airray[j] = tmp; \
            } \
        } \
    } \
    if (list->length) { \
        list->next = array[0]; \
        list->terminal = array[list->length - 1]; \
    } \
}

#define DECL_LIST(list_name, item_name, members, list_new_body, item_new_body, list_free_body, item_free_body) \
    typedef struct list_name { \
        LISTIFY(item_name##_t*) \
        REF_COUNTEDIFY() \
    } list_name##_t; \
    typedef struct item_name { \
        ITEMIFY(item_name##_t*) \
        REF_COUNTEDIFY() \
        members \
    } item_name##_t; \
    list_name##_t* list_name##_new() { \
        list_name##_t* list = malloc(sizeof(list_name##_t)); \
        LIST_INIT(list); \
        REF_COUNTED_INIT(list, list_name##_free); \
        list_new_body \
        return list; \
    } \
    item_name##_t* item_name##_new() { \
        item_name##_t* item = malloc(sizeof(item_name##_t)); \
        LIST_ITEM_INIT(item); \
        REF_COUNTED_INIT(item, item_name##_free); \
        item_new_body \
        return item; \
    } \
    void list_name##_free(list_name##_t* list) { \
        LIST_FOREACH(list, item) \
            item_name##_free(item); \
        END_FOREACH \
        LIST_FOREACH(list, item) \
            LEAVE(item); \
        END_FOREACH \
        list_free_body \
        free(list); \
    } \
    void item_name##_free(item_name##_t* item) { \
        item_free_body \
        free(item); \
    } \
    void list_name##_append(list_name##_t* list, item_name##_t* item) { \
        USE(item); \
        LIST_APPEND(list, item); \
    } \
    void list_name##_remove(list_name##_t* list, item_name##_t* item) { \
        LIST_REMOVE(list, item); \
        LEAVE(item); \
    }

#define R(expr) PKRSRV_REF_BY(expr)

#define REF_COUNTEDIFY() PKRSRV_REF_COUNTEDIFY();
#define REF_COUNTED_INIT(obj, free_f) PKRSRV_REF_COUNTED_INIT(obj, free_f);
#define USE(expr) PKRSRV_REF_COUNTED_USE(expr)
#define LEAVE(expr) PKRSRV_REF_COUNTED_LEAVE(expr)

#define $(expr) expr; USE(expr);

/**
 * @}
 */