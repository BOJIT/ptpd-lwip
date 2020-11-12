#ifndef __LWIP_PTP_H__
#define __LWIP_PTP_H__

/**
 * @file
 * @brief lwip_ptp public interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include <lwip/arch.h>
#include <lwip/sys.h>

#include <stdbool.h>

#include "def/datatypes_public.h"

/*------------------------------ PUBLIC MACROS -------------------------------*/

/**
 * @brief number of timers that LWIP_PTP uses. Can be used by the driver.
 */
#define LWIP_PTP_NUM_TIMERS         6

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
 * @brief Initialise system timers. If the system timers already exist, they
 * must be deallocated before the new timers are created.
 * @retval ERR_OK if all went correctly, or lwIP-style error code otherwise.
 */
err_t LWIP_PTP_INIT_TIMERS(void);

/**
 * @brief Start a system timer.
 * @param idx index of the timer to start. Note that the PTP stack checks
 * if idx exceeds LWIP_PTP_NUM_TIMERS, so the user function
 * does not need to do this.
 * @param interval length of the timer in milliseconds.
 */
void LWIP_PTP_START_TIMER(u32_t, u32_t);

/**
 * @brief Stop a system timer.
 * @param idx index of the timer to stop. Note that the PTP stack will only
 * let this function be called if the timer is active, so this does not need
 * to be checked by the user application.
 */
void LWIP_PTP_STOP_TIMER(u32_t);

/**
 * @brief Check if a system timer has expired.
 * @param idx index of the timer to check.
 * @retval true if timer has expired, false if timer is still running.
 */
bool LWIP_PTP_CHECK_TIMER(u32_t);

/**
 * @brief function to call when a system timer has expired.
 * @param idx index of the timer that has expired.
 */
void lwipPtpTimerExpired(u32_t idx);

/**
 * @brief Initialise PTP software stack.
 * @param priority rtos priority of main PTP handler thread.
 */
void lwipPtpInit(u8_t priority);

/**
 * @brief Notify PTP stack that a transmitted packet's timestamp is
 * ready to be received.
 * For non-DMA-capable Ethernet hardware, this semaphore can be called
 * immediately on sending of a timestamped packet. However, for DMA-capable
 * Ethernet hardware, this may need to be called by a TX interrupt.
 * This only needs to be called on packets where the pbuf's timestamp fields
 * are all 'ones' (UINT32_MAX), as this indicates that a timestamp is expected
 * for this packet.
 */
void lwipPtpTxNotify(void);

#endif /* __LWIP_PTP_H__ */
