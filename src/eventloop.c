/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdbool.h>
#include <pthread.h>

#include "../include/eventloop.h"

#include "../include/sugar.h"
#include "../include/util.h"
#include "../include/ref.h"
#include "../include/uniqid.h"

static int tasks_count = 0;

pkrsrv_eventloop_t* pkrsrv_eventloop_new() {
    pkrsrv_eventloop_t* eventloop = malloc(sizeof(pkrsrv_eventloop_t));
    PKRSRV_REF_COUNTED_INIT(eventloop, pkrsrv_eventloop_free);
    pthread_mutex_init(&eventloop->mutex, NULL);
    pthread_cond_init(&eventloop->cond, NULL);
    eventloop->is_running = false;
    eventloop->tasks = pkrsrv_eventloop_tasks_new();
    PKRSRV_REF_COUNTED_USE(eventloop->tasks);

    return eventloop;
}

void pkrsrv_eventloop_free(pkrsrv_eventloop_t* eventloop) {
    PKRSRV_REF_COUNTED_LEAVE(eventloop->tasks);
    pthread_mutex_destroy(&eventloop->mutex);
    pkrsrv_eventloop_tasks_free(eventloop->tasks);
    free(eventloop);
}

void pkrsrv_eventloop_run(pkrsrv_eventloop_t* eventloop) {
    eventloop->is_running = true;

    while (eventloop->is_running) {
        pthread_mutex_lock(&eventloop->mutex);
        
        while (eventloop->tasks->next == NULL) {
            pthread_cond_wait(&eventloop->cond, &eventloop->mutex);

            if (!eventloop->is_running) {
                LIST_FOREACH(eventloop->tasks, task)
                    pkrsrv_eventloop_tasks_remove(eventloop->tasks, task);
                    PKRSRV_REF_COUNTED_LEAVE(task);
                END_FOREACH

                pthread_mutex_unlock(&eventloop->mutex);

                return;
            }
        }

        pkrsrv_eventloop_task_t* task = pkrsrv_eventloop_tasks_pop(eventloop->tasks);

        if (task) {
            bool is_cancelled = task->is_cancelled;
            pthread_mutex_unlock(&task->eventloop->mutex);
            
            if (!is_cancelled) {
                pkrsrv_eventloop_task_run(task);
                pthread_mutex_lock(&task->eventloop->mutex);
                task->is_called = true;
                pthread_mutex_unlock(&task->eventloop->mutex);
            }

            PKRSRV_REF_COUNTED_LEAVE(task);
        } else {
            pthread_mutex_unlock(&eventloop->mutex);
        }
    }
}

void pkrsrv_eventloop_stop(pkrsrv_eventloop_t* eventloop) {
    pthread_mutex_lock(&eventloop->mutex);
    eventloop->is_running = false;
    pthread_cond_signal(&eventloop->cond);
    pthread_mutex_unlock(&eventloop->mutex);
}

pkrsrv_eventloop_task_t* pkrsrv_eventloop_task_new(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params) {
    pkrsrv_eventloop_task_t* task = malloc(sizeof(pkrsrv_eventloop_task_t));
    PKRSRV_REF_COUNTED_INIT(task, pkrsrv_eventloop_task_free);
    LIST_ITEM_INIT(task);
    task->id = pkrsrv_uniqid_generate();
    task->eventloop = eventloop;
    task->func = func;
    task->params = params;
    task->delay = 0;
    task->is_queued = false;
    task->is_popped = false;
    task->is_called = false;
    task->is_cancelled = false;

    tasks_count++;

    return task;
}

void pkrsrv_eventloop_task_free(pkrsrv_eventloop_task_t* task) {
    if (task->params) {
        free(task->params);
    }

    free(task);

    tasks_count--;
}

void pkrsrv_eventloop_task_run(pkrsrv_eventloop_task_t* task) {
    task->func(task);
}

pkrsrv_eventloop_tasks_t* pkrsrv_eventloop_tasks_new() {
    pkrsrv_eventloop_tasks_t* tasks = malloc(sizeof(pkrsrv_eventloop_tasks_t));
    PKRSRV_REF_COUNTED_INIT(tasks, pkrsrv_eventloop_tasks_free);
    LIST_INIT(tasks);

    return tasks;
}

void pkrsrv_eventloop_tasks_free(pkrsrv_eventloop_tasks_t* tasks) {
    LIST_FOREACH(tasks, task)
        PKRSRV_REF_COUNTED_LEAVE(task);
    END_FOREACH
    
    free(tasks);
}

void pkrsrv_eventloop_tasks_push(pkrsrv_eventloop_tasks_t* tasks, pkrsrv_eventloop_task_t* task) {
    PKRSRV_REF_COUNTED_USE(task);
    LIST_APPEND(tasks, task);
    task->is_queued = true;
}

void pkrsrv_eventloop_tasks_prepend(pkrsrv_eventloop_tasks_t* tasks, pkrsrv_eventloop_task_t* task) {
    PKRSRV_REF_COUNTED_USE(task);
    LIST_PREPEND(tasks, task);
    task->is_queued = true;
}

pkrsrv_eventloop_task_t* pkrsrv_eventloop_tasks_pop(pkrsrv_eventloop_tasks_t* tasks) {
    pkrsrv_eventloop_task_t* task = tasks->next;
    if (!task) {
        return NULL;
    }

    task->is_popped = true;

    LIST_REMOVE(tasks, task);
    PKRSRV_REF_COUNTED_LEAVE(task);

    return task;
}

void pkrsrv_eventloop_tasks_remove(pkrsrv_eventloop_tasks_t* tasks, pkrsrv_eventloop_task_t* task) {
    PKRSRV_UTIL_ASSERT(task->is_queued);
    LIST_REMOVE(tasks, task);
    PKRSRV_REF_COUNTED_LEAVE(task);
}

pkrsrv_eventloop_task_t* pkrsrv_eventloop_call(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params) {
    pkrsrv_eventloop_task_t* task = pkrsrv_eventloop_task_new(eventloop, func, params);
    PKRSRV_REF_COUNTED_USE(task);

    pthread_mutex_lock(&eventloop->mutex);
    pkrsrv_eventloop_tasks_push(eventloop->tasks, task);
    pthread_cond_signal(&eventloop->cond);
    pthread_mutex_unlock(&eventloop->mutex);

    return task;
}

void pkrsrv_eventloop_call_after_thread_f(pkrsrv_eventloop_task_t* task) {
    pkrsrv_util_msleep(task->delay);

    pthread_mutex_lock(&task->eventloop->mutex);

    if (!task->is_cancelled) {
        pkrsrv_eventloop_tasks_push(task->eventloop->tasks, task);
        pthread_cond_signal(&task->eventloop->cond);
    } else {
        PKRSRV_REF_COUNTED_LEAVE(task);
    }

    pthread_mutex_unlock(&task->eventloop->mutex);

    PKRSRV_REF_COUNTED_LEAVE(task);
}

pkrsrv_eventloop_task_t* pkrsrv_eventloop_call_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params, int delay) {
    pkrsrv_eventloop_task_t* task = pkrsrv_eventloop_task_new(eventloop, func, params);
    PKRSRV_REF_COUNTED_USE(task);
    PKRSRV_REF_COUNTED_USE(task);
    task->delay = delay;

    pthread_create(&task->delay_thread, NULL, (void*) &pkrsrv_eventloop_call_after_thread_f, task);

    return task;
}

pkrsrv_eventloop_task_t* pkrsrv_eventloop_call_immediate(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params) {
    pkrsrv_eventloop_task_t* task = pkrsrv_eventloop_task_new(eventloop, func, params);
    PKRSRV_REF_COUNTED_USE(task);

    pthread_mutex_lock(&eventloop->mutex);
    pkrsrv_eventloop_tasks_prepend(eventloop->tasks, task);
    pthread_cond_signal(&eventloop->cond);
    pthread_mutex_unlock(&eventloop->mutex);

    return task;
}

void pkrsrv_eventloop_call_immediate_after_thread_f(pkrsrv_eventloop_task_t* task) {
    pkrsrv_util_msleep(task->delay);

    pthread_mutex_lock(&task->eventloop->mutex);

    if (!task->is_cancelled) {
        pkrsrv_eventloop_tasks_prepend(task->eventloop->tasks, task);
        pthread_cond_signal(&task->eventloop->cond);
    } else {
        PKRSRV_REF_COUNTED_LEAVE(task);
    }

    pthread_mutex_unlock(&task->eventloop->mutex);

    PKRSRV_REF_COUNTED_LEAVE(task);
}

pkrsrv_eventloop_task_t* pkrsrv_eventloop_call_immediate_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params, int delay) {
    pkrsrv_eventloop_task_t* task = pkrsrv_eventloop_task_new(eventloop, func, params);
    PKRSRV_REF_COUNTED_USE(task);
    PKRSRV_REF_COUNTED_USE(task);
    task->delay = delay;

    pthread_create(&task->delay_thread, NULL, (void*) &pkrsrv_eventloop_call_immediate_after_thread_f, task);

    return task;
}

void pkrsrv_eventloop_task_call(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task) {
    PKRSRV_REF_COUNTED_USE(task);
    pthread_mutex_lock(&eventloop->mutex);
    pkrsrv_eventloop_tasks_push(eventloop->tasks, task);
    pthread_cond_signal(&eventloop->cond);
    pthread_mutex_unlock(&eventloop->mutex);
}

void pkrsrv_eventloop_task_call_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task, int delay) {
    PKRSRV_REF_COUNTED_USE(task);
    PKRSRV_REF_COUNTED_USE(task);
    task->delay = delay;
    pthread_create(&task->delay_thread, NULL, (void*) &pkrsrv_eventloop_call_after_thread_f, task);
}

void pkrsrv_eventloop_task_call_immediate(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task) {
    PKRSRV_REF_COUNTED_USE(task);
    pthread_mutex_lock(&eventloop->mutex);
    pkrsrv_eventloop_tasks_prepend(eventloop->tasks, task);
    pthread_cond_signal(&eventloop->cond);
    pthread_mutex_unlock(&eventloop->mutex);
}

void pkrsrv_eventloop_task_call_immediate_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task, int delay) {
    PKRSRV_REF_COUNTED_USE(task);
    PKRSRV_REF_COUNTED_USE(task);
    task->delay = delay;
    pthread_create(&task->delay_thread, NULL, (void*) &pkrsrv_eventloop_call_immediate_after_thread_f, task);
}

void pkrsrv_eventloop_task_cancel(pkrsrv_eventloop_task_t* task) {
    pthread_mutex_lock(&task->eventloop->mutex);

    if (task->is_cancelled) {
        goto RETURN;
    }

    task->is_cancelled = true;

    if (task->is_queued && !task->is_popped) {
        pkrsrv_eventloop_tasks_remove(task->eventloop->tasks, task);
    }

    RETURN:
    
    pthread_mutex_unlock(&task->eventloop->mutex);
}