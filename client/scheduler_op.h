// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef _SCHEDULER_OP_
#define _SCHEDULER_OP_

// SCHEDULER_OP encapsulates the policy and mechanism
// for communicating with scheduling servers.
// It is implemented as a finite-state machine.
// It is active in one of two modes:
//    get_work: the client wants to get work, and possibly to
//       return results as a side-effect
//    return_results: the client wants to return results, and possibly
//       to get work as a side-effect
//

#include "client_types.h"
#include "http.h"
#include "prefs.h"

// default constants related to scheduler RPC policy

#define MASTER_FETCH_PERIOD     10
    // fetch and parse master URL if nrpc_failures is a multiple of this
#define RETRY_BASE_PERIOD       1
    // after failure, back off 2^nrpc_failures times this times random
#define RETRY_CAP               10
    // cap on nrpc_failures in the above formula
#define MASTER_FETCH_RETRY_CAP 3
    // cap on how many times we will contact master_url
    // before moving into a state in which we will not
    // exponentially backoff anymore but rather contact the master URL
    // at the frequency below
#define MASTER_FETCH_INTERVAL (60*60*24*7*2)    // 2 weeks
    // This is the Max on the time to wait after we've contacted the Master URL MASTER_FETCH_RETRY_CAP times.

//The next two constants are used to bound RPC exponential waiting.
#define SCHED_RETRY_DELAY_MIN    60                // 1 minute
#define SCHED_RETRY_DELAY_MAX    (60*60*4)         // 4 hours

#define SCHEDULER_OP_STATE_IDLE         0
    // invariant: in this state, our HTTP_OP is not in the HTTP_OP_SET
#define SCHEDULER_OP_STATE_GET_MASTER   1
#define SCHEDULER_OP_STATE_RPC          2

struct SCHEDULER_OP {
    int state;
    int scheduler_op_retval;
    HTTP_OP http_op;
    HTTP_OP_SET* http_ops;
    PROJECT* project;               // project we're currently contacting
    char scheduler_url[256];
    bool must_get_work;             // true iff in get_work mode
    unsigned int url_index;         // index within project's URL list

    SCHEDULER_OP(HTTP_OP_SET*);
    bool poll();
    int init_get_work();
    int init_return_results(PROJECT*);
    int init_op_project(double ns);
    int init_master_fetch(PROJECT*);
    int set_min_rpc_time(PROJECT*);
    bool update_urls(PROJECT& project, std::vector<STRING256> &urls);
    int start_op(PROJECT*);
    bool check_master_fetch_start();
    void backoff(PROJECT* p, char *error_msg);
    int start_rpc();
    int parse_master_file(std::vector<STRING256>&);
};

struct SCHEDULER_REPLY {
    int hostid;
    double request_delay;
    char message[1024];
    char message_priority[256];
    char* global_prefs_xml;
        // not including <global_preferences> tags;
        // may include <venue> elements
    char* project_prefs_xml;
        // not including <project_preferences> tags
        // may include <venue> elements
    char host_venue[256];
    unsigned int user_create_time;
    std::vector<APP> apps;
    std::vector<FILE_INFO> file_infos;
    std::vector<STRING256> file_deletes;
    std::vector<APP_VERSION> app_versions;
    std::vector<WORKUNIT> workunits;
    std::vector<RESULT> results;
    std::vector<RESULT> result_acks;
    char* code_sign_key;
    char* code_sign_key_signature;
    bool message_ack;
    bool project_is_down;
    bool send_file_list;      

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int parse(FILE*, PROJECT*);
};

#endif
