#ifndef __DATATYPES_PUBLIC__
#define __DATATYPES_PUBLIC__

#include <lwip/arch.h>

/**
 *\file
 * \brief 5.3 Derived data type specifications
 *
 * This header file defines structures that are public to the user application
 */


// 48-bit unsigned integer (for PTP timestamps)
typedef struct {
    u32_t lsb;
    u16_t msb;
} u48_t;

// Standard PTP timestamp format.
typedef struct {
    u48_t tv_sec;
    u32_t tv_nsec;
} ptpTime_t;

typedef struct {
    void (*ptpGetTime)(ptpTime_t*);
    void (*ptpSetTime)(const ptpTime_t*);
    void (*ptpUpdateCoarse)(const ptpTime_t*, s8_t);
    void (*ptpUpdateFine)(s32_t);
    int ptp_priority;
} ptpFunctions_t;

#endif /* __DATATYPES_PUBLIC__ */
