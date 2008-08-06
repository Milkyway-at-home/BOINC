// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include <string.h>

extern void send_work(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);

extern int add_result_to_reply(
    DB_RESULT& result, WORKUNIT& wu, SCHEDULER_REQUEST&, SCHEDULER_REPLY&,
    BEST_APP_VERSION* bavp
);

inline bool anonymous(PLATFORM* platform) {
    return (!strcmp(platform->name, "anonymous"));
}

extern BEST_APP_VERSION* get_app_version(
    SCHEDULER_REQUEST&, SCHEDULER_REPLY&, WORKUNIT&
);

extern bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av);

// values returned by wu_is_infeasible()
//
#define INFEASIBLE_MEM      1
#define INFEASIBLE_DISK     2
#define INFEASIBLE_CPU      3
#define INFEASIBLE_WORK_BUF 4
#define INFEASIBLE_APP_SETTING 5
#define INFEASIBLE_WORKLOAD 6
#define INFEASIBLE_DUP      7
#define INFEASIBLE_HR       8
#define INFEASIBLE_BANDWIDTH 9

extern int wu_is_infeasible_fast(
    WORKUNIT&, SCHEDULER_REQUEST&, SCHEDULER_REPLY&, APP&
);
 
extern double max_allowable_disk(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);

extern bool wu_already_in_reply(WORKUNIT& wu, SCHEDULER_REPLY& reply);

extern double estimate_cpu_duration(WORKUNIT& wu, SCHEDULER_REPLY& reply);

extern int update_wu_transition_time(WORKUNIT wu, time_t x);

extern void lock_sema();
extern void unlock_sema();
extern const char* infeasible_string(int);
extern bool app_not_selected(WORKUNIT&, SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
