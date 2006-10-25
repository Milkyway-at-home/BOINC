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

#ifndef _FILE_XFER_
#define _FILE_XFER_

// A FILE_XFER object represents a file transfer "episode"
// (see pers_file_xfer.h), i.e. an HTTP transaction with a
// particular data server.
// 

#include "client_types.h"
#include "http_curl.h"

#define MAX_FILE_XFERS_PER_PROJECT      2
#define MAX_FILE_XFERS                  8
    // kind of arbitrary

#define MIN_DOWNLOAD_INCREMENT  5000

class FILE_XFER : public HTTP_OP {
public:
    FILE_INFO* fip;
    char pathname[256];
    char header[4096];
    bool file_size_query;
    bool is_upload;
    double starting_size;
        // File size at start of transfer, used for:
        // 1) a "minimum download increment"
        // that rejects partial downloads of less than 5K,
        // since these may be error messages from proxies.
        // 2) lets us recover when server ignored Range request
        // and sent us whole file

    FILE_XFER();
    ~FILE_XFER();

    int parse_upload_response(double &offset);
    int init_download(FILE_INFO&);
    int init_upload(FILE_INFO&);
    bool file_xfer_done;
    int file_xfer_retval;
};

class FILE_XFER_SET {
    HTTP_OP_SET* http_ops;
public:
    bool up_active, down_active;
        // has there been transfer activity since last call to check_active()?
    std::vector<FILE_XFER*> file_xfers;
    FILE_XFER_SET(HTTP_OP_SET*);
    int insert(FILE_XFER*);
    int remove(FILE_XFER*);
    bool poll();
    void check_active(bool&, bool&);
    void set_bandwidth_limits(bool is_upload);
};

#endif
