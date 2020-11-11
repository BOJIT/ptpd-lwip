/**
 * @file
 * @brief lwip_ptp public interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#ifndef __LWIP_PTP_H__
#define __LWIP_PTP_H__

#include "def/datatypes_public.h"

/*----------------------------- PUBLIC FUNCTIONS -----------------------------*/

/**
 * @brief Get current system time.
 * @param timestamp timestamp to fill with current system time.
 */
void LWIP_PTP_GET_TIME(timestamp_t*);

/**
 * @brief Set current system time.
 * @param timestamp timestamp containing new system time.
 */
void LWIP_PTP_SET_TIME(const timestamp_t*);

/**
 * @brief Coarse update the system time (single step).
 * @param timestamp timestamp containing offset (absolute value)
 * @param sign determines whether to add/subtract the timestamp to system time.
 */
void LWIP_PTP_UPDATE_COARSE(const timestamp_t*, s8_t);

/**
 * @brief Fine update the system time (accumulator).
 * @param adj adjustment addend (for fine adjustment of system clock frequency).
 */
void LWIP_PTP_UPDATE_FINE(s32_t);

/**
 * @brief Initialise PTP software stack.
 */
void ptpdInit(u8_t priority);

#endif /* __LWIP_PTP_H__ */
