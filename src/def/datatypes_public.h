#ifndef __DATATYPES_PUBLIC__
#define __DATATYPES_PUBLIC__

#include <lwip/arch.h>
#include <lwip/sys.h>

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

/**
 * \brief Configuration struct that contains pointers to HAL functions
 */

typedef struct {
    void (*ptpGetTime)(timestamp_t*);
    void (*ptpSetTime)(const timestamp_t*);
    void (*ptpUpdateCoarse)(const timestamp_t*, s8_t);
    void (*ptpUpdateFine)(s32_t);
    sys_sem_t *ptpTxNotify;
} ptpFunctions_t;

#endif /* __DATATYPES_PUBLIC__ */
