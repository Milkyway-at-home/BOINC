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

// scheduler code related to sending jobs.
// NOTE: there should be nothing here specific to particular
// scheduling policies (array scan, matchmaking, locality)

#include "config.h"
#include <vector>
#include <list>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "str_util.h"
#include "synch.h"

#include "sched_types.h"
#include "sched_shmem.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_main.h"
#include "sched_array.h"
#include "sched_msgs.h"
#include "sched_hr.h"
#include "hr.h"
#include "sched_locality.h"
#include "sched_timezone.h"
#include "sched_assign.h"
#include "sched_customize.h"

#include "sched_send.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

// if host sends us an impossible RAM size, use this instead
//
const double DEFAULT_RAM_SIZE = 64000000;

void send_work_matchmaker();

int preferred_app_message_index=0;

const char* infeasible_string(int code) {
    switch (code) {
    case INFEASIBLE_MEM: return "Not enough memory";
    case INFEASIBLE_DISK: return "Not enough disk";
    case INFEASIBLE_CPU: return "CPU too slow";
    case INFEASIBLE_APP_SETTING: return "App not selected";
    case INFEASIBLE_WORKLOAD: return "Existing workload";
    case INFEASIBLE_DUP: return "Already in reply";
    case INFEASIBLE_HR: return "Homogeneous redundancy";
    case INFEASIBLE_BANDWIDTH: return "Download bandwidth too low";
    }
    return "Unknown";
}

const double MIN_REQ_SECS = 0;
const double MAX_REQ_SECS = (28*SECONDS_IN_DAY);

const int MAX_CUDA_DEVS = 8;
    // don't believe clients who claim they have more CUDA devices than this

// get limits on #jobs per day and per RPC, on in progress
//
void WORK_REQ::get_job_limits() {
    int n;
    n = g_reply->host.p_ncpus;
    if (g_request->global_prefs.max_ncpus_pct && g_request->global_prefs.max_ncpus_pct < 100) {
        n = (int)((n*g_request->global_prefs.max_ncpus_pct)/100.);
    }
    if (n > config.max_ncpus) n = config.max_ncpus;
    if (n < 1) n = 1;
    effective_ncpus = n;

    n = 0;
    COPROC* cp = g_request->coprocs.lookup("CUDA");
    if (cp) {
        n = cp->count;
        if (n > MAX_CUDA_DEVS) n = MAX_CUDA_DEVS;
    }
    effective_ngpus = n;

    int mult = effective_ncpus + config.gpu_multiplier * effective_ngpus;
    if (config.non_cpu_intensive) {
        mult = 1;
        effective_ncpus = 1;
        if (effective_ngpus) effective_ngpus = 1;
    }

    if (config.max_wus_to_send) {
        g_wreq->max_jobs_per_rpc = mult * config.max_wus_to_send;
    } else {
        g_wreq->max_jobs_per_rpc = 999999;
    }

    if (config.daily_result_quota) {
        if (g_reply->host.max_results_day == 0 || g_reply->host.max_results_day>config.daily_result_quota) {
            g_reply->host.max_results_day = config.daily_result_quota;
        }
        g_wreq->max_jobs_per_day = mult * g_reply->host.max_results_day;
    } else {
        g_wreq->max_jobs_per_day = 999999;
    }

    if (config.max_wus_in_progress) {
        g_wreq->max_jobs_on_host_cpu = config.max_wus_in_progress * effective_ncpus;
        if (config.max_wus_in_progress_gpu) {
            g_wreq->max_jobs_on_host_gpu = config.max_wus_in_progress_gpu * effective_ngpus;
            g_wreq->max_jobs_on_host = g_wreq->max_jobs_on_host_cpu + g_wreq->max_jobs_on_host_gpu;
        } else {
            g_wreq->max_jobs_on_host_gpu = 999999;
            g_wreq->max_jobs_on_host = g_wreq->max_jobs_on_host_cpu;
        }
    } else {
        g_wreq->max_jobs_on_host_cpu = 999999;
        g_wreq->max_jobs_on_host = 999999;
        if (config.max_wus_in_progress_gpu) {
            g_wreq->max_jobs_on_host_gpu = config.max_wus_in_progress_gpu * effective_ngpus;
        } else {
            g_wreq->max_jobs_on_host_gpu = 999999;
        }
    }

    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] effective_ncpus %d max_jobs_on_host_cpu %d max_jobs_on_host %d\n",
            effective_ncpus, g_wreq->max_jobs_on_host_cpu, g_wreq->max_jobs_on_host
        );
        log_messages.printf(MSG_NORMAL,
            "[send] effective_ngpus %d max_jobs_on_host_gpu %d\n",
            effective_ngpus, g_wreq->max_jobs_on_host_gpu
        );
    }
}

static const char* find_user_friendly_name(int appid) {
    APP* app = ssp->lookup_app(appid);
    if (app) return app->user_friendly_name;
    return "deprecated application";
}


// Compute the max additional disk usage we can impose on the host.
// Depending on the client version, it can either send us
// - d_total and d_free (pre 4 oct 2005)
// - the above plus d_boinc_used_total and d_boinc_used_project
//
double max_allowable_disk() {
    HOST host = g_request->host;
    GLOBAL_PREFS prefs = g_request->global_prefs;
    double x1, x2, x3, x;

    // defaults are from config.xml
    // if not there these are used:
    // -default_max_used_gb= 100
    // -default_max_used_pct = 50
    // -default_min_free_gb = .001
    //
    if (prefs.disk_max_used_gb == 0) {
       prefs.disk_max_used_gb = config.default_disk_max_used_gb;
    }
    if (prefs.disk_max_used_pct == 0) {
       prefs.disk_max_used_pct = config.default_disk_max_used_pct;
    }
    if (prefs.disk_min_free_gb < config.default_disk_min_free_gb) {
       prefs.disk_min_free_gb = config.default_disk_min_free_gb;
    }

    // no defaults for total/free disk space (host.d_total, d_free)
    // if they're zero, client will get no work.
    //

    if (host.d_boinc_used_total) {
        // The post 4 oct 2005 case.
        // Compute the max allowable additional disk usage based on prefs
        //
        x1 = prefs.disk_max_used_gb*GIGA - host.d_boinc_used_total;
        x2 = host.d_total*prefs.disk_max_used_pct/100.
            - host.d_boinc_used_total;
        x3 = host.d_free - prefs.disk_min_free_gb*GIGA;      // may be negative
        x = std::min(x1, std::min(x2, x3));

        // see which bound is the most stringent
        //
        if (x==x1) {
            g_reply->disk_limits.max_used = x;
        } else if (x==x2) {
            g_reply->disk_limits.max_frac = x;
        } else {
            g_reply->disk_limits.min_free = x;
        }
    } else {
        // here we don't know how much space BOINC is using.
        // so we're kinda screwed.
        // All we can do is assume that BOINC is using zero space.
        // We can't honor the max_used for max_used_pct preferences.
        // We can only honor the min_free pref.
        //
        x = host.d_free - prefs.disk_min_free_gb*GIGA;      // may be negative
        g_reply->disk_limits.min_free = x;
        x1 = x2 = x3 = 0;
    }

    if (x < 0) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] No disk space available: disk_max_used_gb %.2fGB disk_max_used_pct %.2f disk_min_free_gb %.2fGB\n",
                prefs.disk_max_used_gb/GIGA,
                prefs.disk_max_used_pct,
                prefs.disk_min_free_gb/GIGA
            );
            log_messages.printf(MSG_NORMAL,
                "[send] No disk space available: host.d_total %.2fGB host.d_free %.2fGB host.d_boinc_used_total %.2fGB\n",
                host.d_total/GIGA,
                host.d_free/GIGA,
                host.d_boinc_used_total/GIGA
            );
            log_messages.printf(MSG_NORMAL,
                "[send] No disk space available: x1 %.2fGB x2 %.2fGB x3 %.2fGB x %.2fGB\n",
                x1/GIGA, x2/GIGA, x3/GIGA, x/GIGA
            );
        }
        g_wreq->disk.set_insufficient(-x);
        x = 0;
    }
    return x;
}

static double estimate_duration_unscaled(WORKUNIT& wu, BEST_APP_VERSION& bav) {
    double rsc_fpops_est = wu.rsc_fpops_est;
    if (rsc_fpops_est <= 0) rsc_fpops_est = 1e12;
    return rsc_fpops_est/bav.host_usage.flops;
}

static inline void get_running_frac() {
    double rf;
    if (g_request->core_client_version<=419) {
        rf = g_reply->host.on_frac;
    } else {
        rf = g_reply->host.active_frac * g_reply->host.on_frac;
    }

    // clamp running_frac and DCF to a reasonable range
    //
    if (rf > 1) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL, "running_frac=%f; setting to 1\n", rf);
        }
        rf = 1;
    } else if (rf < .1) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL, "running_frac=%f; setting to 0.1\n", rf);
        }
        rf = .1;
    }
    g_wreq->running_frac = rf;
}

static inline void get_dcf() {
    double dcf = g_reply->host.duration_correction_factor;
    if (dcf > 10) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] DCF=%f; setting to 10\n", dcf
            );
        }
        dcf = 10;
    } else if (dcf < 0.1) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] DCF=%f; setting to 0.1\n", dcf
            );
        }
        dcf = 0.1;
    }
    g_wreq->dcf = dcf;
}

// estimate the amount of real time to complete this WU,
// taking into account active_frac etc.
// Note: don't factor in resource_share_fraction.
// The core client no longer necessarily does round-robin
// across all projects.
//
double estimate_duration(WORKUNIT& wu, BEST_APP_VERSION& bav) {
    double edu = estimate_duration_unscaled(wu, bav);
    double ed = edu/g_wreq->running_frac;
    if (!config.ignore_dcf) {
        ed *= g_wreq->dcf;
    }
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] est. duration for WU %d: unscaled %.2f scaled %.2f\n",
            wu.id, edu, ed
        );
    }
    return ed;
}

static void get_prefs_info() {
    char buf[8096];
    std::string str;
    unsigned int pos = 0;
    int temp_int;
    bool flag;

    extract_venue(g_reply->user.project_prefs, g_reply->host.venue, buf);
    str = buf;

    // scan user's project prefs for elements of the form <app_id>N</app_id>,
    // indicating the apps they want to run.
    //
    g_wreq->preferred_apps.clear();
    while (parse_int(str.substr(pos,str.length()-pos).c_str(), "<app_id>", temp_int)) {
        APP_INFO ai;
        ai.appid = temp_int;
        ai.work_available = false;
        g_wreq->preferred_apps.push_back(ai);

        pos = str.find("<app_id>", pos) + 1;
    }
    if (parse_bool(buf,"allow_non_preferred_apps", flag)) {
        g_wreq->allow_non_preferred_apps = flag;
    }
    if (parse_bool(buf,"allow_beta_work", flag)) {
        g_wreq->allow_beta_work = flag;
    }
    if (parse_bool(buf,"no_gpus", flag)) {
        // deprecated, but need to handle
        if (flag) {
            g_wreq->no_cuda = true;
            g_wreq->no_ati = true;
        }
    }
    if (parse_bool(buf,"no_cpu", flag)) {
        g_wreq->no_cpu = flag;
    }
    if (parse_bool(buf,"no_cuda", flag)) {
        g_wreq->no_cuda = flag;
    }
    if (parse_bool(buf,"no_ati", flag)) {
        g_wreq->no_ati = flag;
    }
}

// Find or compute various info about the host;
// this info affects which jobs are sent to the host.
//
static void get_host_info() {
    double expavg_credit = g_reply->host.expavg_credit;
    double expavg_time = g_reply->host.expavg_time;
    update_average(0, 0, CREDIT_HALF_LIFE, expavg_credit, expavg_time);

    // Decide whether or not this computer is 'reliable'
    // A computer is reliable if the following conditions are true
    // (for those that are set in the config file)
    // 1) The host average turnaround is less than the config
    // max average turnaround
    // 2) The host error rate is less then the config max error rate
    // 3) The host results per day is equal to the config file value

    // Platforms other than Windows, Linux and Intel Macs need a
    // larger set of computers to be marked reliable
    //
    double multiplier = 1.0;
    if (strstr(g_reply->host.os_name,"Windows")
        || strstr(g_reply->host.os_name,"Linux")
        || (strstr(g_reply->host.os_name,"Darwin")
            && !(strstr(g_reply->host.p_vendor,"Power Macintosh"))
    )) {
        multiplier = 1.0;
    } else {
        multiplier = 1.8;
    }

    if (g_reply->host.avg_turnaround > 0 && config.reliable_max_avg_turnaround) {

        if (g_reply->host.avg_turnaround > config.reliable_max_avg_turnaround*multiplier) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] not reliable; avg_turn_hrs: %.3f\n",
                    g_reply->host.id, g_reply->host.avg_turnaround/3600
                );
            }
            g_wreq->reliable = false;
            return;
        }
    }
    if (config.reliable_max_error_rate) {
        if (g_reply->host.error_rate > config.reliable_max_error_rate*multiplier) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] not reliable; error rate: %.6f\n",
                    g_reply->host.id, g_reply->host.error_rate
                );
            }
            g_wreq->reliable = false;
            return;
        }
    }
    if (config.daily_result_quota) {
        if (g_reply->host.max_results_day < config.daily_result_quota) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] not reliable; max_result_day %d\n",
                    g_reply->host.id, g_reply->host.max_results_day
                );
            }
            g_wreq->reliable = false;
            return;
        }
    }
    g_wreq->reliable = true;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] [HOST#%d] is reliable\n",
            g_reply->host.id
        );
    }
}

// Return true if the user has set application preferences,
// and this job is not for a selected app
//
bool app_not_selected(WORKUNIT& wu) {
    unsigned int i;

    if (g_wreq->preferred_apps.size() == 0) return false;
    for (i=0; i<g_wreq->preferred_apps.size(); i++) {
        if (wu.appid == g_wreq->preferred_apps[i].appid) {
            g_wreq->preferred_apps[i].work_available = true;
            return false;
        }
    }
    return true;
}

// see how much RAM we can use on this machine
//
static inline void get_mem_sizes() {
    g_wreq->ram = g_reply->host.m_nbytes;
    if (g_wreq->ram <= 0) g_wreq->ram = DEFAULT_RAM_SIZE;
    g_wreq->usable_ram = g_wreq->ram;
    double busy_frac = g_request->global_prefs.ram_max_used_busy_frac;
    double idle_frac = g_request->global_prefs.ram_max_used_idle_frac;
    double frac = 1;
    if (busy_frac>0 && idle_frac>0) {
        frac = std::max(busy_frac, idle_frac);
        if (frac > 1) frac = 1;
        g_wreq->usable_ram *= frac;
    }
}

static inline int check_memory(WORKUNIT& wu) {
    double diff = wu.rsc_memory_bound - g_wreq->usable_ram;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s needs %0.2f MB RAM but only %0.2f MB is available for use.",
            find_user_friendly_name(wu.appid),
            wu.rsc_memory_bound/MEGA, g_wreq->usable_ram/MEGA
        );
        add_no_work_message(message);

        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [WU#%d %s] needs %0.2fMB RAM; [HOST#%d] has %0.2fMB, %0.2fMB usable\n",
                wu.id, wu.name, wu.rsc_memory_bound/MEGA,
                g_reply->host.id, g_wreq->ram/MEGA, g_wreq->usable_ram/MEGA
            );
        }
        g_wreq->mem.set_insufficient(wu.rsc_memory_bound);
        g_reply->set_delay(DELAY_NO_WORK_TEMP);
        return INFEASIBLE_MEM;
    }
    return 0;
}

static inline int check_disk(WORKUNIT& wu) {
    double diff = wu.rsc_disk_bound - g_wreq->disk_available;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s needs %0.2fMB more disk space.  You currently have %0.2f MB available and it needs %0.2f MB.",
            find_user_friendly_name(wu.appid),
            diff/MEGA, g_wreq->disk_available/MEGA, wu.rsc_disk_bound/MEGA
        );
        add_no_work_message(message);

        g_wreq->disk.set_insufficient(diff);
        return INFEASIBLE_DISK;
    }
    return 0;
}

static inline int check_bandwidth(WORKUNIT& wu) {
    if (wu.rsc_bandwidth_bound == 0) return 0;

    // if n_bwdown is zero, the host has never downloaded anything,
    // so skip this check
    //
    if (g_reply->host.n_bwdown == 0) return 0;

    double diff = wu.rsc_bandwidth_bound - g_reply->host.n_bwdown;
    if (diff > 0) {
        char message[256];
        sprintf(message,
            "%s requires %0.2f KB/sec download bandwidth.  Your computer has been measured at %0.2f KB/sec.",
            find_user_friendly_name(wu.appid),
            wu.rsc_bandwidth_bound/KILO, g_reply->host.n_bwdown/KILO
        );
        add_no_work_message(message);

        g_wreq->bandwidth.set_insufficient(diff);
        return INFEASIBLE_BANDWIDTH;
    }
    return 0;
}

// Determine if the app is "hard",
// and we should send it only to high-end hosts.
// Currently this is specified by setting weight=-1;
// this is a kludge for SETI@home/Astropulse.
//
static inline bool hard_app(APP& app) {
    return (app.weight == -1);
}

static inline double get_estimated_delay(BEST_APP_VERSION& bav) {
    if (bav.host_usage.ncudas) {
        return g_request->coproc_cuda->estimated_delay;
    } else if (bav.host_usage.natis) {
        return g_request->coproc_ati->estimated_delay;
    } else {
        return g_request->cpu_estimated_delay;
    }
}

static inline void update_estimated_delay(BEST_APP_VERSION& bav, double dt) {
    if (bav.host_usage.ncudas) {
        g_request->coproc_cuda->estimated_delay += dt;
    } else if (bav.host_usage.natis) {
        g_request->coproc_ati->estimated_delay += dt;
    } else {
        g_request->cpu_estimated_delay += dt;
    }
}

// return the delay bound to use for this job/host.
// Actually, return two: optimistic (lower) and pessimistic (higher).
// If the deadline check with the optimistic bound fails,
// try the pessimistic bound.
//
static void get_delay_bound_range(
    WORKUNIT& wu,
    int res_server_state, int res_priority, double res_report_deadline,
    BEST_APP_VERSION& bav,
    double& opt, double& pess
) {
    if (res_server_state == RESULT_SERVER_STATE_IN_PROGRESS) {
        double now = dtime();
        if (res_report_deadline < now) {
            // if original deadline has passed, return zeros
            // This will skip deadline check.
            opt = pess = 0;
        }
        opt = res_report_deadline - now;
        pess = wu.delay_bound;
    } else {
        opt = pess = wu.delay_bound;

        // If the workunit needs reliable and is being sent to a reliable host,
        // then shorten the delay bound by the percent specified
        //
        if (config.reliable_on_priority && res_priority >= config.reliable_on_priority && config.reliable_reduced_delay_bound > 0.01
        ) {
            opt = wu.delay_bound*config.reliable_reduced_delay_bound;
            double est_wallclock_duration = estimate_duration(wu, bav);
            // Check to see how reasonable this reduced time is.
            // Increase it to twice the estimated delay bound
            // if all the following apply:
            //
            // 1) Twice the estimate is longer then the reduced delay bound
            // 2) Twice the estimate is less then the original delay bound
            // 3) Twice the estimate is less then the twice the reduced delay bound
            if (est_wallclock_duration*2 > opt
                && est_wallclock_duration*2 < wu.delay_bound
                && est_wallclock_duration*2 < wu.delay_bound*config.reliable_reduced_delay_bound*2
            ) {
                opt = est_wallclock_duration*2;
            }
        }
    }
}

// return 0 if the job, with the given delay bound,
// will complete by its deadline, and won't cause other jobs to miss deadlines.
//
static inline int check_deadline(
    WORKUNIT& wu, APP& app, BEST_APP_VERSION& bav
) {
    if (config.ignore_delay_bound) return 0;

    // skip delay check if host currently doesn't have any work
    // and it's not a hard app.
    // (i.e. everyone gets one result, no matter how slow they are)
    //
    if (get_estimated_delay(bav) == 0 && !hard_app(app)) return 0;

    // if it's a hard app, don't send it to a host with no credit
    //
    if (hard_app(app) && g_reply->host.total_credit == 0) {
        return INFEASIBLE_CPU;
    }

    // do EDF simulation if possible; else use cruder approximation
    //
    if (config.workload_sim && g_request->have_other_results_list) {
        double est_dur = estimate_duration(wu, bav);
        if (g_reply->wreq.edf_reject_test(est_dur, wu.delay_bound)) {
            return INFEASIBLE_WORKLOAD;
        }
        IP_RESULT candidate("", wu.delay_bound, est_dur);
        strcpy(candidate.name, wu.name);
        if (check_candidate(candidate, g_wreq->effective_ncpus, g_request->ip_results)) {
            // it passed the feasibility test,
            // but don't add it to the workload yet;
            // wait until we commit to sending it
        } else {
            g_reply->wreq.edf_reject(est_dur, wu.delay_bound);
            g_reply->wreq.speed.set_insufficient(0);
            return INFEASIBLE_WORKLOAD;
        }
    } else {
        double ewd = estimate_duration(wu, bav);
        if (hard_app(app)) ewd *= 1.3;
        double est_completion_delay = get_estimated_delay(bav) + ewd;
        double est_report_delay = std::max(
            est_completion_delay,
            g_request->global_prefs.work_buf_min()
        );
        double diff = est_report_delay - wu.delay_bound;
        if (diff > 0) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [WU#%d] deadline miss %d > %d\n",
                    wu.id, (int)est_report_delay, wu.delay_bound
                );
            }
            g_reply->wreq.speed.set_insufficient(diff);
            return INFEASIBLE_CPU;
        } else {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [WU#%d] meets deadline: %.2f + %.2f < %d\n",
                    wu.id, get_estimated_delay(bav), ewd, wu.delay_bound
                );
            }
        }
    }
    return 0;
}

// Fast checks (no DB access) to see if the job can be sent to the host.
// Reasons why not include:
// 1) the host doesn't have enough memory;
// 2) the host doesn't have enough disk space;
// 3) based on CPU speed, resource share and estimated delay,
//    the host probably won't get the result done within the delay bound
// 4) app isn't in user's "approved apps" list
//
// If the job is feasible, return 0 and fill in wu.delay_bound
// with the delay bound we've decided to use.
//
int wu_is_infeasible_fast(
    WORKUNIT& wu,
    int res_server_state, int res_priority, double res_report_deadline,
    APP& app, BEST_APP_VERSION& bav
) {
    int retval;

    // project-specific check
    //
    if (wu_is_infeasible_custom(wu, app, bav)) {
        return INFEASIBLE_CUSTOM;
    }

    // homogeneous redundancy, quick check
    //
    if (app_hr_type(app)) {
        if (hr_unknown_platform_type(g_reply->host, app_hr_type(app))) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] [WU#%d %s] host is of unknown class in HR type %d\n",
                    g_reply->host.id, wu.id, wu.name, app_hr_type(app)
                );
            }
            return INFEASIBLE_HR;
        }
        if (already_sent_to_different_platform_quick(wu, app)) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [HOST#%d] [WU#%d %s] failed quick HR check: WU is class %d, host is class %d\n",
                    g_reply->host.id, wu.id, wu.name, wu.hr_class, hr_class(g_request->host, app_hr_type(app))
                );
            }
            return INFEASIBLE_HR;
        }
    }

    if (config.one_result_per_user_per_wu || config.one_result_per_host_per_wu) {
        if (wu_already_in_reply(wu)) {
            return INFEASIBLE_DUP;
        }
    }

    retval = check_memory(wu);
    if (retval) return retval;
    retval = check_disk(wu);
    if (retval) return retval;
    retval = check_bandwidth(wu);
    if (retval) return retval;

    if (config.non_cpu_intensive) {
        return 0;
    }

    // do deadline check last because EDF sim uses some CPU
    //
    double opt, pess;
    get_delay_bound_range(
        wu, res_server_state, res_priority, res_report_deadline, bav, opt, pess
    );
    wu.delay_bound = (int)opt;
    if (opt == 0) {
        // this is a resend; skip deadline check
        return 0;
    }
    retval = check_deadline(wu, app, bav);
    if (retval && (opt != pess)) {
        wu.delay_bound = (int)pess;
        retval = check_deadline(wu, app, bav);
    }
    return retval;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, const char* after, const char* text) {
    char* p;
    char temp[BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > BLOB_SIZE-1) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: overflow: %d %d\n", strlen(buffer), strlen(text)
        );
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: %s not found in %s\n", after, buffer
        );
        return ERR_NULL;
    }
    p += strlen(after);
    strcpy(temp, p);
    strcpy(p, text);
    strcat(p, temp);
    return 0;
}

// add elements to WU's xml_doc,
// in preparation for sending it to a client
//
int insert_wu_tags(WORKUNIT& wu, APP& app) {
    char buf[BLOB_SIZE];

    sprintf(buf,
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n",
        wu.rsc_fpops_est,
        wu.rsc_fpops_bound,
        wu.rsc_memory_bound,
        wu.rsc_disk_bound,
        wu.name,
        app.name
    );
    return insert_after(wu.xml_doc, "<workunit>\n", buf);
}

// add the given workunit to a reply.
// Add the app and app_version to the reply also.
//
int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY& reply, APP* app, BEST_APP_VERSION* bavp
) {
    int retval;
    WORKUNIT wu2, wu3;

    APP_VERSION* avp = bavp->avp;

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there
    //
    if (avp) {
        APP_VERSION av2=*avp, *avp2=&av2;

        if (strlen(config.replace_download_url_by_timezone)) {
            process_av_timezone(avp, av2);
        }

        g_reply->insert_app_unique(*app);
        av2.bavp = bavp;
        g_reply->insert_app_version_unique(*avp2);
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [HOST#%d] Sending app_version %s %d %d %s; %.2f GFLOPS\n",
                g_reply->host.id, app->name,
                avp2->platformid, avp2->version_num, avp2->plan_class,
                bavp->host_usage.flops/1e9
            );
        }
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "insert_wu_tags failed %d\n", retval);
        return retval;
    }
    wu3 = wu2;
    if (strlen(config.replace_download_url_by_timezone)) {
        process_wu_timezone(wu2, wu3);
    }

    g_reply->insert_workunit_unique(wu3);

    // switch to tighter policy for estimating delay
    //
    return 0;
}

int insert_name_tags(RESULT& result, WORKUNIT const& wu) {
    char buf[256];
    int retval;

    sprintf(buf, "<name>%s</name>\n", result.name);
    retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    sprintf(buf, "<wu_name>%s</wu_name>\n", wu.name);
    retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

int insert_deadline_tag(RESULT& result) {
    char buf[256];
    sprintf(buf, "<report_deadline>%d</report_deadline>\n", result.report_deadline);
    int retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

int update_wu_transition_time(WORKUNIT wu, time_t x) {
    DB_WORKUNIT dbwu;
    char buf[256];

    dbwu.id = wu.id;

    // SQL note: can't use min() here
    //
    sprintf(buf,
        "transition_time=if(transition_time<%d, transition_time, %d)",
        (int)x, (int)x
    );
    return dbwu.update_field(buf);
}

// return true iff a result for same WU is already being sent
//
bool wu_already_in_reply(WORKUNIT& wu) {
    unsigned int i;
    for (i=0; i<g_reply->results.size(); i++) {
        if (wu.id == g_reply->results[i].workunitid) {
            return true;
        }
    }
    return false;
}

void lock_sema() {
    lock_semaphore(sema_key);
}

void unlock_sema() {
    unlock_semaphore(sema_key);
}

// return true if additional work is needed,
// and there's disk space left,
// and we haven't exceeded result per RPC limit,
// and we haven't exceeded results per day limit
//
bool work_needed(bool locality_sched) {
    if (locality_sched) {
        // if we've failed to send a result because of a transient condition,
        // return false to preserve invariant
        //
        if (g_wreq->disk.insufficient || g_wreq->speed.insufficient || g_wreq->mem.insufficient || g_wreq->no_allowed_apps_available) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] stopping work search - locality condition\n"
                );
            }
            return false;
        }
    }

    if (g_reply->host.nresults_today >= g_wreq->max_jobs_per_day) {
        g_wreq->daily_result_quota_exceeded = true;
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] stopping work search - daily quota exceeded (%d>=%d)\n",
                g_reply->host.nresults_today, g_wreq->max_jobs_per_day
            );
        }
        return false;
    }

    if (g_wreq->njobs_on_host >= g_wreq->max_jobs_on_host) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] in-progress job limit exceeded; %d >= %d\n",
                g_wreq->njobs_on_host, g_wreq->max_jobs_on_host
            );
        }
        g_wreq->max_jobs_on_host_exceeded = true;
        return false;
    }

    if (g_wreq->njobs_on_host_cpu >= g_wreq->max_jobs_on_host_cpu) {
        g_wreq->clear_cpu_req();
        g_wreq->max_jobs_on_host_cpu_exceeded = true;
    }
    if (g_wreq->njobs_on_host_gpu >= g_wreq->max_jobs_on_host_gpu) {
        g_wreq->clear_gpu_req();
        if (g_wreq->effective_ngpus) {
            g_wreq->max_jobs_on_host_gpu_exceeded = true;
        }
    }

    if (g_wreq->njobs_sent >= g_wreq->max_jobs_per_rpc) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] stopping work search - njobs %d >= max_jobs_per_rpc %d\n",
                g_wreq->njobs_sent, g_wreq->max_jobs_per_rpc
            );
        }
        return false;
    }

#if 0
    log_messages.printf(MSG_NORMAL,
        "work_needed: spec req %d sec to fill %.2f; CPU (%.2f, %.2f) CUDA (%.2f, %.2f) ATI(%.2f, %.2f)\n",
        g_wreq->rsc_spec_request,
        g_wreq->seconds_to_fill,
        g_wreq->cpu_req_secs, g_wreq->cpu_req_instances,
        g_wreq->cuda_req_secs, g_wreq->cuda_req_instances,
        g_wreq->ati_req_secs, g_wreq->ati_req_instances
    );
#endif
    if (g_wreq->rsc_spec_request) {
        if (g_wreq->need_cpu()) {
            return true;
        }
        if (g_wreq->need_cuda()) {
            return true;
        }
        if (g_wreq->need_ati()) {
            return true;
        }
    } else {
        if (g_wreq->seconds_to_fill > 0) {
            return true;
        }
    }
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL, "[send] don't need more work\n");
    }
    return false;
}

int add_result_to_reply(
    DB_RESULT& result, WORKUNIT& wu, BEST_APP_VERSION* bavp,
    bool locality_scheduling
) {
    int retval;
    bool resent_result = false;
    APP* app = ssp->lookup_app(wu.appid);

    retval = add_wu_to_reply(wu, *g_reply, app, bavp);
    if (retval) return retval;

    // Adjust available disk space.
    // In the scheduling locality case,
    // reduce the available space by less than the workunit rsc_disk_bound,
    // if the host already has the file or the file was not already sent.
    //
    if (!locality_scheduling || decrement_disk_space_locality(wu)) {
        g_wreq->disk_available -= wu.rsc_disk_bound;
    }

    // update the result in DB
    //
    result.hostid = g_reply->host.id;
    result.userid = g_reply->user.id;
    result.sent_time = time(0);
    result.report_deadline = result.sent_time + wu.delay_bound;
    result.flops_estimate = bavp->host_usage.flops;
    if (bavp->avp) {
        result.app_version_id = bavp->avp->id;
    } else {
        result.app_version_id = -1;
    }
    int old_server_state = result.server_state;

    if (result.server_state != RESULT_SERVER_STATE_IN_PROGRESS) {
        // We're sending this result for the first time
        //
        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
    } else {
        // Result was already sent to this host but was lost,
        // so we're resending it.
        //
        resent_result = true;

        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [RESULT#%d] [HOST#%d] (resend lost work)\n",
                result.id, g_reply->host.id
            );
        }
    }
    retval = result.mark_as_sent(old_server_state);
    if (retval == ERR_DB_NOT_FOUND) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%d] [HOST#%d]: CAN'T SEND, already sent to another host\n",
            result.id, g_reply->host.id
        );
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't update result: %d\n", retval
        );
    }
    if (retval) return retval;

    double est_dur = estimate_duration(wu, *bavp);
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[HOST#%d] Sending [RESULT#%d %s] (est. dur. %.2f seconds)\n",
            g_reply->host.id, result.id, result.name, est_dur
        );
    }

    retval = update_wu_transition_time(wu, result.report_deadline);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't update WU transition time: %d\n",
            retval
        );
        return retval;
    }

    // The following overwrites the result's xml_doc field.
    // But that's OK cuz we're done with DB updates
    //
    retval = insert_name_tags(result, wu);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't insert name tags: %d\n",
            retval
        );
        return retval;
    }
    retval = insert_deadline_tag(result);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't insert deadline tag: %d\n", retval
        );
        return retval;
    }
    result.bavp = bavp;
    g_reply->insert_result(result);
    if (g_wreq->rsc_spec_request) {
        if (bavp->host_usage.ncudas) {
            g_wreq->cuda_req_secs -= est_dur;
            g_wreq->cuda_req_instances -= bavp->host_usage.ncudas;
        } else if (bavp->host_usage.natis) {
            g_wreq->ati_req_secs -= est_dur;
            g_wreq->ati_req_instances -= bavp->host_usage.natis;
        } else {
            g_wreq->cpu_req_secs -= est_dur;
            g_wreq->cpu_req_instances -= bavp->host_usage.avg_ncpus;
        }
    } else {
        g_wreq->seconds_to_fill -= est_dur;
    }
    update_estimated_delay(*bavp, est_dur);
    g_wreq->njobs_sent++;
    g_wreq->njobs_on_host++;
    if (bavp->host_usage.ncudas > 0) {
        g_wreq->njobs_on_host_gpu++;
    } else if (bavp->host_usage.natis > 0) {
        g_wreq->njobs_on_host_gpu++;
    } else {
        g_wreq->njobs_on_host_cpu++;
    }
    if (!resent_result) g_reply->host.nresults_today++;

    // add this result to workload for simulation
    //
    if (config.workload_sim && g_request->have_other_results_list) {
        IP_RESULT ipr ("", time(0)+wu.delay_bound, est_dur);
        g_request->ip_results.push_back(ipr);
    }

    // mark job as done if debugging flag is set;
    // this is used by sched_driver.C (performance testing)
    //
    if (mark_jobs_done) {
        DB_WORKUNIT dbwu;
        char buf[256];
        sprintf(buf,
            "server_state=%d outcome=%d",
            RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS
        );
        result.update_field(buf);

        dbwu.id = wu.id;
        sprintf(buf, "transition_time=%ld", time(0));
        dbwu.update_field(buf);

    }

    // If we're sending an unreplicated job to an untrusted host,
    // mark it as replicated
    //
    if (wu.target_nresults == 1 && app->target_nresults > 1) {
        if (g_wreq->trust) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [WU#%d] sending to trusted host, not replicating\n", wu.id
                );
            }
        } else {
            DB_WORKUNIT dbwu;
            char buf[256];
            sprintf(buf,
                "target_nresults=%d, min_quorum=%d, transition_time=%ld",
                app->target_nresults, app->target_nresults, time(0)
            );
            dbwu.id = wu.id;
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [WU#%d] sending to untrusted host, replicating\n", wu.id
                );
            }
            retval = dbwu.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "WU update failed: %d", retval
                );
            }
        }
    }

    return 0;
}

// send messages to user about why jobs were or weren't sent
//
static void explain_to_user() {
    char helpful[512];
    unsigned int i;
    int j;

    // If work was sent from apps the user did not select, explain.
    // NOTE: this will have to be done differently with matchmaker scheduling
    //
    if (!config.locality_scheduling && !config.matchmaker) {
        if (g_wreq->njobs_sent && !g_wreq->user_apps_only) {
            g_reply->insert_message(
                "No work can be sent for the applications you have selected",
                "high"
            );

            // Inform the user about applications with no work
            //
            for (i=0; i<g_wreq->preferred_apps.size(); i++) {
                if (!g_wreq->preferred_apps[i].work_available) {
                    APP* app = ssp->lookup_app(g_wreq->preferred_apps[i].appid);
                    // don't write message if the app is deprecated
                    //
                    if (app) {
                        char explanation[256];
                        sprintf(explanation,
                            "No work is available for %s",
                            find_user_friendly_name(g_wreq->preferred_apps[i].appid)
                        );
                        g_reply->insert_message( explanation, "high");
                    }
                }
            }

            // Tell the user about applications they didn't qualify for
            //
            for (j=0; j<preferred_app_message_index; j++){
                g_reply->insert_message(g_wreq->no_work_messages.at(j));
            }
            g_reply->insert_message(
                "You have selected to receive work from other applications if no work is available for the applications you selected",
                "high"
            );
            g_reply->insert_message(
                "Sending work from other applications", "high"
            );
        }
    }

    // if client asked for work and we're not sending any, explain why
    //
    if (g_wreq->njobs_sent == 0) {
        g_reply->set_delay(DELAY_NO_WORK_TEMP);
        g_reply->insert_message("No work sent", "high");

        // Tell the user about applications with no work
        //
        for (i=0; i<g_wreq->preferred_apps.size(); i++) {
             if (!g_wreq->preferred_apps[i].work_available) {
                 APP* app = ssp->lookup_app(g_wreq->preferred_apps[i].appid);
                 // don't write message if the app is deprecated
                 if (app != NULL) {
                       char explanation[256];
                       sprintf(explanation, "No work is available for %s",
                        find_user_friendly_name(g_wreq->preferred_apps[i].appid)
                    );
                       g_reply->insert_message(explanation, "high");
                 }
               }
        }

        // Tell the user about applications they didn't qualify for
        //
        for (i=0; i<g_wreq->no_work_messages.size(); i++){
            g_reply->insert_message(g_wreq->no_work_messages.at(i));
        }
        if (g_wreq->no_allowed_apps_available) {
            g_reply->insert_message(
                "No work available for the applications you have selected.  Please check your settings on the web site.",
                "high"
            );
        }
        if (g_wreq->speed.insufficient) {
            if (g_request->core_client_version>419) {
                sprintf(helpful,
                    "(won't finish in time) "
                    "BOINC runs %.1f%% of time, computation enabled %.1f%% of that",
                    100.0*g_reply->host.on_frac, 100.0*g_reply->host.active_frac
                );
            } else {
                sprintf(helpful,
                    "(won't finish in time) "
                    "Computer available %.1f%% of time",
                    100.0*g_reply->host.on_frac
                );
            }
            g_reply->insert_message(helpful, "high");
        }
        if (g_wreq->hr_reject_temp) {
            g_reply->insert_message(
                "(there was work but it was committed to other platforms)",
                "high"
            );
        }
        if (g_wreq->hr_reject_perm) {
            g_reply->insert_message(
                "(your platform is not supported by this project)",
                "high"
            );
        }
        if (g_wreq->outdated_client) {
            g_reply->insert_message(
                " (your BOINC client is old - please install current version)",
                "high"
            );
            g_reply->set_delay(DELAY_NO_WORK_PERM);
            log_messages.printf(MSG_NORMAL,
                "Not sending work because client is outdated\n"
            );
        }
        if (g_wreq->excessive_work_buf) {
            g_reply->insert_message(
                "(Your network connection interval is longer than WU deadline)",
                "high"
            );
        }
        if (g_wreq->no_cuda_prefs) {
            g_reply->insert_message(
                "Jobs for NVIDIA GPU are available, but your preferences are set to not accept them",
                "low"
            );
        }
        if (g_wreq->no_ati_prefs) {
            g_reply->insert_message(
                "Jobs for ATI GPU are available, but your preferences are set to not accept them",
                "low"
            );
        }
        if (g_wreq->no_cpu_prefs) {
            g_reply->insert_message(
                "Jobs for CPU are available, but your preferences are set to not accept them",
                "low"
            );
        }
        if (g_wreq->daily_result_quota_exceeded) {
            struct tm *rpc_time_tm;
            int delay_time;

            sprintf(helpful, "(reached daily quota of %d tasks)",
                g_wreq->max_jobs_per_day
            );
            g_reply->insert_message(helpful, "high");
            log_messages.printf(MSG_NORMAL,
                "Daily result quota %d exceeded for host %d\n",
                g_wreq->max_jobs_per_day, g_reply->host.id
            );

            // set delay so host won't return until a random time in
            // the first hour of the next day.
            // This is to prevent a lot of hosts from flooding the scheduler
            // with requests at the same time of day.
            //
            time_t t = g_reply->host.rpc_time;
            rpc_time_tm = localtime(&t);
            delay_time  = (23 - rpc_time_tm->tm_hour) * 3600
                + (59 - rpc_time_tm->tm_min) * 60
                + (60 - rpc_time_tm->tm_sec)
                + (int)(3600*(double)rand()/(double)RAND_MAX);
            g_reply->set_delay(delay_time);
        }
        if (g_wreq->max_jobs_on_host_exceeded) {
            sprintf(helpful, "(reached limit of %d tasks in progress)",
                g_wreq->max_jobs_on_host
            );
            g_reply->insert_message(helpful, "high");
            g_reply->set_delay(DELAY_NO_WORK_CACHE);
            log_messages.printf(MSG_NORMAL,
                "host %d already has %d job(s) in progress\n",
                g_reply->host.id, g_wreq->njobs_on_host
            );
        }
        if (g_wreq->max_jobs_on_host_cpu_exceeded) {
            sprintf(helpful, "(reached limit of %d CPU tasks in progress)",
                g_wreq->max_jobs_on_host_cpu
            );
            g_reply->insert_message(helpful, "high");
            g_reply->set_delay(DELAY_NO_WORK_CACHE);
            log_messages.printf(MSG_NORMAL,
                "host %d already has %d CPU job(s) in progress\n",
                g_reply->host.id, g_wreq->njobs_on_host_cpu
            );
        }
        if (g_wreq->max_jobs_on_host_gpu_exceeded) {
            sprintf(helpful, "(reached limit of %d GPU tasks in progress)",
                g_wreq->max_jobs_on_host_gpu
            );
            g_reply->insert_message(helpful, "high");
            g_reply->set_delay(DELAY_NO_WORK_CACHE);
            log_messages.printf(MSG_NORMAL,
                "host %d already has %d GPU job(s) in progress\n",
                g_reply->host.id, g_wreq->njobs_on_host_gpu
            );
        }
    }
}

#define ER_MAX  0.05
// decide whether to unreplicated jobs to this host
//
void set_trust() {
    g_wreq->trust = false;
    if (g_reply->host.error_rate > ER_MAX) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] set_trust: error rate %f > %f, don't trust\n",
                g_reply->host.error_rate, ER_MAX
            );
        }
        return;
    }
    double x = sqrt(g_reply->host.error_rate/ER_MAX);
    if (drand() > x) g_wreq->trust = true;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] set_trust: random choice for error rate %f: %s\n",
            g_reply->host.error_rate, g_wreq->trust?"yes":"no"
        );
    }
}

static double clamp_req_sec(double x) {
    if (x < MIN_REQ_SECS) return MIN_REQ_SECS;
    if (x > MAX_REQ_SECS) return MAX_REQ_SECS;
    return x;
}

// decipher request type, fill in WORK_REQ
//
void send_work_setup() {
    unsigned int i;

    g_wreq->disk_available = max_allowable_disk();
    get_mem_sizes();
    get_running_frac();
    get_dcf();
    g_wreq->get_job_limits();

    g_wreq->seconds_to_fill = clamp_req_sec(g_request->work_req_seconds);
    g_wreq->cpu_req_secs = clamp_req_sec(g_request->cpu_req_secs);
    g_wreq->cpu_req_instances = g_request->cpu_req_instances;
    g_wreq->anonymous_platform = anonymous(g_request->platforms.list[0]);

    if (g_request->coproc_cuda) {
        g_wreq->cuda_req_secs = clamp_req_sec(g_request->coproc_cuda->req_secs);
        g_wreq->cuda_req_instances = g_request->coproc_cuda->req_instances;
        if (g_request->coproc_cuda->estimated_delay < 0) {
            g_request->coproc_cuda->estimated_delay = g_request->cpu_estimated_delay;
        }
    }
    if (g_request->coproc_ati) {
        g_wreq->ati_req_secs = clamp_req_sec(g_request->coproc_ati->req_secs);
        g_wreq->ati_req_instances = g_request->coproc_ati->req_instances;
        if (g_request->coproc_ati->estimated_delay < 0) {
            g_request->coproc_ati->estimated_delay = g_request->cpu_estimated_delay;
        }
    }
    if (g_wreq->cpu_req_secs || g_wreq->cuda_req_secs || g_wreq->ati_req_secs) {
        g_wreq->rsc_spec_request = true;
    } else {
        g_wreq->rsc_spec_request = false;
    }

    // print details of request to log
    //
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] %s matchmaker scheduling; %s EDF sim\n",
            config.matchmaker?"Using":"Not using",
            config.workload_sim?"Using":"Not using"
        );
        log_messages.printf(MSG_NORMAL,
            "[send] CPU: req %.2f sec, %.2f instances; est delay %.2f\n",
            g_wreq->cpu_req_secs, g_wreq->cpu_req_instances,
            g_request->cpu_estimated_delay
        );
        if (g_request->coproc_cuda) {
            log_messages.printf(MSG_NORMAL,
                "[send] CUDA: req %.2f sec, %.2f instances; est delay %.2f\n",
                g_wreq->cuda_req_secs, g_wreq->cuda_req_instances,
                g_request->coproc_cuda->estimated_delay
            );
        }
        if (g_request->coproc_ati) {
            log_messages.printf(MSG_NORMAL,
                "[send] ATI: req %.2f sec, %.2f instances; est delay %.2f\n",
                g_wreq->ati_req_secs, g_wreq->ati_req_instances,
                g_request->coproc_ati->estimated_delay
            );
        }
        log_messages.printf(MSG_NORMAL,
            "[send] work_req_seconds: %.2f secs\n",
            g_wreq->seconds_to_fill
        );
        log_messages.printf(MSG_NORMAL,
            "[send] available disk %.2f GB, work_buf_min %d\n",
            g_wreq->disk_available/GIGA,
            (int)g_request->global_prefs.work_buf_min()
        );
        log_messages.printf(MSG_NORMAL,
            "[send] active_frac %f on_frac %f DCF %f\n",
            g_reply->host.active_frac,
            g_reply->host.on_frac,
            g_reply->host.duration_correction_factor
        );
        if (g_wreq->anonymous_platform) {
            log_messages.printf(MSG_NORMAL,
                "Anonymous platform app versions:\n"
            );
            for (i=0; i<g_request->client_app_versions.size(); i++) {
                CLIENT_APP_VERSION& cav = g_request->client_app_versions[i];
                log_messages.printf(MSG_NORMAL,
                    "   app: %s ver: %d\n",
                    cav.app_name, cav.version_num
                );
            }
        }
    }
}

void send_work() {
    if (!g_wreq->rsc_spec_request && g_wreq->seconds_to_fill == 0) {
        return;
    }

    if (all_apps_use_hr && hr_unknown_platform(g_request->host)) {
        log_messages.printf(MSG_NORMAL,
            "Not sending work because unknown HR class\n"
        );
        g_wreq->hr_reject_perm = true;
        return;
    }

    get_host_info();
    get_prefs_info();

    set_trust();

    if (config.enable_assignment) {
        if (send_assigned_jobs()) {
            if (config.debug_assignment) {
                log_messages.printf(MSG_NORMAL,
                    "[assign] [HOST#%d] sent assigned jobs\n", g_reply->host.id
                );
            }
            return;
        }
    }

    if (config.workload_sim && g_request->have_other_results_list) {
        init_ip_results(
            g_request->global_prefs.work_buf_min(),
            g_wreq->effective_ncpus, g_request->ip_results
        );
    }

    if (config.locality_scheduler_fraction > 0) {
        if (drand() < config.locality_scheduler_fraction) {
           if (config.debug_locality)
             log_messages.printf(MSG_NORMAL, "[mixed] sending locality work first\n");
           send_work_locality();
           if (config.debug_locality)
             log_messages.printf(MSG_NORMAL, "[mixed] sending non-locality work second\n");
           send_work_old();
        } else {
           if (config.debug_locality)
             log_messages.printf(MSG_NORMAL, "[mixed] sending non-locality work first\n");
           send_work_old();
           if (config.debug_locality)
             log_messages.printf(MSG_NORMAL, "[mixed] sending locality work second\n");
           send_work_locality();
        }
    } else if (config.locality_scheduling) {
        send_work_locality();
    } else if (config.matchmaker) {
        send_work_matchmaker();
    } else {
        send_work_old();
    }

    explain_to_user();
}

const char *BOINC_RCSID_32dcd335e7 = "$Id$";
