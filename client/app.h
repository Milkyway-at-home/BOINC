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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _TASK_
#define _TASK_

#ifndef _WIN32
#include <stdio.h>
#include <vector>
#endif

#include "client_types.h"
#include "app_ipc.h"

class CLIENT_STATE;
typedef int PROCESS_ID;

#define MAX_STDERR_LEN  65536
    // The stderr output of an application is truncated to this length
    // before sending to server,
    // to protect against apps that write unbounded amounts.

// process states of an ACTIVE_TASK
//
#define PROCESS_UNINITIALIZED   0

// states in which the process exists
#define PROCESS_EXECUTING       1
    // process is running, as far as we know
#define PROCESS_SUSPENDED       9
    // we've sent it a "suspend" message
#define PROCESS_ABORT_PENDING   5
    // process exceeded limits; killed it, waiting to exit

// states in which the process has exited
#define PROCESS_EXITED          2
#define PROCESS_WAS_SIGNALED    3
#define PROCESS_EXIT_UNKNOWN    4
#define PROCESS_ABORTED         6
    // aborted process has exited
#define PROCESS_COULDNT_START   7
#define PROCESS_IN_LIMBO        8
    // process exited zero, but no finish file; leave the task there.

// CPU scheduler states of an ACTIVE_TASK
#define CPU_SCHED_UNINITIALIZED   0
#define CPU_SCHED_PREEMPTED       1
#define CPU_SCHED_SCHEDULED       2


// Represents a task in progress.
// The execution of a task may be divided into many "episodes"
// (if the host is turned off/on, e.g.)
// A task may checkpoint now and then.
// Each episode begins with the state of the last checkpoint.
//
// "CPU time" refers to the sum over all episodes.
// (not counting the "lost" time after the last checkpoint
// in episodes before the current one)
//
// When an active task is created, it is assigned a "slot"
// which determines the directory it runs in.
// This doesn't change over the life of the active task;
// thus the task can use the slot directory for temp files
// that BOINC doesn't know about.
//
class ACTIVE_TASK {
public:
#ifdef _WIN32
    HANDLE pid_handle, thread_handle, quitRequestEvent, shm_handle;
#else
    //key_t shm_key;
#endif
    SHMEM_SEG_NAME shmem_seg_name;
    RESULT* result;
    WORKUNIT* wup;
    APP_VERSION* app_version;
    PROCESS_ID pid;
    int slot;   // which slot (determines directory)
    int task_state;
    int scheduler_state;
    int next_scheduler_state; // temp
    int signal;
    double fraction_done;
        // App's estimate of how much of the work unit is done.
        // Passed from the application via an API call;
        // will be zero if the app doesn't use this call
    double cpu_time_at_last_sched;
        // CPU time when CPU scheduler last ran
    double episode_start_cpu_time;
        // CPU time at the start of current episode
    double checkpoint_cpu_time;
        // CPU at the last checkpoint
    double current_cpu_time;
        // most recent CPU time reported by app
    double vm_bytes;
        // virtual memory used
    double rss_bytes;
        // resident set size
    int current_disk_usage(double&);
        // disk used by output files and temp files of this task
    char slot_dir[256];      // directory where process runs
    double max_cpu_time;    // abort if total CPU exceeds this
    double max_disk_usage;  // abort if disk usage (in+out+temp) exceeds this
    double max_mem_usage;   // abort if memory usage exceeds this
    bool have_trickle_down;
    bool send_upload_file_status;
    bool pending_suspend_via_quit;  // waiting for task to suspend via quit
    int non_cpu_intensive;
    int want_network;
        // This task wants to do network comm (for F@h)
        // this is passed via share-memory message (app_status channel)
    double abort_time;      // when we sent an abort message to this app
                            // kill it 5 seconds later if it doesn't exit

    APP_CLIENT_SHM app_client_shm;        // core/app shared mem
    MSG_QUEUE graphics_request_queue;
    MSG_QUEUE process_control_queue;

    // info related to app's graphics mode (win, screensaver, etc.)
    //
    int graphics_mode_acked;            // mode acked by app
    int graphics_mode_before_ss;        // mode before last screensaver request
    bool is_ss_app;
    double graphics_mode_ack_timeout;   // safety timeout for Mac screensaver
    bool exit_requested;
#if (defined (__APPLE__) && defined(__i386__))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    int powerpc_emulated_on_i386;
    int is_native_i386_app(char*);
#endif
    GRAPHICS_MSG graphics_msg;
    void request_graphics_mode(GRAPHICS_MSG&);
    int request_reread_prefs();
    int request_reread_app_info();
    void check_graphics_mode_ack();
    int link_user_files();
    int get_shmem_seg_name();

    ACTIVE_TASK();
	~ACTIVE_TASK();
    int init(RESULT*);
    void close_process_handles();
    void cleanup_task();

    int start(bool first_time);         // start a process
    int request_exit();
        // ask the process to exit gracefully,
        // i.e. by sending a <quit> message
    int request_abort();                // send "abort" message
    bool process_exists();
    int kill_task();
        // Kill process forcibly,
        // Unix: send a SIGKILL signal, Windows: TerminateProcess()
    int suspend();
        // ask a process to stop executing (but stay in mem)
        // Done by sending it a <suspend> message
    int unsuspend();
        // Undo a suspend: send a <resume> message
    int abort_task(int exit_status, const char*);
        // can be called whether or not process exists
    bool has_task_exited();             // return true if this task has exited
    int preempt(bool quit_task);        // preempt (via suspend or quit) a running task
    int resume_or_start();
    void send_network_available();
#ifdef _WIN32
    void handle_exited_app(unsigned long);
#else
    void handle_exited_app(int stat);
#endif

    bool check_max_cpu_exceeded();
    bool check_max_disk_exceeded();
    bool check_max_mem_exceeded();

    bool get_app_status_msg();
    bool get_trickle_up_msg();
    double est_cpu_time_to_completion();
    bool read_stderr_file();
    bool finish_file_present();
    bool supports_graphics();
    int write_app_init_file();
    int move_trickle_file();
    int handle_upload_files();
    void upload_notify_app(const FILE_INFO*, const FILE_REF*);

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

class ACTIVE_TASK_SET {
public:
    typedef std::vector<ACTIVE_TASK*> active_tasks_v;
    active_tasks_v active_tasks;
    int remove(ACTIVE_TASK*);
    ACTIVE_TASK* lookup_pid(int);
    ACTIVE_TASK* lookup_result(RESULT*);
    bool poll();
    void suspend_all(bool leave_apps_in_memory=true);
    void unsuspend_all();
    bool is_task_executing();
    int restart_tasks(int max_tasks);
    void request_tasks_exit(PROJECT* p=0);
    int wait_for_exit(double, PROJECT* p=0);
    int exit_tasks(PROJECT* p=0);
    void kill_tasks(PROJECT* p=0);
    int abort_project(PROJECT*);
    bool get_msgs();
    bool check_app_exited();
    bool check_rsc_limits_exceeded();
    bool check_quit_timeout_exceeded();
    bool vm_limit_exceeded(double);
    int get_free_slot();
    void send_heartbeats();
    void send_trickle_downs();
    void report_overdue();
    void handle_upload_files();
    void upload_notify_app(FILE_INFO*);
    bool want_network();    // does any task want network?
    void network_available();   // notify tasks that network is available
    void free_mem();

    // screensaver-related functions
    ACTIVE_TASK* get_ss_app();
    void save_app_modes();
    void hide_apps();
    void restore_apps();
    void graphics_poll();
    void process_control_poll();
    void request_reread_prefs(PROJECT*);
    void request_reread_app_info();

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

#endif
