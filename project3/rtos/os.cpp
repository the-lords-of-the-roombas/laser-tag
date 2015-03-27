/**
 * @file os.c
 *
 * @brief A Real Time Operating System
 *
 * Our implementation of the operating system described by Mantis Cheng in os.h.
 *
 * @author Scott Craig
 * @author Justin Tanner
 */

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>

#include "os.h"
#include "kernel.h"
#include "error_code.h"
#include "arduino_pins.h"

#ifdef INIT_ARDUINO_LIB
#include "Arduino.h"
#endif

/* Needed for memset */
/* #include <string.h> */

/** @brief main function provided by user application. The first task to run. */
extern int r_main();

// FIXME: remove:
#if 0
/** PPP and PT defined in user application. */
extern const unsigned char PPP[];

/** PPP and PT defined in user application. */
extern const unsigned int PT;
#endif

/** The task descriptor of the currently RUNNING task. */
static task_descriptor_t* cur_task = NULL;

/** Since this is a "full-served" model, the kernel is executing using its own stack. */
static volatile uint16_t kernel_sp;

/** This table contains all task descriptors, regardless of state, plus idler. */
static task_descriptor_t task_desc[MAXPROCESS + 1];

/** The special "idle task" at the end of the descriptors array. */
static task_descriptor_t* idle_task = &task_desc[MAXPROCESS];

// Services

static volatile uint16_t service_count = 0;
static Service services[MAX_SERVICE_COUNT];

static volatile uint16_t service_subscription_count = 0;
static ServiceSubscription service_subscriptions[MAX_SERVICE_SUBSCRIPTION_COUNT];

static Service * volatile requested_service = NULL;
static ServiceSubscription * volatile requested_service_subscription = NULL;

/** The current kernel request. */
static volatile kernel_request_t kernel_request;

/** Arguments for Task_Create() request. */
static volatile create_args_t kernel_request_create_args;

/** Return value for Task_Create() request. */
static volatile int kernel_request_retval;

/** Number of tasks created so far */
static queue_t dead_pool_queue;

/** The ready queue for RR tasks. Their scheduling is round-robin. */
static queue_t rr_queue;

/** The ready queue for SYSTEM tasks. Their scheduling is first come, first served. */
static queue_t system_queue;

static queue_t periodic_task_list;
task_descriptor_t *current_periodic_task = NULL;

static uint16_t volatile ticks_since_system_start = 0;

static bool periodic_tasks_running = false;
static uint16_t ticks_at_next_periodic_schedule_check = 0;
static uint16_t ticks_since_current_periodic_task = 0;

// Maybe set current_periodic_task.
// Update ticks_to_next_periodic_task.
static void kernel_select_periodic_task();

/** Error message used in OS_Abort() */
static uint8_t volatile error_msg = ERR_USER_CALLED_OS_ABORT;

#ifdef TRACE_REQUESTS
volatile bool do_trace = true;
static const int trace_size = 500;
static char trace[trace_size];
static volatile int trace_idx = 0;

static void add_to_trace(const char *str)
{
    if (!do_trace)
        return;
    while(str && *str != '\0')
    {
        trace[trace_idx] = *str;
        ++str;
        trace_idx = (trace_idx + 1) % trace_size;
    }
}

static void dump_trace()
{
    Serial.println("++++++++++");

    int ti = trace_idx;
    for(int i = 0; i < trace_size; ++i)
    {
        if (i % 20 == 0)
            Serial.print(i/20 + 1);

        if (trace[ti] != '0')
            Serial.write(trace[ti]);
        else
            Serial.write('*');

        if ((i+1) % 20 == 0)
            Serial.write('\n');

        ti = (ti + 1) % trace_size;
    }
    Serial.println("----------");
    Serial.flush();
}
#else
# define add_to_trace(x)
# define dump_trace()
#endif

/* Forward declarations */
/* kernel */
static void kernel_main_loop(void);
static void kernel_dispatch(void);
static void kernel_handle_request(void);
/* context switching */
static void exit_kernel(void) __attribute((noinline, naked));
static void enter_kernel(void) __attribute((noinline, naked));
extern "C" void TIMER1_COMPA_vect(void) __attribute__ ((signal, naked));

static int kernel_create_task();
static void kernel_terminate_task(void);
static void kernel_enqueue_task(task_descriptor_t*);

static void kernel_service_subscribe();
static void kernel_service_publish();
static void kernel_service_receive();

/* queues */

static void enqueue(queue_t* queue_ptr, task_descriptor_t* task_to_add);
static task_descriptor_t* dequeue(queue_t* queue_ptr);
static void queue_erase(queue_t*, task_descriptor_t*);

static void kernel_update_ticker(void);
static void idle (void);
static void _delay_25ms(void);
static void kernel_abort_with(uint8_t error);
static void kernel_assert(bool flag);

// Arduino

typedef enum pin_mode {
    PIN_INPUT = 0,
    PIN_OUTPUT = 1
} pin_mode_t;

typedef enum pin_value {
    PIN_LOW = 0,
    PIN_HIGH = 1
} pin_value_t;

void set_pin(uint8_t pin)
{
    switch(pin)
    {
    case 2:
        SET_PIN2; break;
    case 3:
        SET_PIN3; break;
    case 4:
        SET_PIN4; break;
    case 5:
        SET_PIN5; break;
    case 6:
        SET_PIN6; break;
    case 7:
        SET_PIN7; break;
    default:
        ;
    }
}

void clear_pin(uint8_t pin)
{
    switch(pin)
    {
    case 2:
        CLEAR_PIN2; break;
    case 3:
        CLEAR_PIN3; break;
    case 4:
        CLEAR_PIN4; break;
    case 5:
        CLEAR_PIN5; break;
    case 6:
        CLEAR_PIN6; break;
    case 7:
        CLEAR_PIN7; break;
    default:
        ;
    }
}

void trace_task_set(task_descriptor_t *task)
{
    uint8_t pin = ((uint8_t) task->arg) + 2;
    set_pin(pin);
}

void trace_task_clear(task_descriptor_t *task)
{
    uint8_t pin = ((uint8_t) task->arg) + 2;
    clear_pin(pin);
}

/*
 * FUNCTIONS
 */
/**
 *  @brief The idle task does nothing but busy loop.
 */
static void idle (void)
{
    for(;;) {};
}


/**
 * @fn kernel_main_loop
 *
 * @brief The heart of the RTOS, the main loop where the kernel is entered and exited.
 *
 * The complete function is:
 *
 *  Loop
 *<ol><li>Select and dispatch a process to run</li>
 *<li>Exit the kernel (The loop is left and re-entered here.)</li>
 *<li>Handle the request from the process that was running.</li>
 *<li>End loop, go to 1.</li>
 *</ol>
 */
static void kernel_main_loop(void)
{
    for(;;)
    {
        kernel_dispatch();

        exit_kernel();

        /* if this task makes a system call, or is interrupted,
         * the thread of control will return to here. */

        kernel_handle_request();
    }
}


/**
 * @fn kernel_dispatch
 *
 *@brief The second part of the scheduler.
 *
 * Chooses the next task to run.
 *
 */
static void kernel_dispatch(void)
{
    /* If the current state is RUNNING, then select it to run again.
     * kernel_handle_request() has already determined it should be selected.
     */

#ifdef TRACE_TASKS
    task_descriptor_t *last_task = cur_task;
#endif

    if(cur_task->state != RUNNING || cur_task == idle_task)
    {
		if(system_queue.head != NULL)
        {
            cur_task = dequeue(&system_queue);
        }
        else if(current_periodic_task)
        {
            /* Keep running the current PERIODIC task. */
            cur_task = current_periodic_task;
        }
        else if(rr_queue.head != NULL)
        {
            cur_task = dequeue(&rr_queue);
        }
        else
        {
            /* No task available, so idle. */
            cur_task = idle_task;
        }

        cur_task->state = RUNNING;
    }
#ifdef TRACE_TASKS
    if (last_task != cur_task)
    {
        trace_task_clear(last_task);
        trace_task_set(cur_task);
    }
#endif
}


/**
 * @fn kernel_handle_request
 *
 *@brief The first part of the scheduler.
 *
 * Perform some action based on the system call or timer tick.
 * Perhaps place the current process in a ready or waitng queue.
 */
static void kernel_handle_request(void)
{
    switch(kernel_request)
    {
    case NONE:
        add_to_trace(".N");
        /* Should not happen. */
        break;

    case TIMER_EXPIRED:
        add_to_trace("|");
        // Pre-empt round robin tasks at each tick
        if (cur_task->level == RR)
            kernel_enqueue_task(cur_task);

        kernel_update_ticker();

        break;

    case TASK_CREATE:
        add_to_trace(".TaC");
        kernel_request_retval = kernel_create_task();

        // Pre-empt current task if lower priority than new task
        if ( kernel_request_retval &&
             kernel_request_create_args.level > cur_task->level )
        {
            kernel_enqueue_task(cur_task);
        }

        break;

    case TASK_TERMINATE:
        add_to_trace(".TaT");
        if(cur_task != idle_task)
        {
            kernel_terminate_task();
        }
        break;

    case TASK_NEXT:
        add_to_trace(".TaN");
        kernel_enqueue_task(cur_task);

        if (cur_task->level == PERIODIC)
            current_periodic_task = NULL;

        break;

    case TASK_GET_ARG:
        add_to_trace(".TaA");
        /* Should not happen. Handled in task itself. */
        break;

    case TASK_PERIODIC_START:
        add_to_trace(".TaPrS");
        if (!periodic_tasks_running)
        {
            periodic_tasks_running = true;

            task_descriptor_t *task = periodic_task_list.head;
            while(task)
            {
                task->next_tick = ticks_since_system_start  + 1 + task->start;
                task = task->next;
            }

            ticks_at_next_periodic_schedule_check = ticks_since_system_start + 1;
        }
        else
        {
            kernel_abort_with(ERR_PERIODIC_SCHEDULE_SETUP);
        }
        break;

    case SERVICE_SUBSCRIBE:
    {
        add_to_trace(".SS");
        kernel_service_subscribe();
        break;
    }
    case SERVICE_PUBLISH:
    {
        add_to_trace(".SP");
        kernel_service_publish();
        break;
    }
    case SERVICE_RECEIVE:
    {
        add_to_trace(".SR");
        kernel_service_receive();
        break;
    }
    default:
        add_to_trace(".!");
        //char request_str[50];
        //sprintf(request_str, "%d", kernel_request);
        //add_to_trace(request_str);

        /* Should never happen */
        kernel_abort_with(ERR_RTOS_INTERNAL_ERROR);
        break;
    }

    kernel_request = NONE;
}


/*
 * Context switching
 */
/**
 * It is important to keep the order of context saving and restoring exactly
 * in reverse. Also, when a new task is created, it is important to
 * initialize its "initial" context in the same order as a saved context.
 *
 * Save r31 and SREG on stack, disable interrupts, then save
 * the rest of the registers on the stack. In the locations this macro
 * is used, the interrupts need to be disabled, or they already are disabled.

   Extended addressing:
   We also need to (re)store the EIND register (0x3C).
 */
#define    SAVE_CTX_TOP()       asm volatile (\
    "push   r31             \n\t"\
    "in     r31,__SREG__    \n\t"\
    "cli                    \n\t"::); /* Disable interrupt */

#define STACK_SREG_SET_I_BIT()    asm volatile (\
    "ori    r31, 0x80        \n\t"::);

#define    SAVE_CTX_BOTTOM()       asm volatile (\
    "push   r31             \n\t"\
    "in     r31,0x3C        \n\t"\
    "push   r31             \n\t"\
    "push   r30             \n\t"\
    "push   r29             \n\t"\
    "push   r28             \n\t"\
    "push   r27             \n\t"\
    "push   r26             \n\t"\
    "push   r25             \n\t"\
    "push   r24             \n\t"\
    "push   r23             \n\t"\
    "push   r22             \n\t"\
    "push   r21             \n\t"\
    "push   r20             \n\t"\
    "push   r19             \n\t"\
    "push   r18             \n\t"\
    "push   r17             \n\t"\
    "push   r16             \n\t"\
    "push   r15             \n\t"\
    "push   r14             \n\t"\
    "push   r13             \n\t"\
    "push   r12             \n\t"\
    "push   r11             \n\t"\
    "push   r10             \n\t"\
    "push   r9              \n\t"\
    "push   r8              \n\t"\
    "push   r7              \n\t"\
    "push   r6              \n\t"\
    "push   r5              \n\t"\
    "push   r4              \n\t"\
    "push   r3              \n\t"\
    "push   r2              \n\t"\
    "push   r1              \n\t"\
    "push   r0              \n\t"::);

/**
 * @brief Push all the registers and SREG onto the stack.
 */
#define    SAVE_CTX()    SAVE_CTX_TOP();SAVE_CTX_BOTTOM();

/**
 * @brief Pop all registers from the stack, including SREG and EIND.
 */
#define    RESTORE_CTX()    asm volatile (\
    "pop    r0                \n\t"\
    "pop    r1                \n\t"\
    "pop    r2                \n\t"\
    "pop    r3                \n\t"\
    "pop    r4                \n\t"\
    "pop    r5                \n\t"\
    "pop    r6                \n\t"\
    "pop    r7                \n\t"\
    "pop    r8                \n\t"\
    "pop    r9                \n\t"\
    "pop    r10             \n\t"\
    "pop    r11             \n\t"\
    "pop    r12             \n\t"\
    "pop    r13             \n\t"\
    "pop    r14             \n\t"\
    "pop    r15             \n\t"\
    "pop    r16             \n\t"\
    "pop    r17             \n\t"\
    "pop    r18             \n\t"\
    "pop    r19             \n\t"\
    "pop    r20             \n\t"\
    "pop    r21             \n\t"\
    "pop    r22             \n\t"\
    "pop    r23             \n\t"\
    "pop    r24             \n\t"\
    "pop    r25             \n\t"\
    "pop    r26             \n\t"\
    "pop    r27             \n\t"\
    "pop    r28             \n\t"\
    "pop    r29             \n\t"\
    "pop    r30             \n\t"\
    "pop    r31             \n\t"\
    "out    0x3C, r31       \n\t"\
    "pop    r31             \n\t"\
    "out    __SREG__, r31   \n\t"\
    "pop    r31             \n\t"::);


/**
 * @fn exit_kernel
 *
 * @brief The actual context switching code begins here.
 *
 * This function is called by the kernel. Upon entry, we are using
 * the kernel stack, on top of which is the address of the instruction
 * after the call to exit_kernel().
 *
 * Assumption: Our kernel is executed with interrupts already disabled.
 *
 * The "naked" attribute prevents the compiler from adding instructions
 * to save and restore register values. It also prevents an
 * automatic return instruction.
 */
static void exit_kernel(void)
{
    /*
     * The PC was pushed on the stack with the call to this function.
     * Now push on the I/O registers and the SREG as well.
     */
     SAVE_CTX();

    /*
     * The last piece of the context is the SP. Save it to a variable.
     */
    kernel_sp = SP;

#ifdef TRACE_KERNEL_MODE
    trace_task_set(cur_task);
#endif

    /*
     * Now restore the task's context, SP first.
     */
    SP = (uint16_t)(cur_task->sp);

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * return explicitly required as we are "naked".
     * Interrupts are enabled or disabled according to SREG
     * recovered from stack, so we don't want to explicitly
     * enable them here.
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/**
 * @fn enter_kernel
 *
 * @brief All system calls eventually enter here.
 *
 * Assumption: We are still executing on cur_task's stack.
 * The return address of the caller of enter_kernel() is on the
 * top of the stack.
 */
static void enter_kernel(void)
{
    /*
     * The PC was pushed on the stack with the call to this function.
     * Now push on the I/O registers and the SREG as well.
     */
    SAVE_CTX();

    /*
     * The last piece of the context is the SP. Save it to a variable.
     */
    cur_task->sp = (uint8_t*)SP;

#ifdef TRACE_KERNEL_MODE
    trace_task_clear(cur_task);
#endif

    /*
     * Now restore the kernel's context, SP first.
     */
    SP = kernel_sp;

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * return explicitly required as we are "naked".
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}


/**
 * @fn TIMER1_COMPA_vect
 *
 * @brief The interrupt handler for output compare interrupts on Timer 1
 *
 * Used to enter the kernel when a tick expires.
 *
 * Assumption: We are still executing on the cur_task stack.
 * The return address inside the current task code is on the top of the stack.
 *
 * The "naked" attribute prevents the compiler from adding instructions
 * to save and restore register values. It also prevents an
 * automatic return instruction.
 */
void TIMER1_COMPA_vect(void)
{
	//PORTB ^= _BV(PB7);		// Arduino LED
    /*
     * Save the interrupted task's context on its stack,
     * and save the stack pointer.
     *
     * On the cur_task's stack, the registers and SREG are
     * saved in the right order, but we have to modify the stored value
     * of SREG. We know it should have interrupts enabled because this
     * ISR was able to execute, but it has interrupts disabled because
     * it was stored while this ISR was executing. So we set the bit (I = bit 7)
     * in the stored value.
     */
    SAVE_CTX_TOP();

    STACK_SREG_SET_I_BIT();

    SAVE_CTX_BOTTOM();

    cur_task->sp = (uint8_t*)SP;

    /*
     * Now that we already saved a copy of the stack pointer
     * for every context including the kernel, we can move to
     * the kernel stack and use it. We will restore it again later.
     */
    SP = kernel_sp;

#if defined(TRACE_KERNEL_MODE) && defined(TRACE_KERNEL_MODE_ON_TICK)
    trace_task_clear(cur_task);
#endif

    /*
     * Inform the kernel that this task was interrupted.
     */
    kernel_request = TIMER_EXPIRED;

    /*
     * Prepare for next tick interrupt.
     */
    OCR1A += TICK_CYCLES;

    /*
     * Restore the kernel context. (The stack pointer is restored again.)
     */
    SP = kernel_sp;

    /*
     * Now restore I/O and SREG registers.
     */
    RESTORE_CTX();

    /*
     * We use "ret" here, not "reti", because we do not want to
     * enable interrupts inside the kernel.
     * Explilictly required as we are "naked".
     *
     * The last piece of the context, the PC, is popped off the stack
     * with the ret instruction.
     */
    asm volatile ("ret\n"::);
}

/*
 * Tasks Functions
 */
/**
 *  @brief Kernel function to create a new task.
 *
 * When creating a new task, it is important to initialize its stack just like
 * it has called "enter_kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * @sa enter_kernel
 */
static int kernel_create_task()
{
    /* The new task. */
    task_descriptor_t *p;
    uint8_t* stack_bottom;


    if (dead_pool_queue.head == NULL)
    {
        /* Too many tasks! */
        return 0;
    }

    if(kernel_request_create_args.level == PERIODIC)
    {
        // Abort if periodic tasks running:
        if (periodic_tasks_running)
        {
            kernel_abort_with(ERR_PERIODIC_SCHEDULE_SETUP);
        }
    }

	/* idling "task" goes in last descriptor. */
	if(kernel_request_create_args.level == IDLE)
	{
		p = &task_desc[MAXPROCESS];
	}
	/* Find an unused descriptor. */
	else
	{
	    p = dequeue(&dead_pool_queue);
	}

    stack_bottom = &(p->stack[WORKSPACE-1]);


    /*
      The stack must look like this (from top to bottom):
      - (1 byte each = 31 bytes) registers 30 to 0.
      - (1 byte) EIND
      - (1 byte) SREG
      - (1 byte) register 31,
      - (3 bytes) the address of Task_Terminate() to destroy the task if it ever returns,
      - (3 bytes) the address of the start of the task to "return" to the first time it runs,
      This is 40 bytes in total.
      The stack grows down in memory, so the top will be 40 bytes below the bottom:
    */
    uint8_t* stack_top = stack_bottom - 40;

    // 0 = above top of stack
    // 1 = r0
    // 2 = r1
    stack_top[2] = (uint8_t) 0; // r1 is the "zero" register
    // ...
    // 31 = r30
    // 32 = EIND
    // 33 = SREG
    stack_top[33] = (uint8_t) _BV(SREG_I); /* set SREG_I bit in stored SREG. */
    // 34 = r31

    /*
    PC on ATMEGA2560 is 3 bytes (for extended addressing).
    However, in C, function pointers are still 2 bytes.
    The compiler solves this using "trampolines".
    The first byte of the PC is always 0.

    We push the pointers onto the stack in reverse byte order.
    This is because the "return" assembly instructions
    (ret and reti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
    second), even though the machine is LITTLE ENDIAN.
    */
    stack_top[35] = (uint8_t)(0x0);
    stack_top[36] = (uint8_t)((uint16_t)(kernel_request_create_args.f) >> 8);
    stack_top[37] = (uint8_t)(uint16_t)(kernel_request_create_args.f);
    stack_top[38] = (uint8_t)(0x0);
    stack_top[39] = (uint8_t)((uint16_t)Task_Terminate >> 8);
    stack_top[40] = (uint8_t)(uint16_t)Task_Terminate;

    /*
     * Store top of stack into the task descriptor
     */
    p->sp = stack_top;

    p->state = READY;
    p->arg = kernel_request_create_args.arg;
    p->level = kernel_request_create_args.level;
    p->period = kernel_request_create_args.period;
    p->wcet = kernel_request_create_args.wcet;
    p->start = kernel_request_create_args.start;
    // Next periodic task time is equal to its start time:
    p->next_tick = kernel_request_create_args.start;

	switch(kernel_request_create_args.level)
	{
	case PERIODIC:
        enqueue(&periodic_task_list, p);
		break;

    case SYSTEM:
    	/* Put SYSTEM and Round Robin tasks on a queue. */
        enqueue(&system_queue, p);
		break;

    case RR:
		/* Put SYSTEM and Round Robin tasks on a queue. */
        enqueue(&rr_queue, p);
		break;

	default:
		/* idle task does not go in a queue */
		break;
	}


    return 1;
}


/**
 * @brief Kernel function to destroy the current task.
 */
static void kernel_terminate_task(void)
{
    /* deallocate all resources used by this task */
    cur_task->state = DEAD;

    // FIXME:
    // For consistency, it might be better if
    // each periodic task gets temporarily removed
    // from the periodic task list while running.
    if (cur_task->level == PERIODIC)
    {
        queue_erase(&periodic_task_list, cur_task);
        current_periodic_task = NULL;
    }

    enqueue(&dead_pool_queue, cur_task);
}

static void kernel_enqueue_task(task_descriptor_t* task)
{
    task->state = READY;

    switch(task->level)
    {
    case SYSTEM:
        enqueue(&system_queue, task);
        break;
    case RR:
        enqueue(&rr_queue, task);
        break;
    default:
        ;
    }
}

static void kernel_service_subscribe()
{
    if (cur_task->level == PERIODIC)
    {
        kernel_abort_with(ERR_SERVICE_SUBSCRIBE);
    }

    Service *srv = requested_service;
    ServiceSubscription *sub = requested_service_subscription;

    sub->service = srv;
    sub->next = srv->subscriptions;
    srv->subscriptions = sub;

    sub->subscriber = cur_task;
    sub->waiting = false;
    sub->unread = false;
}

static void kernel_service_publish()
{
    ServiceSubscription *sub = requested_service->subscriptions;

    bool current_is_subscriber = false;

    while(sub)
    {
        if (sub->subscriber == cur_task)
            current_is_subscriber = true;

        sub->unread = true;

        if (sub->waiting)
        {
            kernel_enqueue_task(sub->subscriber);
            sub->waiting = false;
        }

        sub = sub->next;
    }

    // Pre-empt publisher.
    // We might be in an interrupt, so that the current task is
    // also a subscriber and was already enqueued above.
    // Only enqueue if that's not the case.
    if (!current_is_subscriber)
        kernel_enqueue_task(cur_task);
}

static void kernel_service_receive()
{
    ServiceSubscription *sub = requested_service_subscription;
    kernel_assert(sub->subscriber == cur_task);

    sub->waiting = true;

    cur_task->state = WAITING;
}

/*
 * Queue manipulation.
 */

/**
 * @brief Add a task the head of the queue
 *
 * @param queue_ptr the queue to insert in
 * @param task_to_add the task descriptor to add
 */
static void enqueue(queue_t* queue_ptr, task_descriptor_t* task_to_add)
{
    task_to_add->next = NULL;

    if(queue_ptr->head == NULL)
    {
        /* empty queue */
        queue_ptr->head = task_to_add;
        queue_ptr->tail = task_to_add;
    }
    else
    {
        /* put task at the back of the queue */
        queue_ptr->tail->next = task_to_add;
        queue_ptr->tail = task_to_add;
    }
}


/**
 * @brief Pops head of queue and returns it.
 *
 * @param queue_ptr the queue to pop
 * @return the popped task descriptor
 */
static task_descriptor_t* dequeue(queue_t* queue_ptr)
{
    task_descriptor_t* task_ptr = queue_ptr->head;

    if(queue_ptr->head != NULL)
    {
        queue_ptr->head = queue_ptr->head->next;
        task_ptr->next = NULL;
    }

    return task_ptr;
}

static void queue_erase(queue_t * q, task_descriptor_t * t)
{
    if (!q->head)
        return;

    if (t == q->head)
    {
        q->head = q->head->next;
        t->next = NULL;
    }
    else
    {
        task_descriptor_t * prev = q->head;
        while(prev)
        {
            if (prev->next == t)
            {
                prev->next = t->next;
                t->next = NULL;
                return;
            }
            prev = prev->next;
        }
    }
}

// Update ticks_at_next_periodic_schedule_check.
// Maybe set current_periodic_task.

static void kernel_select_periodic_task()
{
    if (current_periodic_task)
    {
        kernel_abort_with(ERR_PERIODIC_SCHEDULE_RUN);
    }

    uint16_t next_task_distance = UINT16_MAX;

    task_descriptor_t *selected_task = NULL;
    task_descriptor_t *t = periodic_task_list.head;
    while(t)
    {
        if (!selected_task && t->next_tick == ticks_since_system_start)
        {
            selected_task = t;
            t->next_tick += t->period;
            ticks_since_current_periodic_task = 0;
        }

        uint16_t distance = t->next_tick - ticks_since_system_start;
        if (distance < next_task_distance)
            next_task_distance = distance;

        t = t->next;
    }

    if (selected_task && selected_task->wcet > next_task_distance)
    {
        // Task overlap.
        kernel_abort_with(ERR_PERIODIC_SCHEDULE_RUN);
    }

    current_periodic_task = selected_task;
    ticks_at_next_periodic_schedule_check =
            ticks_since_system_start + next_task_distance;
}


/**
 * @brief Update the current time.
 *
 * Perhaps select the next periodic task.
 */
static void kernel_update_ticker(void)
{
    ++ticks_since_system_start;

    // Note: We only count ticks_since_current_periodic_task
    // when the task is running. Effectly, we extend its allowed
    // execution time by the interleaved running time of
    // higher priority tasks.

    // However, this might violate the assertion that there is
    // no running task when the next task should run
    // (see kernel_select_periodic_task function).

    if (current_periodic_task && current_periodic_task == cur_task)
    {
        ++ticks_since_current_periodic_task;
        if (ticks_since_current_periodic_task >=
                current_periodic_task->wcet)
        {
            kernel_abort_with(ERR_PERIODIC_SCHEDULE_RUN);
        }
    }

    if ( periodic_tasks_running &&
         ticks_since_system_start == ticks_at_next_periodic_schedule_check )
    {
        kernel_select_periodic_task();
    }

    return;
}

/**
 * @brief Setup the RTOS and create main() as the first SYSTEM level task.
 *
 * Point of entry from the C runtime crt0.S.
 */
void OS_Init()
{
#ifdef TRACE_REQUESTS
    for(int i = 0; i < trace_size; ++i)
        trace[i] = '_';
#endif

    int i;

    /*
     * Initialize dead pool to contain all but last task descriptor.
     *
     * DEAD == 0, already set in .init4
     */
    for (i = 0; i < MAXPROCESS - 1; i++)
    {
        task_desc[i].state = DEAD;
        task_desc[i].next = &task_desc[i + 1];
    }
    task_desc[MAXPROCESS - 1].next = NULL;
    dead_pool_queue.head = &task_desc[0];
    dead_pool_queue.tail = &task_desc[MAXPROCESS - 1];

    // Initialize services:

    for (i = 0; i < MAX_SERVICE_COUNT; ++i)
    {
        services[i].subscriptions = NULL;
    }

	/* Create idle "task" */
    kernel_request_create_args.f = (voidfuncvoid_ptr)idle;
    kernel_request_create_args.level = IDLE;
    kernel_request_create_args.arg = 0;
    kernel_create_task();

    /* Create "main" task as SYSTEM level. */
    kernel_request_create_args.f = (voidfuncvoid_ptr)r_main;
    kernel_request_create_args.level = SYSTEM;
    // Arg 0 specifies usage of first debug pin:
    kernel_request_create_args.arg = 1;
    kernel_create_task();

    /* First time through. Select "main" task to run first. */
    cur_task = dequeue(&system_queue);
    cur_task->state = RUNNING;

    // Set up status LED:
    DDRB |= (1 << DDB7);

    // Set up debug pins
#if defined(TRACE_KERNEL_MODE) || defined(TRACE_TASKS)
    SET_PIN2_OUT;
    SET_PIN3_OUT;
    SET_PIN4_OUT;
    SET_PIN5_OUT;
    SET_PIN6_OUT;
    SET_PIN7_OUT;

    CLEAR_PIN2;
    CLEAR_PIN3;
    CLEAR_PIN4;
    CLEAR_PIN5;
    CLEAR_PIN6;
    CLEAR_PIN7;
#endif

    // Set up Timer 1 to count ticks...

    // First, clear everything..
    TIMSK1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;

    // Use 1/8 prescaler
    TCCR1B |= (_BV(CS11));
    // Time out after 1 tick
    OCR1A = TCNT1 + TICK_CYCLES;
    // Clear interrupt flag
    TIFR1 = _BV(OCF1A);
    // Enable interrupt
    TIMSK1 |= _BV(OCIE1A);

    /*
     * The main loop of the RTOS kernel.
     */
    kernel_main_loop();
}




/**
 *  @brief Delay function adapted from <util/delay.h>
 */
static void _delay_25ms(void)
{
    //uint16_t i;

    /* 4 * 50000 CPU cycles = 25 ms */
    //asm volatile ("1: sbiw %0,1" "\n\tbrne 1b" : "=w" (i) : "0" (50000));
    _delay_ms(25);
}

static void kernel_abort_with(uint8_t error)
{
    error_msg = error;
    OS_Abort();
}

static void kernel_assert(bool flag)
{
    if (!flag)
    {
        kernel_abort_with(ERR_RTOS_INTERNAL_ERROR);
    }
}

/** @brief Abort the execution of this RTOS due to an unrecoverable erorr.
 */
void OS_Abort(void)
{
#ifdef TRACE_REQUESTS
    __asm volatile( "cli" ::: "memory" );
    do_trace = false;
    __asm volatile( "sei" ::: "memory" );

    dump_trace();
#endif

    uint8_t i, j;
    uint8_t flashes;

    Disable_Interrupt();


#ifdef TRACE_TASKS
    SET_PIN2;
    SET_PIN3;
    SET_PIN4;
    SET_PIN5;
    SET_PIN6;
    SET_PIN7;
#endif

    // Use built-in LED on digital pin 13 (PB7)

    SET_PIN13_OUT;

    flashes = error_msg + 1;

    for(;;)
    {
        SET_PIN13;

        for(i = 0; i < 100; ++i)
        {
               _delay_25ms();
        }

        CLEAR_PIN13;

        for(i = 0; i < 40; ++i)
        {
               _delay_25ms();
        }


        for(j = 0; j < flashes; ++j)
        {
            SET_PIN13;

            for(i = 0; i < 10; ++i)
            {
                _delay_25ms();
            }

            CLEAR_PIN13;

            for(i = 0; i < 10; ++i)
            {
                _delay_25ms();
            }
        }

        for(i = 0; i < 20; ++i)
        {
            _delay_25ms();
        }
    }
}

int8_t Task_Create(uint8_t level, void (*f)(void), int16_t arg,
                   uint16_t period, uint16_t wcet, uint16_t start)
{
    int retval;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request_create_args.f = (voidfuncvoid_ptr)f;
    kernel_request_create_args.arg = arg;
    kernel_request_create_args.level = level;
    kernel_request_create_args.period = period;
    kernel_request_create_args.wcet = wcet;
    kernel_request_create_args.start = start;

    kernel_request = TASK_CREATE;
    enter_kernel();

    retval = kernel_request_retval;
    SREG = sreg;

    return retval;
}

int8_t   Task_Create_System(void (*f)(void), int16_t arg)
{
    return Task_Create(SYSTEM, f, arg, 0, 0, 0);
}

int8_t   Task_Create_RR(    void (*f)(void), int16_t arg)
{
    return Task_Create(RR, f, arg, 0, 0, 0);
}

int8_t   Task_Create_Periodic(void(*f)(void), int16_t arg,
                              uint16_t period, uint16_t wcet, uint16_t start)
{
    return Task_Create(PERIODIC, f, arg, period, wcet, start);
}

void   Task_Periodic_Start()
{
    uint8_t volatile sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_PERIODIC_START;
    enter_kernel();

    SREG = sreg;
}

/**
  * @brief The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
    uint8_t volatile sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_NEXT;
    enter_kernel();

    SREG = sreg;
}


/**
  * @brief The calling task terminates itself.
  */
void Task_Terminate()
{
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    kernel_request = TASK_TERMINATE;
    enter_kernel();

    SREG = sreg;
}


/** @brief Retrieve the assigned parameter.
 */
int Task_GetArg(void)
{
    int arg;
    uint8_t sreg;

    sreg = SREG;
    Disable_Interrupt();

    arg = cur_task->arg;

    SREG = sreg;

    return arg;
}

Service *Service_Init()
{
    Service * service = NULL;

    uint8_t sreg = SREG;
    Disable_Interrupt();

    if (service_count < MAX_SERVICE_COUNT)
    {
        service = &services[service_count];
        ++service_count;
    }

    SREG = sreg;

    return service;
}

ServiceSubscription * Service_Subscribe( Service *s )
{
    ServiceSubscription *sub = 0;

    uint8_t sreg = SREG;
    Disable_Interrupt();

    if (service_subscription_count < MAX_SERVICE_SUBSCRIPTION_COUNT)
    {
        sub = &service_subscriptions[service_subscription_count];
        ++service_subscription_count;

        requested_service = s;
        requested_service_subscription = sub;
        kernel_request = SERVICE_SUBSCRIBE;
        enter_kernel();
    }

    SREG = sreg;

    return sub;
}

void Service_Publish( Service *s, int16_t v )
{
    uint8_t sreg = SREG;
    Disable_Interrupt();

    s->value = v;

    kernel_request = SERVICE_PUBLISH;
    requested_service = s;
    enter_kernel();

    SREG = sreg;
}

int16_t Service_Receive(ServiceSubscription *sub)
{
    int16_t value;

    uint8_t sreg = SREG;
    Disable_Interrupt();

    if (!sub->unread)
    {
        kernel_request = SERVICE_RECEIVE;
        requested_service_subscription = sub;
        enter_kernel();
    }

    value = sub->service->value;
    sub->unread = false;

    SREG = sreg;

    return value;
}

uint16_t Now()
{
    uint16_t ms_now;

    uint8_t sreg = SREG;
    Disable_Interrupt();

    uint16_t last_tick = ticks_since_system_start;
    uint16_t cycles_at_last_tick = OCR1A - TICK_CYCLES;
    uint16_t cycles_extra = TCNT1 - cycles_at_last_tick;
    uint16_t ms_extra = cycles_extra / CYCLES_PER_MS;
    ms_now = last_tick * TICK + ms_extra;

    SREG = sreg;

    return ms_now;
}

/**
 * Runtime entry point into the program; just start the RTOS.  The application layer must define r_main() for its entry point.
 */

int main()
{
#ifdef INIT_ARDUINO_LIB
    init();
#endif

    OS_Init();
    return 0;
}
