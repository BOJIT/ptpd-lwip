/**
 * @file
 * @brief ptpd-lwip entry code
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "ptpd-lwip.h"

#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netbuf.h>

#include "def/datatypes_private.h"
#include "sys_time.h"

/* Include FreeRTOS-Debug library headers */
#include <FreeRTOS-Debug.h> // GET RID OF LATER!!!

ptpClock_t ptpClock;
runTimeOpts_t rtOpts;
foreignMasterRecord_t ptpForeignRecords[DEFAULT_MAX_FOREIGN_RECORDS];


static void ptpd_thread(void *args __attribute((unused))) {

    /// @todo probably need to check link status here too
    #if LWIP_DHCP
        // If DHCP, wait until the default interface has an IP address.
        while (!netif_default->ip_addr.addr) {
            // Sleep for 500 milliseconds.
            sys_msleep(500);
        }
    #endif

    static struct netbuf *buf;
    err_t err;

    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Joining IGMP group");
    ptpClock.netPath.generalConn = netconn_new(NETCONN_UDP);
    ptpClock.netPath.eventConn = netconn_new(NETCONN_UDP);

    if(ptpClock.netPath.generalConn == NULL) {
        DEBUG_MESSAGE(DEBUG_TYPE_ERROR, "Netconn not allocated!");
        netconn_delete(ptpClock.netPath.generalConn);
    }

    netconn_bind(ptpClock.netPath.generalConn, IP_ADDR_ANY, PTP_GENERAL_PORT);

    IP4_ADDR(&ptpClock.netPath.multicastAddr, 224, 0, 1, 129);
    IP4_ADDR(&ptpClock.netPath.localAddr, 192, 168, 42, 42);
    // ptpClock.netPath.multicastAddr.addr = htonl(DEFAULT_PTP_DOMAIN_ADDRESS);
    // ptpClock.netPath.peerMulticastAddr.addr = htonl(PEER_PTP_DOMAIN_ADDRESS);

    // netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.peerMulticastAddr,
    //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);
    // netconn_join_leave_group(ptpClock.netPath.eventConn, &ptpClock.netPath.multicastAddr,
    //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);
    // netconn_join_leave_group(ptpClock.netPath.eventConn, &ptpClock.netPath.peerMulticastAddr,
    //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);

    // ptpClock.netPath.localAddr.addr = netif_default->ip_addr.addr;
    // err_t state = netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.multicastAddr,
    //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);

    err = netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.multicastAddr,
                                                    &netif_default->ip_addr, NETCONN_JOIN);

    err = netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.peerMulticastAddr,
                                                    &netif_default->ip_addr, NETCONN_JOIN);

    if(err == ERR_OK) {
        while(1) {
            if((err = netconn_recv(ptpClock.netPath.generalConn, &buf)) == ERR_OK) {
                DEBUG_MESSAGE(DEBUG_TYPE_INFO, "UDP Arrived!");
                netbuf_delete(buf);
            }
            else {
                DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Here");
            }
        }
    }
    else {
        DEBUG_MESSAGE(DEBUG_TYPE_ERROR, "Failed to join multicast group!");
        netconn_delete(ptpClock.netPath.generalConn);
        vTaskDelete(NULL);
    }
}

/// @todo remember that the PTP timestamps need to be written into the pbuf when
// descriptors are processed!

void ptpdInit(ptpFunctions_t *functions, u8_t priority)
{
    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "PTPD INIT!!!");

    /* Pass HAL function pointers to sys_time module */
    initTimeFunctions(functions);

    // Create the alert queue mailbox.
    // if(sys_mbox_new(&ptp_alert_queue, 8) != ERR_OK) {
    //     printf("PTPD: failed to create ptp_alert_queue mbox");
    // }

    // Create the PTP daemon thread.
    sys_thread_new("ptpd", ptpd_thread, NULL, 1024, priority);
}
