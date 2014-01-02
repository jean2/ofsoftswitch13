/*
 * Resync procedure
 *
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#include <errno.h>
#include <unistd.h>
#include "resync_proc.h"
#include "openflow/ericsson-ext.h"
#include "ofp.h"
#include "ofpbuf.h"
#include "openflow/openflow.h"
#include "../oflib/ofl-utils.h"
#include "rconn.h"
#include "timeval.h"
#include "vlog.h"

#define LOG_MODULE VLM_resync

struct resync_state {
    bool  active;
    bool  modified;
};

typedef enum {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
} conn_state_t;

struct conn_state {
    struct rconn *local_rconn;
    struct rconn *remote_rconn;
    conn_state_t  state;

    time_t        first_conn_estd;
    uint64_t      last_ctrl_id;
    time_t        last_conn_lost;
    uint64_t      curr_ctrl_id;
    time_t        last_ctrl_msg;
};

struct resync_proc_state {
    struct resync_state  resync;
    struct conn_state    conn;
};

static bool resync_proc_local_packet_cb(struct relay *, void *);
static bool resync_proc_remote_packet_cb(struct relay *, void *);
static void resync_proc_periodic_cb(void *);
static void resync_proc_wait_cb(void *);
static void conn_init(struct conn_state *, struct rconn *, struct rconn *);
static void resync_init(struct resync_state *);

static struct hook_class resync_proc_hook_class = {
    resync_proc_local_packet_cb,  /* local_packet_cb */
    resync_proc_remote_packet_cb, /* remote_packet_cb */
    resync_proc_periodic_cb,      /* periodic_cb */
    resync_proc_wait_cb,          /* wait_cb */
    NULL,                         /* closing_cb */
};

void resync_proc_start(struct secchan *secchan, struct rconn *local_rconn,
                       struct rconn *remote_rconn, struct resync_proc_state **statep) {
    struct resync_proc_state *state = xcalloc(1, sizeof *state);

    resync_init(&(state->resync));
    conn_init(&(state->conn), local_rconn, remote_rconn);

    *statep = state;
    add_hook(secchan, &resync_proc_hook_class, state);
}



/***********************************************
             Secchan hook callbacks
 ***********************************************/

static void conn_connected(struct resync_proc_state *);
static void conn_connecting(struct resync_proc_state *);
static void conn_disconnected(struct resync_proc_state *);
static void resync_reset(struct resync_proc_state *);
static void resync_request(struct eric_resync_request *, struct resync_proc_state *);
static void resync_modified(struct resync_proc_state *);


static bool
resync_proc_local_packet_cb(struct relay *r, void *state_) {
    struct resync_proc_state *state = state_;

    struct ofpbuf *msg = r->halves[HALF_LOCAL].rxbuf;
    struct ofp_header *oh = msg->data;

    switch (oh->type) {
        case OFPT_FEATURES_REPLY: {
            conn_connected(state);
            resync_reset(state);
            break;
        }
    }
    return false;
}

static bool
resync_proc_remote_packet_cb(struct relay *r, void *state_) {
    struct resync_proc_state *state = state_;

    struct ofpbuf *msg = r->halves[HALF_REMOTE].rxbuf;
    struct ofp_header *oh = msg->data;

    state->conn.last_ctrl_msg = time_now();

    switch (oh->type) {
        case OFPT_FEATURES_REQUEST: {
            // For now we assume that a new connection is made when seeing this packet
            // Ideally we should also keep an eye on the state of remote_rconn...
            conn_disconnected(state);
            conn_connecting(state);
            break;
        }
        case OFPT_FLOW_MOD: {
            struct ofp_flow_mod *fm = (struct ofp_flow_mod *)oh;
            if (fm->command == OFPFC_ADD || fm->command == OFPFC_MODIFY ||
                                            fm->command == OFPFC_MODIFY_STRICT) {
                resync_modified(state);
            }
            break;
        }
        case OFPT_GROUP_MOD: {
            struct ofp_group_mod *gm = (struct ofp_group_mod *)oh;
            if (gm->command == OFPGC_ADD || gm->command == OFPGC_MODIFY) {
                resync_modified(state);
            }
            break;
        }
        case OFPT_EXPERIMENTER: {
            struct ofp_experimenter_header *eh = (struct ofp_experimenter_header *)oh;
            switch (ntohl(eh->experimenter)) {
                case ERIC_EXPERIMENTER_ID: {
                    struct eric_header *erh = (struct eric_header *)eh;
                    switch (ntohl(erh->exp_type)) {
                        case ERIC_TYPE_RESYNC_REQUEST: {
                            struct eric_resync_request *req = (struct eric_resync_request *)erh;
                            resync_request(req, state);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

static void
resync_proc_periodic_cb(void *state_) {
	struct resync_proc_state *state = state_;

	if (! rconn_is_connected(state->conn.remote_rconn))
	{
		conn_disconnected(state);
	}
}

static void
resync_proc_wait_cb(void *state_) {
    // NOTE: this is where we should wait for polling remote rconn state
}



/***********************************************
          Connection state management
 ***********************************************/

uint64_t gen_ctrl_id(struct conn_state *);

static void
conn_init(struct conn_state *conn,
          struct rconn *local_rconn, struct rconn *remote_rconn) {
    conn->local_rconn = local_rconn;
    conn->remote_rconn = remote_rconn;
    conn->state = DISCONNECTED;

    conn->first_conn_estd = TIME_MIN;
    conn->last_ctrl_id = -1;
    conn->last_conn_lost  = TIME_MIN;
    conn->curr_ctrl_id = -1;
    conn->last_ctrl_msg   = TIME_MIN;
};

static void
conn_connected(struct resync_proc_state *state) {
    switch (state->conn.state) {
        case DISCONNECTED: {
            // ignore. Might be something bogus generated by other secchan hooks.
            break;
        }
        case CONNECTING: {
            if (state->conn.first_conn_estd == TIME_MIN) {
                state->conn.first_conn_estd = time_now();
            }
            state->conn.curr_ctrl_id  = gen_ctrl_id(&(state->conn));
            state->conn.state         = CONNECTED;
            VLOG_DBG(LOG_MODULE, "Controller connected (id = 0x%"PRIx64".", state->conn.curr_ctrl_id);
            break;
        }
        case CONNECTED: {
            // ignore
            break;
        }
    }
}

static void
conn_connecting(struct resync_proc_state *state) {
    switch (state->conn.state) {
        case DISCONNECTED: {
            state->conn.state = CONNECTING;
            VLOG_DBG(LOG_MODULE, "Controller connecting...");
            break;
        }
        case CONNECTING: {
            // ignore
            break;
        }
        case CONNECTED: {
            // ignore
            break;
        }
    }
}

static void
conn_disconnected(struct resync_proc_state *state) {
    switch (state->conn.state) {
        case DISCONNECTED: {
            // ignore
            break;
        }
        case CONNECTING: {
            state->conn.state = DISCONNECTED;
            VLOG_DBG(LOG_MODULE, "Controller disconnected.");
            break;
        }
        case CONNECTED: {
            state->conn.last_ctrl_id   = state->conn.curr_ctrl_id;
            state->conn.last_conn_lost = time_now();
            state->conn.state          = DISCONNECTED;
            VLOG_DBG(LOG_MODULE, "Controller disconnected.");
            break;
        }
    }
};




/***********************************************
             Resync state management
 ***********************************************/

static void send_response(enum eric_resync_reply_response,
                struct eric_resync_request *, struct resync_proc_state *);


static void
resync_init(struct resync_state *resync) {
    resync->active   = false;
    resync->modified = false;
};

static void
resync_reset(struct resync_proc_state *state) {
    state->resync.active   = false;
    // state->resync.modified = false;
}

static void
resync_request(struct eric_resync_request *req, struct resync_proc_state *state) {
    // assume connection is established, and state is OK
    switch (req->command) {
        case ERIC_RSRQC_INIT: {
            if (!state->resync.active) {
                if (state->resync.modified) {
                    send_response(ERIC_RSRPR_INIT_ACK, req, state);
                    state->resync.active = true;
                    VLOG_INFO(LOG_MODULE, "Init request received; sending ACK.");
                } else {
                    send_response(ERIC_RSRPR_INIT_EMPTY, req, state);
                    VLOG_INFO(LOG_MODULE, "Init request received; sending EMPTY.");
                }
            } else {
                // TODO: send error
                VLOG_INFO(LOG_MODULE, "Init request received, but already in resync.");
            }
            break;
        }
        case ERIC_RSRQC_ABORT: {
            if (state->resync.active) {
                send_response(ERIC_RSRPR_ABORT_ACK, req, state);
                state->resync.active = false;
                VLOG_INFO(LOG_MODULE, "Abort request received; sending ACK.");
            } else {
                // TODO: send error
                VLOG_INFO(LOG_MODULE, "Abort request received, but was not in resync.");
            }
            break;
        }
        case ERIC_RSRQC_FINISH: {
            if (state->resync.active) {
                send_response(ERIC_RSRPR_FINISH_OK, req, state);
                state->resync.active = false;
                VLOG_INFO(LOG_MODULE, "Finish request received; sending OK.");
            } else {
                // TODO: send error
                VLOG_INFO(LOG_MODULE, "Finish request received, but was not in resync.");
            }
            break;
        }
    }
}

static void
resync_modified(struct resync_proc_state *state) {
    if (!state->resync.modified) {
        state->resync.modified = true;
        VLOG_DBG(LOG_MODULE, "Forwarding state is now considered to be non-empty.");
    }
}



static void send_response(enum eric_resync_reply_response resp,
                struct eric_resync_request *req, struct resync_proc_state *state) {
    struct ofpbuf *b;
    struct eric_resync_reply *reply = make_openflow_xid(sizeof *reply,
                                       OFPT_EXPERIMENTER, req->header.xid, &b);
    int retval;

    reply->experimenter = htonl(ERIC_EXPERIMENTER_ID);
    reply->exp_type     = htonl(ERIC_TYPE_RESYNC_REPLY);
    reply->response     = resp;

    switch (resp) {
        case ERIC_RSRPR_INIT_EMPTY: {
            reply->controller_id = hton64(state->conn.curr_ctrl_id);
            break;
        }
        default: {
            reply->controller_id = hton64(state->conn.last_ctrl_id);
            break;
        }
    }
    reply->first_conn_estd = htonl(state->conn.first_conn_estd);
    if (state->conn.last_conn_lost == TIME_MIN) {
        reply->conn_lost_timer = -1;
    } else {
        reply->conn_lost_timer = htonl(time_now() - state->conn.last_conn_lost);
    }

    retval = rconn_send(state->conn.remote_rconn, b, NULL);
    VLOG_DBG(LOG_MODULE, "sending resync_reply{resp=\"%s\", ctrl_id=\"%"PRIx64"\","
                            "first=\"%"PRIu32"\", lost=\"%"PRIu32"\"}",
                                    reply->response == ERIC_RSRPR_INIT_ACK ? "init_ack" :
                                    reply->response == ERIC_RSRPR_INIT_EMPTY ? "init_empty" :
                                    reply->response == ERIC_RSRPR_ABORT_ACK ? "abort_ack" :
                                    reply->response == ERIC_RSRPR_FINISH_OK ? "finish_ok" :
                                    reply->response == ERIC_RSRPR_FINISH_INCONSISTENT ? "finish_incons" : "other",
                                ntoh64(reply->controller_id),
                                ntohl(reply->first_conn_estd),
                                ntohl(reply->conn_lost_timer));
    if (retval && retval != EAGAIN) {
        VLOG_WARN(LOG_MODULE, "send failed (%s)", strerror(retval));
    }
}

/***********************************************
             Utilities
 ***********************************************/

uint64_t gen_ctrl_id(struct conn_state *conn) {
    uint64_t curr_ip   = htonl(rconn_get_ip(conn->remote_rconn));
    // TODO: get real port number
    uint64_t curr_port = htons(6633);

    uint64_t ctrl_id = (curr_ip << 32) | curr_port;

    return ctrl_id;
}
