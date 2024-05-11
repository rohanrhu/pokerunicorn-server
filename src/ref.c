/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdlib.h>
#include <assert.h>

#include "../include/ref.h"
#include "../include/util.h"
#include "../include/string.h"

void pkrsrv_ref_counted_init(pkrsrv_ref_counted_t* ref_counted, pkrsrv_ref_free_f_t free_f) {
    ref_counted->count = 0;
    ref_counted->free_f = free_f;
    pthread_mutex_init(&ref_counted->mutex, NULL);
}

void pkrsrv_ref_counted_use(pkrsrv_ref_counted_t* ref_counted) {
    pthread_mutex_lock(&ref_counted->mutex);
    ref_counted->count++;
    pthread_mutex_unlock(&ref_counted->mutex);
}

void pkrsrv_ref_counted_leave(void** obj_vp, pkrsrv_ref_counted_t* ref_counted) {
    pthread_mutex_lock(&ref_counted->mutex);
    
    void* to_free = *obj_vp;
    
    PKRSRV_UTIL_ASSERT(ref_counted->count > 0);
    
    ref_counted->count--;

    if (ref_counted->count == 0) {
        *obj_vp = NULL;
        pthread_mutex_unlock(&ref_counted->mutex);
        pthread_mutex_destroy(&ref_counted->mutex);
        ref_counted->free_f(to_free);
    } else {
        pthread_mutex_unlock(&ref_counted->mutex);
    }
}

pkrsrv_ref_t* pkrsrv_ref_new(void* obj, pkrsrv_ref_counted_t* ref_counted) {
    pkrsrv_ref_t* ref = malloc(sizeof(pkrsrv_ref_t));
    ref->count = 0;
    ref->obj = obj;
    ref->ref_counted = ref_counted;
    return ref;
}

void pkrsrv_ref_free(pkrsrv_ref_t* ref) {
    free(ref);
}

void pkrsrv_ref_use(pkrsrv_ref_t* ref) {
    pkrsrv_ref_counted_use(ref->ref_counted);
    ref->count++;
}

void pkrsrv_ref_leave(pkrsrv_ref_t** p_ref) {
    pkrsrv_ref_t* ref = *p_ref;
    PKRSRV_UTIL_ASSERT(ref);
    if (!ref) {
        return;
    }
    
    pkrsrv_ref_counted_leave(&ref->obj, ref->ref_counted);
    ref->count--;
    PKRSRV_UTIL_ASSERT(ref->count >= 0);
    if (ref->count == 0) {
        pkrsrv_ref_free(ref);
    }
}

void pkrsrv_ref_assign(pkrsrv_ref_t** p_dst, pkrsrv_ref_t** p_src) {
    pkrsrv_ref_t* dst = *p_dst;
    pkrsrv_ref_t* src = *p_src;
    
    dst->ref_counted->count -= dst->count;
    src->ref_counted->count += dst->count;
    PKRSRV_UTIL_ASSERT(dst->ref_counted->count >= 0);
    if (dst->ref_counted->count == 0) {
        dst->ref_counted->free_f(dst->obj);
    }

    src->count = dst->count;
    *p_dst = src;

    free(dst);
}

void pkrsrv_ref_set(pkrsrv_ref_t* ref, void* obj) {
    int offset = ((uint64_t) (ref->ref_counted)) - ((uint64_t) (ref->obj));
    pkrsrv_ref_counted_t ref_counted = *ref->ref_counted;
    void* to_free = ref->obj;
    ref->obj = obj;
    ref->ref_counted = ref->obj + offset;
    *(ref->ref_counted) = ref_counted;
    ref->ref_counted->free_f(to_free);
}