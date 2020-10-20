/* sys.c */

#include "sys_time.h"

#include <stdlib.h>

/* Stores function pointers to timer functions */
static ptpFunctions_t ptpFunctions;

/* Assign function pointers to sys_time functions */
void initTimeFunctions(ptpFunctions_t *functions)
{
    ptpFunctions.ptpGetTime = functions->ptpGetTime;
    ptpFunctions.ptpSetTime = functions->ptpSetTime;
    ptpFunctions.ptpUpdateCoarse = functions->ptpUpdateCoarse;
    ptpFunctions.ptpUpdateFine = functions->ptpUpdateFine;
}

/* display local clock stats */
void displayStats(const ptpClock_t *ptpClock)
{
	// 	const char *s;
	// 	unsigned char *uuid;
	// 	char sign;

	// 	uuid = (unsigned char*) ptpClock->parentDS.parentPortIdentity.clockIdentity;

	// 	/* Master clock UUID */
	// 	printf("%02X%02X:%02X%02X:%02X%02X:%02X%02X\n",
	// 					uuid[0], uuid[1],
	// 					uuid[2], uuid[3],
	// 					uuid[4], uuid[5],
	// 					uuid[6], uuid[7]);

	// 	switch (ptpClock->portDS.portState)
	// 	{
	// 		case PTP_INITIALIZING:  s = "init";  break;
	// 		case PTP_FAULTY:        s = "faulty";   break;
	// 		case PTP_LISTENING:     s = "listening";  break;
	// 		case PTP_PASSIVE:       s = "passive";  break;
	// 		case PTP_UNCALIBRATED:  s = "uncalibrated";  break;
	// 		case PTP_SLAVE:         s = "slave";   break;
	// 		case PTP_PRE_MASTER:    s = "pre master";  break;
	// 		case PTP_MASTER:        s = "master";   break;
	// 		case PTP_DISABLED:      s = "disabled";  break;
	// 		default:                s = "?";     break;
	// 	}

	// 	/* State of the PTP */
	// 	printf("state: %s\n", s);

	// 	/* One way delay */
	// 	switch (ptpClock->portDS.delayMechanism)
	// 	{
	// 		case E2E:
	// 			printf("path delay: %ld nsec\n", ptpClock->currentDS.meanPathDelay.nanoseconds);
	// 			break;
	// 		case P2P:
	// 			printf("path delay: %ld nsec\n", ptpClock->portDS.peerMeanPathDelay.nanoseconds);
	// 			break;
	// 		default:
	// 			printf("path delay: unknown\n");
	// 			/* none */
	// 			break;
	// 	}

	// 	/* Offset from master */
	// 	if (ptpClock->currentDS.offsetFromMaster.seconds)
	// 	{
	// 		printf("offset: %ld sec\n", ptpClock->currentDS.offsetFromMaster.seconds);
	// 	}
	// 	else
	// 	{
	// 		printf("offset: %ld nsec\n", ptpClock->currentDS.offsetFromMaster.nanoseconds);
	// 	}

	// 	/* Observed drift from master */
	// 	sign = ' ';
	// 	if (ptpClock->observedDrift > 0) sign = '+';
	// 	if (ptpClock->observedDrift < 0) sign = '-';

	// 	printf("drift: %c%d.%03d ppm\n", sign, abs(ptpClock->observedDrift / 1000), abs(ptpClock->observedDrift % 1000));
	}

/* get current PTP system time */
void getTime(timeInternal_t *time)
{
    timestamp_t timestamp;
    ptpFunctions.ptpGetTime(&timestamp);
    time->seconds = timestamp.secondsField.lsb;
    time->nanoseconds = timestamp.nanosecondsField;
}

/* set current PTP system time to value in timestamp */
void setTime(const timeInternal_t *time)
{
    timestamp_t timestamp;
    timestamp.secondsField.lsb = time->seconds;
    timestamp.nanosecondsField = time->nanoseconds;
    ptpFunctions.ptpSetTime(&timestamp);
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
    ptpFunctions.ptpUpdateCoarse(&offset, sign);
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
    ptpFunctions.ptpUpdateFine(adj);

    return true;
}

/* Generate random integer up to specified maximum */
u32_t getRand(u32_t randMax)
{
    return rand() % randMax;
}
