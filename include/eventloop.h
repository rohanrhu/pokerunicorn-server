/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

/*
 * Beware of the following:
 * * If you are accessing/modifing a task, it is ok to do that in task functions.
 *   But if you are accessing/modifing a task in another thread, you should use
 *   `task->eventloop->mutex`.
 * * Some of eventloop module functions are thread-safe, some are not.
 *   The ones that are provided as API are thread-safe, internal ones are not.
 *   Internal non-thread-safe ones are used in thread-safe functions.
 * * Task functions are NOT called between `task->eventloop->mutex` lock/unlock.
 */

#pragma once

/**
 * \defgroup eventloop Event Loop
 * \brief Event loop and task management things.
 */

#include <stdbool.h>
#include <pthread.h>

#include "sugar.h"
#include "ref.h"
#include "uniqid.h"

static int tasks_count;

typedef struct pkrsrv_eventloop pkrsrv_eventloop_t;

typedef struct pkrsrv_eventloop_tasks pkrsrv_eventloop_tasks_t;
typedef struct pkrsrv_eventloop_task pkrsrv_eventloop_task_t;

typedef void (*pkrsrv_eventloop_task_func_t)(pkrsrv_eventloop_task_t* task);

/**
 * \ingroup eventloop
 * \implements pkrsrv_ref_counted
 * Event loop structure
 */
struct pkrsrv_eventloop {
    PKRSRV_REF_COUNTEDIFY();
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool is_running;
    pkrsrv_eventloop_tasks_t* tasks;
};

/**
 * \ingroup eventloop
 * \implements pkrsrv_ref_counted
 * Event loop tasks list structure
 */
struct pkrsrv_eventloop_tasks {
    PKRSRV_REF_COUNTEDIFY();
    LISTIFY(pkrsrv_eventloop_task_t*);
};

/**
 * \ingroup eventloop
 * \implements pkrsrv_ref_counted
 * Event loop task structure
 */
struct pkrsrv_eventloop_task {
    PKRSRV_REF_COUNTEDIFY();
    ITEMIFY(pkrsrv_eventloop_task_t*);
    pkrsrv_uniqid_uuid_t id;
    pkrsrv_eventloop_t* eventloop;
    pkrsrv_eventloop_task_func_t func;
    pthread_t delay_thread;
    int delay;
    void* params;
    bool is_queued;
    bool is_popped;
    bool is_called;
    bool is_cancelled;
};

/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 * \return A new event loop
 */
pkrsrv_eventloop_t* pkrsrv_eventloop_new();
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 */
void pkrsrv_eventloop_free(pkrsrv_eventloop_t* eventloop);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 */
void pkrsrv_eventloop_run(pkrsrv_eventloop_t* eventloop);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 */
void pkrsrv_eventloop_stop(pkrsrv_eventloop_t* eventloop);

/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 */
pkrsrv_eventloop_task_t* pkrsrv_eventloop_task_new(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t fn, void* params);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 */
void pkrsrv_eventloop_task_free(pkrsrv_eventloop_task_t* task);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 */
void pkrsrv_eventloop_task_run(pkrsrv_eventloop_task_t* task);

/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_tasks
 */
pkrsrv_eventloop_tasks_t* pkrsrv_eventloop_tasks_new();
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_tasks
 */
void pkrsrv_eventloop_tasks_free(pkrsrv_eventloop_tasks_t* tasks);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_tasks
 */
void pkrsrv_eventloop_tasks_push(pkrsrv_eventloop_tasks_t* tasks, pkrsrv_eventloop_task_t* task);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_tasks
 */
void pkrsrv_eventloop_tasks_prepend(pkrsrv_eventloop_tasks_t* tasks, pkrsrv_eventloop_task_t* task);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_tasks
 */
void pkrsrv_eventloop_tasks_remove(pkrsrv_eventloop_tasks_t* tasks, pkrsrv_eventloop_task_t* task);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_tasks
 */
pkrsrv_eventloop_task_t* pkrsrv_eventloop_tasks_pop(pkrsrv_eventloop_tasks_t* tasks);

/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 * Creates a new task and appends it into the event loop queue
 */
pkrsrv_eventloop_task_t* pkrsrv_eventloop_call(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t fn, void* params);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 * Creates a new task and appends it into the event loop queue after timeout
 */
pkrsrv_eventloop_task_t* pkrsrv_eventloop_call_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params, int delay);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 * Creates a new task and prepends it into the event loop queue
 */
pkrsrv_eventloop_task_t* pkrsrv_eventloop_call_immediate(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop
 * Creates a new task and appends it into the event loop queue after timeout
 */
pkrsrv_eventloop_task_t* pkrsrv_eventloop_call_immediate_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_func_t func, void* params, int delay);

/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 * Appends the task into the event loop queue
 */
void pkrsrv_eventloop_task_call(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 * Appends the task into the event loop queue after timeout
 */
void pkrsrv_eventloop_task_call_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task, int delay);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 * Prepends the task into the event loop queue
 */
void pkrsrv_eventloop_task_call_immediate(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task);
/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 * Appends the task into the event loop queue after timeout
 */
void pkrsrv_eventloop_task_call_immediate_after(pkrsrv_eventloop_t* eventloop, pkrsrv_eventloop_task_t* task, int delay);

/**
 * \ingroup eventloop
 * \memberof pkrsrv_eventloop_task
 * \memberof Cancels the task
 */
void pkrsrv_eventloop_task_cancel(pkrsrv_eventloop_task_t* task);