/**
 * @file
 * @brief arith.c
 * contains basic arithmetic operations that are used by ptpd-lwIP
 *
 * @author @htmlonly &copy; @endhtmlonly 2020 James Bennion-Pedley
 *
 * @date 1 Oct 2020
 */

#include "arith.h"

#if LWIP_PTP || defined __DOXYGEN__

/**
 * \brief Convert scaled nanoseconds into TimeInternal structure
 */
void scaledNanosecondsToInternalTime(const s64_t *scaledNanoseconds,
                                                    timeInternal_t *internal)
{
    int sign;
    s64_t nanoseconds = *scaledNanoseconds;

    /* Determine sign of result big integer number */
    if (nanoseconds < 0) {
        nanoseconds = -nanoseconds;
        sign = -1;
    }
    else {
        sign = 1;
    }

    /* fractional nanoseconds are excluded (see 5.3.2) */
    nanoseconds >>= 16;
    internal->seconds = sign * (nanoseconds / 1000000000);
    internal->nanoseconds = sign * (nanoseconds % 1000000000);
}

/**
 * \brief Convert TimeInternal into Timestamp structure (defined by the spec)
 */
void fromInternalTime(const timeInternal_t *internal, timestamp_t *external)
{
    /* fromInternalTime is only used to convert time given by the system to a timestamp
        * As a consequence, no negative value can normally be found in (internal)
        * Note that offsets are also represented with TimeInternal structure, and can be negative,
        * but offset are never convert into Timestamp so there is no problem here.*/
    if ((internal->seconds & ~INT_MAX) || (internal->nanoseconds & ~INT_MAX)) {
        DBG("Negative value canno't be converted into timestamp \n");
        return;
    }
    else {
        external->secondsField.lsb = internal->seconds;
        external->nanosecondsField = internal->nanoseconds;
        external->secondsField.msb = 0;
    }
}

/**
 * \brief Convert Timestamp to TimeInternal structure (defined by the spec)
 */
void toInternalTime(timeInternal_t *internal, const timestamp_t *external)
{
    /* Program will not run after 2038... */
    if (external->secondsField.lsb < INT_MAX) {
        internal->seconds = external->secondsField.lsb;
        internal->nanoseconds = external->nanosecondsField;
    }
    else {
        DBG("Clock servo canno't be executed : seconds field is higher than signed integer (32bits)\n");
        return;
    }
}

/**
 * \brief Fixes internal time where the number of nanoseconds is higher than
 * the number of nanoseconds in a second
 */
void normalizeTime(timeInternal_t *r)
{
    r->seconds += r->nanoseconds / 1000000000;
    r->nanoseconds -= r->nanoseconds / 1000000000 * 1000000000;

    if (r->seconds > 0 && r->nanoseconds < 0) {
        r->seconds -= 1;
        r->nanoseconds += 1000000000;
    }
    else if (r->seconds < 0 && r->nanoseconds > 0) {
        r->seconds += 1;
        r->nanoseconds -= 1000000000;
    }
}

/**
 * \brief Add two TimeInternal structure and normalize
 */
void addTime(timeInternal_t *r, const timeInternal_t *x, const timeInternal_t *y)
{
    r->seconds = x->seconds + y->seconds;
    r->nanoseconds = x->nanoseconds + y->nanoseconds;

    normalizeTime(r);
}

/**
 * \brief Substract two TimeInternal structure and normalize
 */
void subTime(timeInternal_t *r, const timeInternal_t *x, const timeInternal_t *y)
{
    r->seconds = x->seconds - y->seconds;
    r->nanoseconds = x->nanoseconds - y->nanoseconds;

    normalizeTime(r);
}

/**
 * \brief Divide the TimeInternal by 2 and normalize
 */
void div2Time(timeInternal_t *r)
{
    r->nanoseconds += r->seconds % 2 * 1000000000;
    r->seconds /= 2;
    r->nanoseconds /= 2;

    normalizeTime(r);
}

/**
 * \brief Returns the floor form of binary logarithm for a 32 bit integer.
 * -1 is returned if ''n'' is 0.
 */
s32_t floorLog2(u32_t n)
{
    int pos = 0;

    if (n == 0)
        return -1;

    if (n >= 1<<16) { n >>= 16; pos += 16; }
    if (n >= 1<< 8) { n >>=  8; pos +=  8; }
    if (n >= 1<< 4) { n >>=  4; pos +=  4; }
    if (n >= 1<< 2) { n >>=  2; pos +=  2; }
    if (n >= 1<< 1) {           pos +=  1; }
    return pos;
}

#endif /* LWIP_PTP || defined __DOXYGEN__ */
