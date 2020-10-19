#ifndef __PTPD_SYS_TIME_H__
#define __PTPD_SYS_TIME_H__

/**
 * @file
 * @brief ptpd-lwip 5.3 interface to system timer
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "def/datatypes_private.h"

/* Assign function pointers to sys_time functions */
void initTimeFunctions(ptpFunctions_t *functions);

/* display local clock stats */
void displayStats(const ptpClock_t *ptpClock);

/* get current PTP system time */
void getTime(timeInternal_t *time);

/* set current PTP system time to value in timestamp */
void setTime(const timeInternal_t *time);

/* modify the PTP system time by adding the value in timestamp */
void updateTime(const timeInternal_t *time);

/* modify the PTP system time by fine adjustment (using an accumulator) */
bool adjFreq(s32_t adj);

/* Generate random integer up to specified maximum */
u32_t getRand(u32_t randMax);

#endif /* __PTPD_SYS_TIME_H__ */
