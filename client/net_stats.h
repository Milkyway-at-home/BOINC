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

// keep track of the network performance of this host,
// namely exponentially weighted averages of upload and download speeds
//

#ifndef _NET_STATS_
#define _NET_STATS_

#ifndef _WIN32
#include <cstdio>
#endif

#include "miofile.h"

class FILE_XFER_SET;
class HTTP_OP_SET;

/// there's one of these each for upload and download

struct NET_INFO {
        /// estimate of max transfer rate; computed as an average of
        /// the rates of recent file transfers, weighted by file size.
        /// This ignores concurrency of transfers.
    double max_rate;
        /// recent average transfer rate
    double avg_rate;
        /// when avg_rate was last updated
    double avg_time;
        /// updates the above vars
    void update(double nbytes, double dt);

};

class NET_STATS {
public:
    NET_INFO up;
    NET_INFO down;

    NET_STATS();

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

class NET_STATUS {
public:
		/// contact the reference site as soon as GUI_HTTP is idle
		/// polled from NET_STATS::poll(), for want of a better place
	bool need_to_contact_reference_site;
	void contact_reference_site();
    bool show_ref_message;
        /// client wants to do network comm and no physical connection exists.

        /// Initially false; set whenever a Curl operation
        /// returns CURLE_COULDNT_RESOLVE_HOST,
        /// and a subsequent request to a highly-available site
        /// also returns CURLE_COULDNT_RESOLVE_HOST.
        /// cleared whenever we transfer data,
        /// or an operation returns some other value
    bool need_physical_connection;
        /// we have a network connection, but it's likely to go away soon,
        /// so do as much network comm as possible
        /// (e.g. report completed results)
    bool have_sporadic_connection;
	double last_comm_time;

    int network_status();
    void network_available();
    void got_http_error();
    NET_STATUS() {
        need_physical_connection = false;
        have_sporadic_connection = false;
		need_to_contact_reference_site = false;
        show_ref_message = false;
		last_comm_time = 0;
    }
    void poll();
};

/// This is used to access a reference website (like yahoo or google)
/// that is assumed to be 100% available.
/// It is used ONLY from the HTTP code, when a transaction fails

struct LOOKUP_WEBSITE_OP: public GUI_HTTP_OP {
    int error_num;

    virtual ~LOOKUP_WEBSITE_OP(){}
    int do_rpc(std::string&);
    virtual void handle_reply(int http_op_retval);
    LOOKUP_WEBSITE_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
};

extern NET_STATUS net_status;

#endif
