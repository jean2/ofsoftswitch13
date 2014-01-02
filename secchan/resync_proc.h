/*
 * Resync procedure
 *
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#ifndef RESYNC_PROC_H
#define RESYNC_PROC_H 1

#include "secchan.h"

struct secchan;
struct rconn;
struct resync_proc_state;

void resync_proc_start(struct secchan *, struct rconn *,
                    struct rconn *, struct resync_proc_state **);

#endif /* resync_proc.h */
