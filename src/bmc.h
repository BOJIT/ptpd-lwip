#ifndef __LWIP_PTP_BMC_H__
#define __LWIP_PTP_BMC_H__

/**
 * @file
 * @brief ptpd-lwip best master clock algorithms
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "def/datatypes_private.h"

/**
 * \brief Initialize datas
 */
void initData(ptpClock_t *ptpClock);

/**
 * \brief Compare two port identities
 */
bool isSamePortIdentity(const portIdentity_t *A, const portIdentity_t *B);

/**
 * \brief Add foreign record defined by announce message
 */
void addForeign(ptpClock_t *ptpClock, const msgHeader_t *header,
                                                const msgAnnounce_t *announce);

/**
 * \brief When recommended state is Master, copy local data into parent and
 * grandmaster dataset
 * Local clock is becoming Master. Table 13 (9.3.5) of the spec
 */
void m1(ptpClock_t *ptpClock);

/**
 * \brief When recommended state is Passive
 */
void p1(ptpClock_t *ptpClock);

/**
 * \brief When recommended state is Slave, copy dataset of master into parent
 * and grandmaster dataset
 * Local clock is synchronized to Ebest Table 16 (9.3.5) of the spec
 */
void s1(ptpClock_t *ptpClock, const msgHeader_t *header,
                                                const msgAnnounce_t *announce);

/**
 * \brief Compare data set of foreign masters and local data set
 * \return The recommended state for the port
 */
u8_t bmc(ptpClock_t *ptpClock);

#endif /* __LWIP_PTP_BMC_H__ */
