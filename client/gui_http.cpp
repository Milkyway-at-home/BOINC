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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include <cstring>
#include "client_state.h"
#include "filesys.h"
#include "error_numbers.h"

#include "gui_http.h"

int GUI_HTTP::do_rpc(GUI_HTTP_OP* op, string url, string output_file) {
    int retval;

    if (state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    boinc_delete_file(output_file.c_str());
    retval = http_op.init_get(url.c_str(), output_file.c_str(), true);
    if (retval) return retval;
    gstate.http_ops->insert(&http_op);
    gui_http_op = op;
    state = GUI_HTTP_STATE_BUSY;
    return 0;
}

int GUI_HTTP::do_rpc_post(GUI_HTTP_OP* op, string url, string input_file, string output_file) {
    int retval;

    if (state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    boinc_delete_file(output_file.c_str());
    retval = http_op.init_post(url.c_str(), input_file.c_str(), output_file.c_str());
    if (retval) return retval;
    gstate.http_ops->insert(&http_op);
    gui_http_op = op;
    state = GUI_HTTP_STATE_BUSY;
    return 0;
}

bool GUI_HTTP::poll() {
    if (state == GUI_HTTP_STATE_IDLE) return false;
    static double last_time=0;
    if (gstate.now-last_time < GUI_HTTP_POLL_PERIOD) return false;
    last_time = gstate.now;

    if (http_op.http_op_state == HTTP_STATE_DONE) {
        gstate.http_ops->remove(&http_op);
        gui_http_op->handle_reply(http_op.http_op_retval);
        gui_http_op = NULL;
        state = GUI_HTTP_STATE_IDLE;
    }
    return true;
}

const char *BOINC_RCSID_7c374a67d3="$Id$";
