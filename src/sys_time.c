/* sys.c */

#include "sys_time.h"

#if LWIP_PTP || defined __DOXYGEN__

#include <stdlib.h>

#include "lwip-ptp.h"
#include "net.h"

/* get current PTP system time */
void getTime(timeInternal_t *time)
{
    timestamp_t timestamp;
    LWIP_PTP_GET_TIME(&timestamp);
    time->seconds = timestamp.secondsField.lsb;
    time->nanoseconds = timestamp.nanosecondsField;
}

/* set current PTP system time to value in timestamp */
void setTime(const timeInternal_t *time)
{
    timestamp_t timestamp;
    timestamp.secondsField.lsb = time->seconds;
    timestamp.nanosecondsField = time->nanoseconds;
    LWIP_PTP_SET_TIME(&timestamp);
    DBG("resetting system clock to %d sec %d nsec\n", time->seconds, time->nanoseconds);
}

/* modify the PTP system time by adding the value in timestamp */
void updateTime(const timeInternal_t *time)
{
    timestamp_t offset;
    s8_t sign = 0;  // non-zero value indicates a negative offset

    DBGV("updateTime: %d sec %d nsec\n", time->seconds, time->nanoseconds);

    /* assuming 2 negative signs don't correspond to a positive offset! */
    if((time->seconds < 0) || (time->nanoseconds < 0)) {
        sign = 1;
    }

    offset.secondsField.lsb = abs(time->seconds);
    offset.nanosecondsField = abs(time->nanoseconds);

    /* Coarse update method */
    LWIP_PTP_UPDATE_COARSE(&offset, sign);
    DBGV("updateTime: updated\n");
}

/* modify the PTP system time by fine adjustment (using an accumulator) */
bool adjFreq(s32_t adj)
{
    DBGV("adjFreq %d\n", adj);

    if (adj > ADJ_FREQ_MAX)
        adj = ADJ_FREQ_MAX;
    else if (adj < -ADJ_FREQ_MAX)
        adj = -ADJ_FREQ_MAX;

    /* Fine update method */
    LWIP_PTP_UPDATE_FINE(adj);

    return true;
}

/* Generate random integer up to specified maximum */
u32_t getRand(u32_t randMax)
{
    return rand() % randMax;
}

#endif /* LWIP_PTP || defined __DOXYGEN__ */
