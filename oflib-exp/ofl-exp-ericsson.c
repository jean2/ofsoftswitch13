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

#define LOG_MODULE ofl_exp_eric
OFL_LOG_INIT(LOG_MODULE)

int
ofl_exp_ericsson_msg_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len) {
    if (msg->experimenter_id == ERIC_EXPERIMENTER_ID) {
        OFL_LOG_WARN(LOG_MODULE, "Trying to pack unknown Ericsson Experimenter message.");
        return -1;
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
        OFL_LOG_WARN(LOG_MODULE, "Trying to unpack unknown Ericsson Experimenter message.");
        return ofl_error(OFPET_BAD_REQUEST, OFPBRC_BAD_EXPERIMENTER);
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
        OFL_LOG_WARN(LOG_MODULE, "Trying to free unknown Ericsson Experimenter message.");
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
        struct eric_header *exp = (struct eric_header *)msg;
        OFL_LOG_WARN(LOG_MODULE, "Trying to print unknown Ericsson Experimenter message.");
        fprintf(stream, "ofexp{type=\"%u\"}", exp->exp_type);
    } else {
        OFL_LOG_WARN(LOG_MODULE, "Trying to print non-Ericsson Experimenter message.");
        fprintf(stream, "exp{exp_id=\"%u\"}", msg->experimenter_id);
    }

    fclose(stream);
    return str;
}
