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

// static sys_mbox_t ptp_alert_queue;

static void ptpd_thread(void *args __attribute((unused))) {

    /// @todo probably need to check link status here too
    #if LWIP_DHCP
        // If DHCP, wait until the default interface has an IP address.
        while (!netif_default->ip_addr.addr) {
            // Sleep for 500 milliseconds.
            sys_msleep(500);
        }
    #endif

    // DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Joining IGMP group");
    // ptpClock.netPath.generalConn = netconn_new(NETCONN_UDP);
    // ptpClock.netPath.eventConn = netconn_new(NETCONN_UDP);

    // if(ptpClock.netPath.generalConn == NULL) {
    //     DEBUG_MESSAGE(DEBUG_TYPE_ERROR, "Netconn not allocated!");
    //     netconn_delete(ptpClock.netPath.generalConn);
    // }

    // err_t netstate = netconn_bind(ptpClock.netPath.generalConn, IP_ADDR_ANY, PTP_GENERAL_PORT);
    // DEBUG_MESSAGE(DEBUG_TYPE_INFO, "bindstate: %d", netstate);

    // IP4_ADDR(&ptpClock.netPath.multicastAddr, 239, 0, 0, 0);
    // IP4_ADDR(&ptpClock.netPath.localAddr, 192, 168, 42, 42);
    // // ptpClock.netPath.multicastAddr.addr = htonl(DEFAULT_PTP_DOMAIN_ADDRESS);
    // // ptpClock.netPath.peerMulticastAddr.addr = htonl(PEER_PTP_DOMAIN_ADDRESS);
    // DEBUG_MESSAGE(DEBUG_TYPE_INFO, "addr: %lu", ptpClock.netPath.multicastAddr.addr);
    // // ptpClock.netPath.localAddr.addr = netif_default->ip_addr.addr;
    // // err_t state = netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.multicastAddr,
    // //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);
    // err_t state = netconn_join_leave_group_netif(ptpClock.netPath.generalConn, &ptpClock.netPath.multicastAddr,
    //                                                 1, NETCONN_JOIN);
    // DEBUG_MESSAGE(DEBUG_TYPE_INFO, "localaddr: %lu", ptpClock.netPath.localAddr.addr);
    // DEBUG_MESSAGE(DEBUG_TYPE_INFO, "igmpstate: %d", state);
    // // netconn_join_leave_group(ptpClock.netPath.generalConn, &ptpClock.netPath.peerMulticastAddr,
    // //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);
    // // netconn_join_leave_group(ptpClock.netPath.eventConn, &ptpClock.netPath.multicastAddr,
    // //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);
    // // netconn_join_leave_group(ptpClock.netPath.eventConn, &ptpClock.netPath.peerMulticastAddr,
    // //                                                 &ptpClock.netPath.localAddr, NETCONN_JOIN);

    // struct netbuf *buf;
    // err_t err;

    // DEBUG_MESSAGE(DEBUG_TYPE_WARNING, "got here");

    // for(;;) {
    //     while((err = netconn_recv(ptpClock.netPath.generalConn, &buf)) == ERR_OK) {
    //         DEBUG_MESSAGE(DEBUG_TYPE_INFO, "UDP Arrived!");
    //     }
    // }

  static struct netconn *conn;

  static struct netbuf *buf;

  ip_addr_t MulticastIPaddr, OwnAddr;

  err_t err, recv_err;

//   netif_default->flags = NETIF_FLAG_UP | NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;

  IP4_ADDR(&MulticastIPaddr, 239, 0, 0, 0);

  IP4_ADDR(&OwnAddr, 192, 168, 42, 42);

  conn = netconn_new(NETCONN_UDP);

  if (conn!= NULL)

  {

    netconn_bind(conn, IP_ADDR_ANY, 60004);  // returns ERR_OK

    err = netconn_join_leave_group(conn, &MulticastIPaddr, &OwnAddr, NETCONN_JOIN); // returns ERR_OK
    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Error: %d", err);

    if (err == ERR_OK) {
        DEBUG_MESSAGE(DEBUG_TYPE_INFO, "RECV");
      while (1)
      {
        recv_err = netconn_recv(conn, &buf);

        if (recv_err == ERR_OK) {
            // process the packet
        }
      }
    }

    else {
        DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Not RECV");
      netconn_delete(conn);
    }
  }

    while(1){
        vTaskDelay(10000);
    }
}


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
