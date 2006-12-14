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

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <string>

#ifdef HAVE_PTHREAD

#include <pthread.h>

#endif

// Ideally, we would access this using wxGetApp().m_use_sandbox in the Manager
// and gstate.m_use_sandbox in the Client, but it is used by some source files
// (filesys.C, check_security.C) that are linked with both Manager and Client 
// so the most practical solution is to use a global.
extern int      g_use_sandbox;



#if !defined(HAVE_STRLCPY)
extern size_t strlcpy(char*, const char*, size_t);
#endif

#if !defined(HAVE_STRLCAT)
extern size_t strlcat(char *dst, const char *src, size_t size);
#endif

extern int ndays_to_string(double x, int smallest_timescale, char *buf);
extern void nbytes_to_string(double nbytes, double total_bytes, char* str, int len);
extern double dtime();
extern double dday();
extern void validate_time(double&);
extern void boinc_sleep(double);
extern int parse_command_line( char *, char ** );
extern void c2x(char *what);
extern void strip_whitespace(char *str);
extern void strip_whitespace(std::string&);
extern void unescape_url(std::string& url);
extern void unescape_url(char *url);
extern void escape_url(std::string& url);
extern void escape_url(char *in, char*out);
extern void escape_url_readable(char* in, char* out);
extern bool valid_master_url(char*);
extern void canonicalize_master_url(char *url);
extern void canonicalize_master_url(std::string&);
#define safe_strcpy(x, y) strlcpy(x, y, sizeof(x))
#define safe_strcat(x, y) if (strlen(x)+strlen(y)<sizeof(x)) strcat(x, y)
extern char* time_to_string(double);
extern char* precision_time_to_string(double);
extern std::string timediff_format(double);
extern int read_file_string(const char* pathname, std::string& result);
extern void escape_project_url(char *in, char* out);

// remove duplicated words in a comma or space delimited string.
// result is a space delimited string.
// "this is this a is test" -> "this is a test"
extern void remove_duplicate_words(char *str);
extern void remove_duplicate_words(std::string &str);

inline bool ends_with(std::string const& s, std::string const& suffix) {
    return
        s.size()>=suffix.size() &&
        s.substr(s.size()-suffix.size()) == suffix;
}

inline bool starts_with(std::string const& s, std::string const& prefix) {
    return s.substr(0, prefix.size()) == prefix;
}

//extern bool ends_with(const char*, const char* suffix);
//extern bool starts_with(const char*, const char* prefix);

// http://lists.debian.org/debian-gcc/2002/debian-gcc-200204/msg00092.html
inline void downcase_string(
    std::string::iterator begin, std::string::iterator end, std::string::iterator src
) {
	std::transform(begin, end, src, (int(*)(int))tolower);
}

inline void downcase_string(std::string& w) {
    downcase_string(w.begin(), w.end(), w.begin());
}

// NOTE: use #include <functional>   to get max,min

#define SECONDS_PER_DAY 86400
#define MEGA (1048576.0)

static inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

#ifdef _WIN32

#include <windows.h>

extern char* windows_error_string(char* pszBuf, int iSize);
extern char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize
);
extern int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu);
extern int boinc_process_cpu_time(double& cpu);

#endif

extern void update_average(double, double, double, double&, double&);
extern int update_credit_per_cpu_sec(double credit, double cpu_time, double& credit_per_cpu_sec);

extern int boinc_calling_thread_cpu_time(double&);

// convert UNIX time to MySQL timestamp (yyyymmddhhmmss)
//
extern void mysql_timestamp(double, char*);

// returns short text description of error corresponding to
// int errornumber from error_numbers.h
//
extern const char* boincerror(int which_error);
extern const char* network_status_string(int);
extern const char* rpc_reason_string(int);

#ifdef HAVE_PTHREAD
extern pthread_mutex_t getrusage_mutex;
#endif

#ifndef _WIN32
extern int lookup_group(char*, gid_t& gid);
extern int check_security(int use_sandbox, int isManager);
#endif

extern int run_program(char* path, char* cdir, int argc, char** argv);
extern int wait_client_mutex(char* dir, double timeout);

#endif
