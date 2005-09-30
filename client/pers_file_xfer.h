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

#ifndef _PERS_FILE_XFER_H
#define _PERS_FILE_XFER_H

#include "client_types.h"
#include "file_xfer.h"


// PERS_FILE_XFER represents a "persistent file transfer",
// i.e. a long-term effort to upload or download a file.
// This may consist of several "episodes",
// which are HTTP operations to a particular server.
//
// The FILE_INFO has a list of URLs.
// For download, the object attempts to download the file
// from any of the URLs.
// If one fails or is not available, try another,
// using an exponential backoff policy to avoid flooding servers.
//
// For upload, try to upload the file to the first URL;
// if that fails try the others.

// a PERS_FILE_XFER is created and added to pers_file_xfer_set
// 1) when read from the client state file
//   in (FILE_INFO::parse(), CLIENT_STATE::parse_state_file()
// 2) when a FILE_INFO is ready to transfer
//   in CLIENT_STATE::handle_pers_file_xfers()

// a PERS_FILE_XFER p is removed from pers_file_xfer_set and freed
// 1) when p->pers_xfer_done is true
//   in CLIENT_STATE::handle_pers_file_xfers()

// A FILE_XFER is created and added to file_xfer_set and linked from PFX
// 1) in PERS_FILE_XFER::start_xfer()

// A FILE_XFER is erased from file_xfer_set, unlinked from PFX and freed
// 1) when the FILE_XFER is done, in
//   PERS_FILE_XFER::poll()
//   PERS_FILE_XFER::check_giveup()
// 2) user request, in
//   PERS_FILE_XFER::abort()
//   PERS_FILE_XFER::suspend()
// 3) if the FILE_XFER_SET::insert() fails
//   PERS_FILE_XFER::start_xfer()
// NOTE: when this is done, pers_xfer_done is set
//
// pointers:
// PERS_FILE_XFER -> FILE_XFER
//   set in PERS_FILE_XFER::start_xfer()
//    zeroed (see above)
// PERS_FILE_XFER -> FILE_INFO
//   set in PERS_FILE_XFER::init()
// FILE_INFO -> PERS_FILE_XFER
//   set in FILE_INFO::parse(), CLIENT_STATE::handle_pers_file_xfers()
//   zeroed in PERS_FILE_XFER destructor


// Default values for exponential backoff
#define PERS_RETRY_DELAY_MIN    60                // 1 minute
#define PERS_RETRY_DELAY_MAX    (60*60*4)         // 4 hours
#define PERS_GIVEUP             (60*60*24*7*2)    // 2 weeks
    // give up on xfer if this time elapses since last byte xferred

class PERS_FILE_XFER {
    int nretry;                // # of retries so far
    double first_request_time;    // time of first transfer request
    void do_backoff();

public:
    bool is_upload;
    double next_request_time;     // time to next retry the file request
    double time_so_far;
        // Total time there's been an active FILE_XFER for this PFX
        // Currently not used for anything;  not meaningful for throughput
        // because could include repeated transfer
    double last_time;
        // when the above was last updated.
        // Defined only while a transfer is active
    bool pers_xfer_done;
    FILE_XFER* fxp;     // nonzero if file xfer in progress
    FILE_INFO* fip;

    PERS_FILE_XFER();
    ~PERS_FILE_XFER();
    int init(FILE_INFO*, bool is_file_upload);
    bool poll();
    void handle_xfer_failure();
    void retry_or_backoff();
    void check_giveup(const char*);
    void abort();
    int write(MIOFILE& fout);
    int parse(MIOFILE& fin);
    int start_xfer();
    void suspend();
};

class PERS_FILE_XFER_SET {
    FILE_XFER_SET* file_xfers;
public:
    std::vector<PERS_FILE_XFER*>pers_file_xfers;

    PERS_FILE_XFER_SET(FILE_XFER_SET*);
    int insert(PERS_FILE_XFER*);
    int remove(PERS_FILE_XFER*);
    bool poll();
    void suspend();
};

#endif
