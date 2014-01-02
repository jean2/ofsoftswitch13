/*
 * Structures and wire protocol for Ericsson Extension
 *
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#ifndef ERIC_EXT_H
#define ERIC_EXT_H 1

#include "openflow/openflow.h"

#define ERIC_EXPERIMENTER_ID 0x00D0F0DB

/***************************** MESSAGES *****************************/

/* Message types */
enum eric_type {
    /* Resync messages */
    ERIC_TYPE_RESYNC_REQUEST = 100,
    ERIC_TYPE_RESYNC_REPLY   = 101
};

/* Common header for all messages */
struct eric_header {
    struct ofp_header header;        /* OFPT_EXPERIMENTER. */
    uint32_t          experimenter;  /* ERIC_EXPERIMENTER_ID. */
    uint32_t          exp_type;      /* One of ERIC_TYPE_* above. */
};
OFP_ASSERT(sizeof(struct eric_header) == sizeof(struct ofp_experimenter_header));


/* Message structure for ERIC_TYPE_RESYNC_REQUEST */
struct eric_resync_request {
    struct ofp_header header;  /* OFPT_EXPERIMENTER. */
    uint32_t experimenter;     /* ERIC_EXPERIMENTER_ID. */
    uint32_t exp_type;         /* ERIC_TYPE_RESYNC_REQUEST. */
    uint8_t  command;          /* One of ERIC_RSRQC_*. */
    uint8_t  pad[7];
};
OFP_ASSERT(sizeof(struct eric_resync_request) == sizeof(struct eric_header) + 8);


/* Resync request commands */
enum eric_resync_request_command {
    ERIC_RSRQC_INIT,    /* Initialize resync. */
    ERIC_RSRQC_ABORT,   /* Abort the resync procedure. */
    ERIC_RSRQC_FINISH   /* Resync finished. */
};


/* Message structure for ERIC_TYPE_RESYNC_REPLY */
struct eric_resync_reply {
    struct ofp_header header;  /* OFPT_EXPERIMENTER. */
    uint32_t experimenter;     /* ERIC_EXPERIMENTER_ID. */
    uint32_t exp_type;         /* ERIC_TYPE_RESYNC_REPLY. */
    uint8_t  response;         /* One of ERIC_RSRPR_* */
    uint8_t  pad[7];
    uint64_t controller_id;
    uint32_t first_conn_estd;
    uint32_t conn_lost_timer;
};
OFP_ASSERT(sizeof(struct eric_resync_reply) == sizeof(struct eric_header) + 24);

/* Resync reply commands */
enum eric_resync_reply_response {
    ERIC_RSRPR_INIT_ACK,            /* Init acknowledged. */
    ERIC_RSRPR_INIT_EMPTY,          /* Init acknowledged, but tables are empty. */
    ERIC_RSRPR_ABORT_ACK,           /* Abort acknowledged. */
    ERIC_RSRPR_FINISH_OK,           /* Finish acknowledged, result is consistent. */
    ERIC_RSRPR_FINISH_INCONSISTENT  /* The resync resulted in inconsistent state. */
};


#endif /* ERIC_EXT_H */
