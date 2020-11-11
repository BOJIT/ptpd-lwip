#ifndef __LWIP_PTP_OPT_H__
#define __LWIP_PTP_OPT_H__

/**
 *\file
 * \brief Default configuration options in LWIP_PTP
 *
 * This header file includes all default values used during initialization
 * and enumeration defined in the spec
 */

#define LWIP_PTP_CONTEXT // allows lwipopt.h options to be masked from LWIP_PTP.

/* Pull configuration data from lwipopts.h config file. */
#include "lwipopts.h"

#include <lwip/netif.h>

/*--------------------------- LWIP_PTP Configuration -------------------------*/


#if LWIP_PTP || defined __DOXYGEN__

    /**
     * LWIP_PTP_GET_TIME
     * @brief function used to get the system PTP time (absolute).
     */
    #if !defined LWIP_PTP_GET_TIME || defined __DOXYGEN__
        #error "No 'LWIP_PTP_GET_TIME' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_GET_TIME || defined __DOXYGEN__ */

    /**
     * LWIP_PTP_SET_TIME
     * @brief function used to set the system PTP time (absolute).
     */
    #if !defined LWIP_PTP_SET_TIME || defined __DOXYGEN__
        #error "No 'LWIP_PTP_SET_TIME' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_SET_TIME || defined __DOXYGEN__ */

    /**
     * LWIP_PTP_UPDATE_COARSE
     * @brief function used to update the system time by a time interval in a
     * single step (coarse correction mode)
     */
    #if !defined LWIP_PTP_UPDATE_COARSE || defined __DOXYGEN__
        #error "No 'LWIP_PTP_UPDATE_COARSE' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_UPDATE_COARSE || defined __DOXYGEN__ */

    /**
     * LWIP_PTP_UPDATE_FINE
     * @brief function used to update the system time by adjusting the frequency
     * of the system clock.
     */
    #if !defined LWIP_PTP_UPDATE_FINE || defined __DOXYGEN__
        #error "No 'LWIP_PTP_UPDATE_FINE' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_UPDATE_FINE || defined __DOXYGEN__ */

    /**
     * LWIP_PTP_INIT_TIMERS
     * @brief function used to initialise system timers for PTP timeouts.
     */
    #if !defined LWIP_PTP_INIT_TIMERS || defined __DOXYGEN__
        #error "No 'LWIP_PTP_INIT_TIMERS' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_INIT_TIMERS || defined __DOXYGEN__ */

    /**
     * LWIP_PTP_START_TIMER
     * @brief function used to start a timer with a specified timeout/callback.
     */
    #if !defined LWIP_PTP_START_TIMER || defined __DOXYGEN__
        #error "No 'LWIP_PTP_START_TIMER' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_START_TIMER || defined __DOXYGEN__ */

    /**
     * LWIP_PTP_STOP_TIMER
     * @brief function used to stop a timer. Note that the stack will only
     * let this function be called if the timer is active, so this does
     * not need to be checked by the user application.
     */
    #if !defined LWIP_PTP_STOP_TIMER || defined __DOXYGEN__
        #error "No 'LWIP_PTP_STOP_TIMER' function configured in lwipopts.h!"
    #endif /* !defined LWIP_PTP_STOP_TIMER || defined __DOXYGEN__ */

#endif /* LWIP_PTP || defined __DOXYGEN__ */


/*----------------------------- LWIP_PTP Constants ---------------------------*/

/* 5.3.4 ClockIdentity */
#define CLOCK_IDENTITY_LENGTH 8

#define LWIP_PTP_MANUFACTURER_ID \
        "PTPd;2.0.1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

/* Implementation specific constants */
#define DEFAULT_INBOUND_LATENCY         0       /* in nsec */
#define DEFAULT_OUTBOUND_LATENCY        0       /* in nsec */
#define DEFAULT_NO_RESET_CLOCK          false
#define DEFAULT_DOMAIN_NUMBER           0
#define DEFAULT_DELAY_MECHANISM         E2E
#define DEFAULT_AP                      2
#define DEFAULT_AI                      16
#define DEFAULT_DELAY_S                 6 /* exponencial smoothing - 2^s */
#define DEFAULT_OFFSET_S                1 /* exponencial smoothing - 2^s */
#define DEFAULT_ANNOUNCE_INTERVAL       1 /* 0 in 802.1AS */
#define DEFAULT_UTC_OFFSET              34
#define DEFAULT_UTC_VALID               false
#define DEFAULT_PDELAYREQ_INTERVAL      1 /* -4 in 802.1AS */
#define DEFAULT_DELAYREQ_INTERVAL       3 /* from DEFAULT_SYNC_INTERVAL to DEFAULT_SYNC_INTERVAL + 5 */
#define DEFAULT_SYNC_INTERVAL           0 /* -7 in 802.1AS */
#define DEFAULT_SYNC_RECEIPT_TIMEOUT    3
#define DEFAULT_ANNOUNCE_RECEIPT_TIMEOUT 6 /* 3 by default */
#define DEFAULT_QUALIFICATION_TIMEOUT   -9 /* DEFAULT_ANNOUNCE_INTERVAL + N */
#define DEFAULT_FOREIGN_MASTER_TIME_WINDOW 4
#define DEFAULT_FOREIGN_MASTER_THRESHOLD 2
#define DEFAULT_CLOCK_CLASS             248
#define DEFAULT_CLOCK_CLASS_SLAVE_ONLY  255
#define DEFAULT_CLOCK_ACCURACY          0xFE
#define DEFAULT_PRIORITY1               248
#define DEFAULT_PRIORITY2               248
#define DEFAULT_CLOCK_VARIANCE          5000 /* To be determined in 802.1AS */
#define DEFAULT_MAX_FOREIGN_RECORDS     5
#define DEFAULT_PARENTS_STATS           false
#define DEFAULT_TWO_STEP_FLAG           true /* Transmitting only SYNC message or SYNC and FOLLOW UP */
#define DEFAULT_TIME_SOURCE             INTERNAL_OSCILLATOR
#define DEFAULT_TIME_TRACEABLE          false /* time derived from atomic clock? */
#define DEFAULT_FREQUENCY_TRACEABLE     false /* frequency derived from frequency standard? */
#define DEFAULT_TIMESCALE               ARB_TIMESCALE /* PTP_TIMESCALE or ARB_TIMESCALE */

#define DEFAULT_CALIBRATED_OFFSET_NS    10000 /* offset from master < 10us -> calibrated */
#define DEFAULT_UNCALIBRATED_OFFSET_NS  1000000 /* offset from master > 1000us -> uncalibrated */
#define MAX_ADJ_OFFSET_NS       100000000 /* max offset to try to adjust it < 100ms */

/* features, only change to refelect changes in implementation */
#define NUMBER_PORTS      1
#define VERSION_PTP       2
#define BOUNDARY_CLOCK    false
#define SLAVE_ONLY        true
/// @todo check if master can be implemented easily?
#define NO_ADJUST         false


/** \name Packet length
 Minimal length values for each message.
 If TLV used length could be higher.*/
/**\{*/
#define HEADER_LENGTH                 34
#define ANNOUNCE_LENGTH               64
#define SYNC_LENGTH                   44
#define FOLLOW_UP_LENGTH              44
#define PDELAY_REQ_LENGTH             54
#define DELAY_REQ_LENGTH              44
#define DELAY_RESP_LENGTH             54
#define PDELAY_RESP_LENGTH            54
#define PDELAY_RESP_FOLLOW_UP_LENGTH  54
#define MANAGEMENT_LENGTH             48
/** \}*/

/* lwIP constants */
#define IF_NAMESIZE             2
#ifndef INET_ADDRSTRLEN
    #define INET_ADDRSTRLEN         16
#endif

#define IFACE_NAME_LENGTH         IF_NAMESIZE
#define NET_ADDRESS_LENGTH        INET_ADDRSTRLEN

#define IFCONF_LENGTH 10

#if BYTE_ORDER == LITTLE_ENDIAN
#define PTPD_LSBF
#elif BYTE_ORDER == BIG_ENDIAN
#define PTPD_MSBF
#endif

/* pow2ms(a) = round(pow(2,a)*1000) */

#define pow2ms(a) (((a)>0) ? (1000 << (a)) : (1000 >>(-(a))))

#define ADJ_FREQ_MAX  512000

/* UDP/IPv4 dependent */

#define SUBDOMAIN_ADDRESS_LENGTH  4
#define PORT_ADDRESS_LENGTH       2
#define PTP_UUID_LENGTH     NETIF_MAX_HWADDR_LEN
#define CLOCK_IDENTITY_LENGTH   8
#define FLAG_FIELD_LENGTH    2

#define PACKET_SIZE  300 /* ptpdv1 value kept because of use of TLV... */

#define PTP_EVENT_PORT    319
#define PTP_GENERAL_PORT  320

#define DEFAULT_PTP_DOMAIN_ADDRESS  htonl(0xe0000181) // 224.0.1.129
#define PEER_PTP_DOMAIN_ADDRESS     htonl(0xe000006b) // 224.0.0.107

#define MM_STARTING_BOUNDARY_HOPS  0x7fff

/* Must be a power of 2 */
#define PBUF_QUEUE_SIZE 4
#define PBUF_QUEUE_MASK (PBUF_QUEUE_SIZE - 1)

/* others */

#define SCREEN_BUFSZ  128
#define SCREEN_MAXSZ  80

/*---------------- Enumerations defined in tables of the spec ----------------*/

/**
 * \brief Domain Number (Table 2 in the spec)
 */
enum
{
    DFLT_DOMAIN_NUMBER = 0,
    ALT1_DOMAIN_NUMBER,
    ALT2_DOMAIN_NUMBER,
    ALT3_DOMAIN_NUMBER
};

/**
 * \brief Network Protocol  (Table 3 in the spec)
 */
enum
{
    UDP_IPV4 = 1,
    UDP_IPV6,
    IEE_802_3,
    DeviceNet,
    ControlNet,
    PROFINET
};

/**
 * \brief Time Source (Table 7 in the spec)
 */
enum
{
    ATOMIC_CLOCK = 0x10,
    GPS = 0x20,
    TERRESTRIAL_RADIO = 0x30,
    PTP = 0x40,
    NTP = 0x50,
    HAND_SET = 0x60,
    OTHER = 0x90,
    INTERNAL_OSCILLATOR = 0xA0
};


/**
 * \brief PTP State (Table 8 in the spec)
 */
enum
{
    PTP_INITIALIZING = 0,
    PTP_FAULTY,
    PTP_DISABLED,
    PTP_LISTENING,
    PTP_PRE_MASTER,
    PTP_MASTER,
    PTP_PASSIVE,
    PTP_UNCALIBRATED,
    PTP_SLAVE
};

/**
 * \brief Delay mechanism (Table 9 in the spec)
 */
enum
{
    E2E = 1,
    P2P = 2,
    DELAY_DISABLED = 0xFE
};

/**
 * \brief PTP timers
 */
enum
{
    PDELAYREQ_INTERVAL_TIMER = 0,/**<\brief Timer handling the PdelayReq Interval */
    DELAYREQ_INTERVAL_TIMER,/**<\brief Timer handling the delayReq Interva */
    SYNC_INTERVAL_TIMER,/**<\brief Timer handling Interval between master sends two Syncs messages */
    ANNOUNCE_RECEIPT_TIMER,/**<\brief Timer handling announce receipt timeout */
    ANNOUNCE_INTERVAL_TIMER, /**<\brief Timer handling interval before master sends two announce messages */
    QUALIFICATION_TIMEOUT
};

/**
 * \brief PTP Messages (Table 19)
 */
enum
{
    SYNC = 0x0,
    DELAY_REQ,
    PDELAY_REQ,
    PDELAY_RESP,
    FOLLOW_UP = 0x8,
    DELAY_RESP,
    PDELAY_RESP_FOLLOW_UP,
    ANNOUNCE,
    SIGNALING,
    MANAGEMENT,
};

/**
 * \brief PTP Messages control field (Table 23)
 */
enum
{
    CTRL_SYNC = 0x00,
    CTRL_DELAY_REQ,
    CTRL_FOLLOW_UP,
    CTRL_DELAY_RESP,
    CTRL_MANAGEMENT,
    CTRL_OTHER,
};

/**
 * \brief Output statistics
 */
enum
{
    PTP_NO_STATS = 0,
    PTP_TEXT_STATS,
    PTP_CSV_STATS /* not implemented */
};

/**
 * \brief message flags
 */
enum
{
    FLAG0_ALTERNATE_MASTER = 0x01,
    FLAG0_TWO_STEP = 0x02,
    FLAG0_UNICAST = 0x04,
    FLAG0_PTP_PROFILE_SPECIFIC_1 = 0x20,
    FLAG0_PTP_PROFILE_SPECIFIC_2 = 0x40,
    FLAG0_SECURITY = 0x80,
};

/**
 * \brief message flags
 */
 enum
{
    FLAG1_LEAP61 = 0x01,
    FLAG1_LEAP59 = 0x02,
    FLAG1_UTC_OFFSET_VALID = 0x04,
    FLAG1_PTP_TIMESCALE = 0x08,
    FLAG1_TIME_TRACEABLE = 0x10,
    FLAG1_FREQUENCY_TRACEABLE = 0x20,
};

/**
 * \brief ptp stack events
 */
enum
{
    POWERUP = 0x0001,
    INITIALIZE = 0x0002,
    DESIGNATED_ENABLED = 0x0004,
    DESIGNATED_DISABLED = 0x0008,
    FAULT_CLEARED = 0x0010,
    FAULT_DETECTED = 0x0020,
    STATE_DECISION_EVENT = 0x0040,
    QUALIFICATION_TIMEOUT_EXPIRES = 0x0080,
    ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES = 0x0100,
    SYNCHRONIZATION_FAULT = 0x0200,
    MASTER_CLOCK_SELECTED = 0x0400,
    /* non spec */
    MASTER_CLOCK_CHANGED = 0x0800,
};

/**
 * \brief ptp time scale
 */
enum
{
    ARB_TIMESCALE,
    PTP_TIMESCALE
};

/*--------------------------------- Debugging --------------------------------*/

#ifdef PTPD_DBG
#define PTPD_ERR
#define DBG(...)  { TimeInternal tmpTime; getTime(&tmpTime); printf("(D %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(__VA_ARGS__); }
#else
#define DBG(...)
#endif

#ifdef PTPD_DBGVV
#define PTPD_DBGV
#define PTPD_DBG
#define PTPD_ERR
#define DBGVV(...) printf("(V) " __VA_ARGS__)
#else
#define DBGVV(...)
#endif

#ifdef PTPD_DBGV
#define PTPD_DBG
#define PTPD_ERR
#define DBGV(...)  { TimeInternal tmpTime; getTime(&tmpTime); printf("(d %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(__VA_ARGS__); }
#else
#define DBGV(...)
#endif

#ifdef PTPD_ERR
#define ERROR(...)  { TimeInternal tmpTime; getTime(&tmpTime); printf("(E %d.%09d) ", tmpTime.seconds, tmpTime.nanoseconds); printf(__VA_ARGS__); }
/* #define ERROR(...)  { printf("(E) "); printf(__VA_ARGS__); } */
#else
#define ERROR(...)
#endif

/*---------------------------- Architecture Macros ---------------------------*/

/** @todo get endianness from LWIP to save it being defined separately */

/* Bit manipulation macros */
#define getFlag(flagField, mask) (bool)(((flagField)  & (mask)) == (mask))
#define setFlag(flagField, mask) (flagField) |= (mask)
#define clearFlag(flagField, mask) (flagField) &= ~(mask)

/* Endianness corrections */
#if defined(PTPD_MSBF)
#define shift8(x,y)   ( (x) << ((3-y)<<3) )
#define shift16(x,y)  ( (x) << ((1-y)<<4) )
#elif defined(PTPD_LSBF)
#define shift8(x,y)   ( (x) << ((y)<<3) )
#define shift16(x,y)  ( (x) << ((y)<<4) )
#endif

#define flip16(x) htons(x)
#define flip32(x) htonl(x)

/* Macro for intentionally suppressing unused variable warnings */
#define UNUSED(x) (void)(x)

/*----------------------------------------------------------------------------*/

#endif /* __LWIP_PTP_OPT_H__ */
