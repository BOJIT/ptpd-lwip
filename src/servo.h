#ifndef __PTPD_SERVO_H__
#define __PTPD_SERVO_H__

/**
 * @file
 * @brief ptpd-lwip 5.3 servo interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "def/datatypes_private.h"

/* Initialise servo and clear network queue */
void initClock(ptpClock_t *ptpClock);

/* 11.2 - actual offset correction calculation based ib timestamps */
void updateOffset(ptpClock_t *ptpClock, const timeInternal_t *syncEventIngressTimestamp,
                                            const timeInternal_t *preciseOriginTimestamp,
                                            const timeInternal_t *correctionField);

/* 11.3 - internally update the network path delay (based on basic filtering) */
void updateDelay(ptpClock_t *ptpClock, const timeInternal_t *delayEventEgressTimestamp,
                                            const timeInternal_t *recieveTimestamp,
                                            const timeInternal_t *correctionField);

/* Update peer delay with a filter */
void updatePeerDelay(ptpClock_t *ptpClock, const timeInternal_t *correctionField,
                                                                    bool twoStep);

/* Update local clock based on timestamps */
void updateClock(ptpClock_t *ptpClock);

#endif /* __PTPD_SERVO_H__ */
