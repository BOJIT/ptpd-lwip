/**
 * @file
 * @brief ptpd-lwip public interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#ifndef __PTPD_LWIP_H__
#define __PTPD_LWIP_H__

#include "def/datatypes_public.h"

/*----------------------------- PUBLIC FUNCTIONS -----------------------------*/

void ptpdInit(ptpFunctions_t *ptpd_functions);

#endif /* __PTPD_LWIP_H__ */
