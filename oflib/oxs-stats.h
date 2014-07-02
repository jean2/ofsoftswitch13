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
 * Copyright (c) 2010 Nicira Networks.
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

#ifndef OX_STATS_H
#define OX_STATS_H 1

#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "ofpbuf.h"
#include "hmap.h"
#include "packets.h"
#include "../oflib/ofl-structs.h"

#define OXS_HEADER__(CLASS, FIELD, RESERVED, LENGTH) \
    (((CLASS) << 16) | ((FIELD) << 9) | ((RESERVED) << 8) | (LENGTH))
#define OXS_HEADER(CLASS, FIELD, LENGTH) \
    OXS_HEADER__(CLASS, FIELD, 0, LENGTH)

#define OXS_HEADER_VL(CLASS,FIELD) \
    OXS_HEADER__(CLASS,FIELD,0,0)

#define OXS_HEADER_VL_W(CLASS,FIELD) \
   OXS_HEADER__(CLASS,FIELD,1,0)

#define OXS_CLASS(HEADER) ((HEADER) >> 16)
#define OXS_FIELD(HEADER) (((HEADER) >> 9) & 0x7f)
#define OXS_TYPE(HEADER) (((HEADER) >> 9) & 0x7fffff)
#define OXS_RESERVED(HEADER) (((HEADER) >> 8) & 1)
#define OXS_LENGTH(HEADER) ((HEADER) & 0xff)


/* ## ------------------------------- ## */
/* ## OpenFlow 1.5-compatible fields. ## */
/* ## ------------------------------- ## */

/* OpenFlow flow duration.
 * Time flow entry has been alive in seconds and nanoseconds.
 *
 * Format: A pair of 32-bit integer in network byte order.
 *    First 32-bit number is duration in seconds.
 *    Second 32-bit number is nanoseconds beyond duration in seconds.
 * Second number must be set to zero if not supported.
 */
#define OXS_OF_DURATION      OXS_HEADER  (0x8002, OFPXST_OFB_DURATION, 8)

/* OpenFlow flow idle time.
 * Time flow entry has been idle (no packets matched) in seconds
 * and nanoseconds.
 *
 * Format: A pair of 32-bit integer in network byte order.
 *    First 32-bit number is idle time in seconds.
 *    Second 32-bit number is nanoseconds beyond idle time in seconds.
 * Second number must be set to zero if not supported.
 */
#define OXS_OF_IDLE_TIME      OXS_HEADER  (0x8002, OFPXST_OFB_IDLE_TIME, 8)

/* OpenFlow flow entry count.
 * Number of aggregated flow entries.
 * Required in OFPMP_AGGREGATE replies, undefined in other context.
 *
 * Format: 32-bit integer in network byte order.
 */
#define OXS_OF_FLOW_COUNT    OXS_HEADER  (0x8002, OFPXST_OFB_FLOW_COUNT, 4)

/* OpenFlow flow entry packet count.
 * Number of packets matched by a flow entry.
 *
 * Format: 64-bit integer in network byte order.
 */
#define OXS_OF_PACKET_COUNT  OXS_HEADER  (0x8002, OFPXST_OFB_PACKET_COUNT, 8)

/* OpenFlow flow entry packet count.
 * Number of bytes matched by a flow entry.
 *
 * Format: 64-bit integer in network byte order.
 */
#define OXS_OF_BYTE_COUNT    OXS_HEADER  (0x8002, OFPXST_OFB_BYTE_COUNT, 8)


enum oxs_field_index {
#define DEFINE_FIELD(HEADER) \
        OFI_OXS_##HEADER,
#include "oxs-stats.def"
    NUM_OXS_FIELDS = 5
};

struct oxs_field {
    struct hmap_node hmap_node;
    enum oxs_field_index index;       /* OFI_* value. */
    uint32_t header;                  /* OXS_* value. */
};

/* All the known fields. */
extern struct oxs_field all_stats_fields[NUM_OXS_FIELDS];

struct oxs_field *
oxs_field_lookup(uint32_t header);

int
oxs_pull_stats(struct ofpbuf * buf, struct ofl_stats *stats_dst, int stats_len);

int oxs_put_stats(struct ofpbuf *buf, struct ofl_stats *omt);

struct ofl_stats_tlv *
oxs_stats_lookup(uint32_t header, const struct ofl_stats *omt);

uint32_t oxs_entry_ok(const void *, unsigned int );

int
oxs_field_bytes(uint32_t header);

int
oxs_field_bits(uint32_t header);



#endif /* oxs-stats.h */
