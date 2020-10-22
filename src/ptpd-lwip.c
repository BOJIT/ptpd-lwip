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

runTimeOpts_t rtOpts;
ptpClock_t ptpClock;
foreignMasterRecord_t ptpForeignRecords[DEFAULT_MAX_FOREIGN_RECORDS];

static sys_mbox_t ptp_alert_queue;

void ptpd_thread(void *args __attribute((unused))) {

    ptpClock.netPath.generalConn = netconn_new(NETCONN_UDP);
    ptpClock.netPath.eventConn = netconn_new(NETCONN_UDP);

    netconn_bind(ptpClock.netPath.generalConn, IP_ADDR_ANY, PTP_GENERAL_PORT);
    netconn_bind(ptpClock.netPath.eventConn, IP_ADDR_ANY, PTP_EVENT_PORT);

    ptpClock.netPath.multicastAddr.addr = htonl(DEFAULT_PTP_DOMAIN_ADDRESS);
    ptpClock.netPath.peerMulticastAddr.addr = htonl(PEER_PTP_DOMAIN_ADDRESS);
    ptpClock.netPath.localAddr.addr = netif_default->ip_addr.addr;
    netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.multicastAddr,
                                                    &ptpClock.netPath.localAddr, NETCONN_JOIN);
    netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.peerMulticastAddr,
                                                    &ptpClock.netPath.localAddr, NETCONN_JOIN);
    netconn_join_leave_group(ptpClock.netPath.eventConn, &ptpClock.netPath.multicastAddr,
                                                    &ptpClock.netPath.localAddr, NETCONN_JOIN);
    netconn_join_leave_group(ptpClock.netPath.eventConn, &ptpClock.netPath.peerMulticastAddr,
                                                    &ptpClock.netPath.localAddr, NETCONN_JOIN);

    for(;;) {

    }
}


void ptpdInit(ptpFunctions_t *functions, u8_t priority)
{
    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "PTPD INIT!!!");

    /* Pass HAL function pointers to sys_time module */
    initTimeFunctions(functions);

    // // Create the alert queue mailbox.
    // if(sys_mbox_new(&ptp_alert_queue, 8) != ERR_OK) {
    //     printf("PTPD: failed to create ptp_alert_queue mbox");
    // }

    // Create the PTP daemon thread.
    sys_thread_new("ptpd", ptpd_thread, NULL, 1024, priority);
}
