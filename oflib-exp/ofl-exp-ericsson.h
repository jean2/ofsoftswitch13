/*
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#ifndef OFL_EXP_ERICSSON_H
#define OFL_EXP_ERICSSON_H 1


#include "../oflib/ofl-structs.h"
#include "../oflib/ofl-messages.h"


/* Common header for all messages */
struct ofl_exp_eric_header {
    struct ofl_msg_experimenter   header; /* ERIC_EXPERIMENTER_ID */
    uint32_t   exp_type;      /* One of ERIC_TYPE_*. */
};


/* Message structure for ERIC_TYPE_RESYNC_REQUEST */
struct ofl_exp_eric_resync_request {
    struct ofl_exp_eric_header header;  /* ERIC_TYPE_RESYNC_REQUEST */
    uint8_t  command;          /* One of ERIC_RSRQC_*. */
    uint8_t  pad[7];
};


/* Message structure for ERIC_TYPE_RESYNC_REPLY */
struct ofl_exp_eric_resync_reply {
    struct ofl_exp_eric_header header;  /* ERIC_TYPE_RESYNC_REPLY */
    uint8_t  response;         /* One of ERIC_RSRPR_* */
    uint8_t  pad[7];
    uint64_t controller_id;
    uint32_t first_conn_estd;
    uint32_t conn_lost_timer;
};



int
ofl_exp_ericsson_msg_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len);

ofl_err
ofl_exp_ericsson_msg_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg);

int
ofl_exp_ericsson_msg_free(struct ofl_msg_experimenter *msg);

char *
ofl_exp_ericsson_msg_to_string(struct ofl_msg_experimenter *msg);

#endif /* OFL_EXP_ERICSSON_H */
