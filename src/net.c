/* net.c */

#include "net.h"

/// @todo Horrendously inefficient PBUF copying about 3 times! Fix soon.

/* Process an incoming message */
static void netRecvCallback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                                const ip_addr_t *addr, u16_t port)
{
    packetHandler_t *handler = (packetHandler_t *)arg;

    static int bufIdx = 0;

    /* Back up existing packet and overwrite with new data */
    packet_t packetTmp = handler->inboxBuf[bufIdx];
    handler->inboxBuf[bufIdx].pbuf = p;
    handler->inboxBuf[bufIdx].destAddr.addr = addr->addr;

    if(sys_mbox_trypost(&handler->inbox, &handler->inboxBuf[bufIdx]) != ERR_OK) {
        ERROR("netRecvEventCallback: queue full\n");
        pbuf_free(p);
        handler->inboxBuf[bufIdx] = packetTmp;
        return;
    }

    /* Cycle the buffer index */
    if(bufIdx < PBUF_QUEUE_SIZE) {
        bufIdx++;
    }
    else {
        bufIdx = 0;
    }

    /* Alert the PTP thread there is now something to do. */
    ptpd_alert(); /// @todo fix dependencies later
}

/* Initialize network handler */
static void netHandlerInit(packetHandler_t *handler, ip_addr_t *defaultAddr)
{
    /* Initialise mailboxes */
    sys_mbox_new(&handler->inbox, PBUF_QUEUE_SIZE);
    sys_mbox_new(&handler->outbox, PBUF_QUEUE_SIZE);

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
    udp_bind(handler->pcb, IP_ADDR_ANY, PTP_EVENT_PORT);
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
    sys_mbox_free(&handler->inbox);
    sys_mbox_free(&handler->outbox);
}

/* Start all of the UDP stuff - LOCKS CORE */
bool netInit(netPath_t *netPath, ptpClock_t *ptpClock)
{
    LOCK_TCPIP_CORE(); // System must support core locking!

    DBG("netInit\n");

    /* Join multicast group (for receiving) on specified interface */
    netPath->multicastAddr.addr = DEFAULT_PTP_DOMAIN_ADDRESS;
    igmp_joingroup(&netif_default->ip_addr, &netPath->multicastAddr);

    /* Join peer multicast group (for receiving) on specified interface */
    netPath->peerMulticastAddr.addr = PEER_PTP_DOMAIN_ADDRESS;
    igmp_joingroup(&netif_default->ip_addr, &netPath->peerMulticastAddr);

    /* Initialize the buffer queues. */
    netHandlerInit(&netPath->eventHandler, &netPath->multicastAddr.addr, netRecvEventCallback);
    netHandlerInit(&netPath->generalHandler, &netPath->multicastAddr.addr, netRecvGeneralCallback);

    /* Return a success code. */
    UNLOCK_TCPIP_CORE();
    return true;
}

/* Shut down the UDP and network stuff - LOCKS CORE */
void netShutdown(netPath_t *netPath)
{
    LOCK_TCPIP_CORE();
    ip_addr_t multicastAaddr;

    DBG("netShutdown\n");

    /* leave multicast group */
    igmp_leavegroup(IP_ADDR_ANY, &netPath->multicastAddr);

    /* leave multicast group */
    igmp_leavegroup(IP_ADDR_ANY, &netPath->peerMulticastAddr);

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
    packet_t *packet;

    /* Fetch message from mailbox - return if inbox is empty */
    if(sys_mbox_tryfetch(&handler->inbox, packet) == SYS_MBOX_EMPTY) {
        return 0;
    }
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

    if (time != NULL) {
        #if LWIP_PTP
            time->seconds = p->tv_sec;
            time->nanoseconds = p->tv_nsec;
        #else
            getTime(time);
        #endif
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

// Problem...
static ssize_t netSend(const octet_t *buf, int16_t  length, timeInternal_t *time,
                                        const int32_t * addr, struct udp_pcb * pcb)
{
	err_t result;
	struct pbuf * p;

	/* Allocate the tx pbuf based on the current size. */
	p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	if (NULL == p) {
		ERROR("netSend: Failed to allocate Tx Buffer\n");
		goto fail01;
	}

	/* Copy the incoming data into the pbuf payload. */
	result = pbuf_take(p, buf, length);
	if (ERR_OK != result) {
		ERROR("netSend: Failed to copy data to Pbuf (%d)\n", result);
		goto fail02;
	}

	/* send the buffer. */
	result = udp_sendto(pcb, p, (void *)addr, pcb->local_port);
	if (ERR_OK != result) {
		ERROR("netSend: Failed to send data (%d)\n", result);
		goto fail02;
	}

	if (time != NULL) {
        #if LWIP_PTP
            time->seconds = p->tv_sec;
            time->nanoseconds = p->tv_nsec;
        #else
            getTime(time);
        #endif
		DBGV("netSend: %d sec %d nsec\n", time->seconds, time->nanoseconds);
	}
    else {
		DBGV("netSend\n");
	}


fail02:
    pbuf_free(p);

fail01:
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
    return netSend(buf, length, time, &netPath->multicastAddr, netPath->eventPcb);
}

/* Send Network Packet on General Port */
ssize_t netSendGeneral(netPath_t *netPath, const octet_t *buf, int16_t length)
{
    return netSend(buf, length, NULL, &netPath->multicastAddr, netPath->generalPcb);
}

/* Send Network Packet on Peer Event Port */
ssize_t netSendPeerEvent(netPath_t *netPath, const octet_t *buf,
                                        int16_t length, timeInternal_t *time)
{
    return netSend(buf, length, time, &netPath->peerMulticastAddr, netPath->eventPcb);
}

/* Send Network Packet on Peer General Port */
ssize_t netSendPeerGeneral(netPath_t *netPath, const octet_t *buf, int16_t length)
{
    return netSend(buf, length, NULL, &netPath->peerMulticastAddr, netPath->generalPcb);
}
