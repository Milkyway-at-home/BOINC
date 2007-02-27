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

#ifndef _HOSTINFO_
#define _HOSTINFO_

// Description of a host's hardware and software.
// This is used a few places:
// - it's part of the client's state file, client_state.xml
// - it's passed in the reply to the get_host_info GUI RPC
// - it's included in scheduler RPC requests
//
// Other host-specific info is kept in
// TIME_STATS (on/connected/active fractions)
// NET_STATS (average network bandwidths)

#include "miofile.h"

class HOST_INFO {
public:
    int timezone;                 // local STANDARD time - UTC time (in seconds)
    char domain_name[256];
    char serialnum[256];
    char ip_addr[256];
    char host_cpid[64];

    int p_ncpus;
    char p_vendor[256];
    char p_model[256];
    double p_fpops;
    double p_iops;
    double p_membw;
    double p_calculated;          // when benchmarks were last run, or zero

    double m_nbytes;              // Total amount of memory in bytes
    double m_cache;
    double m_swap;                // Total amount of swap space in bytes

    double d_total;               // Total amount of disk in bytes
    double d_free;                // Total amount of free disk in bytes

    char os_name[256];
    char os_version[256];

    char accelerators[256];
        // text description of graphics coprocessors or other accelerators

    HOST_INFO();
    int parse(MIOFILE&);
    int write(MIOFILE&);
    int parse_cpu_benchmarks(FILE*);
    int write_cpu_benchmarks(FILE*);
    void print();

    bool host_is_running_on_batteries();
#ifdef __APPLE__
    bool users_idle(bool check_all_logins, double idle_time_to_run, double *actual_idle_time=NULL);
#else
    bool users_idle(bool check_all_logins, double idle_time_to_run);
#endif
    int get_host_info();
    int get_local_network_info();
    void clear_host_info();
    void make_random_string(const char* salt, char* out);
    void generate_host_cpid();
};

#endif
