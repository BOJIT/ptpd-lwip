/* net.c */

#include "net.h"

#if LWIP_PTP || defined __DOXYGEN__

#include <lwip/tcpip.h>
#include <lwip/igmp.h>

/// @todo Horrendously inefficient PBUF copying about 3 times! Fix soon.

// TEMPORARY - this alert system will eventually be restructured.
extern sys_mbox_t ptpAlert;

/* Process an incoming message */
static void netRecvCallback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                                const ip_addr_t *addr, u16_t port)
{
    (void)(pcb);    // UNUSED
    (void)(port);   // UNUSED

    // DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Packet Arrived!");

    packetHandler_t *handler = (packetHandler_t *)arg;

    /* Back up existing packet and overwrite with new data */
    packet_t packetTmp = handler->inboxBuf[handler->inboxHead];
    handler->inboxBuf[handler->inboxHead].pbuf = p;
    handler->inboxBuf[handler->inboxHead].destAddr.addr = addr->addr;

    if(sys_mbox_trypost(&handler->inbox, &handler->inboxBuf[handler->inboxHead])
                                                                        != ERR_OK) {
        ERROR("netRecvEventCallback: queue full - %lu\n", handler);
        pbuf_free(p);
        handler->inboxBuf[handler->inboxHead] = packetTmp;
        return;
    }

    /* Cycle the buffer index */
    handler->inboxHead++;
    if(handler->inboxHead == PBUF_QUEUE_SIZE) {
        handler->inboxHead = 0;
    }

    /* Alert the PTP thread there is now something to do. */
    if(sys_mbox_trypost(&ptpAlert, NULL) != ERR_OK) {
        DBGVV("netRecvEventCallback: Mailbox Full!\n");
    }
}

/* Process an outgoing message */
static void netSendCallback(void *arg)
{
    packetHandler_t *handler = (packetHandler_t *)arg;
    void* msg;
    packet_t *packet;

    DBGVV("netSendEventCallback: %lu\n", &handler->inbox);

    /* Fetch message from mailbox - return if inbox is empty */
    if(sys_arch_mbox_tryfetch(&handler->outbox, &msg) == SYS_MBOX_EMPTY) {
        return;
    }

    DBGVV("netSendEventCallback: queue\n");

    packet = (packet_t *)msg;

    /* send the buffer. */
    udp_sendto(handler->pcb, packet->pbuf, &packet->destAddr,
                                            handler->pcb->local_port);
}

/* Initialize network handler */
static void netHandlerInit(packetHandler_t *handler, ip_addr_t *defaultAddr,
                                                                    u16_t port)
{
    /* Initialise mailboxes */
    sys_mbox_new(&handler->inbox, PBUF_QUEUE_SIZE);
    sys_mbox_new(&handler->outbox, PBUF_QUEUE_SIZE);

    DBGV("netHandlerInit: handler - %lu\n", handler);

    /* Create UDP PCB */
    handler->pcb = udp_new();
    if (NULL == handler->pcb) {
        ERROR("netInit: Failed to open UDP PCB\n");
        return;
    }

    /* Multicast send only on specified interface. */
    handler->pcb->mcast_ip4.addr = defaultAddr->addr;

    /* Establish the appropriate UDP bindings/connections for events. */
    udp_recv(handler->pcb, netRecvCallback, handler);
    udp_bind(handler->pcb, IP_ADDR_ANY, port);
}

/* Shutdown network handler */
static void netHandlerShutdown(packetHandler_t *handler)
{
    /* Disconnect and close the UDP interface */
    if(handler->pcb) {
        udp_disconnect(handler->pcb);
        udp_remove(handler->pcb);
        handler->pcb = NULL;
    }

    /* Delete mailboxes */
    if(sys_mbox_valid(&handler->inbox))
        sys_mbox_free(&handler->inbox);
    if(sys_mbox_valid(&handler->outbox))
        sys_mbox_free(&handler->outbox);
}

/* Start all of the UDP stuff - LOCKS CORE */
bool netInit(netPath_t *netPath, ptpClock_t *ptpClock)
{
    (void)(ptpClock); // UNUSED

    LOCK_TCPIP_CORE(); // System must support core locking!

    DBG("netInit\n");

    /* Join multicast group (for receiving) on specified interface */
    netPath->multicastAddr.addr = DEFAULT_PTP_DOMAIN_ADDRESS;
    igmp_joingroup(&netif_default->ip_addr, &netPath->multicastAddr);

    /* Join peer multicast group (for receiving) on specified interface */
    netPath->peerMulticastAddr.addr = PEER_PTP_DOMAIN_ADDRESS;
    igmp_joingroup(&netif_default->ip_addr, &netPath->peerMulticastAddr);

    /* Initialize the buffer queues. */
    netHandlerInit(&netPath->eventHandler, &netPath->multicastAddr, PTP_EVENT_PORT);
    netHandlerInit(&netPath->generalHandler, &netPath->multicastAddr, PTP_GENERAL_PORT);

    /* Return a success code. */
    UNLOCK_TCPIP_CORE();
    return true;
}

/* Shut down the UDP and network stuff - LOCKS CORE */
void netShutdown(netPath_t *netPath)
{
    LOCK_TCPIP_CORE();

    DBG("netShutdown\n");

    /* leave multicast group */
    if(netPath->multicastAddr.addr != 0) {
        igmp_leavegroup(IP_ADDR_ANY, &netPath->multicastAddr);
        netPath->multicastAddr.addr = 0;
    }

    /* leave peer multicast group */
    if(netPath->peerMulticastAddr.addr != 0) {
        igmp_leavegroup(IP_ADDR_ANY, &netPath->peerMulticastAddr);
        netPath->peerMulticastAddr.addr = 0;
    }

    /* Disconnect and close the UDP interfaces */
    netHandlerShutdown(&netPath->eventHandler);
    netHandlerShutdown(&netPath->generalHandler);

    /* Clear the network addresses. */
    netPath->multicastAddr.addr = 0;
    netPath->peerMulticastAddr.addr = 0;
    UNLOCK_TCPIP_CORE();
}

/* Wait for a packet  to come in on either port.  For now, there is no wait.
 * Simply check to  see if a packet is available on either port and return 1,
 *  otherwise return 0. */
// int32_t netSelect(netPath_t *netPath)
// {
//     /* Check the packet queues.  If there is data, return TRUE. */
//     if (netQCheck(&netPath->eventQ) || netQCheck(&netPath->generalQ)) return 1;

//     return 0;
// }

/* Delete all waiting packets in event queue. */
void netEmptyEventQ(netPath_t *netPath)
{
    void *msg;
    while(sys_arch_mbox_tryfetch(&netPath->eventHandler.inbox, &msg) != SYS_MBOX_EMPTY);
}

/* Recieve network message and copy across timestamp */
static ssize_t netRecv(octet_t *buf, timeInternal_t *time, packetHandler_t *handler)
{
    int i;
    int j;
    u16_t length;
    struct pbuf *p;
    struct pbuf *pcopy;
    void *msg;
    packet_t *packet;

    /* Fetch message from mailbox - return if inbox is empty */
    if(sys_arch_mbox_tryfetch(&handler->inbox, &msg) == SYS_MBOX_EMPTY) {
        DBGV("NetRecv: Empty Mailbox!!!\n");
        return 0;
    }
    packet = (packet_t *)msg;
    p = packet->pbuf;

    /* Verify that we have enough space to store the contents. */
    if (p->tot_len > PACKET_SIZE) {
        ERROR("netRecv: received truncated message\n");
        pbuf_free(p);
        return 0;
    }

    /* Verify there is contents to copy. */
    if (p->tot_len == 0) {
        ERROR("netRecv: received empty packet\n");
        pbuf_free(p);
        return 0;
    }

    /* Copy across timestamp if required. */
    if (time != NULL) {
        DBGV("netRecv: getting timestamp: \n");
        #if LWIP_PTP /** @todo this is potentially redundant */
            // DBGV("ptp: s - %lu\n", p->tv_sec);
            time->seconds = p->tv_sec;
            time->nanoseconds = p->tv_nsec;
        #else
            getTime(time);
        #endif
        DBGV("netRecv: %d sec %d nsec\n", time->seconds, time->nanoseconds);
    }

    /* Get the length of the buffer to copy. */
    length = p->tot_len;

    /* Copy the pbuf payload into the buffer. */
    pcopy = p;
    j = 0;
    for (i = 0; i < length; i++) {
        // Copy the next byte in the payload.
        buf[i] = ((u8_t *)pcopy->payload)[j++];

        // Skip to the next buffer in the payload?
        if (j == pcopy->len) {
            // Move to the next buffer.
            pcopy = pcopy->next;
            j = 0;
        }
    }

    /* Free up the pbuf (chain). */
    pbuf_free(p);

    return length;
}

static ssize_t netSend(const octet_t *buf, int16_t length, timeInternal_t *time,
                        const ip_addr_t *addr, packetHandler_t *handler, sys_sem_t *ptpTxNotify)
{
    err_t result;
    struct pbuf *p;

    DBGV("netSend: something\n");

    /* Allocate the tx pbuf based on the current size. */
    p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
    if (NULL == p) {
        ERROR("netSend: Failed to allocate Tx Buffer\n");
        return 0;
    }

    /* Copy the incoming data into the pbuf payload. */
    result = pbuf_take(p, buf, length);
    if (ERR_OK != result) {
        ERROR("netSend: Failed to copy data to Pbuf (%d)\n", result);
        pbuf_free(p);
        return 0;
    }

    if (time != NULL) {
        /* Tag the pbuf to tell stack that a timestamp will be taken */
        p->tv_sec = UINT32_MAX;
        p->tv_nsec = UINT32_MAX;
    }

    /* Back up existing packet and overwrite with new data */
    packet_t packetTmp = handler->outboxBuf[handler->outboxHead];
    handler->outboxBuf[handler->outboxHead].pbuf = p;
    handler->outboxBuf[handler->outboxHead].destAddr.addr = addr->addr;

    if(sys_mbox_trypost(&handler->outbox, &handler->outboxBuf[handler->outboxHead])
                                                                        != ERR_OK) {
        ERROR("netSend: queue full\n");
        pbuf_free(p);
        handler->outboxBuf[handler->outboxHead] = packetTmp;
        return 0;
    }

    /// @todo potentially replace mailbox buffers with a linked list?
    /* Cycle the buffer index */
    handler->outboxHead++;
    if(handler->outboxHead == PBUF_QUEUE_SIZE) {
        handler->outboxHead = 0;
    }

    /* Notify Transmit Handler that there is a packet to process */
    if(tcpip_callback(netSendCallback, (void *)handler) != ERR_OK)
        return 0;

    /* Write timestamp to PTP internal time if required (wait for tx) */
    if (time != NULL) {
        #if LWIP_PTP /** @todo!!! remove this macro */
            sys_arch_sem_wait(ptpTxNotify, 0);

            if((p->tv_sec & p->tv_nsec) != UINT32_MAX) {
            time->seconds = p->tv_sec;
            time->nanoseconds = p->tv_nsec;
            }
            else {
                DBGVV("Timestamp Corrupted!\n");
                return 0;
            }
        #else
            getTime(time);  // This does not take use hardware timstamping.
        #endif
        DBGV("netSend: %d sec %d nsec\n", time->seconds, time->nanoseconds);
    }
    else {
        DBGV("netSend\n");
    }

    return length;
}

/* Recieve Network Packet on Event Port */
ssize_t netRecvEvent(netPath_t *netPath, octet_t *buf, timeInternal_t *time)
{
    return netRecv(buf, time, &netPath->eventHandler);
}

/* Recieve Network Packet on General Port */
ssize_t netRecvGeneral(netPath_t *netPath, octet_t *buf, timeInternal_t *time)
{
    return netRecv(buf, time, &netPath->generalHandler);
}

/* Send Network Packet on Event Port */
ssize_t netSendEvent(netPath_t *netPath, const octet_t *buf,
                                        int16_t length, timeInternal_t *time)
{
    return netSend(buf, length, time, &netPath->multicastAddr,
                                &netPath->eventHandler, &netPath->ptpTxNotify);
}

/* Send Network Packet on General Port */
ssize_t netSendGeneral(netPath_t *netPath, const octet_t *buf, int16_t length)
{
    return netSend(buf, length, NULL, &netPath->multicastAddr,
                                                &netPath->generalHandler, NULL);
}

/* Send Network Packet on Peer Event Port */
ssize_t netSendPeerEvent(netPath_t *netPath, const octet_t *buf,
                                        int16_t length, timeInternal_t *time)
{
    return netSend(buf, length, time, &netPath->peerMulticastAddr,
                                &netPath->eventHandler, &netPath->ptpTxNotify);
}

/* Send Network Packet on Peer General Port */
ssize_t netSendPeerGeneral(netPath_t *netPath, const octet_t *buf, int16_t length)
{
    return netSend(buf, length, NULL, &netPath->peerMulticastAddr,
                                                &netPath->generalHandler, NULL);
}

#endif /* LWIP_PTP || defined __DOXYGEN__ */
