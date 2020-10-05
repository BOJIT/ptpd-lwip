/**
 * @file
 * @brief ptpd-lwip public interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#ifndef __PTPD_LWIP__
#define __PTPD_LWIP__

#include <lwip/arch.h>

/*---------------------------- PUBLIC STRUCTURES -----------------------------*/

// 48-bit unsigned integer (for PTP timestamps)
typedef struct
{
    u32_t lsb;
    u16_t msb;
} u48_t;


typedef struct {
    u48_t tv_sec;
    u32_t tv_nsec;
} ptpTime_t;

typedef struct {
    void (*ptpGetTime)(ptpTime_t*);
    void (*ptpSetTime)(const ptpTime_t*);
    void (*ptpUpdateCoarse)(const ptpTime_t*, s8_t);
    void (*ptpUpdateFine)(s32_t);
} ptpConfig_t;

/*----------------------------- PUBLIC FUNCTIONS -----------------------------*/

void ptpdInit(ptpConfig_t ptpd_config);

#endif /* __PTPD_LWIP__ */
