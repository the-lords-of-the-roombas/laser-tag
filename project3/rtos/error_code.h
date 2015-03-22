/**
 * @file   error_code.h
 *
 * @brief Error messages returned in OS_Abort().
 *        Green errors are initialization errors
 *        Red errors are runt time errors
 *
 * CSC 460/560 Real Time Operating Systems - Mantis Cheng
 *
 * @author Scott Craig
 * @author Justin Tanner
 */
#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

enum {

    // 1: User called OS_Abort().
    ERR_USER_CALLED_OS_ABORT = 0,

    // 2: Periodic task created after periodic schedule start
    ERR_PERIODIC_SCHEDULE_SETUP,

    // 3: Periodic schedule invalid (tasks overlap)
    ERR_PERIODIC_SCHEDULE_RUN,

    // 4: Periodic task not allowed to subscribe to service
    ERR_SERVICE_SUBSCRIBE,

    // 5: RTOS Internal error.
    ERR_RTOS_INTERNAL_ERROR,
};


#endif
