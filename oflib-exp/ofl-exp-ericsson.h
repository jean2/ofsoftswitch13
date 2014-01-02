/*
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#ifndef OFL_EXP_ERICSSON_H
#define OFL_EXP_ERICSSON_H 1


#include "../oflib/ofl-structs.h"
#include "../oflib/ofl-messages.h"


int
ofl_exp_ericsson_msg_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len);

ofl_err
ofl_exp_ericsson_msg_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg);

int
ofl_exp_ericsson_msg_free(struct ofl_msg_experimenter *msg);

char *
ofl_exp_ericsson_msg_to_string(struct ofl_msg_experimenter *msg);

#endif /* OFL_EXP_ERICSSON_H */
