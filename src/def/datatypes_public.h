#ifndef __LWIP_PTP_DATATYPES_PUBLIC__
#define __LWIP_PTP_DATATYPES_PUBLIC__

/**
 *\file
 * \brief 5.3 Derived data type specifications
 *
 * This header file defines structures that are public to the user application
 */


/**
 * \brief 48-bit number format. The upper 16 bits are currently not handled
 */

typedef struct {
    u32_t lsb;
    u16_t msb;
} u48_t;

/**
 * \brief 5.3.3 The Timestamp type represents a positive time with respect to the epoch
 */

typedef struct {
    u48_t secondsField;
    u32_t nanosecondsField;
} timestamp_t;

#endif /* __LWIP_PTP_DATATYPES_PUBLIC__ */
