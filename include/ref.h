/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#pragma once

/**
 * \defgroup ref Atomic Reference Counting
 * \brief Atomic reference counting for thread-safe reference counting
 */

#include <stdio.h>
#include <pthread.h>

#include "../include/util.h"

typedef void (*pkrsrv_ref_free_f_t)(void*);

/**
 * \ingroup ref
 * \memberof pkrsrv_ref_counted
 * \attention Always use this after the dynamic allocation of a new object.
 * \brief Initializes a reference-counted object.
 * Should be called in the constructor of the object after the dynamic allocation.
 */
#define PKRSRV_REF_COUNTED_INIT(obj, free_f) \
    pkrsrv_ref_counted_init(&(obj->ref_counted), (pkrsrv_ref_free_f_t) free_f);

/**
 * \ingroup ref
 * \memberof pkrsrv_ref_counted
 * \note This method is thread-safe per every ref-counted object.
 * \brief Increments the reference count of a reference-counted object.
 */
#define PKRSRV_REF_COUNTED_USE(obj) \
    PKRSRV_UTIL_ASSERT(obj); \
    if (obj != NULL) { \
        pkrsrv_util_verbose("Referencing: %s\n", #obj); \
        \
        pkrsrv_ref_counted_use(&(obj->ref_counted)); \
    }

/**
 * \ingroup ref
 * \memberof pkrsrv_ref_counted
 * \note This method is thread-safe per every ref-counted object.
 * \brief Decrements the reference count of a reference-counted object.
 */
#define PKRSRV_REF_COUNTED_LEAVE(obj) \
    PKRSRV_UTIL_ASSERT(obj); \
    { \
        if (obj != NULL) { \
            __typeof__(obj) derefing = obj; \
            pkrsrv_util_verbose("Dereferencing: %s\n", #obj); \
            pkrsrv_ref_counted_leave((void **) (&(derefing)), &(derefing->ref_counted)); \
        } \
    }

/**
 * \ingroup ref
 * \attention Use in struct definition to make a structure reference-counted
 * \brief Makes a structure reference-counted
 */
#define PKRSRV_REF_COUNTEDIFY() \
    pkrsrv_ref_counted_t ref_counted;
#define PKRSRV_REF_BY(name) name##_ref
#define PKRSRV_REF(type, name) \
    type name; \
    pkrsrv_ref_t* PKRSRV_REF_BY(name);
#define PKRSRV_REF_NEW(path, expr) \
    path = expr; \
    PKRSRV_REF_BY(path) = pkrsrv_ref_new((void *) path, &(path->ref_counted));
#define PKRSRV_REF_ASSIGN(var, val) \
    pkrsrv_ref_assign(&(PKRSRV_REF_BY(var)), &(PKRSRV_REF_BY(val))); \
    var = val;
#define PKRSRV_REF_SET(ref, obj) \
    ref = obj; \
    pkrsrv_ref_set(PKRSRV_REF_BY(ref), (void *) (ref));
#define PKRSRV_REF_USE(name) pkrsrv_ref_use(PKRSRV_REF_BY(name));
#define PKRSRV_REF_LEAVE(name) pkrsrv_ref_leave(&(PKRSRV_REF_BY(name)));
#define PKRSRV_REF_ARG(type, name) type name, pkrsrv_ref_t* name##_ref
#define PKRSRV_REF_PASS(name) pkrsrv_ref_new((void *) name, &(name->ref_counted));

typedef struct pkrsrv_ref_counted pkrsrv_ref_counted_t;
typedef struct pkrsrv_ref pkrsrv_ref_t;

/**
 * \ingroup ref
 * \brief Reference-counted object structure
 */
struct pkrsrv_ref_counted {
    int count;
    pkrsrv_ref_free_f_t free_f;
    pthread_mutex_t mutex;
};

struct pkrsrv_ref {
    int count;
    void* obj;
    pkrsrv_ref_counted_t* ref_counted;
};

/**
 * \ingroup ref
 * \memberof pkrsrv_ref_counted
 * \protected
 * \brief Initializes a reference-counted object.
 * Should be called in the constructor of the object after the dynamic allocation.
 */
void pkrsrv_ref_counted_init(pkrsrv_ref_counted_t* ref_counted, pkrsrv_ref_free_f_t free_f);
/**
 * \ingroup ref
 * \memberof pkrsrv_ref_counted
 * \protected
 * \brief Increments the reference count of a reference-counted object.
 */
void pkrsrv_ref_counted_use(pkrsrv_ref_counted_t* ref_counted);
/**
 * \ingroup ref
 * \memberof pkrsrv_ref_counted
 * \protected
 * \brief Decrements the reference count of a reference-counted object.
 */
void pkrsrv_ref_counted_leave(void** obj_vp, pkrsrv_ref_counted_t* ref_counted);

pkrsrv_ref_t* pkrsrv_ref_new(void* obj, pkrsrv_ref_counted_t* ref_counted);
void pkrsrv_ref_free(pkrsrv_ref_t* ref);
void pkrsrv_ref_use(pkrsrv_ref_t* ref);
void pkrsrv_ref_leave(pkrsrv_ref_t** ref);
void pkrsrv_ref_assign(pkrsrv_ref_t** dst, pkrsrv_ref_t** src);
void pkrsrv_ref_set(pkrsrv_ref_t* ref, void* obj);