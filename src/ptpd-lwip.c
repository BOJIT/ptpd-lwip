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

static sys_mbox_t ptp_alert_queue;

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
