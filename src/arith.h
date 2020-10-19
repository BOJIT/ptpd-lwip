/**
 * @file
 * @brief ptpd-lwip public interface
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#ifndef __PTPD_ARITH_H__
#define __PTPD_ARITH_H__


#include "def/datatypes_private.h"

/**
 * \brief Convert scaled nanoseconds into TimeInternal structure
 */
void scaledNanosecondsToInternalTime(const s64_t *scaledNanoseconds,
                                                    timeInternal_t *internal);
/**
 * \brief Convert TimeInternal into Timestamp structure (defined by the spec)
 */
void fromInternalTime(const timeInternal_t *internal, timestamp_t *external);

/**
 * \brief Convert Timestamp to TimeInternal structure (defined by the spec)
 */
void toInternalTime(timeInternal_t *internal, const timestamp_t *external);

/**
 * \brief Fixes internal time where the number of nanoseconds is higher than
 * the number of nanoseconds in a second
 */
void normalizeTime(timeInternal_t *r);

/**
 * \brief Add two TimeInternal structure and normalize
 */
void addTime(timeInternal_t *r, const timeInternal_t *x, const timeInternal_t *y);

/**
 * \brief Substract two TimeInternal structure and normalize
 */
void subTime(timeInternal_t *r, const timeInternal_t *x, const timeInternal_t *y);

/**
 * \brief Divide the TimeInternal by 2 and normalize
 */
void div2Time(timeInternal_t *r);

/**
 * \brief Returns the floor form of binary logarithm for a 32 bit integer.
 * -1 is returned if ''n'' is 0.
 */
s32_t floorLog2(u32_t n);

#endif /* __PTPD_ARITH_H__ */
