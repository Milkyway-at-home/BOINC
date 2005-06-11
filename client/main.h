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

#ifdef _WIN32
extern int boinc_main_loop();
extern void quit_client();
extern void suspend_client();
extern void resume_client();
extern int check_unique_instance();
#endif

#define ADD_FAIL_INIT       1
#define ADD_FAIL_DOWNLOAD   2
#define ADD_FAIL_PARSE      3
#define ADD_FAIL_BAD_KEY    4
#define ADD_FAIL_FILE_WRITE 5

class PROJECT;
extern void project_add_failed(PROJECT*, int reason);