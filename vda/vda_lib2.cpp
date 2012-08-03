// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// The part of the implementation of vda_lib.h
// that's NOT used by the simulator

#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

#include "vda_lib.h"

using std::set;
using std::vector;

#define DATA_FILENAME "data.vda"

///////////////// Utility funcs ///////////////////////

char* host_alive_clause() {
    static char buf[256];
    sprintf(buf, "rpc_time > %f", dtime() - VDA_HOST_TIMEOUT);
    return buf;
}

inline bool alive(DB_HOST& h) {
    return (h.rpc_time > dtime()-VDA_HOST_TIMEOUT);
}

// return the name of a file created by Jerasure's encoder
//
// encoder creates files with names of the form
// Coding/fname_k01.ext
// Coding/fname_m01.ext
//
// Assume there's no extension
//
void encoder_filename(
    const char* base, const char* ext, CODING& c, int i, char* buf
) {
    int ndigits = 1;
    if (c.m > 9) ndigits = 2;
    else if (c.m > 99) ndigits = 3;
    else if (c.m > 999) ndigits = 4;
    int j;
    char ch;
    if (i >= c.n) {
        j = i-c.n + 1;
        ch = 'm';
    } else {
        j = i+1;
        ch = 'k';
    }
    sprintf(buf, "%s_%c%0*d.%s", base, ch, ndigits, j, ext);
}

int get_chunk_numbers(VDA_CHUNK_HOST& vch, vector<int>& chunk_numbers) {
    char buf[256];
    strcpy(buf, vch.physical_file_name);   // vda_hostid_chunknums_filename
    char* p = buf;
    p = strchr(p, '_') + 1;
    p = strchr(p, '_') + 1;
    char* q = strchr(p, '_') + 1;
    *q = 0;
    while (1) {
        int i = atoi(p);
        chunk_numbers.push_back(i);
        p = strchr(p, '.');
        if (!p) break;
        p++;
    }
    return 0;
}

///////////////// DATA_UNIT ///////////////////////

int DATA_UNIT::delete_file() {
    char path[1024], buf[1024];
    sprintf(path, "%s/data.vda", dir);
    size_t n = readlink(path, buf, 1024);
    buf[n] = 0;
    return unlink(buf);
}

///////////////// META_CHUNK ///////////////////////

META_CHUNK::META_CHUNK(VDA_FILE_AUX* d, META_CHUNK* p, int index) {
    dfile = d;
    parent = p;
    if (parent) {
        if (strlen(parent->name)) {
            sprintf(name, "%s.%d", parent->name, index);
        } else {
            sprintf(name, "%d", index);
        }
    } else {
        strcpy(name, "");
    }
}

// initialize a meta-chunk:
// encode it, then recursively initialize its meta-chunk children
//
int META_CHUNK::init(const char* _dir, POLICY& p, int coding_level) {
    double size;
    char child_dir[1024];

    strcpy(dir, _dir);
    coding = p.codings[coding_level];
    int retval = encode();
    if (retval) return retval;
    p.chunk_sizes[coding_level] = child_size;

    if (coding_level < p.coding_levels - 1) {
        for (int i=0; i<coding.m; i++) {
            sprintf(child_dir, "%s/%d", dir, i);
            META_CHUNK* mc = new META_CHUNK(dfile, parent, i);
            retval = mc->init(child_dir, p, coding_level+1);
            if (retval) return retval;
            children.push_back(mc);
        }
        bottom_level = false;
    } else {
        for (int i=0; i<coding.m; i++) {
            CHUNK* cp = new CHUNK(this, p.chunk_sizes[coding_level], i);
            children.push_back(cp);

            // write the chunk's MD5 to a file
            //
            char file_path[1024], md5_file_path[1024];
            sprintf(file_path, "%s/%d/%s", dir, i, DATA_FILENAME);
            sprintf(md5_file_path, "%s/%d/md5.txt", dir, i);
            char md5[64];
            md5_file(file_path, md5, size);
            FILE* f = fopen(md5_file_path, "w");
            fprintf(f, "%s\n", md5);
            fclose(f);
        }
        bottom_level = true;
    }
    return 0;
}

int META_CHUNK::get_state(const char* _dir, POLICY& p, int coding_level) {
    int retval;

    strcpy(dir, _dir);
    coding = p.codings[coding_level];
    if (coding_level < p.coding_levels - 1) {
        for (int i=0; i<coding.m; i++) {
            char child_dir[1024];
            sprintf(child_dir, "%s/%s.%d", dir, DATA_FILENAME, i);
            META_CHUNK* mc = new META_CHUNK(dfile, this, i);
            retval = mc->get_state(child_dir, p, coding_level+1);
            if (retval) return retval;
            children.push_back(mc);
        }
        bottom_level = false;
    } else {
        for (int i=0; i<coding.m; i++) {
            CHUNK* ch = new CHUNK(this, p.chunk_sizes[coding_level], i);
            children.push_back(ch);
        }
        bottom_level = true;
    }
    return 0;
}

// encode a meta-chunk.
// precondition: "dir" contains a file "data.vda".
// postcondition: dir contains
//   a subdir Coding with encoded chunks
//   subdirs 0/ .. m/
//     each containing a symbolic link "data.vda" to the corresponding chunk
//
// The size of these chunks is returned in "size"
//
int META_CHUNK::encode() {
    char cmd[1024];
    sprintf(cmd,
        "cd %s; /mydisks/b/users/boincadm/vda_test/encoder %s %d %d cauchy_good 32 1024 500000",
        dir, DATA_FILENAME, coding.n, coding.k
    );
    printf("%s\n", cmd);
    int s = system(cmd);
    if (WIFEXITED(s)) {
        int st = WEXITSTATUS(s);
        if (st != 32) return -1;    // encoder returns 32 for some reason
    }

    // make links
    //
    for (int i=0; i<coding.m; i++) {
        char enc_filename[1024], target_path[1024];
        char dir_name[1024], link_name[1024];
        encoder_filename("data", "vda", coding, i, enc_filename);
        sprintf(target_path, "%s/Coding/%s", dir, enc_filename);
        sprintf(dir_name, "%s/%d", dir, i);
        int retval = mkdir(dir_name, 0777);
        if (retval) {
            perror("mkdir");
            return retval;
        }
        sprintf(link_name, "%s/%s", dir_name, DATA_FILENAME);
        retval = link(target_path, link_name);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "encode(): link %s %s failed\n", target_path, link_name
            );
            return retval;
        }
        if (i == 0) {
            file_size(target_path, child_size);
        }
    }
    return 0;
}

int META_CHUNK::decode() {
    char cmd[1024];
    sprintf(cmd,
        "cd %s; /mydisks/b/users/boincadm/vda_test/decoder %s",
        dir, DATA_FILENAME
    );
    printf("%s\n", cmd);
    int s = system(cmd);
    if (WIFEXITED(s)) {
        int st = WEXITSTATUS(s);
        if (st != 32) return -1;    // encoder returns 32 for some reason
    }
    return 0;
}

///////////////// CHUNK ///////////////////////

CHUNK::CHUNK(META_CHUNK* mc, double s, int index) {
    parent = mc;
    size = s;
    if (strlen(parent->name)) {
        sprintf(name, "%s.%d", parent->name, index);
    } else {
        sprintf(name, "%d", index);
    }
    sprintf(dir, "%s/%d", mc->dir, index);
    char path[256];
    double fsize;
    sprintf(path, "%s/data.vda", dir);
    int retval = file_size(path, fsize);
    if (retval || fsize != size) {
        present_on_server = false;
    } else {
        present_on_server = true;
    }
}

// assign this chunk to a host
//
int CHUNK::assign() {
    int host_id = parent->dfile->choose_host();
    if (!host_id) {
        return ERR_NOT_FOUND;
    }
    DB_VDA_CHUNK_HOST ch;
    ch.create_time = dtime();
    ch.vda_file_id = parent->dfile->id;
    ch.host_id = host_id;
    physical_file_name(
        host_id, name, parent->dfile->file_name, ch.physical_file_name
    );
    ch.present_on_host = 0;
    ch.transfer_in_progress = true;
    ch.transfer_wait = true;
    ch.transfer_request_time = ch.create_time;
    ch.transfer_send_time = 0;
    int retval = ch.insert();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "ch.insert() failed\n");
        return retval;
    }
    log_messages.printf(MSG_NORMAL,
        "   assigning chunk %s to host %d\n", name, host_id
    );
    return 0;
}

int CHUNK::start_upload() {
    // if no upload of this chunk is in progress, start one.
    // NOTE: all instances are inherently present_on_host,
    // since this is only called if chunk is not present on server
    //
    VDA_CHUNK_HOST* chp;
    set<VDA_CHUNK_HOST*>::iterator i;
    for (i=hosts.begin(); i!=hosts.end(); i++) {
        chp = *i;
        if (chp->transfer_in_progress) return 0;
    }
    chp = *(hosts.begin());
    DB_VDA_CHUNK_HOST dch;
    char set_clause[256], where_clause[256];
    sprintf(set_clause,
        "transfer_in_progress=1, transfer_wait=1, transfer_request_time=%f",
        dtime()
    );
    sprintf(where_clause,
        "where vda_file_id=%d and host_id=%d and name='%s'",
        chp->vda_file_id,
        chp->host_id,
        name
    );
    int retval = dch.update_fields_noid(set_clause, where_clause);
    return retval;
}

///////////////// VDA_FILE_AUX ///////////////////////

// initialize a file: create its directory hierarchy
// and expand out its encoding tree,
// leaving only the bottom-level chunks
//
int VDA_FILE_AUX::init() {
    char buf[1024], buf2[1024];
    sprintf(buf, "%s/%s", dir, DATA_FILENAME);
    sprintf(buf2, "%s/%s", dir, file_name);
    int retval = symlink(buf2, buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "symlink %s %s failed\n", buf2, buf);
        return ERR_SYMLINK;
    }

    meta_chunk = new META_CHUNK(this, NULL, 0);
    retval = meta_chunk->init(dir, policy, 0);
    if (retval) return retval;
    sprintf(buf, "%s/chunk_sizes.txt", dir);
    FILE* f = fopen(buf, "w");
    for (int i=0; i<policy.coding_levels; i++) {
        fprintf(f, "%.0f\n", policy.chunk_sizes[i]);
    }
    fclose(f);

    // create symlink from download dir
    //
    dir_hier_path(file_name, config.download_dir, config.uldl_dir_fanout, buf);
    retval = symlink(dir, buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "symlink %s %s failed\n", buf2, buf);
        return ERR_SYMLINK;
    }

    return 0;
}

// get the state of an already-initialized file:
// expand the encoding tree,
// enumerate the VDA_HOST_CHUNKs from the DB
// and put them in the appropriate lists
//
int VDA_FILE_AUX::get_state() {
    char buf[256];

    sprintf(buf, "%s/chunk_sizes.txt", dir);
    FILE* f = fopen(buf, "r");
    if (!f) return -1;
    for (int i=0; i<policy.coding_levels; i++) {
        int n = fscanf(f, "%lf\n", &(policy.chunk_sizes[i]));
        if (n != 1) {
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    meta_chunk = new META_CHUNK(this, NULL, 0);
    int retval = meta_chunk->get_state(dir, policy, 0);
    if (retval) return retval;

    // enumerate the VDA_CHUNK_HOST records from DB and store in memory
    //
    DB_VDA_CHUNK_HOST vch;
    sprintf(buf, "where vda_file_id=%d", id);
    while (1) {
        retval = vch.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        vector<int> chunk_numbers;
        retval = get_chunk_numbers(vch, chunk_numbers);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "get_chunk_numbers(): %d\n", retval
            );
            return retval;
        }
        if ((int)(chunk_numbers.size()) != policy.coding_levels) {
            log_messages.printf(MSG_CRITICAL,
                "wrong get_chunk_numbers: got %d, expected %d\n",
                (int)(chunk_numbers.size()), policy.coding_levels
            );
            return -1;
        }
        META_CHUNK* mc = meta_chunk;
        for (int i=0; i<policy.coding_levels; i++) {
            if (i == policy.coding_levels-1) {
                CHUNK* c = (CHUNK*)(mc->children[chunk_numbers[i]]);
                VDA_CHUNK_HOST* vchp = new VDA_CHUNK_HOST();
                *vchp = vch;
                c->hosts.insert(vchp);
            } else {
                mc = (META_CHUNK*)(mc->children[chunk_numbers[i]]);
            }
        }

    }
    return 0;
}

// Pick a host to send a chunk of this file to.
// The host must:
// 1) be alive (recent RPC time)
// 2) not have any chunks of this file
//
// We maintain a cache of such hosts
// The policy is:
//
// - scan the cache, removing hosts that are no longer alive;
//   return if find a live host
// - pick a random starting point in host ID space,
//   and enumerate 100 live hosts; wrap around if needed.
//   Return one and put the rest in cache
//
int VDA_FILE_AUX::choose_host() {
    int retval;
    DB_HOST host;

    // replenish cache if needed
    //
    if (!available_hosts.size()) {
        int nhosts_scanned = 0;
        int rand_id;
        for (int i=0; i<2; i++) {
            char buf[256];
            if (i == 0) {
                retval = host.max_id(rand_id, "");
                if (retval) {
                    log_messages.printf(MSG_CRITICAL, "host.max_id() failed\n");
                    return 0;
                }
                rand_id = (int)(((double)id)*drand());
                sprintf(buf,
                    "where %s and id>=%d order by id limit 100",
                    host_alive_clause(), rand_id
                );
            } else {
                sprintf(buf,
                    "where %s and id<%d order by id limit %d",
                    host_alive_clause(), rand_id, 100-nhosts_scanned
                );
            }

            // debugging
            //
            strcpy(buf, "where id=467");
            //strcpy(buf, "where id=467 or id=166");

            while (1) {
                retval = host.enumerate(buf);
                if (retval == ERR_DB_NOT_FOUND) break;
                if (retval) {
                    log_messages.printf(MSG_CRITICAL, "host enum failed\n");
                    return 0;
                }
                nhosts_scanned++;
                DB_VDA_CHUNK_HOST ch;
                char buf2[256];
                int count;
                sprintf(buf2, "where vda_file_id=%d and host_id=%d", id, host.id);
#if 0
                retval = ch.count(count, buf2);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL, "ch.count failed\n");
                    return 0;
                }
#else
                count = 0;
#endif
                if (count == 0) {
                    available_hosts.push_back(host.id);
                }
                if (nhosts_scanned == 100) break;
            }
            if (nhosts_scanned == 100) break;
        }
    }

    while (available_hosts.size()) {
        int hostid = available_hosts.back();
        available_hosts.pop_back();
        retval = host.lookup_id(hostid);
        if (retval || !alive(host)) {
            continue;
        }
        return hostid;
    }

    log_messages.printf(MSG_CRITICAL, "No hosts available\n");
    return 0;
}

