#ifndef __PTPD_DATATYPES_PRIVATE_H__
#define __PTPD_DATATYPES_PRIVATE_H__

/**
 *\file
 * \brief 5.3 Derived data type specifications
 *
 * This header file defines structures defined by the spec,
 * main program data structure, and all messages structures
 */

#include <stdbool.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>

#include "datatypes_public.h"
#include "constants.h"

/* -------------------------- Constructor Datatypes ------------------------- */

/**
 * \brief 4-bit data without numerical representation
 */
typedef unsigned char nibble_t;

/**
 * \brief 8-bit data without numerical representation
 */
typedef char octet_t;

/**
 * \brief 5.3.2 The TimeInterval type represents time intervals
 * in scaled nanoseconds where scaledNanoseconds = time[ns] * 2^16
 */

typedef struct {
    s64_t scaledNanoseconds;
} timeInterval_t;

/**
 * \brief Time structure to handle Linux time information
 */

typedef struct {
    s32_t seconds;
    s32_t nanoseconds;
} timeInternal_t;

/**
 * \brief 5.3.4 The ClockIdentity type identifies a clock
 */
typedef octet_t clockIdentity_t[CLOCK_IDENTITY_LENGTH];

/**
 * \brief 5.3.5 The PortIdentity identifies a PTP port.
 */

typedef struct {
    clockIdentity_t clockIdentity;
    s16_t portNumber;
} portIdentity_t;

/**
 * \brief 5.3.6 The PortAddress type represents the protocol address of a PTP port
 */

typedef struct {
    u16_t networkProtocol;
    s16_t adressLength;
    octet_t* adressField;
} portAddress_t;

/**
 * \brief 5.3.7 The ClockQuality represents the quality of a clock
 */

typedef struct {
    u8_t clockClass;
    u8_t clockAccuracy;
    s16_t offsetScaledLogVariance;
} clockQuality_t;

/**
 * \brief 5.3.8 The TLV type represents TLV extension fields
 */

typedef struct {
    u16_t tlvType;
    s16_t lengthField;
    octet_t* valueField;
} tlv_t;

/**
 * \brief 5.3.9 The PTPText data type is used to represent textual material in PTP messages
 * textField - UTF-8 encoding
 */

typedef struct {
    u8_t lengthField;
    octet_t* textField;
} ptpText_t;

/* --------------------------- Packet Definitions --------------------------- */

/**
 * \brief 5.3.10 The FaultRecord type is used to construct fault logs
 */

typedef struct {
    s16_t faultRecordLength;
    timestamp_t faultTime;
    u8_t severityCode;
    ptpText_t faultName;
    ptpText_t faultValue;
    ptpText_t faultDescription;
} faultRecord_t;


/**
 * \brief The common header for all PTP messages (Table 18 of the spec)
 */

typedef struct {
    nibble_t transportSpecific;
    nibble_t messageType;
    nibble_t versionPTP;
    s16_t messageLength;
    u8_t domainNumber;
    octet_t flagField[2];
    s64_t correctionfield;
    portIdentity_t sourcePortIdentity;
    s16_t sequenceId;
    u8_t controlField;
    s8_t logMessageInterval;
} msgHeader_t;


/**
 * \brief Announce message fields (Table 25 of the spec)
 */

typedef struct {
    timestamp_t originTimestamp;
    s16_t currentUtcOffset;
    u8_t grandmasterPriority1;
    clockQuality_t grandmasterClockQuality;
    u8_t grandmasterPriority2;
    clockIdentity_t grandmasterIdentity;
    s16_t stepsRemoved;
    u8_t timeSource;
} msgAnnounce_t;

/**
 * \brief Sync message fields (Table 26 of the spec)
 */

typedef struct {
    timestamp_t originTimestamp;
} msgSync_t;

/**
 * \brief DelayReq message fields (Table 26 of the spec)
 */

typedef struct {
    timestamp_t originTimestamp;
} msgDelayReq_t;

/**
 * \brief DelayResp message fields (Table 30 of the spec)
 */

typedef struct {
    timestamp_t receiveTimestamp;
    portIdentity_t requestingPortIdentity;
} msgDelayResp_t;

/**
 * \brief FollowUp message fields (Table 27 of the spec)
 */

typedef struct {
    timestamp_t preciseOriginTimestamp;
} msgFollowUp_t;

/**
 * \brief PDelayReq message fields (Table 29 of the spec)
 */

typedef struct {
    timestamp_t originTimestamp;
} msgPDelayReq_t;

/**
 * \brief PDelayResp message fields (Table 30 of the spec)
 */

typedef struct {
    timestamp_t requestReceiptTimestamp;
    portIdentity_t requestingPortIdentity;
} msgPDelayResp_t;

/**
 * \brief PDelayRespFollowUp message fields (Table 31 of the spec)
 */

typedef struct {
    timestamp_t responseOriginTimestamp;
    portIdentity_t requestingPortIdentity;
} msgPDelayRespFollowUp_t;

/**
 * \brief Signaling message fields (Table 33 of the spec)
 */

typedef struct {
    portIdentity_t targetPortIdentity;
    char* tlv;
} msgSignaling_t;

/**
 * \brief Management message fields (Table 37 of the spec)
 */

typedef struct {
    portIdentity_t targetPortIdentity;
    u8_t startingBoundaryHops;
    u8_t boundaryHops;
    nibble_t actionField;
    char* tlv;
} msgManagement_t;

/* ---------------------------- System Structures --------------------------- */

/**
 * \brief ForeignMasterRecord is used to manage foreign masters
 */

typedef struct {
    portIdentity_t foreignMasterPortIdentity;
    s16_t foreignMasterAnnounceMessages;

    /* This one is not in the spec */
    msgAnnounce_t announce;
    msgHeader_t header;
} foreignMasterRecord_t;

/**
 * \struct DefaultDS
 * \brief spec 8.2.1 default data set
 * spec 7.6.2, spec 7.6.3 PTP device attributes
 */

typedef struct {
    bool twoStepFlag;
    clockIdentity_t clockIdentity; /**< spec 7.6.2.1 */
    s16_t numberPorts;  /**< spec 7.6.2.7 */
    clockQuality_t clockQuality; /**< spec 7.6.2.4, 7.6.3.4 and 7.6.3 */
    u8_t priority1; /**< spec 7.6.2.2 */
    u8_t priority2; /**< spec 7.6.2.3 */
    u8_t domainNumber;
    bool  slaveOnly;
} defaultDS_t;


/**
 * \struct CurrentDS
 * \brief spec 8.2.2 current data set
 */

typedef struct {
    s16_t stepsRemoved;
    timeInternal_t offsetFromMaster;
    timeInternal_t meanPathDelay;
} currentDS_t;


/**
 * \struct ParentDS
 * \brief spec 8.2.3 parent data set
 */

typedef struct {
    portIdentity_t parentPortIdentity;
    /* 7.6.4 Parent clock statistics - parentDS */
    bool parentStats; /**< spec 7.6.4.2 */
    s16_t observedParentOffsetScaledLogVariance; /**< spec 7.6.4.3 */
    s32_t observedParentClockPhaseChangeRate; /**< spec 7.6.4.4 */

    clockIdentity_t grandmasterIdentity;
    clockQuality_t grandmasterClockQuality;
    u8_t grandmasterPriority1;
    u8_t grandmasterPriority2;
} parentDS_t;

/**
 * \struct TimePropertiesDS
 * \brief spec 8.2.4 time properties data set
 */

typedef struct {
    s16_t currentUtcOffset;
    bool currentUtcOffsetValid;
    bool leap59;
    bool leap61;
    bool timeTraceable;
    bool frequencyTraceable;
    bool ptpTimescale;
    u8_t timeSource; /**< spec 7.6.2.6 */
} timePropertiesDS_t;


/**
 * \struct PortDS
 * \brief spec 8.2.5 port data set
 */

typedef struct {
    portIdentity_t portIdentity;
    u8_t portState;
    s8_t logMinDelayReqInterval; /**< spec 7.7.2.4 */
    timeInternal_t peerMeanPathDelay;
    s8_t logAnnounceInterval; /**< spec 7.7.2.2 */
    u8_t announceReceiptTimeout; /**< spec 7.7.3.1 */
    s8_t logSyncInterval; /**< spec 7.7.2.3 */
    u8_t delayMechanism;
    s8_t logMinPdelayReqInterval; /**< spec 7.7.2.5 */
    nibble_t versionNumber;
} portDS_t;


/**
 * \struct ForeignMasterDS
 * \brief Foreign master data set
 */

typedef struct {
    foreignMasterRecord_t* records;

    /* Other things we need for the protocol */
    s16_t count;
    s16_t capacity;
    s16_t i;
    s16_t best;
} foreignMasterDS_t;

/**
 * \struct Servo
 * \brief Clock servo filters and PI regulator values
 */

typedef struct {
    bool noResetClock;
    bool noAdjust;
    s16_t ap, ai;
    s16_t sDelay;
    s16_t sOffset;
} servo_t;

/**
 * \struct Filter
 * \brief Offset filter for averaging one way delay
 */

typedef struct {
    s32_t y_prev;
    s32_t y_sum;
    s16_t s;
    s16_t s_prev;
    s32_t n;
} filter_t;

/**
 * \struct RunTimeOpts
 * \brief Program options set at run-time
 */

typedef struct {
    s8_t announceInterval;
    s8_t syncInterval;
    clockQuality_t clockQuality;
    u8_t priority1;
    u8_t priority2;
    u8_t domainNumber;
    bool slaveOnly;
    s16_t currentUtcOffset;
    octet_t ifaceName[IFACE_NAME_LENGTH];
    u8_t stats;
    octet_t unicastAddress[NET_ADDRESS_LENGTH];
    timeInternal_t inboundLatency, outboundLatency;
    s16_t maxForeignRecords;
    u8_t delayMechanism;
    servo_t servo;
} runTimeOpts_t;

/**
 * \struct Packet
 * \brief Stores pbuf pointer and associated
 * data that is required by the sending callback.
 */

typedef struct {
    struct pbuf *pbuf;
    ip_addr_t destAddr;
} packet_t;

/**
 * \struct PacketHandler
 * \brief Stores input and output queues associated with
 * a protocol control block.
 */

typedef struct {
    struct udp_pcb *pcb;
    packet_t inboxBuf[PBUF_QUEUE_SIZE];
    sys_mbox_t inbox;
    packet_t outboxBuf[PBUF_QUEUE_SIZE];
    sys_mbox_t outbox;
} packetHandler_t;

/**
 * \struct NetPath
 * \brief Network Interface Information
 */

typedef struct {
    ip_addr_t multicastAddr;
    ip_addr_t peerMulticastAddr;

    sys_sem_t *ptpTxNotify;

    packetHandler_t eventHandler;
    packetHandler_t generalHandler;
} netPath_t;

/**
 * \struct PtpClock
 * \brief Main program data structure
 */
/* main program data structure */

typedef struct {
    defaultDS_t defaultDS; /**< default data set */
    currentDS_t currentDS; /**< current data set */
    parentDS_t parentDS; /**< parent data set */
    timePropertiesDS_t timePropertiesDS; /**< time properties data set */
    portDS_t portDS; /**< port data set */
    foreignMasterDS_t foreignMasterDS; /**< foreign master data set */

    msgHeader_t msgTmpHeader; /**< buffer for incomming message header */

    union {
        msgSync_t sync;
        msgFollowUp_t follow;
        msgDelayReq_t req;
        msgDelayResp_t resp;
        msgPDelayReq_t preq;
        msgPDelayResp_t presp;
        msgPDelayRespFollowUp_t prespfollow;
        msgManagement_t manage;
        msgAnnounce_t announce;
        msgSignaling_t signaling;
    } msgTmp; /**< buffer for incomming message body */

    octet_t msgObuf[PACKET_SIZE]; /**< buffer for outgoing message */
    octet_t msgIbuf[PACKET_SIZE]; /** <buffer for incomming message - TO BE REMOVED AT SOME POINT */
    ssize_t msgIbufLength; /**< length of incomming message */

    timeInternal_t Tms; /**< Time Master -> Slave */
    timeInternal_t Tsm; /**< Time Slave -> Master */

    timeInternal_t pdelay_t1; /**< peer delay time t1 */
    timeInternal_t pdelay_t2; /**< peer delay time t2 */
    timeInternal_t pdelay_t3; /**< peer delay time t3 */
    timeInternal_t pdelay_t4; /**< peer delay time t4 */

    timeInternal_t timestamp_syncRecieve; /**< timestamp of Sync message */
    timeInternal_t timestamp_delayReqSend; /**< timestamp of delay request message */
    timeInternal_t timestamp_delayReqRecieve; /**< timestamp of delay request message */

    timeInternal_t correctionField_sync; /**< correction field of Sync and FollowUp messages */
    timeInternal_t correctionField_pDelayResp; /**< correction fieald of peedr delay response */

    s16_t sentPDelayReqSequenceId;
    s16_t sentDelayReqSequenceId;
    s16_t sentSyncSequenceId;
    s16_t sentAnnounceSequenceId;

    s16_t recvPDelayReqSequenceId;
    s16_t recvSyncSequenceId;

    bool waitingForFollowUp; /**< true if sync message was recieved and 2step flag is set */
    bool waitingForPDelayRespFollowUp; /**< true if PDelayResp message was recieved and 2step flag is set */

    filter_t ofm_filt; /**< filter offset from master */
    filter_t owd_filt; /**< filter one way delay */
    filter_t slv_filt; /**< filter scaled log variance */
    s16_t offsetHistory[2];
    s32_t observedDrift;

    bool messageActivity;

    u8_t recommendedState;

    octet_t portUuidField[PTP_UUID_LENGTH]; /**< Usefull to init network stuff */

    timeInternal_t inboundLatency, outboundLatency;

    servo_t servo;

    s32_t  events;

    u8_t  stats;

    netPath_t netPath;

    sys_mbox_t timerAlerts;
    sys_mbox_t packetAlerts;

    runTimeOpts_t* rtOpts;
} ptpClock_t;

#endif /* __PTPD_DATATYPES_PRIVATE_H__ */
