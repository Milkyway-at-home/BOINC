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

#ifndef _WIN32
#include <stdio.h>
#include <string>
#include <vector>

#endif

#include "miofile.h"

#define GUI_RPC_PORT 31416

struct PROJECT;

struct FILE_INFO {
  std::string name;
    bool generated_locally;
    bool uploaded;
    bool upload_when_present;
    bool sticky;
    bool pers_xfer_active;
    bool xfer_active;
    int num_retries;
    double bytes_xferred;
    double file_offset;
    double xfer_speed;
    string hostname;
    PROJECT* project;

    int parse(MIOFILE&);
    void print();
};

struct PROJECT {
    string master_url;
    double resource_share;
    string project_name;
    string user_name;
    string team_name;
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    int nrpc_failures;          // # of consecutive times we've failed to
                                // contact all scheduling servers
    int master_fetch_failures;
    int min_rpc_time;           // earliest time to contact any server

    bool master_url_fetch_pending; // need to fetch and parse the master URL
    bool sched_rpc_pending;     // contact scheduling server for preferences
    bool tentative;             // master URL and account ID not confirmed

    int parse(MIOFILE&);
    void print();
};

struct APP {
    string name;
    PROJECT* project;

    int parse(MIOFILE&);
    void print();
};

struct APP_VERSION {
    string app_name;
    int version_num;
    APP* app;
    PROJECT* project;

    int parse(MIOFILE&);
    void print();
};

struct WORKUNIT {
    string name;
    string app_name;
    int version_num;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;

    int parse(MIOFILE&);
    void print();
};

struct RESULT {
    string name;
    string wu_name;
    int report_deadline;
    bool ready_to_report;
    bool got_server_ack;
    double final_cpu_time;
    int state;
    int exit_status;
    int signal;
    int active_task_state;
    string stderr_out;
    APP* app;
    WORKUNIT* wup;
    PROJECT* project;

    int parse(MIOFILE&);
    void print();
};

struct ACTIVE_TASK {
    string result_name;
    int app_version_num;
    double checkpoint_cpu_time;
    double current_cpu_time;
    double fraction_done;
    PROJECT* project;
    RESULT* result;

    int parse(MIOFILE&);
    void print();
};

struct PROXY_INFO {
    bool use_http_proxy;
    bool use_socks_proxy;
    int socks_version;
    char socks_server_name[256];
    char http_server_name[256];
    int socks_server_port;
    int http_server_port;
    char socks5_user_name[256];
    char socks5_user_passwd[256];
};

struct MESSAGE_DESC {
    string project;
    int priority;
    int timestamp;
    string body;
};

class RPC_CLIENT {
    int sock;
    int send_request(char*);
    int get_reply(char*&);
public:
    std::vector<PROJECT*> projects;
    std::vector<FILE_INFO*> file_infos;
    std::vector<APP*> apps;
    std::vector<APP_VERSION*> app_versions;
    std::vector<WORKUNIT*> wus;
    std::vector<RESULT*> results;
    std::vector<ACTIVE_TASK*> active_tasks;

    APP* lookup_app(string&);
    WORKUNIT* lookup_wu(string&);
    APP_VERSION* lookup_app_version(string&, int);
    RESULT* lookup_result(string&);

    void link();

    ~RPC_CLIENT();
    int init(char*);
    int get_state();
    int result_show_graphics(RESULT&);
    int project_reset(PROJECT&);
    int project_attach(char* url, char* auth);
    int project_detach(PROJECT&);
    int project_update(PROJECT&);
    int set_run_mode(int mode);
    int run_benchmarks();
    int set_proxy_settings(PROXY_INFO&);
    int get_messages(int nmessages, int seqno, std::vector<MESSAGE_DESC>&);
    void print();
};

