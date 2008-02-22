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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

extern void send_work_locality(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);

extern int decrement_disk_space_locality(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
);

extern int delete_file_from_host(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);

extern void send_file_deletes(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
