/**
 * @file   kernel.h
 *
 * @brief kernel data structures used in os.c.
 *
 * CSC 460/560 Real Time Operating Systems - Mantis Cheng
 *
 * @author Scott Craig
 * @author Justin Tanner
 */
#ifndef __KERNEL_H__
#define __KERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include "os.h"

/** Disable default prescaler to make processor speed 8 MHz. */

#define Disable_Interrupt()     asm volatile ("cli"::)
#define Enable_Interrupt()     asm volatile ("sei"::)

/* Task stack size */
#define WORKSPACE 256

/** The RTOS timer's prescaler divisor */
#define TIMER_PRESCALER 8

#define CYCLES_PER_MS ((F_CPU / TIMER_PRESCALER) / 1000)

/** The number of clock cycles in one "tick" or 5 ms */
#define TICK_CYCLES (CYCLES_PER_MS * TICK)

#define MAX_SERVICE_COUNT 15
#define MAX_SERVICE_SUBSCRIPTION_COUNT (MAX_SERVICE_COUNT * 3)

/* Typedefs and data structures. */

typedef void (*voidfuncvoid_ptr) (void);      /* pointer to void f(void) */

/**
 * @brief This is the set of states that a task can be in at any given time.
 */
typedef enum
{
    DEAD = 0,
    RUNNING,
    READY,
    WAITING
}
task_state_t;

/**
 * @brief This is the set of kernel requests, i.e., a request code for each system call.
 */
typedef enum
{
    NONE = 0,
    TIMER_EXPIRED,
    TASK_CREATE,
    TASK_TERMINATE,
    TASK_NEXT,
    TASK_GET_ARG,
    TASK_PERIODIC_START,
    SERVICE_SUBSCRIBE,
    SERVICE_PUBLISH,
    SERVICE_RECEIVE
}
kernel_request_t;

/**
 * @brief The arguments required to create a task.
 */
typedef struct
{
    /** The code the new task is to run.*/
    voidfuncvoid_ptr f;
    /** A new task may be created with an argument that it can retrieve later. */
    int arg;
    /** Priority of the new task: RR, PERIODIC, SYSTEM */
    uint8_t level;
    /** If the new task is PERIODIC, this is its name in the PPP array. */
    uint8_t name;

    // FIXME: make more lean (sometimes redundant data)

    /** Periodic task parameters */
    uint16_t period, wcet, start;
}
create_args_t;


typedef struct td_struct task_descriptor_t;
/**
 * @brief All the data needed to describe the task, including its context.
 */
struct td_struct
{
    /** The stack used by the task. SP points in here when task is RUNNING. */
    uint8_t                         stack[WORKSPACE];
    /** A variable to save the hardware SP into when the task is suspended. */
    uint8_t*               volatile sp;   /* stack pointer into the "workSpace" */
    /** The state of the task in this descriptor. */
    task_state_t                    state;
    /** The argument passed to Task_Create for this task. */
    int                             arg;
    /** The priority (type) of this task. */
    uint8_t                         level;
    /* Periodic task parameters */
    uint16_t period, wcet, start;
    uint16_t next_tick;

    /** A link to the next task descriptor in the queue holding this task. */
    task_descriptor_t*              next;
};


/**
 * @brief Contains pointers to head and tail of a linked list.
 */
typedef struct
{
    /** The first item in the queue. NULL if the queue is empty. */
    task_descriptor_t*  head;
    /** The last item in the queue. Undefined if the queue is empty. */
    task_descriptor_t*  tail;
}
queue_t;

/**
   @brief Represents a service (publish-subscribe communication channel)
 */

struct service
{
    service_subscription *subscriptions;
    volatile int16_t value;
};

struct service_subscription
{
    struct service *service;
    struct service_subscription *next;
    task_descriptor_t *subscriber;
    bool waiting;
    volatile bool unread;
};

#ifdef __cplusplus
}
#endif

#endif

