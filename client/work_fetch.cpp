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

#include "util.h"

#include "client_types.h"
#include "client_msgs.h"
#ifdef SIM
#include "sim.h"
#else
#include "client_state.h"
#endif

#include "scheduler_op.h"
#include "work_fetch.h"

using std::vector;

RSC_WORK_FETCH cuda_work_fetch;
RSC_WORK_FETCH ati_work_fetch;
RSC_WORK_FETCH cpu_work_fetch;
WORK_FETCH work_fetch;

#define MIN_BACKOFF_INTERVAL    60
#define MAX_BACKOFF_INTERVAL    86400
    // if we ask a project for work for a resource and don't get it,
    // we do exponential backoff.
    // This constant is an upper bound for this.
    // E.g., if we need GPU work, we'll end up asking once a day,
    // so if the project develops a GPU app,
    // we'll find out about it within a day.

static inline const char* rsc_name(int t) {
    switch (t) {
    case RSC_TYPE_CPU: return "CPU";
    case RSC_TYPE_CUDA: return "NVIDIA GPU";
    case RSC_TYPE_ATI: return "ATI GPU";
    }
    return "Unknown";
}

RSC_PROJECT_WORK_FETCH& RSC_WORK_FETCH::project_state(PROJECT* p) {
    switch(rsc_type) {
    case RSC_TYPE_CPU: return p->cpu_pwf;
    case RSC_TYPE_CUDA: return p->cuda_pwf;
    case RSC_TYPE_ATI: return p->ati_pwf;
    default: return p->cpu_pwf;
    }
}

bool RSC_WORK_FETCH::may_have_work(PROJECT* p) {
    switch(rsc_type) {
    case RSC_TYPE_CPU:
        if (p->no_cpu_pref) return false;
    case RSC_TYPE_CUDA:
        if (p->no_cuda_pref) return false;
    case RSC_TYPE_ATI:
        if (p->no_ati_pref) return false;
    }
    RSC_PROJECT_WORK_FETCH& w = project_state(p);
    return (w.backoff_time < gstate.now);
}

bool RSC_PROJECT_WORK_FETCH::compute_may_have_work(PROJECT* p, int rsc_type) {
    switch(rsc_type) {
    case RSC_TYPE_CPU: if (p->no_cpu_pref) return false; break;
    case RSC_TYPE_CUDA: if (p->no_cuda_pref) return false; break;
    case RSC_TYPE_ATI: if (p->no_ati_pref) return false; break;
    }
    return (backoff_time < gstate.now);
}

void RSC_PROJECT_WORK_FETCH::rr_init(PROJECT* p, int rsc_type) {
    may_have_work = compute_may_have_work(p, rsc_type);
    runnable_share = 0;
    fetchable_share = 0;
    has_runnable_jobs = false;
    sim_nused = 0;
    deadlines_missed = 0;
}

void RSC_WORK_FETCH::rr_init() {
    shortfall = 0;
    nidle_now = 0;
    sim_nused = 0;
    total_fetchable_share = 0;
    total_runnable_share = 0;
    deadline_missed_instances = 0;
    saturated_time = 0;
    pending.clear();
    busy_time_estimator.reset();
}

void WORK_FETCH::rr_init() {
    cpu_work_fetch.rr_init();
    if (coproc_cuda) {
        cuda_work_fetch.rr_init();
    }
    if (coproc_ati) {
        ati_work_fetch.rr_init();
    }
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.can_fetch_work = p->pwf.compute_can_fetch_work(p);
        p->pwf.has_runnable_jobs = false;
        p->cpu_pwf.rr_init(p, RSC_TYPE_CPU);
        if (coproc_cuda) {
            p->cuda_pwf.rr_init(p, RSC_TYPE_CUDA);
        }
        if (coproc_ati) {
            p->ati_pwf.rr_init(p, RSC_TYPE_ATI);
        }
    }
}

bool PROJECT_WORK_FETCH::compute_can_fetch_work(PROJECT* p) {
    if (p->non_cpu_intensive) return false;
    if (p->suspended_via_gui) return false;
    if (p->master_url_fetch_pending) return false;
    if (p->min_rpc_time > gstate.now) return false;
    if (p->dont_request_more_work) return false;
    if (p->some_download_stalled()) return false;
    if (p->some_result_suspended()) return false;
    if (p->too_many_uploading_results) return false;
    return true;
}

void PROJECT_WORK_FETCH::reset(PROJECT* p) {
    p->cpu_pwf.reset();
    p->cuda_pwf.reset();
    p->ati_pwf.reset();
}

void RSC_WORK_FETCH::accumulate_shortfall(double d_time) {
    double idle = ninstances - sim_nused;
    if (idle > 1e-6) {
        shortfall += idle*d_time;
    }
#if 0
    msg_printf(0, MSG_INFO, "accum shortf (%s): idle %f dt %f sf %f",
        rsc_name(rsc_type), idle, d_time, shortfall
    );
#endif
}

void RSC_WORK_FETCH::update_saturated_time(double dt) {
    double idle = ninstances - sim_nused;
    if (idle < 1e-6) {
        saturated_time = dt;
    }
}

void RSC_WORK_FETCH::update_busy_time(double dur, double nused) {
    busy_time_estimator.update(dur, nused);
}

// see if the project's debt is beyond what would normally happen;
// if so we conclude that it had a long job that ran in EDF mode;
// avoid asking it for work unless absolutely necessary.
//
bool RSC_PROJECT_WORK_FETCH::overworked() {
    double x = gstate.work_buf_total() + gstate.global_prefs.cpu_scheduling_period(); 
    if (x < 86400) x = 86400;
    return (debt < -x);
}

#define FETCH_IF_IDLE_INSTANCE          0
    // If resource has an idle instance,
    // get work for it from the project with greatest LTD,
    // even if it's overworked.
#define FETCH_IF_MAJOR_SHORTFALL        1
    // If resource is saturated for less than work_buf_min(),
    // get work for it from the project with greatest LTD,
    // even if it's overworked.
#define FETCH_IF_MINOR_SHORTFALL        2
    // If resource is saturated for less than work_buf_total(),
    // get work for it from the non-overworked project with greatest LTD.
#define FETCH_IF_PROJECT_STARVED        3
    // If any project is not overworked and has no runnable jobs
    // (for any resource, not just this one)
    // get work from the one with greatest LTD.

// Choose the best project to ask for work for this resource,
// given the specific criterion
//
PROJECT* RSC_WORK_FETCH::choose_project(int criterion) {
    double req;
    PROJECT* pbest = NULL;

    switch (criterion) {
    case FETCH_IF_IDLE_INSTANCE:
        if (nidle_now == 0) return NULL;
        break;
    case FETCH_IF_MAJOR_SHORTFALL:
        if (saturated_time > gstate.work_buf_min()) return NULL;
        break;
    case FETCH_IF_MINOR_SHORTFALL:
        if (saturated_time > gstate.work_buf_total()) return NULL;
        break;
    case FETCH_IF_PROJECT_STARVED:
        if (deadline_missed_instances >= ninstances) return NULL;
        break;
    }

    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->pwf.can_fetch_work) continue;
        if (!project_state(p).may_have_work) continue;
        RSC_PROJECT_WORK_FETCH& rpwf = project_state(p);
        switch (criterion) {
        case FETCH_IF_MINOR_SHORTFALL:
            if (rpwf.overworked()) continue;
            break;
        case FETCH_IF_PROJECT_STARVED:
            if (rpwf.overworked()) continue;
            if (p->pwf.has_runnable_jobs) continue;
            break;
        }
        if (pbest) {
            if (pbest->pwf.overall_debt > p->pwf.overall_debt) {
                continue;
            }

        }
        pbest = p;
    }
    if (!pbest) return NULL;

    // decide how much work to request from each resource
    //
    work_fetch.clear_request();
    switch (criterion) {
    case FETCH_IF_IDLE_INSTANCE:
        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO,
                "chosen: %s idle instance", rsc_name(rsc_type)
            );
        }
        req = share_request(pbest);
        if (req > shortfall) req = shortfall;
        set_request(pbest, req);
        break;
    case FETCH_IF_MAJOR_SHORTFALL:
        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO,
                "chosen: %s major shortfall", rsc_name(rsc_type)
            );
        }
        req = share_request(pbest);
        if (req > shortfall) req = shortfall;
        set_request(pbest, req);
        break;
    case FETCH_IF_MINOR_SHORTFALL:
        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO,
                "chosen: %s minor shortfall", rsc_name(rsc_type)
            );
        }
        work_fetch.set_shortfall_requests(pbest);
        break;
    case FETCH_IF_PROJECT_STARVED:
        if (log_flags.work_fetch_debug) {
            msg_printf(pbest, MSG_INFO,
                "chosen: %s starved", rsc_name(rsc_type)
            );
        }
        req = share_request(pbest);
        set_request(pbest, req);
        break;
    }
    return pbest;
}

void WORK_FETCH::set_shortfall_requests(PROJECT* p) {
    cpu_work_fetch.set_shortfall_request(p);
    if (coproc_cuda && coproc_cuda->usable) {
        cuda_work_fetch.set_shortfall_request(p);
    }
    if (coproc_ati && coproc_ati->usable) {
        ati_work_fetch.set_shortfall_request(p);
    }
}

void RSC_WORK_FETCH::set_shortfall_request(PROJECT* p) {
    if (!shortfall) return;
    RSC_PROJECT_WORK_FETCH& w = project_state(p);
    if (!w.may_have_work) return;
    if (w.overworked()) return;
    set_request(p, shortfall);
}

void WORK_FETCH::set_overall_debts() {
    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->pwf.overall_debt = p->cpu_pwf.debt;
        if (coproc_cuda) {
            p->pwf.overall_debt += cuda_work_fetch.speed*p->cuda_pwf.debt;
        }
        if (coproc_ati) {
            p->pwf.overall_debt += ati_work_fetch.speed*p->ati_pwf.debt;
        }
    }
}

void WORK_FETCH::zero_debts() {
    for (unsigned i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->cpu_pwf.debt = 0;
        if (coproc_cuda) {
            p->cuda_pwf.debt = 0;
        }
        if (coproc_ati) {
            p->ati_pwf.debt = 0;
        }
    }
}

void RSC_WORK_FETCH::print_state(const char* name) {
    msg_printf(0, MSG_INFO,
        "[wfd] %s: shortfall %.2f nidle %.2f saturated %.2f busy %.2f RS fetchable %.2f runnable %.2f",
        name,
        shortfall, nidle_now, saturated_time, busy_time_estimator.get_busy_time(),
        total_fetchable_share, total_runnable_share
    );
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        RSC_PROJECT_WORK_FETCH& pwf = project_state(p);
        double bt = pwf.backoff_time>gstate.now?pwf.backoff_time-gstate.now:0;
        bool blocked_by_prefs = false;
        switch (rsc_type) {
        case RSC_TYPE_CPU:
            if (p->no_cpu_pref) blocked_by_prefs = true;
            break;
        case RSC_TYPE_CUDA:
            if (p->no_cuda_pref) blocked_by_prefs = true;
            break;
        case RSC_TYPE_ATI:
            if (p->no_ati_pref) blocked_by_prefs = true;
            break;
        }
        msg_printf(p, MSG_INFO,
            "[wfd] %s: fetch share %.2f debt %.2f backoff dt %.2f int %.2f%s%s%s%s%s%s%s",
            name,
            pwf.fetchable_share, pwf.debt, bt, pwf.backoff_interval,
            p->suspended_via_gui?" (susp via GUI)":"",
            p->master_url_fetch_pending?" (master fetch pending)":"",
            p->min_rpc_time > gstate.now?" (comm deferred)":"",
            p->dont_request_more_work?" (no new tasks)":"",
            pwf.overworked()?" (overworked)":"",
            p->too_many_uploading_results?" (too many uploads)":"",
            blocked_by_prefs?" (blocked by prefs)":""
        );
    }
}

void WORK_FETCH::print_state() {
    msg_printf(0, MSG_INFO, "[wfd] ------- start work fetch state -------");
    msg_printf(0, MSG_INFO, "[wfd] target work buffer: %.2f + %.2f sec",
        gstate.work_buf_min(), gstate.work_buf_additional()
    );
    cpu_work_fetch.print_state("CPU");
    if (coproc_cuda) {
        cuda_work_fetch.print_state("NVIDIA GPU");
    }
    if (coproc_ati) {
        ati_work_fetch.print_state("ATI GPU");
    }
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        msg_printf(p, MSG_INFO, "[wfd] overall_debt %.0f", p->pwf.overall_debt);
    }
    msg_printf(0, MSG_INFO, "[wfd] ------- end work fetch state -------");
}

void RSC_WORK_FETCH::clear_request() {
    req_secs = 0;
    req_instances = 0;
}

void WORK_FETCH::clear_request() {
    cpu_work_fetch.clear_request();
    cuda_work_fetch.clear_request();
    ati_work_fetch.clear_request();
}

// does the project have a downloading or runnable job?
//
static bool has_a_job(PROJECT* p) {
    for (unsigned int j=0; j<gstate.results.size(); j++) {
        RESULT* rp = gstate.results[j];
        if (rp->project != p) continue;
        if (rp->state() <= RESULT_FILES_DOWNLOADED) {
            return true;
        }
    }
    return false;
}

// we're going to contact this project for reasons other than work fetch;
// decide if we should piggy-back a work fetch request.
//
void WORK_FETCH::compute_work_request(PROJECT* p) {
    clear_request();
    if (p->dont_request_more_work) return;
    if (p->non_cpu_intensive) {
        if (!has_a_job(p)) {
            cpu_work_fetch.req_secs = 1;
        }
        return;
    }

    // See if this is the project we'd ask for work anyway.
    // Temporarily clear resource backoffs,
    // since we're going to contact this project in any case.
    //
    double cpu_save = p->cpu_pwf.backoff_time;
    double cuda_save = p->cuda_pwf.backoff_time;
    double ati_save = p->ati_pwf.backoff_time;
    p->cpu_pwf.backoff_time = 0;
    p->cuda_pwf.backoff_time = 0;
    p->ati_pwf.backoff_time = 0;
    PROJECT* pbest = choose_project();
    p->cpu_pwf.backoff_time = cpu_save;
    p->cuda_pwf.backoff_time = cuda_save;
    p->ati_pwf.backoff_time = ati_save;
    if (p == pbest) {
        // Ask for work for all devices w/ a shortfall.
        // Otherwise we can have a situation where a GPU is idle,
        // we ask only for GPU work, and the project never has any
        //
        work_fetch.set_shortfall_requests(pbest);
        return;
    }

    // if not, don't request any work
    //
    clear_request();
}

// see if there's a fetchable non-CPU-intensive project without work
//
PROJECT* WORK_FETCH::non_cpu_intensive_project_needing_work() {
    for (unsigned int i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        if (!p->non_cpu_intensive) continue;
        if (!p->can_request_work()) continue;
        if (p->cpu_pwf.backoff_time > gstate.now) continue;
        if (has_a_job(p)) continue;
        clear_request();
        cpu_work_fetch.req_secs = 1;
        return p;
    }
    return 0;
}

// choose a project to fetch work from,
// and set the request fields of resource objects
//
PROJECT* WORK_FETCH::choose_project() {
    PROJECT* p = 0;

    p = non_cpu_intensive_project_needing_work();
    if (p) return p;

    gstate.compute_nuploading_results();

    gstate.rr_simulation();
    set_overall_debts();

    bool cuda_usable = coproc_cuda && coproc_cuda->usable;
    bool ati_usable = coproc_ati && coproc_ati->usable;

    if (cuda_usable) {
        p = cuda_work_fetch.choose_project(FETCH_IF_IDLE_INSTANCE);
    }
    if (ati_usable) {
        p = ati_work_fetch.choose_project(FETCH_IF_IDLE_INSTANCE);
    }
    if (!p) {
        p = cpu_work_fetch.choose_project(FETCH_IF_IDLE_INSTANCE);
    }
    if (!p && cuda_usable) {
        p = cuda_work_fetch.choose_project(FETCH_IF_MAJOR_SHORTFALL);
    }
    if (!p && ati_usable) {
        p = ati_work_fetch.choose_project(FETCH_IF_MAJOR_SHORTFALL);
    }
    if (!p) {
        p = cpu_work_fetch.choose_project(FETCH_IF_MAJOR_SHORTFALL);
    }
    if (!p && cuda_usable) {
        p = cuda_work_fetch.choose_project(FETCH_IF_MINOR_SHORTFALL);
    }
    if (!p && ati_usable) {
        p = ati_work_fetch.choose_project(FETCH_IF_MINOR_SHORTFALL);
    }
    if (!p) {
        p = cpu_work_fetch.choose_project(FETCH_IF_MINOR_SHORTFALL);
    }
#if 0
    // don't try to maintain GPU work for all projects,
    // since we don't use round-robin scheduling for GPUs
    //
    if (!p && cuda_usable) {
        p = cuda_work_fetch.choose_project(FETCH_IF_PROJECT_STARVED);
    }
    if (!p && ati_usable) {
        p = ati_work_fetch.choose_project(FETCH_IF_PROJECT_STARVED);
    }
#endif
    if (!p) {
        p = cpu_work_fetch.choose_project(FETCH_IF_PROJECT_STARVED);
    }

    if (log_flags.work_fetch_debug) {
        print_state();
        if (!p) {
            msg_printf(0, MSG_INFO, "[wfd] No project chosen for work fetch");
        }
    }

    return p;
}

double RSC_WORK_FETCH::share_request(PROJECT* p) {
    double dcf = p->duration_correction_factor;
    if (dcf < 0.02 || dcf > 80.0) {
        // if project's DCF is too big or small,
        // its completion time estimates are useless; just ask for 1 second
        //
        return 1;
    } else {
        // otherwise ask for the project's share
        //
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        return gstate.work_buf_total()*w.fetchable_share;
    }
}

void RSC_WORK_FETCH::set_request(PROJECT* p, double r) {
    RSC_PROJECT_WORK_FETCH& w = project_state(p);
    req_secs = r;
    req_instances = (int)ceil(w.fetchable_share*nidle_now);
}

void WORK_FETCH::accumulate_inst_sec(ACTIVE_TASK* atp, double dt) {
    APP_VERSION* avp = atp->result->avp;
    PROJECT* p = atp->result->project;
    double x = dt*avp->avg_ncpus;
    p->cpu_pwf.secs_this_debt_interval += x;
    cpu_work_fetch.secs_this_debt_interval += x;
    if (coproc_cuda) {
        x = dt*avp->ncudas;
        p->cuda_pwf.secs_this_debt_interval += x;
        cuda_work_fetch.secs_this_debt_interval += x;
    }
    if (coproc_ati) {
        x = dt*avp->natis;
        p->ati_pwf.secs_this_debt_interval += x;
        ati_work_fetch.secs_this_debt_interval += x;
    }
}

// update long-term debts for a resource.
//
void RSC_WORK_FETCH::update_debts() {
    unsigned int i;
    int neligible = 0;
    double ders = 0;
    PROJECT* p;

    // find the total resource share of eligible projects
    //
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        if (w.debt_eligible(p, *this)) {
            ders += p->resource_share;
            neligible++;
        }
    }
    if (!neligible) {
        if (log_flags.debt_debug) {
            msg_printf(0, MSG_INFO,
                "[debt] %s: no eligible projects", rsc_name(rsc_type)
            );
        }
        return;
    }

    double max_debt=0;
    bool first = true;
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        if (w.debt_eligible(p, *this)) {
            double share_frac = p->resource_share/ders;

            // the change to a project's debt is:
            // (how much it's owed) - (how much it got)
            //
            double delta = share_frac*secs_this_debt_interval - w.secs_this_debt_interval;
            w.debt += delta;
            if (log_flags.debt_debug) {
                msg_printf(p, MSG_INFO,
                    "[debt] %s debt %.2f delta %.2f share frac %.2f (%.2f/%.2f) secs %.2f rsc_secs %.2f",
                    rsc_name(rsc_type),
                    w.debt, delta, share_frac, p->resource_share, ders, secs_this_debt_interval,
                    w.secs_this_debt_interval
                );
            }
            if (first) {
                max_debt = w.debt;
                first = false;
            } else {
                if (w.debt > max_debt) {
                    max_debt = w.debt;
                }
            }
        } else {
            if (log_flags.debt_debug) {
                msg_printf(p, MSG_INFO,
                    "[debt] %s ineligible; debt %.2f",
                    rsc_name(rsc_type), w.debt
                );
            }
        }
    }

    // The net change may be
    // - positive if the resource wasn't fully utilized during the debt interval
    // - negative it was overcommitted (e.g., CPU)
    // We need to keep eligible projects from diverging from non-eligible ones;
    // also, if all the debts are large negative we need to gradually
    // shift them towards zero.
    // To do this, we add an offset as follows:
    // delta_limit is the largest rate at which any project's debt
    // could increase or decrease.
    // If the largest debt is close to zero (relative to delta_limit)
    // than add an offset that will bring it exactly to zero.
    // Otherwise add an offset of 2*delta_limit,
    // which will gradually bring all the debts towards zero
    //
    // The policy of keeping the max debt at zero is important;
    // it means that new projects will begin in parity with high-debt project,
    // and won't wait for months to get work.
    //
    double offset;
    double delta_limit = secs_this_debt_interval*ninstances;
    if (max_debt > -2*delta_limit) {
        offset = max_debt?-max_debt:0;  // avoid -0
    } else {
        offset = 2*delta_limit;
    }
    if (log_flags.debt_debug) {
        msg_printf(0, MSG_INFO, "[debt] %s debt: adding offset %.2f",
            rsc_name(rsc_type), offset
        );
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        RSC_PROJECT_WORK_FETCH& w = project_state(p);
        if (w.debt_eligible(p, *this)) {
            w.debt += offset;
        }
    }
}

// find total and per-project resource shares for each resource
//
void WORK_FETCH::compute_shares() {
    unsigned int i;
    PROJECT* p;
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->cpu_pwf.has_runnable_jobs) {
            cpu_work_fetch.total_runnable_share += p->resource_share;
        }
        if (p->cuda_pwf.has_runnable_jobs) {
            cuda_work_fetch.total_runnable_share += p->resource_share;
        }
        if (p->ati_pwf.has_runnable_jobs) {
            ati_work_fetch.total_runnable_share += p->resource_share;
        }
        if (!p->pwf.can_fetch_work) continue;
        if (p->cpu_pwf.may_have_work) {
            cpu_work_fetch.total_fetchable_share += p->resource_share;
        }
        if (coproc_cuda && p->cuda_pwf.may_have_work) {
            cuda_work_fetch.total_fetchable_share += p->resource_share;
        }
        if (coproc_ati && p->ati_pwf.may_have_work) {
            ati_work_fetch.total_fetchable_share += p->resource_share;
        }
    }
    for (i=0; i<gstate.projects.size(); i++) {
        p = gstate.projects[i];
        if (p->non_cpu_intensive) continue;
        if (p->cpu_pwf.has_runnable_jobs) {
            p->cpu_pwf.runnable_share = p->resource_share/cpu_work_fetch.total_runnable_share;
        }
        if (p->cuda_pwf.has_runnable_jobs) {
            p->cuda_pwf.runnable_share = p->resource_share/cuda_work_fetch.total_runnable_share;
        }
        if (p->ati_pwf.has_runnable_jobs) {
            p->ati_pwf.runnable_share = p->resource_share/ati_work_fetch.total_runnable_share;
        }
        if (!p->pwf.can_fetch_work) continue;
        if (p->cpu_pwf.may_have_work) {
            p->cpu_pwf.fetchable_share = p->resource_share/cpu_work_fetch.total_fetchable_share;
        }
        if (coproc_cuda && p->cuda_pwf.may_have_work) {
            p->cuda_pwf.fetchable_share = p->resource_share/cuda_work_fetch.total_fetchable_share;
        }
        if (coproc_ati && p->ati_pwf.may_have_work) {
            p->ati_pwf.fetchable_share = p->resource_share/ati_work_fetch.total_fetchable_share;
        }
    }
}

// should this project be accumulating debt for this resource?
//
bool RSC_PROJECT_WORK_FETCH::debt_eligible(PROJECT* p, RSC_WORK_FETCH& rwf) {
    if (p->non_cpu_intensive) return false;
    if (p->suspended_via_gui) return false;
    if (p->dont_request_more_work) return false;
    if (has_runnable_jobs) return true;
    if (backoff_time > gstate.now) return false;

    // The last time we asked for work we didn't get any,
    // but it's been a while since we asked.
    // In this case, accumulate debt until we reach (around) zero, then stop.
    //
    if (backoff_interval == MAX_BACKOFF_INTERVAL) {
        if (debt > -rwf.ninstances*DEBT_ADJUST_PERIOD) {
            return false;
        }
    }
    if (p->min_rpc_time > gstate.now) return false;
    return true;
}

inline bool has_coproc_app(PROJECT* p, int rsc_type) {
    unsigned int i;
    for (i=0; i<gstate.app_versions.size(); i++) {
        APP_VERSION* avp = gstate.app_versions[i];
        if (avp->project != p) continue;
        switch(rsc_type) {
        case RSC_TYPE_CUDA: if (avp->ncudas) return true;
        case RSC_TYPE_ATI: if (avp->natis) return true;
        }
    }
    return false;
}

void WORK_FETCH::write_request(FILE* f, PROJECT* p) {
    double work_req = cpu_work_fetch.req_secs;

    // if project is anonymous platform, set the overall work req
    // to the max of the requests of resource types for which we have versions.
    // Otherwise projects with old schedulers won't send us work.
    // THIS CAN BE REMOVED AT SOME POINT
    //
    if (p->anonymous_platform) {
        if (has_coproc_app(p, RSC_TYPE_CUDA)) {
            if (cuda_work_fetch.req_secs > work_req) {
                work_req = cuda_work_fetch.req_secs;
            }
        }
        if (has_coproc_app(p, RSC_TYPE_ATI)) {
            if (ati_work_fetch.req_secs > work_req) {
                work_req = ati_work_fetch.req_secs;
            }
        }
    }
    fprintf(f,
        "    <work_req_seconds>%f</work_req_seconds>\n"
        "    <cpu_req_secs>%f</cpu_req_secs>\n"
        "    <cpu_req_instances>%d</cpu_req_instances>\n"
        "    <estimated_delay>%f</estimated_delay>\n",
        work_req,
        cpu_work_fetch.req_secs,
        cpu_work_fetch.req_instances,
        cpu_work_fetch.req_secs?cpu_work_fetch.busy_time_estimator.get_busy_time():0
    );
    if (log_flags.work_fetch_debug) {
        char buf[256], buf2[256];
        sprintf(buf,
            "[wfd] request: %.2f sec CPU (%.2f sec, %d)",
            work_req,
            cpu_work_fetch.req_secs, cpu_work_fetch.req_instances
        );
        if (coproc_cuda) {
            sprintf(buf2, " NVIDIA GPU (%.2f sec, %d)",
                cuda_work_fetch.req_secs, cuda_work_fetch.req_instances
            );
            strcat(buf, buf2);
        }
        if (coproc_ati) {
            sprintf(buf2, " ATI GPU (%.2f sec, %d)",
                ati_work_fetch.req_secs, ati_work_fetch.req_instances
            );
            strcat(buf, buf2);
        }
        msg_printf(p, MSG_INFO, buf);
    }
}

// we just got a scheduler reply with the given jobs; update backoffs
//
void WORK_FETCH::handle_reply(
    PROJECT* p, SCHEDULER_REPLY* srp, vector<RESULT*> new_results
) {
    unsigned int i;
    bool got_cpu = false, got_cuda = false, got_ati = false;

    // handle project-supplied backoff requests
    //
    if (srp->cpu_backoff) {
        p->cpu_pwf.backoff_time = gstate.now + srp->cpu_backoff;
    }
    if (srp->cuda_backoff) {
        p->cuda_pwf.backoff_time = gstate.now + srp->cuda_backoff;
    }
    if (srp->ati_backoff) {
        p->ati_pwf.backoff_time = gstate.now + srp->ati_backoff;
    }

    // if didn't get any jobs, back off on requested resource types
    //
    if (!new_results.size()) {
        // but not if RPC was requested by project
        //
        if (p->sched_rpc_pending != RPC_REASON_PROJECT_REQ) {
            if (cpu_work_fetch.req_secs && !srp->cpu_backoff) {
                p->cpu_pwf.backoff(p, "CPU");
            }
            if (coproc_cuda && coproc_cuda->req_secs && !srp->cuda_backoff) {
                p->cuda_pwf.backoff(p, "NVIDIA GPU");
            }
            if (coproc_ati && coproc_ati->req_secs && !srp->ati_backoff) {
                p->ati_pwf.backoff(p, "ATI GPU");
            }
        }
        return;
    }

    // if we did get jobs, clear backoff on resource types
    //
    for (i=0; i<new_results.size(); i++) {
        RESULT* rp = new_results[i];
        if (rp->avp->ncudas) got_cuda = true;
        else if (rp->avp->natis) got_ati = true;
        else got_cpu = true;
    }
    if (got_cpu) p->cpu_pwf.clear_backoff();
    if (got_cuda) p->cuda_pwf.clear_backoff();
    if (got_ati) p->ati_pwf.clear_backoff();
}

// set up for initial RPC.
// arrange to always get one job, even if we don't need it or can't handle it.
// (this is probably what user wants)
//
void WORK_FETCH::set_initial_work_request() {
    cpu_work_fetch.req_secs = 1;
    cpu_work_fetch.req_instances = 0;
    cpu_work_fetch.busy_time_estimator.reset();
    if (coproc_cuda) {
        cuda_work_fetch.req_secs = 1;
        cuda_work_fetch.req_instances = 0;
        cuda_work_fetch.busy_time_estimator.reset();
    }
    if (coproc_ati) {
        ati_work_fetch.req_secs = 1;
        ati_work_fetch.req_instances = 0;
        ati_work_fetch.busy_time_estimator.reset();
    }
}

// called once, at client startup
//
void WORK_FETCH::init() {
    cpu_work_fetch.init(RSC_TYPE_CPU, gstate.ncpus, 1);
    double cpu_flops = gstate.ncpus*gstate.host_info.p_fpops;

    // use 20% as a rough estimate of GPU efficiency

    if (coproc_cuda) {
        cuda_work_fetch.init(
            RSC_TYPE_CUDA, coproc_cuda->count,
            coproc_cuda->count*0.2*coproc_cuda->peak_flops()/cpu_flops
        );
    }
    if (coproc_ati) {
        ati_work_fetch.init(
            RSC_TYPE_ATI,
            coproc_ati->count,
            coproc_ati->count*0.2*coproc_ati->peak_flops()/cpu_flops
        );
    }

    if (config.zero_debts) {
        zero_debts();
    }
}

void RSC_PROJECT_WORK_FETCH::backoff(PROJECT* p, const char* name) {
    if (backoff_interval) {
        backoff_interval *= 2;
        if (backoff_interval > MAX_BACKOFF_INTERVAL) backoff_interval = MAX_BACKOFF_INTERVAL;
    } else {
        backoff_interval = MIN_BACKOFF_INTERVAL;
    }
    double x = drand()*backoff_interval;
    backoff_time = gstate.now + x;
    if (log_flags.work_fetch_debug) {
        msg_printf(p, MSG_INFO,
            "[wfd] backing off %s %.0f sec", name, x
        );
    }
}

////////////////////////

void CLIENT_STATE::compute_nuploading_results() {
    unsigned int i;

    for (i=0; i<projects.size(); i++) {
        projects[i]->nuploading_results = 0;
        projects[i]->too_many_uploading_results = false;
    }
    for (i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->state() == RESULT_FILES_UPLOADING) {
            rp->project->nuploading_results++;
        }
    }
    int n = gstate.ncpus;
    if (coproc_cuda && coproc_cuda->count > n) {
        n = coproc_cuda->count;
    }
    if (coproc_ati && coproc_ati->count > n) {
        n = coproc_ati->count;
    }
    n *= 2;
    for (i=0; i<projects.size(); i++) {
        if (projects[i]->nuploading_results > n) {
            projects[i]->too_many_uploading_results = true;
        }
    }
}

bool PROJECT::runnable() {
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->runnable()) return true;
    }
    return false;
}

bool PROJECT::downloading() {
    if (suspended_via_gui) return false;
    for (unsigned int i=0; i<gstate.results.size(); i++) {
        RESULT* rp = gstate.results[i];
        if (rp->project != this) continue;
        if (rp->downloading()) return true;
    }
    return false;
}

bool PROJECT::some_result_suspended() {
    unsigned int i;
    for (i=0; i<gstate.results.size(); i++) {
         RESULT *rp = gstate.results[i];
         if (rp->project != this) continue;
         if (rp->suspended_via_gui) return true;
     }
    return false;
}

bool PROJECT::can_request_work() {
    if (suspended_via_gui) return false;
    if (master_url_fetch_pending) return false;
    if (min_rpc_time > gstate.now) return false;
    if (dont_request_more_work) return false;
    if (gstate.in_abort_sequence) return false;
    return true;
}

bool PROJECT::potentially_runnable() {
    if (runnable()) return true;
    if (can_request_work()) return true;
    if (downloading()) return true;
    return false;
}

bool PROJECT::nearly_runnable() {
    if (runnable()) return true;
    if (downloading()) return true;
    return false;
}

bool RESULT::runnable() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state() != RESULT_FILES_DOWNLOADED) return false;
    if (coproc_missing) return false;
    return true;
}

bool RESULT::nearly_runnable() {
    return runnable() || downloading();
}

// Return true if the result is waiting for its files to download,
// and nothing prevents this from happening soon
//
bool RESULT::downloading() {
    if (suspended_via_gui) return false;
    if (project->suspended_via_gui) return false;
    if (state() > RESULT_FILES_DOWNLOADING) return false;
    return true;
}

double RESULT::estimated_duration_uncorrected() {
    return wup->rsc_fpops_est/avp->flops;
}

// estimate how long a result will take on this host
//
#ifdef SIM
double RESULT::estimated_duration(bool for_work_fetch) {
    SIM_PROJECT* spp = (SIM_PROJECT*)project;
    if (dual_dcf && for_work_fetch && spp->completions_ratio_mean) {
        return estimated_duration_uncorrected()*spp->completions_ratio_mean;
    }
    return estimated_duration_uncorrected()*project->duration_correction_factor;
}
#else
double RESULT::estimated_duration(bool) {
    return estimated_duration_uncorrected()*project->duration_correction_factor;
}
#endif

double RESULT::estimated_time_remaining(bool for_work_fetch) {
    if (computing_done()) return 0;
    ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(this);
    if (atp) {
        return atp->est_time_to_completion(for_work_fetch);
    }
    return estimated_duration(for_work_fetch);
}

// Returns the estimated elapsed time to completion (in seconds) of this task.
// Compute this as a weighted average of estimates based on
// 1) the workunit's flops count (static estimate)
// 2) the current elapsed time and fraction done (dynamic estimate)
//
double ACTIVE_TASK::est_time_to_completion(bool for_work_fetch) {
    if (fraction_done >= 1) return 0;
    double wu_est = result->estimated_duration(for_work_fetch);
    if (fraction_done <= 0) return wu_est;
    double frac_est = (elapsed_time / fraction_done) - elapsed_time;
#if 0
    // commenting this out for now - could cause big discontinuity
    //
    if (elapsed_time >= wu_est) {
        // if the job has already run longer than static estimate,
        // just use the dynamic estimate.
        //
        return frac_est;
    }
#endif
    double fraction_left = 1-fraction_done;
    double wu_weight = fraction_left * fraction_left;
    double fd_weight = 1 - wu_weight;
    double x = fd_weight*frac_est + wu_weight*fraction_left*wu_est;
    return x;
}

// the fraction of time BOINC is processing
//
double CLIENT_STATE::overall_cpu_frac() {
    double running_frac = time_stats.on_frac * time_stats.active_frac;
    if (running_frac < 0.01) running_frac = 0.01;
    if (running_frac > 1) running_frac = 1;
    return running_frac;
}

// called when benchmarks change
//
void CLIENT_STATE::scale_duration_correction_factors(double factor) {
    if (factor <= 0) return;
    for (unsigned int i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->duration_correction_factor *= factor;
    }
    if (log_flags.dcf_debug) {
        msg_printf(NULL, MSG_INFO,
            "[dcf] scaling all duration correction factors by %f",
            factor
        );
    }
}

// Choose a new host CPID.
// If using account manager, do scheduler RPCs
// to all acct-mgr-attached projects to propagate the CPID
//
void CLIENT_STATE::generate_new_host_cpid() {
    host_info.generate_host_cpid();
    for (unsigned int i=0; i<projects.size(); i++) {
        if (projects[i]->attached_via_acct_mgr) {
            projects[i]->sched_rpc_pending = RPC_REASON_ACCT_MGR_REQ;
            projects[i]->set_min_rpc_time(now + 15, "Sending new host CPID");
        }
    }
}

