#ifndef __LWIP_PTP_TIMER_H__
#define __LWIP_PTP_TIMER_H__

/**
 *\file
 * \brief 5.3 lwIP Timer Interface
 */

#include "def/datatypes_private.h"

#include <lwip/arch.h>

/* Initialise array of lwIP timers */
void initTimer(void);

/* Stop a specific timer */
void timerStop(s32_t index);

/* Create a specific timer */
void timerStart(s32_t index, u32_t interval_ms);

/* Poll a timer to see if it has expired */
bool timerExpired(s32_t index);

#endif /* __LWIP_PTP_TIMER_H__ */
