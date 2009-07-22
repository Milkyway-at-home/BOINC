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
#include <cstdio>
#include <cstring>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "str_util.h"
#include "str_replace.h"
#include "parse.h"
#include "util.h"
#include "file_names.h"
#include "client_msgs.h"
#include "error_numbers.h"
#include "mac_address.h"

#include "hostinfo.h"

// get domain name and IP address of this host
//
int HOST_INFO::get_local_network_info() {
    struct in_addr addr;
    struct hostent* he;
    
    strcpy(domain_name, "");
    strcpy(ip_addr, "");

    // it seems like we should use getdomainname() instead of gethostname(),
    // but on FC6 it returns "(none)".
    //
    if (gethostname(domain_name, 256)) return ERR_GETHOSTBYNAME;
    he = gethostbyname(domain_name);
    if (!he || !he->h_addr_list[0]) {
        //msg_printf(NULL, MSG_ERROR, "gethostbyname (%s) failed", domain_name);
        return ERR_GETHOSTBYNAME;
    }
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    strlcpy(ip_addr, inet_ntoa(addr), sizeof(ip_addr));
    return 0;
}

// make a random string using host info.
// Not recommended for password generation;
// use as a last resort if more secure methods fail
//
void HOST_INFO::make_random_string(const char* salt, char* out) {
    char buf[1024];

    sprintf(buf, "%f%s%s%f%s", dtime(), domain_name, ip_addr, d_free, salt);
    md5_block((const unsigned char*) buf, (int)strlen(buf), out);
}

// make a host cross-project ID.
// Should be unique across hosts with very high probability
//
void HOST_INFO::generate_host_cpid() {
// Assume that get_mac_addresses can be ported to any unix system.
// If not, it can return false.
//
#if defined(__linux__) || defined(_WIN32) || defined(__APPLE__ ) || defined(__unix)
    char buffer[8192] = "";
        // must be big enough to accommodate aa:bb:cc:dd:ee:ff
        // times the number of network interfaces,
        // plus the domain name, IP addr, and OS name.
        // 8K should suffice

    if (!get_mac_addresses(buffer) || ! strcmp(buffer, "")) {
        make_random_string("", host_cpid);
        return;
    }
    strcat(buffer, domain_name);
    strcat(buffer, ip_addr);
    strcat(buffer, os_name);
    md5_block((unsigned char*)buffer, (int)strlen(buffer), host_cpid);
#else
    make_random_string("", host_cpid);
#endif
}


const char *BOINC_RCSID_9275b20aa5 = "$Id$";
