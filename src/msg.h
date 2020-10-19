#ifndef __PTPD_MSG_H__
#define __PTPD_MSG_H__

#include "def/datatypes_private.h"

/**
 *\file
 * \brief 5.3 Message Handler
 */

/* Unpack header message */
void msgUnpackHeader(const octet_t *buf, msgHeader_t *header);

/* Pack header message */
void msgPackHeader(const ptpClock_t *ptpClock, octet_t *buf);

/* Pack Announce message */
void msgPackAnnounce(const ptpClock_t *ptpClock, octet_t *buf);

/* Unpack Announce message */
void msgUnpackAnnounce(const octet_t *buf, msgAnnounce_t *announce);

/* Pack SYNC message */
void msgPackSync(const ptpClock_t *ptpClock, octet_t *buf,
                                            const timestamp_t *originTimestamp);

/* Unpack Sync message */
void msgUnpackSync(const octet_t *buf, msgSync_t *sync);

/* Pack delayReq message */
void msgPackDelayReq(const ptpClock_t *ptpClock, octet_t *buf,
                                            const timestamp_t *originTimestamp);

/* Unpack delayReq message */
void msgUnpackDelayReq(const octet_t *buf, msgDelayReq_t *delayreq);

/* Pack Follow_up message */
void msgPackFollowUp(const ptpClock_t *ptpClock, octet_t *buf,
                                    const timestamp_t *preciseOriginTimestamp);

/* Unpack Follow_up message */
void msgUnpackFollowUp(const octet_t *buf, msgFollowUp_t *follow);

/* Pack delayResp message */
void msgPackDelayResp(const ptpClock_t *ptpClock, octet_t *buf,
                const msgHeader_t *header, const timestamp_t *receiveTimestamp);

/* Unpack delayResp message */
void msgUnpackDelayResp(const octet_t *buf, msgDelayResp_t *resp);

/* Pack PdelayReq message */
void msgPackPDelayReq(const ptpClock_t *ptpClock, octet_t *buf,
                                            const timestamp_t *originTimestamp);

/* Unpack PdelayReq message */
void msgUnpackPDelayReq(const octet_t *buf, msgDelayReq_t *pdelayreq);

/* Pack PdelayResp message */
void msgPackPDelayResp(octet_t *buf, const msgHeader_t *header,
                                    const timestamp_t *requestReceiptTimestamp);

/* Unpack PdelayResp message */
void msgUnpackPDelayResp(const octet_t *buf, msgPDelayResp_t *presp);

/* Pack PdelayRespfollowup message */
void msgPackPDelayRespFollowUp(octet_t *buf, const msgHeader_t *header,
                                    const timestamp_t *responseOriginTimestamp);

/* Unpack PdelayResp message */
void msgUnpackPDelayRespFollowUp(const octet_t *buf ,
                                        msgPDelayRespFollowUp_t *prespfollow);

#endif /* __PTPD_MSG_H__ */
