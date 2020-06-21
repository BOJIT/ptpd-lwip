/* timer.c */

/// @todo test practicality of lwIP's built-in timeout engine

#include "../ptpd.h"

/* An array to hold the various system timer handles. */
//static struct lwip_cyclic_timer ptpdTimers[TIMER_ARRAY_SIZE];
static int ptpdTimersArgs[TIMER_ARRAY_SIZE];
static bool ptpdTimersExpired[TIMER_ARRAY_SIZE];
 
static void timer_callback(void *arg)
{
	int index = (int) arg;

	// Sanity check the index.
	if (index < TIMER_ARRAY_SIZE)
	{
		/* Mark the indicated timer as expired. */
		ptpdTimersExpired[index] = TRUE;

		/* Notify the PTP thread of a pending operation. */
		ptpd_alert();
	}
}

void initTimer(void)
{
	int i;

	DBG("initTimer\n");

	/* Create the various timers used in the system. */
	for (i = 0; i < TIMER_ARRAY_SIZE; i++)
	{
		// Mark the timer as not expired.	/// Mostly unused
		// Initialize the timer.
		ptpdTimersArgs[i] = i;	// Bodge - sort out later
		//sys_timer_new(&ptpdTimers[i], timerCallback, osTimerOnce, (void *) i);
		ptpdTimersExpired[i] = FALSE;
	}
}

void timerStop(int32_t index)
{
	/* Sanity check the index. */
	if (index >= TIMER_ARRAY_SIZE) return;

	// Cancel the timer and reset the expired flag.
	DBGV("timerStop: stop timer %d\n", index);
	sys_untimeout(timer_callback, LWIP_CONST_CAST(void *, &ptpdTimersArgs[index]));
	ptpdTimersExpired[index] = FALSE;
}

void timerStart(int32_t index, uint32_t interval_ms)
{
	/* Sanity check the index. */
	if (index >= TIMER_ARRAY_SIZE) return;

	// Set the timer duration and start the timer.
	DBGV("timerStart: set timer %d to %d\n", index, interval_ms);
	ptpdTimersExpired[index] = FALSE;
	sys_timeout(interval_ms, timer_callback, LWIP_CONST_CAST(void *, &ptpdTimersArgs[index]));
}

bool timerExpired(int32_t index)
{
	/* Sanity check the index. */
	if (index >= TIMER_ARRAY_SIZE) return FALSE;

	/* Determine if the timer expired. */
	if (!ptpdTimersExpired[index]) return FALSE;
	DBGV("timerExpired: timer %d expired\n", index);
	ptpdTimersExpired[index] = FALSE;

	return TRUE;
}
