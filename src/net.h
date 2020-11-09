#ifndef __PTPD_NET_H__
#define __PTPD_NET_H__

/**
 * @file
 * @brief ptpd-lwip 5.3 network handler
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "def/datatypes_private.h"

/* Start all of the UDP stuff - LOCKS CORE */
bool netInit(netPath_t *netPath, ptpClock_t *ptpClock);

/* Shut down the UDP and network stuff - LOCKS CORE */
void netShutdown(netPath_t *netPath);

/* Delete all waiting packets in event queue. */
void netEmptyEventQ(netPath_t *netPath);

/* Recieve Network Packet on Event Port */
ssize_t netRecvEvent(netPath_t *netPath, octet_t *buf, timeInternal_t *time);

/* Recieve Network Packet on General Port */
ssize_t netRecvGeneral(netPath_t *netPath, octet_t *buf, timeInternal_t *time);

/* Send Network Packet on Event Port */
ssize_t netSendEvent(netPath_t *netPath, const octet_t *buf,
                                        int16_t length, timeInternal_t *time);

/* Send Network Packet on General Port */
ssize_t netSendGeneral(netPath_t *netPath, const octet_t *buf, int16_t length);

/* Send Network Packet on Peer Event Port */
ssize_t netSendPeerEvent(netPath_t *netPath, const octet_t *buf,
                                        int16_t length, timeInternal_t *time);

/* Send Network Packet on Peer General Port */
ssize_t netSendPeerGeneral(netPath_t *netPath, const octet_t *buf, int16_t length);


#endif /* __PTPD_NET_H__ */
