#ifndef __LWIP_PTP_PROTOCOL_H__
#define __LWIP_PTP_PROTOCOL_H__

/**
 * @file
 * @brief ptpd-lwip protocol interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "def/datatypes_private.h"

/**
 * \brief Run PTP stack in current state
 * Handle actions and events for 'port_state'
 */
void doState(ptpClock_t *ptpClock);

/**
 * \brief Change state of PTP stack
 * Perform actions required when leaving 'port_state' and entering 'state'
 */
void toState(ptpClock_t *ptpClock, u8_t state);

#endif /* __LWIP_PTP_PROTOCOL_H__ */
