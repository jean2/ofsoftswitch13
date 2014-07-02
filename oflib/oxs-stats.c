/* Copyright (c) 2012, CPqD, Brazil
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Ericsson Research nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */
/*
 *  * Copyright (c) 2010 Nicira Networks.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include <config.h>

#include "oxs-stats.h"

#include <netinet/icmp6.h>
#include "hmap.h"
#include "hash.h"
#include "ofp.h"
#include "ofpbuf.h"
#include "byte-order.h"
#include "packets.h"
#include "ofpbuf.h"
#include "oflib/ofl-structs.h"
#include "oflib/ofl-utils.h"
#include "oflib/ofl-print.h"
#include "unaligned.h"
#include "byte-order.h"
#include "../include/openflow/openflow.h"

#define LOG_MODULE VLM_oxs_stats
#include "vlog.h"

static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 5);

struct oxs_field all_stats_fields[NUM_OXS_FIELDS] = {
#define DEFINE_FIELD(HEADER)     \
    { HMAP_NODE_NULL_INITIALIZER, OFI_OXS_##HEADER, OXS_##HEADER },
#include "oxs-stats.def"
};

/* Hash table of 'oxs_fields'. */
static struct hmap all_oxs_fields = HMAP_INITIALIZER(&all_oxs_fields);

static void
oxs_init(void)
{
    if (hmap_is_empty(&all_oxs_fields)) {
        int i;

        for (i = 0; i < NUM_OXS_FIELDS; i++) {
            struct oxs_field *f = &all_stats_fields[i];
            hmap_insert(&all_oxs_fields, &f->hmap_node,
                        hash_int(f->header, 0));
        }

        /* Verify that the header values are unique (duplicate "case" values
         * cause a compile error). */
        switch (0) {
#define DEFINE_FIELD(HEADER)  \
        case OXS_##HEADER: break;
#include "oxs-stats.def"
        }
    }
}

struct oxs_field *
oxs_field_lookup(uint32_t header)
{
    struct oxs_field *f;

    oxs_init();
    HMAP_FOR_EACH_WITH_HASH(f, struct oxs_field, hmap_node, hash_int(header, 0),
                            &all_oxs_fields) {
        if (f->header == header) {
            return f;
        }
    }
    return NULL;
}


struct ofl_stats_tlv *
oxs_stats_lookup(uint32_t header, const struct ofl_stats *ost)
{
    struct ofl_stats_tlv *f;

    HMAP_FOR_EACH_WITH_HASH(f, struct ofl_stats_tlv, hmap_node, hash_int(header, 0),
    					    &ost->stats_fields) {
        if (f->header == header) {
            return f;
        }
    }
    return NULL;
}


static bool
check_oxs_dup(struct ofl_stats *stats,const struct oxs_field *om){

    struct ofl_stats_tlv *t;
    HMAP_FOR_EACH_WITH_HASH(t, struct ofl_stats_tlv, hmap_node ,hash_int(om->header, 0),
                             &stats->stats_fields) {
        return true;
    }
    return false;

}

static uint8_t* get_oxs_value(struct ofl_stats *m, uint32_t header){

     struct ofl_stats_tlv *t;
     HMAP_FOR_EACH_WITH_HASH (t, struct ofl_stats_tlv, hmap_node, hash_int(header, 0),
          &m->stats_fields) {
         return t->value;
     }

     return NULL;
}

static int
parse_oxs_entry(struct ofl_stats *stats, const struct oxs_field *f,
                const void *value){

    switch (f->index) {
        case OFI_OXS_OF_DURATION: {
            ofl_structs_stats_put64(stats, f->header, ntoh64(*((uint64_t*) value)));
            return 0;
        }
        case OFI_OXS_OF_IDLE_TIME: {
            ofl_structs_stats_put64(stats, f->header, ntoh64(*((uint64_t*) value)));
            return 0;
        }
        case OFI_OXS_OF_FLOW_COUNT: {
            uint32_t* flow_countp = (uint32_t*) value;
            ofl_structs_stats_put32(stats, f->header, ntohl(*flow_countp));
            return 0;
        }
        case OFI_OXS_OF_PACKET_COUNT: {
            ofl_structs_stats_put64(stats, f->header, ntoh64(*((uint64_t*) value)));
            return 0;
        }
        case OFI_OXS_OF_BYTE_COUNT: {
            ofl_structs_stats_put64(stats, f->header, ntoh64(*((uint64_t*) value)));
            return 0;
        }
        case NUM_OXS_FIELDS:
            NOT_REACHED();
    }
    NOT_REACHED();
}
 /*hmap_insert(stats_dst, &f->hmap_node,
                hash_int(f->header, 0));               */


/* oxs_pull_stats() and helpers. */


/* Puts the stats in a hash_map structure */
int
oxs_pull_stats(struct ofpbuf *buf, struct ofl_stats * stats_dst, int stats_len)
{

    uint32_t header;
    uint8_t *p;
    p = ofpbuf_try_pull(buf, stats_len);

    if (!p) {
        VLOG_DBG_RL(LOG_MODULE,&rl, "oxs_stats length %u, rounded up to a "
                    "multiple of 8, is longer than space in message (max "
                    "length %zu)", stats_len, buf->size);

        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }

    /* Initialize the stats hashmap */
    ofl_structs_stats_init(stats_dst);

    while ((header = oxs_entry_ok(p, stats_len)) != 0) {

        unsigned length = OXS_LENGTH(header);
        const struct oxs_field *f;
        int error;
        f = oxs_field_lookup(header);

        if (!f) {
            error = ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
        }
        else if (check_oxs_dup(stats_dst,f)){
            error = ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
        }
        else {
            /* 'length' is known to be correct at this point
             * because it is included in 'header' and oxs_field_lookup()
             * checked them already. */
            error = parse_oxs_entry(stats_dst, f, p + 4);
        }
        if (error) {
            VLOG_DBG_RL(LOG_MODULE,&rl, "bad oxs_entry with class=%"PRIu32", "
                        "field=%"PRIu32", reserved=%"PRIu32", type=%"PRIu32" "
                        "(error %x)",
                        OXS_CLASS(header), OXS_FIELD(header),
                        OXS_RESERVED(header), OXS_TYPE(header),
                        error);
            return error;
        }
        p += 4 + length;
        stats_len -= 4 + length;
    }
    return stats_len ? ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN) : 0;
}


uint32_t
oxs_entry_ok(const void *p, unsigned int stats_len)
{
    unsigned int payload_len;
    uint32_t header;

    if (stats_len <= 4) {
        if (stats_len) {
            VLOG_DBG(LOG_MODULE,"oxs_stats ends with partial oxs_header");
        }
        return 0;
    }

    memcpy(&header, p, 4);
    header = ntohl(header);
    payload_len = OXS_LENGTH(header);
    if (!payload_len) {
        VLOG_DBG(LOG_MODULE, "oxs_entry %08"PRIx32" has invalid payload "
                    "length 0", header);
        return 0;
    }
    if (stats_len < payload_len + 4) {
        VLOG_DBG(LOG_MODULE, "%"PRIu32"-byte oxs_entry but only "
                    "%u bytes left in ox_stats", payload_len + 4, stats_len);
        VLOG_DBG(LOG_MODULE, "Header ==  %d"
                    ,  OXS_FIELD(header));
        return 0;
    }
    return header;
}

/* oxs_put_stats() and helpers.
 *
 * 'put' functions add stats fields, we don't have to deal with wildcards.
 */

static void
oxs_put_header(struct ofpbuf *buf, uint32_t header)
{
    uint32_t n_header = htonl(header);
    ofpbuf_put(buf, &n_header, sizeof n_header);

}

static void
oxs_put_8(struct ofpbuf *buf, uint32_t header, uint8_t value)
{
    oxs_put_header(buf, header);
    ofpbuf_put(buf, &value, sizeof value);
}

static void
oxs_put_16(struct ofpbuf *buf, uint32_t header, uint16_t value)
{
    oxs_put_header(buf, header);
    ofpbuf_put(buf, &value, sizeof value);
}

static void
oxs_put_32(struct ofpbuf *buf, uint32_t header, uint32_t value)
{
    oxs_put_header(buf, header);
    ofpbuf_put(buf, &value, sizeof value);
}

static void
oxs_put_64(struct ofpbuf *buf, uint32_t header, uint64_t value)
{
    oxs_put_header(buf, header);
    ofpbuf_put(buf, &value, sizeof value);
}


/* Puts the stats in the buffer */
int oxs_put_stats(struct ofpbuf *buf, struct ofl_stats *omt){

    struct ofl_stats_tlv *oft;
    int start_len = buf->size;
    int stats_len;


    /* Loop through all fields */
    HMAP_FOR_EACH(oft, struct ofl_stats_tlv, hmap_node, &omt->stats_fields){

            uint8_t length = OXS_LENGTH(oft->header) ;
            switch (length){
                case (sizeof(uint8_t)):{
                    uint8_t value;
                    memcpy(&value, oft->value,sizeof(uint8_t));
                    oxs_put_8(buf,oft->header, value);
                    break;
                  }
                case (sizeof(uint16_t)):{
                    uint16_t value;
                    memcpy(&value, oft->value,sizeof(uint16_t));
                    oxs_put_16(buf,oft->header, htons(value));
                    break;
                }
                case (sizeof(uint32_t)):{
                    uint32_t value;
                    memcpy(&value, oft->value,sizeof(uint32_t));
                    oxs_put_32(buf,oft->header, htonl(value));
                    break;

                }
                case (sizeof(uint64_t)):{
                     uint64_t value;
                     memcpy(&value, oft->value,sizeof(uint64_t));
                     oxs_put_64(buf,oft->header, hton64(value));
                     break;
                }
            }
    }

    stats_len = buf->size - start_len;
    ofpbuf_put_zeros(buf, ROUND_UP(stats_len - 4, 8) - (stats_len -4));
    return stats_len;
}



