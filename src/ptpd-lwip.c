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

/* Include FreeRTOS-Debug library headers */
#include <FreeRTOS-Debug.h> // GET RID OF LATER!!!


static sys_mbox_t ptp_alert_queue;

void ptpdInit(ptpFunctions_t *ptpd_functions)
{
    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "PTPD INIT!!!");

    /* // // Create the alert queue mailbox.
    // if(sys_mbox_new(&ptp_alert_queue, 8) != ERR_OK) {
    //     printf("PTPD: failed to create ptp_alert_queue mbox");
    // }

    // // Create the PTP daemon thread.
    // sys_thread_new("PTPD", ptpd_thread, NULL,
    //                 1024, FREERTOS_PRIORITIES-4); */
}
