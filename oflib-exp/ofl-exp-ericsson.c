/*
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "openflow/openflow.h"
#include "openflow/ericsson-ext.h"
#include "ofl-exp-ericsson.h"
#include "../oflib/ofl-log.h"
#include "../oflib/ofl-print.h"
#include "../oflib/ofl-utils.h"

#define LOG_MODULE ofl_exp_eric
OFL_LOG_INIT(LOG_MODULE)

int
ofl_exp_ericsson_msg_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len) {
    if (msg->experimenter_id == ERIC_EXPERIMENTER_ID) {
        struct ofl_exp_eric_header *exp = (struct ofl_exp_eric_header *)msg;
        switch (exp->exp_type) {
            case (ERIC_TYPE_RESYNC_REQUEST): {
                struct ofl_exp_eric_resync_request *resync = (struct ofl_exp_eric_resync_request *)exp;
                struct eric_resync_request *ofp;

                *buf_len = sizeof(struct eric_resync_request);
                *buf     = (uint8_t *)malloc(*buf_len);

                ofp = (struct eric_resync_request *)(*buf);
                ofp->experimenter = htonl(exp->header.experimenter_id);
                ofp->exp_type     = htonl(exp->exp_type);

                ofp->command      = resync->command;

                return 0;
            }
            case (ERIC_TYPE_RESYNC_REPLY): {
                struct ofl_exp_eric_resync_reply *resync = (struct ofl_exp_eric_resync_reply *)exp;
                struct eric_resync_reply *ofp;

                *buf_len = sizeof(struct eric_resync_reply);
                *buf     = (uint8_t *)malloc(*buf_len);

                ofp = (struct eric_resync_reply *)(*buf);
                ofp->experimenter = htonl(exp->header.experimenter_id);
                ofp->exp_type     = htonl(exp->exp_type);

                ofp->response        = resync->response;
                ofp->controller_id   = hton64(resync->controller_id);
                ofp->first_conn_estd = htonl( resync->first_conn_estd);
                ofp->conn_lost_timer = htonl( resync->conn_lost_timer);

                return 0;
            }
            default: {
                OFL_LOG_WARN(LOG_MODULE, "Trying to pack unknown Ericsson Experimenter message.");
                return -1;
            }
        }
    } else {
        OFL_LOG_WARN(LOG_MODULE, "Trying to pack non-Ericsson Experimenter message.");
        return -1;
    }
}


ofl_err
ofl_exp_ericsson_msg_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg) {
    struct eric_header *exp;

    if (*len < sizeof(struct eric_header)) {
        OFL_LOG_WARN(LOG_MODULE, "Received EXPERIMENTER message has invalid length (%zu).", *len);
        return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }

    exp = (struct eric_header *)oh;

    if (ntohl(exp->experimenter) == ERIC_EXPERIMENTER_ID) {
        switch (ntohl(exp->exp_type)) {
            case (ERIC_TYPE_RESYNC_REQUEST): {
                struct eric_resync_request *src;
                struct ofl_exp_eric_resync_request *dst;

                if (*len < sizeof(struct eric_resync_request)) {
                    OFL_LOG_WARN(LOG_MODULE, "Received ERIC_TYPE_RESYNC_REQUEST message has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
                }
                *len -= sizeof(struct eric_resync_request);

                src = (struct eric_resync_request *)exp;
                dst = (struct ofl_exp_eric_resync_request *)malloc(sizeof(struct ofl_exp_eric_resync_request));
                dst->header.header.experimenter_id = ntohl(exp->experimenter);
                dst->header.exp_type               = ntohl(exp->exp_type);

                dst->command                       = src->command;

                (*msg) = (struct ofl_msg_experimenter *)dst;
                return 0;
            }
            case (ERIC_TYPE_RESYNC_REPLY): {
                struct eric_resync_reply *src;
                struct ofl_exp_eric_resync_reply *dst;

                if (*len < sizeof(struct eric_resync_reply)) {
                    OFL_LOG_WARN(LOG_MODULE, "Received ERIC_TYPE_RESYNC_REPLY message has invalid length (%zu).", *len);
                    return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
                }
                *len -= sizeof(struct eric_resync_reply);

                src = (struct eric_resync_reply *)exp;
                dst = (struct ofl_exp_eric_resync_reply *)malloc(sizeof(struct ofl_exp_eric_resync_reply));
                dst->header.header.experimenter_id = ntohl(exp->experimenter);
                dst->header.exp_type               = ntohl(exp->exp_type);

                dst->response                      =        src->response;
                dst->controller_id                 = ntoh64(src->controller_id);
                dst->first_conn_estd               = ntohl( src->first_conn_estd);
                dst->conn_lost_timer               = ntohl( src->conn_lost_timer);

                (*msg) = (struct ofl_msg_experimenter *)dst;
                return 0;
            }
            default: {
                OFL_LOG_WARN(LOG_MODULE, "Trying to unpack unknown Ericsson Experimenter message.");
                return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
            }
        }
    } else {
        OFL_LOG_WARN(LOG_MODULE, "Trying to unpack non-Ericsson Experimenter message.");
        return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
    }
    free(msg);
    return 0;
}

int
ofl_exp_ericsson_msg_free(struct ofl_msg_experimenter *msg) {
    if (msg->experimenter_id == ERIC_EXPERIMENTER_ID) {
        struct ofl_exp_eric_header *exp = (struct ofl_exp_eric_header *)msg;
        switch (exp->exp_type) {
            case (ERIC_TYPE_RESYNC_REQUEST):
            case (ERIC_TYPE_RESYNC_REPLY): {
                break;
            }
            default: {
                OFL_LOG_WARN(LOG_MODULE, "Trying to free unknown Ericsson Experimenter message.");
            }
        }
    } else {
        OFL_LOG_WARN(LOG_MODULE, "Trying to free non-Ericsson Experimenter message.");
    }
    free(msg);
    return 0;
}

char *
ofl_exp_ericsson_msg_to_string(struct ofl_msg_experimenter *msg) {
    char *str;
    size_t str_size;
    FILE *stream = open_memstream(&str, &str_size);

    if (msg->experimenter_id == ERIC_EXPERIMENTER_ID) {
        struct ofl_exp_eric_header *exp = (struct ofl_exp_eric_header *)msg;

        switch (exp->exp_type) {
            case (ERIC_TYPE_RESYNC_REQUEST): {
                struct ofl_exp_eric_resync_request *resync = (struct ofl_exp_eric_resync_request *)exp;
                fprintf(stream, "resync_req{cmd=\"%s\"}",
                                    resync->command == ERIC_RSRQC_INIT ? "init" :
                                    resync->command == ERIC_RSRQC_ABORT ? "abort" :
                                    resync->command == ERIC_RSRQC_FINISH ? "finish" : "other");
                break;
            }
            case (ERIC_TYPE_RESYNC_REPLY): {
                struct ofl_exp_eric_resync_reply *resync = (struct ofl_exp_eric_resync_reply *)exp;
                fprintf(stream, "resync_repl{resp=\"%s\", ctrl_id=\"0x%016"PRIx64"\", "
                                    "first_conn=\"%u\", lost_timer=\"%u\"}",
                                    resync->response == ERIC_RSRPR_INIT_ACK ? "init_ack" :
                                    resync->response == ERIC_RSRPR_INIT_EMPTY ? "init_empty" :
                                    resync->response == ERIC_RSRPR_ABORT_ACK ? "abort_ack" :
                                    resync->response == ERIC_RSRPR_FINISH_OK ? "finish_ok" :
                                    resync->response == ERIC_RSRPR_FINISH_INCONSISTENT ? "finish_incons" : "other",
                                    resync->controller_id,
                                    resync->first_conn_estd,
                                    resync->conn_lost_timer);

                break;
            }
            default: {
                OFL_LOG_WARN(LOG_MODULE, "Trying to print unknown Ericsson Experimenter message.");
                fprintf(stream, "eric_exp{type=\"%u\"}", exp->exp_type);
            }
        }
    } else {
        OFL_LOG_WARN(LOG_MODULE, "Trying to print non-Ericsson Experimenter message.");
        fprintf(stream, "exp{exp_id=\"%u\"}", msg->experimenter_id);
    }

    fclose(stream);
    return str;
}
