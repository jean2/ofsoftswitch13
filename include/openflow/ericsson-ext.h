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

/* Common header for all messages */
struct eric_header {
    struct ofp_header header;        /* OFPT_EXPERIMENTER. */
    uint32_t          experimenter;  /* ERIC_EXPERIMENTER_ID. */
    uint32_t          exp_type;      /* One of ERIC_TYPE_* above. */
};
OFP_ASSERT(sizeof(struct eric_header) == sizeof(struct ofp_experimenter_header));

#endif /* ERIC_EXT_H */
