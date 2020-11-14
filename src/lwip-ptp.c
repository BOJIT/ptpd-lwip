/**
 * @file
 * @brief ptpd-lwip entry code
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "lwip-ptp.h"
#include "def/datatypes_private.h"

#if LWIP_PTP || defined __DOXYGEN__

#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netbuf.h>

#include "protocol.h"
#include "sys_time.h"

ptpClock_t ptpClock;
runTimeOpts_t rtOpts;
foreignMasterRecord_t ptpForeignRecords[DEFAULT_MAX_FOREIGN_RECORDS];

sys_mbox_t ptpAlert;

/*---------------------------- Private Functions -----------------------------*/




static void ptpd_thread(void *args __attribute((unused))) {

    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "PTPd thread started...");

    // Initialize run-time options to default values.
    rtOpts.announceInterval = DEFAULT_ANNOUNCE_INTERVAL;
    rtOpts.syncInterval = DEFAULT_SYNC_INTERVAL;
    rtOpts.clockQuality.clockAccuracy = DEFAULT_CLOCK_ACCURACY;
    rtOpts.clockQuality.clockClass = DEFAULT_CLOCK_CLASS;
    rtOpts.clockQuality.offsetScaledLogVariance = DEFAULT_CLOCK_VARIANCE; /* 7.6.3.3 */
    rtOpts.priority1 = DEFAULT_PRIORITY1;
    rtOpts.priority2 = DEFAULT_PRIORITY2;
    rtOpts.domainNumber = DEFAULT_DOMAIN_NUMBER;
    rtOpts.slaveOnly = SLAVE_ONLY;
    rtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
    rtOpts.servo.noResetClock = DEFAULT_NO_RESET_CLOCK;
    rtOpts.servo.noAdjust = NO_ADJUST;
    rtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
    rtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
    rtOpts.servo.sDelay = DEFAULT_DELAY_S;
    rtOpts.servo.sOffset = DEFAULT_OFFSET_S;
    rtOpts.servo.ap = DEFAULT_AP;
    rtOpts.servo.ai = DEFAULT_AI;
    rtOpts.maxForeignRecords = sizeof(ptpForeignRecords) / sizeof(ptpForeignRecords[0]);
    rtOpts.stats = PTP_TEXT_STATS;
    rtOpts.delayMechanism = DEFAULT_DELAY_MECHANISM;

    ptpClock.rtOpts = &rtOpts;
    ptpClock.foreignMasterDS.records = ptpForeignRecords;

    /* 9.2.2 */
    if (rtOpts.slaveOnly) rtOpts.clockQuality.clockClass = DEFAULT_CLOCK_CLASS_SLAVE_ONLY;

    /* No negative or zero attenuation */
    if (rtOpts.servo.ap < 1) rtOpts.servo.ap = 1;
    if (rtOpts.servo.ai < 1) rtOpts.servo.ai = 1;

    toState(&ptpClock, PTP_INITIALIZING);

    /// @todo probably need to check link status here too
    #if LWIP_DHCP
        // If DHCP, wait until the default interface has an IP address.
        while (!netif_default->ip_addr.addr) {
            // Sleep for 500 milliseconds.
            sys_msleep(500);
        }
    #endif

    for (;;) {
        // Process the current state.
        doState(&ptpClock);

        DBGV("------------------------------------\n");

        sys_arch_mbox_fetch(&ptpAlert, NULL, 0);
    }
}

/*----------------------------- Public Functions -----------------------------*/

/**
 * @brief function to call when a system timer has expired.
 * @param idx index of the timer that has expired.
 */
void lwipPtpTimerExpired(u32_t idx)
{
    UNUSED(idx);
    if(sys_mbox_trypost(&ptpAlert, NULL) != ERR_OK) {
        DBGVV("Mailbox Full!");
    }
}

/**
 * @brief Initialise PTP software stack.
 * @param priority rtos priority of main PTP handler thread.
 */
void lwipPtpInit(u8_t priority)
{
    DEBUG_MESSAGE(DEBUG_TYPE_INFO, "PTPd initialising...");

    // Create the alert queue mailbox.
    if(sys_mbox_new(&ptpAlert, 16) != ERR_OK) {
        DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Could not create alert queue");
        return;
    }

    /* Create PTP Tx Timestamp Semaphore */
    if(sys_sem_new(ptpClock.netPath.ptpTxNotify, 0) != ERR_OK) {
        DEBUG_MESSAGE(DEBUG_TYPE_INFO, "Could not create tx timestamp smphr!");
        return;
    }

    // Create the PTP daemon thread.
    sys_thread_new("ptpd", ptpd_thread, NULL, 2048, priority);
}

/**
 * @brief Notify PTP stack that a transmitted packet's timestamp is
 * ready to be received.
 * For non-DMA-capable Ethernet hardware, this semaphore can be called
 * immediately on sending of a timestamped packet. However, for DMA-capable
 * Ethernet hardware, this may need to be called by a TX interrupt.
 * This only needs to be called on packets where the pbuf's timestamp fields
 * are all 'ones' (UINT32_MAX), as this indicates that a timestamp is expected
 * for this packet.
 */
void lwipPtpTxNotify(void)
{
    sys_sem_signal(ptpClock.netPath.ptpTxNotify);
}

/*----------------------------------------------------------------------------*/

#else

void lwipPtpTimerExpired(u32_t idx) { UNUSED(idx); }

/* If LWIP_PTP is not defined map the init function to an empty function */
void lwipPtpInit(u8_t priority) { UNUSED(priority); }

/* If LWIP_PTP is not defined map the notify function to an empty function */
void lwipPtpTxNotify(void) {}

#endif /* LWIP_PTP || defined __DOXYGEN__ */
