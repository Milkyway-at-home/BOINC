// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _CLIENT_STATE_
#define _CLIENT_STATE_

#ifndef _WIN32
#include <string>
#include <vector>
#include <ctime>
#endif

using std::string;
using std::vector;

#include "acct_mgr.h"
#include "acct_setup.h"
#include "app.h"
#include "client_types.h"
#include "file_xfer.h"
#include "gui_rpc_server.h"
#include "gui_http.h"
#include "hostinfo.h"
#include "miofile.h"
#include "net_stats.h"
#include "pers_file_xfer.h"
#include "prefs.h"
#include "scheduler_op.h"
#include "time_stats.h"
#include "http_curl.h"
#include "coproc.h"

#define WORK_FETCH_DONT_NEED 0
    // project: suspended, deferred, or no new work (can't ask for more work)
    // overall: not work_fetch_ok (from CPU policy)
#define WORK_FETCH_OK        1
    // project: has more than min queue * share, not suspended/def/nonewwork
    // overall: at least min queue, work fetch OK
#define WORK_FETCH_NEED      2
    // project: less than min queue * resource share of DL/runnable results
    // overall: less than min queue
#define WORK_FETCH_NEED_IMMEDIATELY 3
    // project: no downloading or runnable results
    // overall: at least one idle CPU

// CLIENT_STATE encapsulates the global variables of the core client.
// If you add anything here, initialize it in the constructor
//
class CLIENT_STATE {
public:
    vector<PLATFORM> platforms;
    vector<PROJECT*> projects;
    vector<APP*> apps;
    vector<FILE_INFO*> file_infos;
    vector<APP_VERSION*> app_versions;
    vector<WORKUNIT*> workunits;
    vector<RESULT*> results;

    PERS_FILE_XFER_SET* pers_file_xfers;
    HTTP_OP_SET* http_ops;
    FILE_XFER_SET* file_xfers;
    ACTIVE_TASK_SET active_tasks;
    HOST_INFO host_info;
    GLOBAL_PREFS global_prefs;
    NET_STATS net_stats;
    GUI_RPC_CONN_SET gui_rpcs;
    TIME_STATS time_stats;
    PROXY_INFO proxy_info;
    GUI_HTTP gui_http;
    COPROCS coprocs;

    VERSION_INFO core_client_version;
    string statefile_platform_name;
    int file_xfer_giveup_period;
    MODE run_mode;
    MODE network_mode;
    bool started_by_screensaver;
    bool exit_when_idle;
    bool exit_before_start;
    bool exit_after_finish;
    bool check_all_logins;
    bool user_active;       // there has been recent mouse/kbd input
    bool allow_remote_gui_rpc;
    int cmdline_gui_rpc_port;
    bool show_projects;
    bool requested_exit;
    char detach_project_url[256];
        // stores URL for -detach_project option
    char reset_project_url[256];
        // stores URL for -reset_project option
    char update_prefs_url[256];
        // stores URL for -update_prefs option
    char main_host_venue[256];
        // venue from project that gave us general prefs
        // or from account manager
    char attach_project_url[256];
    char attach_project_auth[256];
    bool exit_before_upload;
        // exit when about to upload a file
#ifndef _WIN32
    gid_t boinc_project_gid;
#endif

    // backoff-related variables
    //
    int master_fetch_period;
        // fetch project's master URL (and stop doing scheduler RPCs)
        // if get this many successive RPC failures (default 10)
    int retry_cap;
        // cap project->nrpc_failures at this number
    int master_fetch_retry_cap;
        // after this many master-fetch failures,
        // move into a state in which we retry master fetch
        // at the frequency below
    int master_fetch_interval;
        // see above

    int sched_retry_delay_min;
    int sched_retry_delay_max;
    int pers_retry_delay_min;
    int pers_retry_delay_max;
    int pers_giveup;

    bool tasks_suspended;
        // Don't do CPU.  See check_suspend_activities for logic
    bool network_suspended;
        // Don't do network.  See check_suspend_network for logic
    int suspend_reason;
    int network_suspend_reason;
	bool executing_as_daemon;
        // true if --daemon is on the commandline
        // this means we are running as a daemon on unix,
        // or as a service on Windows
    bool redirect_io;
        // redirect stdout, stderr to log files
    bool disable_graphics;
        // a condition has occurred in which we know graphics will
        // not be displayable.
        // So GUIs shouldn't offer graphics.
    bool detach_console;
    bool launched_by_manager;
        // this affects auto-update
    bool run_by_updater;
    double now;
    double client_start_time;
    double last_wakeup_time;
    bool initialized;
    bool cant_write_state_file;
        // failed to write state file.
        // In this case we continue to run for 1 minute,
        // handling GUI RPCs but doing nothing else,
        // so that the Manager can tell the user what the problem is
private:
    bool client_state_dirty;
    int old_major_version;
    int old_minor_version;
    int old_release;
    bool skip_cpu_benchmarks;
        // if set, use hardwired numbers rather than running benchmarks
    bool run_cpu_benchmarks;
        // if set, run benchmarks on client startup
    bool cpu_benchmarks_pending;
        // set if a benchmark fails to start because of a process that doesn't stop.
        // Persists so that the next start of BOINC runs the benchmarks.

    int exit_after_app_start_secs;
        // if nonzero, exit this many seconds after starting an app
    double app_started;
        // when the most recent app was started

// --------------- acct_mgr.C:
public:
    ACCT_MGR_OP acct_mgr_op;
    ACCT_MGR_INFO acct_mgr_info;

// --------------- acct_setup.C:
public:
    PROJECT_INIT project_init;
    PROJECT_ATTACH project_attach;
    LOOKUP_WEBSITE_OP lookup_website_op;
    GET_CURRENT_VERSION_OP get_current_version_op;
    GET_PROJECT_LIST_OP get_project_list_op;
    void new_version_check();
    void all_projects_list_check();
    double new_version_check_time;
    double all_projects_list_check_time;
    string newer_version;

// --------------- auto_update.C:
public:
    AUTO_UPDATE auto_update;

// --------------- client_state.C:
public:
    CLIENT_STATE();
    void show_host_info();
    int init();
    bool poll_slow_events();
        // Never blocks.
        // Returns true if it actually did something,
        // in which case it should be called again immediately.
    void do_io_or_sleep(double dt);
    bool time_to_exit();
    PROJECT* lookup_project(const char*);
    APP* lookup_app(PROJECT*, const char*);
    FILE_INFO* lookup_file_info(PROJECT*, const char* name);
    RESULT* lookup_result(PROJECT*, const char*);
    WORKUNIT* lookup_workunit(PROJECT*, const char*);
    APP_VERSION* lookup_app_version(
        APP*, char* platform, int ver, char* plan_class
    );
    int detach_project(PROJECT*);
    int report_result_error(RESULT&, const char *format, ...);
    int reset_project(PROJECT*, bool detaching);
    bool no_gui_rpc;
private:
    int link_app(PROJECT*, APP*);
    int link_file_info(PROJECT*, FILE_INFO*);
    int link_file_ref(PROJECT*, FILE_REF*);
    int link_app_version(PROJECT*, APP_VERSION*);
    int link_workunit(PROJECT*, WORKUNIT*);
    int link_result(PROJECT*, RESULT*);
    void print_summary();
    bool garbage_collect();
    bool garbage_collect_always();
    bool update_results();
    int nresults_for_project(PROJECT*);

// --------------- cpu_sched.C:
private:
    double debt_interval_start;
    double total_wall_cpu_time_this_debt_interval;
        // "wall CPU time" accumulated since last adjust_debts()
    double total_cpu_time_this_debt_interval;
    double cpu_shortfall;
	bool work_fetch_no_new_work;
    bool must_enforce_cpu_schedule;
    bool must_schedule_cpus;
    bool must_check_work_fetch;
    std::vector <RESULT*> ordered_scheduled_results;
    void assign_results_to_projects();
    RESULT* largest_debt_project_best_result();
    RESULT* earliest_deadline_result();
    void reset_debt_accounting();
    void adjust_debts();
    bool possibly_schedule_cpus();
    void schedule_cpus();
    bool enforce_schedule();
    bool no_work_for_a_cpu();
    void rr_simulation();
    void make_running_task_heap(vector<ACTIVE_TASK*>&, double&);
    void print_deadline_misses();
public:
    double retry_shmem_time;
        // if we fail to start a task due to no shared-mem segments,
        // wait until at least this time to try running
        // another task that needs a shared-mem seg
	inline double work_buf_min() {
		return global_prefs.work_buf_min_days * 86400;
	}
    inline double work_buf_additional() {
        return global_prefs.work_buf_additional_days *86400;
    }
    inline double work_buf_total() {
        double x = work_buf_min() + work_buf_additional();
        if (x < 1) x = 1;
        return x;
    }
    void request_enforce_schedule(const char*);
    void request_schedule_cpus(const char*);
        // Check for reschedule CPUs ASAP.  Called when:
        // - core client starts (CS::init())
        // - an app exits (ATS::check_app_exited())
        // - Tasks are killed (ATS::exit_tasks())
        // - a result's input files finish downloading (CS::update_results())
        // - an app fails to start (CS::schedule_cpus())
        // - any project op is done via RPC (suspend/resume)
        // - any result op is done via RPC (suspend/resume)

// --------------- cs_account.C:
public:
    int add_project(
        const char* master_url, const char* authenticator,
        const char* project_name, bool attached_via_acct_mgr
    );
private:
    int parse_account_files();
    int parse_account_files_venue();
    int parse_preferences_for_user_files();
    int parse_statistics_files();
        // should be move to a new file, but this will do it for testing

// --------------- cs_apps.C:
private:
    double total_resource_share();
    double potentially_runnable_resource_share();
    double nearly_runnable_resource_share();
    double fetchable_resource_share();
public:
    double runnable_resource_share();
    void request_work_fetch(const char*);
        // Check if work fetch needed.  Called when:
        // - core client starts (CS::init())
        // - task is completed or fails
        // - tasks are killed
        // - an RPC completes
        // - project suspend/detch/attach/reset GUI RPC
        // - result suspend/abort GUI RPC
    int quit_activities();
    void set_ncpus();
    double estimate_cpu_time(WORKUNIT&);
    double get_fraction_done(RESULT* result);
    int input_files_available(RESULT*, bool, FILE_INFO** f=0);
    ACTIVE_TASK* lookup_active_task_by_result(RESULT*);
    int ncpus;
        // number of usable cpus
private:
    int nslots;

    int latest_version(APP*, char*);
    int app_finished(ACTIVE_TASK&);
    bool start_apps();
    bool handle_finished_apps();
public:
    ACTIVE_TASK* get_task(RESULT*);

// --------------- cs_benchmark.C:
public:
    bool should_run_cpu_benchmarks();
	void start_cpu_benchmarks();
    bool cpu_benchmarks_poll();
    void abort_cpu_benchmarks();
    bool are_cpu_benchmarks_running();
	bool cpu_benchmarks_done();
	void cpu_benchmarks_set_defaults();
    void print_benchmark_results();

// --------------- cs_cmdline.C:
public:
    void parse_cmdline(int argc, char** argv);
    void parse_env_vars();
    void do_cmdline_actions();

// --------------- cs_files.C:
public:
    void check_file_existence();
    bool start_new_file_xfer(PERS_FILE_XFER&);
private:
    int make_project_dirs();
    bool handle_pers_file_xfers();

// --------------- cs_platforms.C:
public:
    const char* get_primary_platform();
private:
    void add_platform(const char*);
    void detect_platforms();
    void write_platforms(PROJECT*, MIOFILE&);
    bool is_supported_platform(const char*);

// --------------- cs_prefs.C:
public:
    int project_disk_usage(PROJECT*, double&);
    int total_disk_usage(double&);
        // returns the total disk usage of BOINC on this host
    double allowed_disk_usage();
    int allowed_project_disk_usage(double&);
    int suspend_tasks(int reason);
    int resume_tasks(int reason=0);
    int suspend_network(int reason);
    int resume_network();
    void read_global_prefs();
    int save_global_prefs(char* prefs, char* url, char* sched);
	double available_ram();
	double max_available_ram();
private:
    int check_suspend_processing();
    int check_suspend_network();
    void install_global_prefs();
    PROJECT* global_prefs_source_project();
    void show_global_prefs_source(bool);

// --------------- cs_scheduler.C:
public:
    int make_scheduler_request(PROJECT*);
    int handle_scheduler_reply(PROJECT*, char* scheduler_url, int& nresults);
    SCHEDULER_OP* scheduler_op;
private:
    bool contacted_sched_server;
    int overall_work_fetch_urgency;

    bool scheduler_rpc_poll();
    double avg_proc_rate();
    bool should_get_work();

// --------------- cs_statefile.C:
public:
    void set_client_state_dirty(const char*);
    int parse_state_file();
    int write_state(MIOFILE&);
    int write_state_file();
    int write_state_file_if_needed();
	void check_anonymous();
	int parse_app_info(PROJECT*, FILE*);
    int write_state_gui(MIOFILE&);
    int write_file_transfers_gui(MIOFILE&);
    int write_tasks_gui(MIOFILE&);

// --------------- cs_trickle.C:
private:
    int read_trickle_files(PROJECT*, FILE*);
    int remove_trickle_files(PROJECT*);
public:
    int handle_trickle_down(PROJECT*, FILE*);

// --------------- check_state.C:
// stuff related to data-structure integrity checking
//
public:
    void check_project_pointer(PROJECT*);
    void check_app_pointer(APP*);
    void check_file_info_pointer(FILE_INFO*);
    void check_app_version_pointer(APP_VERSION*);
    void check_workunit_pointer(WORKUNIT*);
    void check_result_pointer(RESULT*);
    void check_pers_file_xfer_pointer(PERS_FILE_XFER*);
    void check_file_xfer_pointer(FILE_XFER*);

    void check_app(APP&);
    void check_file_info(FILE_INFO&);
    void check_file_ref(FILE_REF&);
    void check_app_version(APP_VERSION&);
    void check_workunit(WORKUNIT&);
    void check_result(RESULT&);
    void check_active_task(ACTIVE_TASK&);
    void check_pers_file_xfer(PERS_FILE_XFER&);
    void check_file_xfer(FILE_XFER&);

    void check_all();
    void free_mem();

// --------------- work_fetch.C:
public:
    int proj_min_results(PROJECT*, double);
	void check_project_timeout();
    PROJECT* next_project_master_pending();
    PROJECT* next_project_sched_rpc_pending();
    PROJECT* next_project_trickle_up_pending();
    PROJECT* next_project_need_work();
    PROJECT* find_project_with_overdue_results();
	double overall_cpu_frac();
    double time_until_work_done(PROJECT*, int, double);
    bool compute_work_requests();
    void scale_duration_correction_factors(double);
    void generate_new_host_cpid();
    void compute_nuploading_results();

};

extern CLIENT_STATE gstate;

// return a random double in the range [MIN,min(e^n,MAX))
//
extern double calculate_exponential_backoff(
    int n, double MIN, double MAX
);

extern void print_suspend_tasks_message(int);

#define POLL_INTERVAL   1.0
    // the client will handle I/O (including GUI RPCs)
    // for up to POLL_INTERVAL seconds before calling poll_slow_events()
    // to call the polling functions

#endif
