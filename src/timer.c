/* timer.c */

/// @todo test practicality of lwIP's built-in timeout engine
/// THIS WHOLE SECTION SHOULD BE REPLACED WITH SEMAPHORES or Ticks

#include "timer.h"

#if LWIP_PTP || defined __DOXYGEN__

#include <lwip/timeouts.h>

/* An array to hold the various system timer handles. */
//static struct lwip_cyclic_timer ptpdTimers[TIMER_ARRAY_SIZE];
static int ptpdTimersArgs[TIMER_ARRAY_SIZE];
static bool ptpdTimersExpired[TIMER_ARRAY_SIZE];

/* mark the relevant timer as expired, then unblock the main task */
static void timer_callback(void *arg)
{

    /// @todo I don't think this is thread-safe. Replace with other rtos IPC
    int index = (int) arg;

    // Sanity check the index.
    if (index < TIMER_ARRAY_SIZE) {
        /* Mark the indicated timer as expired. */
        ptpdTimersExpired[index] = true;

        /* Notify the PTP thread of a pending operation. */
        // ptpd_alert();
        /// @todo unblock main process using private function call
    }
}

/* Initialise array of lwIP timers */
void initTimer(void)
{
    /// @todo this function may be obsolete
    int i;

    DBG("initTimer\n");

    /* Create the various timers used in the system. */
    for (i = 0; i < TIMER_ARRAY_SIZE; i++) {
        // Mark the timer as not expired.	/// Mostly unused
        // Initialize the timer.
        ptpdTimersArgs[i] = i;	// Bodge - sort out later
        //sys_timer_new(&ptpdTimers[i], timerCallback, osTimerOnce, (void *) i);
        ptpdTimersExpired[i] = false;
    }
}


/* Stop a specific timer */
void timerStop(s32_t index)
{
    /* Sanity check the index. */
    if (index >= TIMER_ARRAY_SIZE) return;

    // Cancel the timer and reset the expired flag.
    DBGV("timerStop: stop timer %d\n", index);
    sys_untimeout(timer_callback, LWIP_CONST_CAST(void *, &ptpdTimersArgs[index]));
    ptpdTimersExpired[index] = false;
}

/* Create a specific timer */
void timerStart(s32_t index, u32_t interval_ms)
{
    /* Sanity check the index. */
    if (index >= TIMER_ARRAY_SIZE) return;

    // Set the timer duration and start the timer.
    DBGV("timerStart: set timer %d to %d\n", index, interval_ms);
    ptpdTimersExpired[index] = false;
    sys_timeout(interval_ms, timer_callback, LWIP_CONST_CAST(void *, &ptpdTimersArgs[index]));
}

/* Poll a timer to see if it has expired */
bool timerExpired(s32_t index)
{
    /* Sanity check the index. */
    if (index >= TIMER_ARRAY_SIZE) return false;

    /* Determine if the timer expired. */
    if (!ptpdTimersExpired[index]) return false;
    DBGV("timerExpired: timer %d expired\n", index);
    ptpdTimersExpired[index] = false;

    return true;
}

#endif /* LWIP_PTP || defined __DOXYGEN__ */
