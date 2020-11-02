/* protocol.c */

#include "protocol.h"

#include "arith.h"
#include "bmc.h"
#include "msg.h"
#include "servo.h"
#include "timer.h"

/* Local (static) function declarations */

/* Check and handle received messages */
static void handle(ptpClock_t *ptpClock);

/* Handle the announce message - spec 9.5.3 */
static void handleAnnounce(ptpClock_t *ptpClock, bool isFromSelf);

/* Handle sync messages */
static void handleSync(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf);

/* Handle follow-up messages */
static void handleFollowUp(ptpClock_t *ptpClock, bool isFromSelf);

/* Handle delay requests */
static void handleDelayReq(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf);

/* Handle delay responses */
static void handleDelayResp(ptpClock_t *ptpClock, bool  isFromSelf);

/* Handle peer delay requests */
static void handlePDelayReq(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf);

/* Handle peer delay responses */
static void handlePDelayResp(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf);

/* Handle peer delay follow-up mesages */
static void handlePDelayRespFollowUp(ptpClock_t *ptpClock, bool isFromSelf);

/* Handle management messages (not implemented) */
static void handleManagement(ptpClock_t *ptpClock, bool isFromSelf);

/* Handle signalling messages (not implemented) */
static void handleSignaling(ptpClock_t *ptpClock, bool isFromSelf);

/* Issue delay requests when the timers have expired */
static void issueDelayReqTimerExpired(ptpClock_t *ptpClock);

/* Pack and send on general multicast ip adress an Announce message */
static void issueAnnounce(ptpClock_t *ptpClock);

/* Pack and send on event multicast ip adress a Sync message */
static void issueSync(ptpClock_t *ptpClock);

/* Pack and send on general multicast ip adress a FollowUp message */
static void issueFollowup(ptpClock_t *ptpClock, const timeInternal_t *time);

/* Pack and send on event multicast ip address a DelayReq message */
static void issueDelayReq(ptpClock_t *ptpClock);

/* Pack and send on event multicast ip adress a DelayResp message */
static void issueDelayResp(ptpClock_t *ptpClock, const timeInternal_t *time,
                                            const msgHeader_t *delayReqHeader);

/* Pack and send on event multicast ip adress a PDelayReq message */
static void issuePDelayReq(ptpClock_t *ptpClock);

/* Pack and send on event multicast ip adress a PDelayResp message */
static void issuePDelayResp(ptpClock_t *ptpClock, timeInternal_t *time,
                                            const msgHeader_t *pDelayReqHeader);

/* Pack and send on general multicast ip adress a PDelayRespFollowUp message */
static void issuePDelayRespFollowUp(ptpClock_t *ptpClock, const timeInternal_t *time,
                                                    const msgHeader_t *pDelayReqHeader);

//static void issueManagement(const MsgHeader*,MsgManagement*,PtpClock*);

/**
 * \brief Initialise services from scratch
 */
static bool doInit(ptpClock_t *ptpClock);

/* -------------------------------------------------------------------------- */

#ifdef PTPD_DBG
static char *stateString(u8_t state)
{
    switch (state) {
        case PTP_INITIALIZING: return (char *) "PTP_INITIALIZING";
        case PTP_FAULTY: return (char *) "PTP_FAULTY";
        case PTP_DISABLED: return (char *) "PTP_DISABLED";
        case PTP_LISTENING: return (char *) "PTP_LISTENING";
        case PTP_PRE_MASTER: return (char *) "PTP_PRE_MASTER";
        case PTP_MASTER: return (char *) "PTP_MASTER";
        case PTP_PASSIVE: return (char *) "PTP_PASSIVE";
        case PTP_UNCALIBRATED: return (char *) "PTP_UNCALIBRATED";
        case PTP_SLAVE: return (char *) "PTP_SLAVE";
        default: break;
    }
    return (char *) "UNKNOWN";
}
#endif

/**
 * \brief Change state of PTP stack
 * Perform actions required when leaving 'port_state' and entering 'state'
 */
void toState(ptpClock_t *ptpClock, u8_t state)
{
    ptpClock->messageActivity = true;

    DBG("leaving state %s\n", stateString(ptpClock->portDS.portState));

    /* leaving state tasks */
    switch (ptpClock->portDS.portState) {
        case PTP_MASTER:

            initClock(ptpClock);
            timerStop(SYNC_INTERVAL_TIMER);
            timerStop(ANNOUNCE_INTERVAL_TIMER);
            timerStop(PDELAYREQ_INTERVAL_TIMER);
            break;

        case PTP_UNCALIBRATED:
        case PTP_SLAVE:

            if (state == PTP_UNCALIBRATED || state == PTP_SLAVE) {
                break;
            }
            timerStop(ANNOUNCE_RECEIPT_TIMER);
            switch (ptpClock->portDS.delayMechanism) {
                case E2E:
                    timerStop(DELAYREQ_INTERVAL_TIMER);
                    break;
                case P2P:
                    timerStop(PDELAYREQ_INTERVAL_TIMER);
                    break;
                default:
                    /* none */
                    break;
            }
            initClock(ptpClock);

            break;

        case PTP_PASSIVE:

            initClock(ptpClock);
            timerStop(PDELAYREQ_INTERVAL_TIMER);
            timerStop(ANNOUNCE_RECEIPT_TIMER);
            break;

        case PTP_LISTENING:

            initClock(ptpClock);
            timerStop(ANNOUNCE_RECEIPT_TIMER);
            break;

        case PTP_PRE_MASTER:

            initClock(ptpClock);
            timerStop(QUALIFICATION_TIMEOUT);
            break;

        default:
            break;
    }

    DBG("entering state %s\n", stateString(state));

    /* Entering state tasks */
    switch (state) {
        case PTP_INITIALIZING:

            ptpClock->portDS.portState = PTP_INITIALIZING;
            ptpClock->recommendedState = PTP_INITIALIZING;
            break;

        case PTP_FAULTY:

            ptpClock->portDS.portState = PTP_FAULTY;
            break;

        case PTP_DISABLED:

            ptpClock->portDS.portState = PTP_DISABLED;
            break;

        case PTP_LISTENING:

            timerStart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->portDS.announceReceiptTimeout) * (pow2ms(ptpClock->portDS.logAnnounceInterval)));
            ptpClock->portDS.portState = PTP_LISTENING;
            ptpClock->recommendedState = PTP_LISTENING;
            break;

        case PTP_PRE_MASTER:

            /* If you implement not ordinary clock, you can manage this code */
            /* timerStart(QUALIFICATION_TIMEOUT, pow2ms(DEFAULT_QUALIFICATION_TIMEOUT));
            ptpClock->portDS.portState = PTP_PRE_MASTER;
            break;
            */

        case PTP_MASTER:

            ptpClock->portDS.logMinDelayReqInterval = DEFAULT_DELAYREQ_INTERVAL; /* it may change during slave state */
            timerStart(SYNC_INTERVAL_TIMER, pow2ms(ptpClock->portDS.logSyncInterval));
            DBG("SYNC INTERVAL TIMER : %d \n", pow2ms(ptpClock->portDS.logSyncInterval));
            timerStart(ANNOUNCE_INTERVAL_TIMER, pow2ms(ptpClock->portDS.logAnnounceInterval));

            switch (ptpClock->portDS.delayMechanism) {
                case E2E:
                    /* none */
                    break;
                case P2P:
                    timerStart(PDELAYREQ_INTERVAL_TIMER, getRand(pow2ms(ptpClock->portDS.logMinPdelayReqInterval) + 1));
                    break;
                default:
                    break;
            }

            ptpClock->portDS.portState = PTP_MASTER;

            break;

        case PTP_PASSIVE:

            timerStart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->portDS.announceReceiptTimeout)*(pow2ms(ptpClock->portDS.logAnnounceInterval)));
            if (ptpClock->portDS.delayMechanism == P2P) {
                timerStart(PDELAYREQ_INTERVAL_TIMER, getRand(pow2ms(ptpClock->portDS.logMinPdelayReqInterval + 1)));
            }
            ptpClock->portDS.portState = PTP_PASSIVE;

            break;

        case PTP_UNCALIBRATED:

            timerStart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->portDS.announceReceiptTimeout)*(pow2ms(ptpClock->portDS.logAnnounceInterval)));
            switch (ptpClock->portDS.delayMechanism) {
                case E2E:
                    timerStart(DELAYREQ_INTERVAL_TIMER, getRand(pow2ms(ptpClock->portDS.logMinDelayReqInterval + 1)));
                    break;
                case P2P:
                    timerStart(PDELAYREQ_INTERVAL_TIMER, getRand(pow2ms(ptpClock->portDS.logMinPdelayReqInterval + 1)));
                    break;
                default:
                    /* none */
                    break;
            }
            ptpClock->portDS.portState = PTP_UNCALIBRATED;

            break;

        case PTP_SLAVE:

            ptpClock->portDS.portState = PTP_SLAVE;

            break;

        default:

            break;
    }
}


/**
 * \brief Initialise services from scratch
 */
static bool doInit(ptpClock_t *ptpClock)
{
    DBG("manufacturerIdentity: %s\n", MANUFACTURER_ID);

    /* initialize networking */
    netShutdown(&ptpClock->netPath);

    if (!netInit(&ptpClock->netPath, ptpClock)) {
        ERROR("doInit: failed to initialize network\n");
        return false;
    }
    else {
        /* initialize other stuff */
        initData(ptpClock);
        initTimer();
        initClock(ptpClock);
        m1(ptpClock);
        msgPackHeader(ptpClock, ptpClock->msgObuf);
        return true;
    }
}

/**
 * \brief Run PTP stack in current state
 * Handle actions and events for 'port_state'
 */
void doState(ptpClock_t *ptpClock)
{
    ptpClock->messageActivity = false;

    switch (ptpClock->portDS.portState) {
        case PTP_LISTENING:
        case PTP_UNCALIBRATED:
        case PTP_SLAVE:
        case PTP_PRE_MASTER:
        case PTP_MASTER:
        case PTP_PASSIVE:

            /* State decision Event */
            if (getFlag(ptpClock->events, STATE_DECISION_EVENT)) {
                DBGV("event STATE_DECISION_EVENT\n");
                clearFlag(ptpClock->events, STATE_DECISION_EVENT);
                ptpClock->recommendedState = bmc(ptpClock);
                DBGV("recommending state %s\n", stateString(ptpClock->recommendedState));

                switch (ptpClock->recommendedState) {
                    case PTP_MASTER:
                    case PTP_PASSIVE:
                        if (ptpClock->defaultDS.slaveOnly || ptpClock->defaultDS.clockQuality.clockClass == 255) {
                            ptpClock->recommendedState = PTP_LISTENING;
                            DBGV("recommending state %s\n", stateString(ptpClock->recommendedState));
                        }
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }

    switch (ptpClock->recommendedState) {
        case PTP_MASTER:
            switch (ptpClock->portDS.portState) {
                case PTP_PRE_MASTER:
                    if (timerExpired(QUALIFICATION_TIMEOUT)) toState(ptpClock, PTP_MASTER);
                    break;
                case PTP_MASTER:
                    break;
                default:
                    toState(ptpClock, PTP_PRE_MASTER);
                    break;
            }
            break;

        case PTP_PASSIVE:
            if (ptpClock->portDS.portState != ptpClock->recommendedState) toState(ptpClock, PTP_PASSIVE);
            break;

        case PTP_SLAVE:

            switch (ptpClock->portDS.portState) {
                case PTP_UNCALIBRATED:

                    if (getFlag(ptpClock->events, MASTER_CLOCK_SELECTED)) {
                        DBG("event MASTER_CLOCK_SELECTED\n");
                        clearFlag(ptpClock->events, MASTER_CLOCK_SELECTED);
                        toState(ptpClock, PTP_SLAVE);
                    }

                    if (getFlag(ptpClock->events, MASTER_CLOCK_CHANGED)) {
                        DBG("event MASTER_CLOCK_CHANGED\n");
                        clearFlag(ptpClock->events, MASTER_CLOCK_CHANGED);
                    }

                    break;

                case PTP_SLAVE:

                    if (getFlag(ptpClock->events, SYNCHRONIZATION_FAULT)) {
                        DBG("event SYNCHRONIZATION_FAULT\n");
                        clearFlag(ptpClock->events, SYNCHRONIZATION_FAULT);
                        toState(ptpClock, PTP_UNCALIBRATED);
                    }

                    if (getFlag(ptpClock->events, MASTER_CLOCK_CHANGED)) {
                        DBG("event MASTER_CLOCK_CHANGED\n");
                        clearFlag(ptpClock->events, MASTER_CLOCK_CHANGED);
                        toState(ptpClock, PTP_UNCALIBRATED);
                    }

                    break;

                default:

                    toState(ptpClock, PTP_UNCALIBRATED);
                    break;
            }

            break;

        case PTP_LISTENING:

            if (ptpClock->portDS.portState != ptpClock->recommendedState) {
                toState(ptpClock, PTP_LISTENING);
            }

            break;

        case PTP_INITIALIZING:
            break;

        default:
            DBG("doState: unrecognized recommended state %d\n", ptpClock->recommendedState);
            break;
    }

    switch (ptpClock->portDS.portState) {
        case PTP_INITIALIZING:

            if (doInit(ptpClock) == true) {
                toState(ptpClock, PTP_LISTENING);
            }
            else {
                toState(ptpClock, PTP_FAULTY);
            }

            break;

        case PTP_FAULTY:

            /* Imaginary troubleshooting */
            DBG("event FAULT_CLEARED for state PTP_FAULT\n");
            toState(ptpClock, PTP_INITIALIZING);
            return;

        case PTP_DISABLED:
            handle(ptpClock);
            break;

        case PTP_LISTENING:
        case PTP_UNCALIBRATED:
        case PTP_SLAVE:
        case PTP_PASSIVE:

            if (timerExpired(ANNOUNCE_RECEIPT_TIMER)) {
                DBGV("event ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES for state %s\n", stateString(ptpClock->portDS.portState));
                ptpClock->foreignMasterDS.count = 0;
                ptpClock->foreignMasterDS.i = 0;

                if (!(ptpClock->defaultDS.slaveOnly || ptpClock->defaultDS.clockQuality.clockClass == 255)) {
                    m1(ptpClock);
                    ptpClock->recommendedState = PTP_MASTER;
                    DBGV("recommending state %s\n", stateString(ptpClock->recommendedState));
                    toState(ptpClock, PTP_MASTER);
                }
                else if (ptpClock->portDS.portState != PTP_LISTENING) {
                    toState(ptpClock, PTP_LISTENING);
                }

                break;
            }

            handle(ptpClock);

            break;

        case PTP_MASTER:

            if (timerExpired(SYNC_INTERVAL_TIMER)) {
                DBGV("event SYNC_INTERVAL_TIMEOUT_EXPIRES for state PTP_MASTER\n");
                issueSync(ptpClock);
            }

            if (timerExpired(ANNOUNCE_INTERVAL_TIMER)) {
                DBGV("event ANNOUNCE_INTERVAL_TIMEOUT_EXPIRES for state PTP_MASTER\n");
                issueAnnounce(ptpClock);
            }

            handle(ptpClock);
            issueDelayReqTimerExpired(ptpClock);

            break;

        default:
            DBG("doState: do unrecognized state %d\n", ptpClock->portDS.portState);
            break;
    }
}


/* Check and handle received messages */
static void handle(ptpClock_t *ptpClock)
{
    int ret;
    bool isFromSelf;
    timeInternal_t time = { 0, 0};

    // if (false == ptpClock->messageActivity) {
    //         /// @todo network code!
    //         ret = netSelect(&ptpClock->netPath, 0);

    //         if (ret < 0) {
    //             ERROR("handle: failed to poll sockets\n");
    //             toState(ptpClock, PTP_FAULTY);
    //             return;
    //         }
    //         else if (!ret) {
    //             DBGVV("handle: nothing\n");
    //             return;
    //         }
    // }

    /// @todo the above code prevents the network events being polled if definitely empty - may help speed

    DBGVV("handle: something\n");

    /* Receive an event. */
    /// @todo network code!
    ptpClock->msgIbufLength = netRecvEvent(&ptpClock->netPath, ptpClock->msgIbuf, &time);
    /* local time is not UTC, we can calculate UTC on demand, otherwise UTC time is not used */
    /* time.seconds += ptpClock->timePropertiesDS.currentUtcOffset; */
    DBGV("handle: netRecvEvent returned %d\n", ptpClock->msgIbufLength);

    if (ptpClock->msgIbufLength < 0) {
        ERROR("handle: failed to receive on the event socket\n");
        toState(ptpClock, PTP_FAULTY);
        return;
    }
    else if (!ptpClock->msgIbufLength) {
        /* Receive a general packet. */
        /// @todo network code!
        ptpClock->msgIbufLength = netRecvGeneral(&ptpClock->netPath, ptpClock->msgIbuf, &time);
        DBGV("handle: netRecvGeneral returned %d\n", ptpClock->msgIbufLength);

        if (ptpClock->msgIbufLength < 0) {
            ERROR("handle: failed to receive on the general socket\n");
            toState(ptpClock, PTP_FAULTY);
            return;
        }
        else if (!ptpClock->msgIbufLength)
            return; // No packet present
    }

    ptpClock->messageActivity = true;

    if (ptpClock->msgIbufLength < HEADER_LENGTH) {
        ERROR("handle: message shorter than header length\n");
        toState(ptpClock, PTP_FAULTY);
        return;
    }

    msgUnpackHeader(ptpClock->msgIbuf, &ptpClock->msgTmpHeader);
    DBGV("handle: unpacked message type %d\n", ptpClock->msgTmpHeader.messageType);

    if (ptpClock->msgTmpHeader.versionPTP != ptpClock->portDS.versionNumber) {
        DBGV("handle: ignore version %d message\n", ptpClock->msgTmpHeader.versionPTP);
        return;
    }

    if (ptpClock->msgTmpHeader.domainNumber != ptpClock->defaultDS.domainNumber) {
        DBGV("handle: ignore message from domainNumber %d\n", ptpClock->msgTmpHeader.domainNumber);
        return;
    }

    /* Spec 9.5.2.2 */
    isFromSelf = isSamePortIdentity(
    &ptpClock->portDS.portIdentity,
    &ptpClock->msgTmpHeader.sourcePortIdentity);

    /* Subtract the inbound latency adjustment if it is not a loop back and the
            time stamp seems reasonable */
    if (!isFromSelf && time.seconds > 0)
        subTime(&time, &time, &ptpClock->inboundLatency);

    switch (ptpClock->msgTmpHeader.messageType) {

        case ANNOUNCE:
            handleAnnounce(ptpClock, isFromSelf);
            break;

        case SYNC:
            handleSync(ptpClock, &time, isFromSelf);
            break;

        case FOLLOW_UP:
            handleFollowUp(ptpClock, isFromSelf);
            break;

        case DELAY_REQ:
            handleDelayReq(ptpClock, &time, isFromSelf);
            break;

        case PDELAY_REQ:
            handlePDelayReq(ptpClock, &time, isFromSelf);
            break;

        case DELAY_RESP:
            handleDelayResp(ptpClock, isFromSelf);
            break;

        case PDELAY_RESP:
            handlePDelayResp(ptpClock, &time, isFromSelf);
            break;

        case PDELAY_RESP_FOLLOW_UP:
            handlePDelayRespFollowUp(ptpClock, isFromSelf);
            break;

        case MANAGEMENT:
            handleManagement(ptpClock, isFromSelf);
            break;

        case SIGNALING:
            handleSignaling(ptpClock, isFromSelf);
            break;

        default:
            DBG("handle: unrecognized message %d\n", ptpClock->msgTmpHeader.messageType);
            break;
    }
}

/* Handle announce messages - spec 9.5.3 */
static void handleAnnounce(ptpClock_t *ptpClock, bool isFromSelf)
{
    bool isFromCurrentParent = false;

    DBGV("handleAnnounce: received in state %s\n", stateString(ptpClock->portDS.portState));

    if (ptpClock->msgIbufLength < ANNOUNCE_LENGTH) {
        ERROR("handleAnnounce: short message\n");
        toState(ptpClock, PTP_FAULTY);
        return;
    }

    if (isFromSelf) {
        DBGV("handleAnnounce: ignore from self\n");
        return;
    }

    switch (ptpClock->portDS.portState) {
        case PTP_INITIALIZING:
        case PTP_FAULTY:
        case PTP_DISABLED:

            DBGV("handleAnnounce: disreguard\n");
            break;

        case PTP_UNCALIBRATED:
        case PTP_SLAVE:

            /* Valid announce message is received : BMC algorithm will be executed */
            setFlag(ptpClock->events, STATE_DECISION_EVENT);
            isFromCurrentParent = isSamePortIdentity(
            &ptpClock->parentDS.parentPortIdentity,
            &ptpClock->msgTmpHeader.sourcePortIdentity);
            msgUnpackAnnounce(ptpClock->msgIbuf, &ptpClock->msgTmp.announce);
            if (isFromCurrentParent) {
                    s1(ptpClock, &ptpClock->msgTmpHeader, &ptpClock->msgTmp.announce);
                    /* Reset  Timer handling Announce receipt timeout */
                    timerStart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->portDS.announceReceiptTimeout) * (pow2ms(ptpClock->portDS.logAnnounceInterval)));
            }
            else {
                DBGV("handleAnnounce: from another foreign master\n");
                /* addForeign takes care  of AnnounceUnpacking */
                addForeign(ptpClock, &ptpClock->msgTmpHeader, &ptpClock->msgTmp.announce);
            }

            break;

        case PTP_PASSIVE:
            timerStart(ANNOUNCE_RECEIPT_TIMER, (ptpClock->portDS.announceReceiptTimeout)*(pow2ms(ptpClock->portDS.logAnnounceInterval)));
        case PTP_MASTER:
        case PTP_PRE_MASTER:
        case PTP_LISTENING:
        default :

            DBGV("handleAnnounce: from another foreign master\n");
            msgUnpackAnnounce(ptpClock->msgIbuf, &ptpClock->msgTmp.announce);

            /* Valid announce message is received : BMC algorithm will be executed */
            setFlag(ptpClock->events, STATE_DECISION_EVENT);
            addForeign(ptpClock, &ptpClock->msgTmpHeader, &ptpClock->msgTmp.announce);

            break;
    }
}

/* Handle sync messages */
static void handleSync(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf)
{
    timeInternal_t originTimestamp;
    timeInternal_t correctionField;
    bool isFromCurrentParent = false;

    DBGV("handleSync: received in state %s\n", stateString(ptpClock->portDS.portState));

    if (ptpClock->msgIbufLength < SYNC_LENGTH) {
        ERROR("handleSync: short message\n");
        toState(ptpClock, PTP_FAULTY);
        return;
    }

    switch (ptpClock->portDS.portState) {
        case PTP_INITIALIZING:
        case PTP_FAULTY:
        case PTP_DISABLED:

            DBGV("handleSync: disreguard\n");
            break;

        case PTP_UNCALIBRATED:
        case PTP_SLAVE:

            if (isFromSelf) {
                DBGV("handleSync: ignore from self\n");
                break;
            }

            isFromCurrentParent = isSamePortIdentity(
            &ptpClock->parentDS.parentPortIdentity,
            &ptpClock->msgTmpHeader.sourcePortIdentity);

            if (!isFromCurrentParent) {
                DBGV("handleSync: ignore from another master\n");
                break;
            }

            ptpClock->timestamp_syncRecieve = *time;
            scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionfield, &correctionField);

            if (getFlag(ptpClock->msgTmpHeader.flagField[0], FLAG0_TWO_STEP)) {
                ptpClock->waitingForFollowUp = true;
                ptpClock->recvSyncSequenceId = ptpClock->msgTmpHeader.sequenceId;
                /* Save correctionField of Sync message for future use */
                ptpClock->correctionField_sync = correctionField;
            }
            else {
                msgUnpackSync(ptpClock->msgIbuf, &ptpClock->msgTmp.sync);
                ptpClock->waitingForFollowUp = false;
                /* Synchronize  local clock */
                toInternalTime(&originTimestamp, &ptpClock->msgTmp.sync.originTimestamp);
                /* use correctionField of Sync message for future use */
                updateOffset(ptpClock, &ptpClock->timestamp_syncRecieve, &originTimestamp, &correctionField);
                updateClock(ptpClock);
                issueDelayReqTimerExpired(ptpClock);
            }

            break;

        case PTP_MASTER:

            if (!isFromSelf) {
                DBGV("handleSync: from another master\n");
                break;
            }
            else {
                DBGV("handleSync: ignore from self\n");
                break;
            }

    //      if waitingForLoopback && TWO_STEP_FLAG
    //        {
    //            /* Add  latency */
    //            addTime(time, time, &rtOpts->outboundLatency);
    //
    //            issueFollowup(ptpClock, time);
    //            break;
    //        }

        case PTP_PASSIVE:

            DBGV("handleSync: disreguard\n");
            issueDelayReqTimerExpired(ptpClock);

            break;

        default:

            DBGV("handleSync: disreguard\n");
            break;
    }
}

/* Handle follow-up messages */
static void handleFollowUp(ptpClock_t *ptpClock, bool isFromSelf)
{
    timeInternal_t preciseOriginTimestamp;
    timeInternal_t correctionField;
    bool isFromCurrentParent = false;

    DBGV("handleFollowup: received in state %s\n", stateString(ptpClock->portDS.portState));

    if (ptpClock->msgIbufLength < FOLLOW_UP_LENGTH) {
        ERROR("handleFollowup: short message\n");
        toState(ptpClock, PTP_FAULTY);
        return;
    }

    if (isFromSelf) {
        DBGV("handleFollowup: ignore from self\n");
        return;
    }

    switch (ptpClock->portDS.portState) {
        case PTP_INITIALIZING:
        case PTP_FAULTY:
        case PTP_DISABLED:
        case PTP_LISTENING:

            DBGV("handleFollowup: disreguard\n");
            break;

        case PTP_UNCALIBRATED:
        case PTP_SLAVE:

            isFromCurrentParent = isSamePortIdentity(
            &ptpClock->parentDS.parentPortIdentity,
            &ptpClock->msgTmpHeader.sourcePortIdentity);

            if (!ptpClock->waitingForFollowUp) {
                DBGV("handleFollowup: not waiting a message\n");
                break;
            }

            if (!isFromCurrentParent) {
                DBGV("handleFollowup: not from current parent\n");
                break;
            }

            if (ptpClock->recvSyncSequenceId !=  ptpClock->msgTmpHeader.sequenceId) {
                DBGV("handleFollowup: SequenceID doesn't match with last Sync message\n");
                break;
            }

            msgUnpackFollowUp(ptpClock->msgIbuf, &ptpClock->msgTmp.follow);

            ptpClock->waitingForFollowUp = false;
            /* synchronize local clock */
            toInternalTime(&preciseOriginTimestamp, &ptpClock->msgTmp.follow.preciseOriginTimestamp);
            scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionfield, &correctionField);
            addTime(&correctionField, &correctionField, &ptpClock->correctionField_sync);
            updateOffset(ptpClock, &ptpClock->timestamp_syncRecieve, &preciseOriginTimestamp, &correctionField);
            updateClock(ptpClock);

            issueDelayReqTimerExpired(ptpClock);
            break;

        case PTP_MASTER:

            DBGV("handleFollowup: from another master\n");
            break;

        case PTP_PASSIVE:

            DBGV("handleFollowup: disreguard\n");
            issueDelayReqTimerExpired(ptpClock);
            break;

        default:

            DBG("handleFollowup: unrecognized state\n");
            break;
    }
}

/* Handle delay requests */
static void handleDelayReq(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf)
{
    switch (ptpClock->portDS.delayMechanism) {
        case E2E:

            DBGV("handleDelayReq: received in mode E2E in state %s\n", stateString(ptpClock->portDS.portState));
            if (ptpClock->msgIbufLength < DELAY_REQ_LENGTH) {
                ERROR("handleDelayReq: short message\n");
                toState(ptpClock, PTP_FAULTY);
                return;
            }

            switch (ptpClock->portDS.portState) {
                case PTP_INITIALIZING:
                case PTP_FAULTY:
                case PTP_DISABLED:
                case PTP_UNCALIBRATED:
                case PTP_LISTENING:
                    DBGV("handleDelayReq: disreguard\n");
                    return;

                case PTP_SLAVE:
                    DBGV("handleDelayReq: disreguard\n");
    //            if (isFromSelf)
    //            {
    //    /* waitingForLoopback? */
    //                /* Get sending timestamp from IP stack with So_TIMESTAMP */
    //                ptpClock->delay_req_send_time = *time;

    //                /* Add  latency */
    //                addTime(&ptpClock->delay_req_send_time, &ptpClock->delay_req_send_time, &rtOpts->outboundLatency);
    //                break;
    //            }
                    break;

                case PTP_MASTER:
                    /* TODO: manage the value of ptpClock->logMinDelayReqInterval form logSyncInterval to logSyncInterval + 5 */
                    issueDelayResp(ptpClock, time, &ptpClock->msgTmpHeader);
                    break;

                default:
                    DBG("handleDelayReq: unrecognized state\n");
                    break;
            }

            break;

        case P2P:

            ERROR("handleDelayReq: disreguard in P2P mode\n");
            break;

        default:

            /* none */
            break;
    }
}

/* Handle delay responses */
static void handleDelayResp(ptpClock_t *ptpClock, bool  isFromSelf)
{
    bool isFromCurrentParent = false;
    bool isCurrentRequest = false;
    timeInternal_t correctionField;

    switch (ptpClock->portDS.delayMechanism)
    {
        case E2E:

            DBGV("handleDelayResp: received in mode E2E in state %s\n", stateString(ptpClock->portDS.portState));
            if (ptpClock->msgIbufLength < DELAY_RESP_LENGTH) {
                ERROR("handleDelayResp: short message\n");
                toState(ptpClock, PTP_FAULTY);
                return;
            }

            switch (ptpClock->portDS.portState) {
                case PTP_INITIALIZING:
                case PTP_FAULTY:
                case PTP_DISABLED:
                case PTP_LISTENING:
                    DBGV("handleDelayResp: disreguard\n");
                    return;

                case PTP_UNCALIBRATED:
                case PTP_SLAVE:

                    msgUnpackDelayResp(ptpClock->msgIbuf, &ptpClock->msgTmp.resp);

                    isFromCurrentParent = isSamePortIdentity(
                    &ptpClock->parentDS.parentPortIdentity,
                    &ptpClock->msgTmpHeader.sourcePortIdentity);

                    isCurrentRequest = isSamePortIdentity(
                    &ptpClock->portDS.portIdentity,
                    &ptpClock->msgTmp.resp.requestingPortIdentity);

                    if (((ptpClock->sentDelayReqSequenceId - 1) == ptpClock->msgTmpHeader.sequenceId) && isCurrentRequest && isFromCurrentParent) {
                        /* TODO: revisit 11.3 */
                        toInternalTime(&ptpClock->timestamp_delayReqRecieve, &ptpClock->msgTmp.resp.receiveTimestamp);

                        scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionfield, &correctionField);
                        updateDelay(ptpClock, &ptpClock->timestamp_delayReqSend, &ptpClock->timestamp_delayReqRecieve, &correctionField);

                        ptpClock->portDS.logMinDelayReqInterval = ptpClock->msgTmpHeader.logMessageInterval;
                    }
                    else {
                        DBGV("handleDelayResp: doesn't match with the delayReq\n");
                        break;
                    }
            }
            break;

        case P2P:

            ERROR("handleDelayResp: disreguard in P2P mode\n");
            break;

        default:

            break;
    }
}

/* Handle peer delay requests */
static void handlePDelayReq(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf)
{
    switch (ptpClock->portDS.delayMechanism) {
        case E2E:
            ERROR("handlePDelayReq: disreguard in E2E mode\n");
            break;

        case P2P:

            DBGV("handlePDelayReq: received in mode P2P in state %s\n", stateString(ptpClock->portDS.portState));
            if (ptpClock->msgIbufLength < PDELAY_REQ_LENGTH) {
                ERROR("handlePDelayReq: short message\n");
                toState(ptpClock, PTP_FAULTY);
                return;
            }

            switch (ptpClock->portDS.portState) {
                case PTP_INITIALIZING:
                case PTP_FAULTY:
                case PTP_DISABLED:
                case PTP_UNCALIBRATED:
                case PTP_LISTENING:
                    DBGV("handlePDelayReq: disreguard\n");
                    return;

                case PTP_PASSIVE:
                case PTP_SLAVE:
                case PTP_MASTER:

                    if (isFromSelf) {
                            DBGV("handlePDelayReq: ignore from self\n");
                            break;
                    }

    //            if (isFromSelf) /* && loopback mode */
    //            {
    //                /* Get sending timestamp from IP stack with So_TIMESTAMP */
    //                ptpClock->pdelay_req_send_time = *time;
    //
    //                /* Add  latency */
    //                addTime(&ptpClock->pdelay_req_send_time, &ptpClock->pdelay_req_send_time, &rtOpts->outboundLatency);
    //                break;
    //            }
    //            else
    //            {
                        //ptpClock->PdelayReqHeader = ptpClock->msgTmpHeader;

                    issuePDelayResp(ptpClock, time, &ptpClock->msgTmpHeader);

                    if ((time->seconds != 0) && getFlag(ptpClock->msgTmpHeader.flagField[0], FLAG0_TWO_STEP)) /* not loopback mode */ {
                            issuePDelayRespFollowUp(ptpClock, time, &ptpClock->msgTmpHeader);
                    }

                    break;

    //            }

                default:

                    DBG("handlePDelayReq: unrecognized state\n");
                    break;
            }
            break;

        default:

            break;
    }
}

/* Handle peer delay responses */
static void handlePDelayResp(ptpClock_t *ptpClock, timeInternal_t *time, bool isFromSelf)
{
    timeInternal_t requestReceiptTimestamp;
    timeInternal_t correctionField;
    bool isCurrentRequest;

    switch (ptpClock->portDS.delayMechanism) {
        case E2E:

            ERROR("handlePDelayResp: disreguard in E2E mode\n");
            break;

        case P2P:

            DBGV("handlePDelayResp: received in mode P2P in state %s\n", stateString(ptpClock->portDS.portState));
            if (ptpClock->msgIbufLength < PDELAY_RESP_LENGTH) {
                ERROR("handlePDelayResp: short message\n");
                toState(ptpClock, PTP_FAULTY);
                return;
            }

            switch (ptpClock->portDS.portState) {
                case PTP_INITIALIZING:
                case PTP_FAULTY:
                case PTP_DISABLED:
                case PTP_UNCALIBRATED:
                case PTP_LISTENING:

                DBGV("handlePDelayResp: disreguard\n");
                return;

                case PTP_MASTER:
                case PTP_SLAVE:

                    if (isFromSelf) {
                        DBGV("handlePDelayResp: ignore from self\n");
                        break;
                    }

    //            if (isFromSelf)  && loopback mode
    //            {
    //                addTime(time, time, &rtOpts->outboundLatency);
    //                issuePDelayRespFollowUp(time, ptpClock);
    //                break;
    //            }


                    msgUnpackPDelayResp(ptpClock->msgIbuf, &ptpClock->msgTmp.presp);

                    isCurrentRequest = isSamePortIdentity(
                    &ptpClock->portDS.portIdentity,
                    &ptpClock->msgTmp.presp.requestingPortIdentity);

                    if (((ptpClock->sentPDelayReqSequenceId - 1) == ptpClock->msgTmpHeader.sequenceId) && isCurrentRequest) {
                        if (getFlag(ptpClock->msgTmpHeader.flagField[0], FLAG0_TWO_STEP)) {
                            ptpClock->waitingForPDelayRespFollowUp = true;

                            /* Store  t4 (Fig 35)*/
                            ptpClock->pdelay_t4 = *time;

                            /* store  t2 (Fig 35)*/
                            toInternalTime(&requestReceiptTimestamp, &ptpClock->msgTmp.presp.requestReceiptTimestamp);
                            ptpClock->pdelay_t2 = requestReceiptTimestamp;

                            scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionfield, &correctionField);
                            ptpClock->correctionField_pDelayResp = correctionField;
                        } //Two Step Clock
                        else { // One Step Clock
                            ptpClock->waitingForPDelayRespFollowUp = false;

                            /* Store  t4 (Fig 35)*/
                            ptpClock->pdelay_t4 = *time;

                            scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionfield, &correctionField);
                            updatePeerDelay(ptpClock, &correctionField, false);
                        }
                    }
                    else {
                            DBGV("handlePDelayResp: PDelayResp doesn't match with the PDelayReq.\n");
                    }

                    break;

                default:

                    DBG("handlePDelayResp: unrecognized state\n");
                    break;
            }
            break;

        default:

            break;
    }
}

/* Handle peer delay follow-up mesages */
static void handlePDelayRespFollowUp(ptpClock_t *ptpClock, bool isFromSelf)
{
    timeInternal_t responseOriginTimestamp;
    timeInternal_t correctionField;

    switch (ptpClock->portDS.delayMechanism) {
        case E2E:

            ERROR("handlePDelayRespFollowUp: disreguard in E2E mode\n");
            break;

        case P2P:

            DBGV("handlePDelayRespFollowUp: received in mode P2P in state %s\n", stateString(ptpClock->portDS.portState));
            if (ptpClock->msgIbufLength < PDELAY_RESP_FOLLOW_UP_LENGTH) {
                ERROR("handlePDelayRespFollowUp: short message\n");
                toState(ptpClock, PTP_FAULTY);
                return;
            }

            switch (ptpClock->portDS.portState) {
                case PTP_INITIALIZING:
                case PTP_FAULTY:
                case PTP_DISABLED:
                case PTP_UNCALIBRATED:
                    DBGV("handlePDelayRespFollowUp: disreguard\n");
                    return;

                case PTP_SLAVE:
                case PTP_MASTER:

                    if (!ptpClock->waitingForPDelayRespFollowUp) {
                        DBG("handlePDelayRespFollowUp: not waiting a message\n");
                        break;
                    }

                    if (ptpClock->msgTmpHeader.sequenceId == ptpClock->sentPDelayReqSequenceId - 1) {
                        msgUnpackPDelayRespFollowUp(ptpClock->msgIbuf, &ptpClock->msgTmp.prespfollow);
                        toInternalTime(&responseOriginTimestamp, &ptpClock->msgTmp.prespfollow.responseOriginTimestamp);
                        ptpClock->pdelay_t3 = responseOriginTimestamp;
                        scaledNanosecondsToInternalTime(&ptpClock->msgTmpHeader.correctionfield, &correctionField);
                        addTime(&correctionField, &correctionField, &ptpClock->correctionField_pDelayResp);
                        updatePeerDelay(ptpClock, &correctionField, true);
                        ptpClock->waitingForPDelayRespFollowUp = false;
                        break;
                    }

                default:

                    DBGV("handlePDelayRespFollowUp: unrecognized state\n");
            }
            break;

        default:

            break;
    }
}

/* Handle management messages (not implemented) */
static void handleManagement(ptpClock_t *ptpClock, bool isFromSelf)
{
    /* ENABLE_PORT -> DESIGNATED_ENABLED -> toState(PTP_INITIALIZING) */
    /* DISABLE_PORT -> DESIGNATED_DISABLED -> toState(PTP_DISABLED) */
}

/* Handle signalling messages (not implemented) */
static void handleSignaling(ptpClock_t *ptpClock, bool isFromSelf)
{
}

/* Issue delay requests when the timers have expired */
static void issueDelayReqTimerExpired(ptpClock_t *ptpClock)
{
    switch (ptpClock->portDS.delayMechanism) {
        case E2E:

            if (ptpClock->portDS.portState != PTP_SLAVE) {
                break;
            }

            if (timerExpired(DELAYREQ_INTERVAL_TIMER)) {
                timerStart(DELAYREQ_INTERVAL_TIMER, getRand(pow2ms(ptpClock->portDS.logMinDelayReqInterval + 1)));
                DBGV("event DELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
                issueDelayReq(ptpClock);
            }

            break;

        case P2P:

            if (timerExpired(PDELAYREQ_INTERVAL_TIMER)) {
                timerStart(PDELAYREQ_INTERVAL_TIMER, getRand(pow2ms(ptpClock->portDS.logMinPdelayReqInterval + 1)));
                DBGV("event PDELAYREQ_INTERVAL_TIMEOUT_EXPIRES\n");
                issuePDelayReq(ptpClock);
            }
            break;

        default:
            break;
    }
}

/* Pack and send on general multicast ip adress an Announce message */
static void issueAnnounce(ptpClock_t *ptpClock)
{
    msgPackAnnounce(ptpClock, ptpClock->msgObuf);

    /// @todo network code!
    if (!netSendGeneral(&ptpClock->netPath, ptpClock->msgObuf, ANNOUNCE_LENGTH)) {
        ERROR("issueAnnounce: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issueAnnounce\n");
        ptpClock->sentAnnounceSequenceId++;
    }
}

/* Pack and send on event multicast ip adress a Sync message */
static void issueSync(ptpClock_t *ptpClock)
{
    timestamp_t originTimestamp;
    timeInternal_t internalTime;

    /* try to predict outgoing time stamp */
    getTime(&internalTime);
    fromInternalTime(&internalTime, &originTimestamp);
    msgPackSync(ptpClock, ptpClock->msgObuf, &originTimestamp);

    /// @todo network code!
    if (!netSendEvent(&ptpClock->netPath, ptpClock->msgObuf, SYNC_LENGTH, &internalTime)) {
        ERROR("issueSync: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issueSync\n");
        ptpClock->sentSyncSequenceId++;

        /* sync TX timestamp is valid */
        if ((internalTime.seconds != 0) && (ptpClock->defaultDS.twoStepFlag)) {
            // waitingForLoopback = false;
            addTime(&internalTime, &internalTime, &ptpClock->outboundLatency);
            issueFollowup(ptpClock, &internalTime);
        }
        else {
            // waitingForLoopback = ptpClock->twoStepFlag;
        }
    }
}

/* Pack and send on general multicast ip adress a FollowUp message */
static void issueFollowup(ptpClock_t *ptpClock, const timeInternal_t *time)
{
    timestamp_t preciseOriginTimestamp;

    fromInternalTime(time, &preciseOriginTimestamp);
    msgPackFollowUp(ptpClock, ptpClock->msgObuf, &preciseOriginTimestamp);

    /// @todo network code!
    if (!netSendGeneral(&ptpClock->netPath, ptpClock->msgObuf, FOLLOW_UP_LENGTH)) {
        ERROR("issueFollowup: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issueFollowup\n");
    }
}


/* Pack and send on event multicast ip address a DelayReq message */
static void issueDelayReq(ptpClock_t *ptpClock)
{
    timestamp_t originTimestamp;
    timeInternal_t internalTime;

    getTime(&internalTime);
    fromInternalTime(&internalTime, &originTimestamp);

    msgPackDelayReq(ptpClock, ptpClock->msgObuf, &originTimestamp);

    /// @todo network code!
    if (!netSendEvent(&ptpClock->netPath, ptpClock->msgObuf, DELAY_REQ_LENGTH, &internalTime)) {
        ERROR("issueDelayReq: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issueDelayReq\n");
        ptpClock->sentDelayReqSequenceId++;

        /* Delay req TX timestamp is valid */
        if (internalTime.seconds != 0) {
            addTime(&internalTime, &internalTime, &ptpClock->outboundLatency);
            ptpClock->timestamp_delayReqSend = internalTime;
        }
    }
}

/* Pack and send on event multicast ip adress a DelayResp message */
static void issueDelayResp(ptpClock_t *ptpClock, const timeInternal_t *time,
                                            const msgHeader_t *delayReqHeader)
{
    timestamp_t requestReceiptTimestamp;

    fromInternalTime(time, &requestReceiptTimestamp);
    msgPackDelayResp(ptpClock, ptpClock->msgObuf, delayReqHeader, &requestReceiptTimestamp);

    /// @todo network code!
    if (!netSendGeneral(&ptpClock->netPath, ptpClock->msgObuf, PDELAY_RESP_LENGTH)) {
        ERROR("issueDelayResp: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issueDelayResp\n");
    }
}

/* Pack and send on event multicast ip adress a PDelayReq message */
static void issuePDelayReq(ptpClock_t *ptpClock)
{
    timestamp_t originTimestamp;
    timeInternal_t internalTime;

    getTime(&internalTime);
    fromInternalTime(&internalTime, &originTimestamp);

    msgPackPDelayReq(ptpClock, ptpClock->msgObuf, &originTimestamp);

    /// @todo network code!
    if (!netSendPeerEvent(&ptpClock->netPath, ptpClock->msgObuf, PDELAY_REQ_LENGTH, &internalTime)) {
        ERROR("issuePDelayReq: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issuePDelayReq\n");
        ptpClock->sentPDelayReqSequenceId++;

        /* Delay req TX timestamp is valid */
        if (internalTime.seconds != 0) {
            addTime(&internalTime, &internalTime, &ptpClock->outboundLatency);
            ptpClock->pdelay_t1 = internalTime;
        }
    }
}

/* Pack and send on event multicast ip adress a PDelayResp message */
static void issuePDelayResp(ptpClock_t *ptpClock, timeInternal_t *time,
                                            const msgHeader_t *pDelayReqHeader)
{
    timestamp_t requestReceiptTimestamp;

    fromInternalTime(time, &requestReceiptTimestamp);
    msgPackPDelayResp(ptpClock->msgObuf, pDelayReqHeader, &requestReceiptTimestamp);

    /// @todo network code!
    if (!netSendPeerEvent(&ptpClock->netPath, ptpClock->msgObuf, PDELAY_RESP_LENGTH, time)) {
        ERROR("issuePDelayResp: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        if (time->seconds != 0) {
            /* Add  latency */
            addTime(time, time, &ptpClock->outboundLatency);
        }

        DBGV("issuePDelayResp\n");
    }
}

/* Pack and send on general multicast ip adress a PDelayRespFollowUp message */
static void issuePDelayRespFollowUp(ptpClock_t *ptpClock, const timeInternal_t *time,
                                                    const msgHeader_t *pDelayReqHeader)
{
    timestamp_t responseOriginTimestamp;
    fromInternalTime(time, &responseOriginTimestamp);

    msgPackPDelayRespFollowUp(ptpClock->msgObuf, pDelayReqHeader, &responseOriginTimestamp);

    /// @todo network code!
    if (!netSendPeerGeneral(&ptpClock->netPath, ptpClock->msgObuf, PDELAY_RESP_FOLLOW_UP_LENGTH)) {
        ERROR("issuePDelayRespFollowUp: can't sent\n");
        toState(ptpClock, PTP_FAULTY);
    }
    else {
        DBGV("issuePDelayRespFollowUp\n");
    }
}

