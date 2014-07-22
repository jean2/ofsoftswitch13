/* Copyright (c) 2011, CPqD, Brazil
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

#include "ofl-structs.h"
#include "lib/hash.h"
#include "oxs-stats.h"

void
ofl_structs_stats_init(struct ofl_stats *stats){

    stats->header.length = 0;
    stats->stats_fields = (struct hmap) HMAP_INITIALIZER(&stats->stats_fields);
    stats->reason = OFPFSR_STATS_REQUEST;
    stats->trigger_done = false;
}


void
ofl_structs_stats_put8(struct ofl_stats *stats, uint32_t header, uint8_t value){
    struct ofl_stats_tlv *m = malloc(sizeof (struct ofl_stats_tlv));
    int len = sizeof(uint8_t);

    m->header = header;
    m->value = malloc(len);
    m->last_trigger = 0ll;
    memcpy(m->value, &value, len);
    hmap_insert(&stats->stats_fields,&m->hmap_node,hash_int(header, 0));
    stats->header.length += len + 4;
}

void
ofl_structs_stats_put16(struct ofl_stats *stats, uint32_t header, uint16_t value){
    struct ofl_stats_tlv *m = malloc(sizeof (struct ofl_stats_tlv));
    int len = sizeof(uint16_t);

    m->header = header;
    m->value = malloc(len);
    m->last_trigger = 0ll;
    memcpy(m->value, &value, len);
    hmap_insert(&stats->stats_fields,&m->hmap_node,hash_int(header, 0));
    stats->header.length += len + 4;
}

void
ofl_structs_stats_put32(struct ofl_stats *stats, uint32_t header, uint32_t value){
    struct ofl_stats_tlv *m = xmalloc(sizeof (struct ofl_stats_tlv));

    int len = sizeof(uint32_t);

    m->header = header;
    m->value = malloc(len);
    m->last_trigger = 0ll;
    memcpy(m->value, &value, len);
    hmap_insert(&stats->stats_fields,&m->hmap_node,hash_int(header, 0));
    stats->header.length += len + 4;

}

void
ofl_structs_stats_put64(struct ofl_stats *stats, uint32_t header, uint64_t value){
    struct ofl_stats_tlv *m = malloc(sizeof (struct ofl_stats_tlv));
    int len = sizeof(uint64_t);

    m->header = header;
    m->value = malloc(len);
    m->last_trigger = 0ll;
    memcpy(m->value, &value, len);
    hmap_insert(&stats->stats_fields,&m->hmap_node,hash_int(header, 0));
    stats->header.length += len + 4;

}
