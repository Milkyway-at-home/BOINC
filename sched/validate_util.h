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

#ifndef H_VALIDATE_UTIL
#define H_VALIDATE_UTIL

#include <vector>
#include <string>

#include "boinc_db.h"

extern int get_output_file_path(RESULT const& result, std::string& path);
extern int get_output_file_paths(RESULT const& result, std::vector<std::string>&);
extern double median_mean_credit(WORKUNIT&, std::vector<RESULT>& results);
extern double get_credit_from_wu(WORKUNIT&, std::vector<RESULT>& results);
extern int update_credit_per_cpu_sec(
    double credit, double cpu_time, double& credit_per_cpu_sec
);
#endif
