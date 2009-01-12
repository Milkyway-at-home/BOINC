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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#endif
#include <dlfcn.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_util.h"

#include "coproc.h"

using std::string;
using std::vector;

COPROC_CUDA* coproc_cuda;

#ifndef _USING_FCGI_
void COPROC::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc>\n"
        "   <type>%s</type>\n"
        "   <count>%d</count>\n"
        "</coproc>\n",
        type, count
    );
}
#endif

int COPROC::parse(MIOFILE& fin) {
    char buf[1024];
    strcpy(type, "");
    count = 0;
    used = 0;
    req_secs = 0;
    req_instances = 0;
    while (fin.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coproc>")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (parse_str(buf, "<type>", type, sizeof(type))) continue;
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_int(buf, "<req_instances>", req_instances)) continue;
    }
    return ERR_XML_PARSE;
}

void COPROCS::summary_string(char* buf, int len) {
    char bigbuf[8192], buf2[1024];

    strcpy(bigbuf, "");
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(cp->type, "CUDA")) {
            COPROC_CUDA* cp2 = (COPROC_CUDA*) cp;
            int mem = (int)(cp2->prop.dtotalGlobalMem/MEGA);
            sprintf(buf2, "[CUDA|%s|%d|%dMB]", cp2->prop.name, cp2->count, mem);
            strcat(bigbuf, buf2);
        }
    }
    bigbuf[len-1] = 0;
    strcpy(buf, bigbuf);
}

vector<string> COPROCS::get() {
    vector<string> strings;
    string s = COPROC_CUDA::get(*this);
    if (s.size()) strings.push_back(s);
    s = COPROC_CELL_SPE::get(*this);
    if (s.size()) strings.push_back(s);
    return strings;
}

int COPROCS::parse(FILE* fin) {
    char buf[1024];

    while (fgets(buf, sizeof(buf), fin)) {
        if (match_tag(buf, "</coprocs>")) {
            return 0;
        }
        if (strstr(buf, "<coproc_cuda>")) {
            COPROC_CUDA* cc = new COPROC_CUDA;
            int retval = cc->parse(fin);
            if (!retval) {
                coprocs.push_back(cc);
                coproc_cuda = cc;
            }
        }
    }
    return ERR_XML_PARSE;
}

COPROC* COPROCS::lookup(const char* type) {
    for (unsigned int i=0; i<coprocs.size(); i++) {
        COPROC* cp = coprocs[i];
        if (!strcmp(type, cp->type)) return cp;
    }
    return NULL;
}

#ifdef _WIN32


#endif

string COPROC_CUDA::get(COPROCS& coprocs) {
    int count;

#ifdef _WIN32

    typedef int (__stdcall *PCGDC)(int *count);
    typedef int (__stdcall *PCGDP)(struct cudaDeviceProp *prop, int device);

    PCGDC __cudaGetDeviceCount = NULL;
    PCGDP __cudaGetDeviceProperties = NULL;

    HMODULE cudalib = LoadLibrary("cudart.dll");
    if (!cudalib) {
        return "Can't load library cudart.dll";
    }

    __cudaGetDeviceCount = (PCGDC)GetProcAddress( cudalib, "cudaGetDeviceCount" );
    if (!__cudaGetDeviceCount) {
        return "Library doesn't have cudaGetDeviceCount()";
    }

    __cudaGetDeviceProperties = (PCGDP)GetProcAddress( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) {
        return "Library doesn't have cudaGetDeviceProperties()";
    }

#else
    void* cudalib;
    void (*__cudaGetDeviceCount)(int*);
    void (*__cudaGetDeviceProperties)(cudaDeviceProp*, int);

#ifdef __APPLE__
    cudalib = dlopen("/usr/local/cuda/lib/libcudart.dylib", RTLD_NOW);
#else
    cudalib = dlopen("./libcudart.so", RTLD_NOW);
#endif
    if (!cudalib) {
        return "Can't load library libcudart";
        perror("dlopen");
    }
    __cudaGetDeviceCount = (void(*)(int*)) dlsym(cudalib, "cudaGetDeviceCount");
    if(!__cudaGetDeviceCount) {
        return "Library doesn't have cudaGetDeviceCount()";
    }
    __cudaGetDeviceProperties = (void(*)(cudaDeviceProp*, int)) dlsym( cudalib, "cudaGetDeviceProperties" );
    if (!__cudaGetDeviceProperties) {
        return "Library doesn't have cudaGetDeviceProperties()";
    }
#endif

    // NOTE: our design is slightly flawed:
    // there's no provision for having two coprocs of type CUDA.
    // So on systems with two GPUs of different hardware types
    // we have to count them as two of the same type.
    // Pick the fastest.
    //
    (*__cudaGetDeviceCount)(&count);
    int real_count = 0;
    COPROC_CUDA cc, cc2;
    string s;
    for (int i=0; i<count; i++) {
        (*__cudaGetDeviceProperties)(&cc.prop, i);
        if (cc.prop.major <= 0) continue;  // major == 0 means emulation
        if (cc.prop.major > 100) continue;  // e.g. 9999 is an error

        if (real_count) {
            if (cc.flops_estimate() > cc2.flops_estimate()) {
                cc2 = cc;
            }
            s += ", ";
            s += cc.prop.name;
        } else {
            s = cc.prop.name;
            cc2 = cc;
        }
        real_count++;
    }
    if (!real_count) {
        return "No CUDA devices found";
    }
    COPROC_CUDA* ccp = new COPROC_CUDA;
    *ccp = cc2;
    ccp->count = real_count;
    strcpy(ccp->type, "CUDA");
    coprocs.coprocs.push_back(ccp);
    coproc_cuda = ccp;
    if (real_count == 1) {
        return "CUDA device: "+s;
    } else {
        return "CUDA devices: "+s;
    }
}

// add a non-existent CUDA coproc (for debugging)
//
void fake_cuda(COPROCS& coprocs, int count) {
   COPROC_CUDA* cc = new COPROC_CUDA;
   strcpy(cc->type, "CUDA");
   cc->count = count;
   strcpy(cc->prop.name, "CUDA NVIDIA chip");
   cc->prop.totalGlobalMem = 256*1024*1024;
   cc->prop.sharedMemPerBlock = 100;
   cc->prop.regsPerBlock = 8;
   cc->prop.warpSize = 10;
   cc->prop.memPitch = 10;
   cc->prop.maxThreadsPerBlock = 20;
   cc->prop.maxThreadsDim[0] = 2;
   cc->prop.maxThreadsDim[1] = 2;
   cc->prop.maxThreadsDim[2] = 2;
   cc->prop.maxGridSize[0] = 10;
   cc->prop.maxGridSize[1] = 10;
   cc->prop.maxGridSize[2] = 10;
   cc->prop.totalConstMem = 10;
   cc->prop.major = 1;
   cc->prop.minor = 2;
   cc->prop.clockRate = 1250000;
   cc->prop.textureAlignment = 1000;
   cc->prop.multiProcessorCount = 14;
   coprocs.coprocs.push_back(cc);
   coproc_cuda = cc;
}

#ifndef _USING_FCGI_
void COPROC_CUDA::write_xml(MIOFILE& f) {
    f.printf(
        "<coproc_cuda>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%d</req_instances>\n"
        "   <totalGlobalMem>%u</totalGlobalMem>\n"
        "   <sharedMemPerBlock>%u</sharedMemPerBlock>\n"
        "   <regsPerBlock>%d</regsPerBlock>\n"
        "   <warpSize>%d</warpSize>\n"
        "   <memPitch>%u</memPitch>\n"
        "   <maxThreadsPerBlock>%d</maxThreadsPerBlock>\n"
        "   <maxThreadsDim>%d %d %d</maxThreadsDim>\n"
        "   <maxGridSize>%d %d %d</maxGridSize>\n"
        "   <totalConstMem>%u</totalConstMem>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <clockRate>%d</clockRate>\n"
        "   <textureAlignment>%u</textureAlignment>\n"
        "   <deviceOverlap>%d</deviceOverlap>\n"
        "   <multiProcessorCount>%d</multiProcessorCount>\n"
        "</coproc_cuda>\n",
        count,
        prop.name,
        req_secs,
        req_instances,
        (unsigned int)prop.totalGlobalMem,
        (unsigned int)prop.sharedMemPerBlock,
        prop.regsPerBlock,
        prop.warpSize,
        (unsigned int)prop.memPitch,
        prop.maxThreadsPerBlock,
        prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2],
        prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2],
        (unsigned int)prop.totalConstMem,
        prop.major,
        prop.minor,
        prop.clockRate,
        (unsigned int)prop.textureAlignment,
        prop.deviceOverlap,
        prop.multiProcessorCount
    );
}
#endif

void COPROC_CUDA::clear() {
    count = 0;
    strcpy(prop.name, "");
    prop.totalGlobalMem = 0;
    prop.sharedMemPerBlock = 0;
    prop.regsPerBlock = 0;
    prop.warpSize = 0;
    prop.memPitch = 0;
    prop.maxThreadsPerBlock = 0;
    prop.maxThreadsDim[0] = 0;
    prop.maxThreadsDim[1] = 0;
    prop.maxThreadsDim[2] = 0;
    prop.maxGridSize[0] = 0;
    prop.maxGridSize[1] = 0;
    prop.maxGridSize[2] = 0;
    prop.clockRate = 0;
    prop.totalConstMem = 0;
    prop.major = 0;
    prop.minor = 0;
    prop.textureAlignment = 0;
    prop.deviceOverlap = 0;
    prop.multiProcessorCount = 0;
}

int COPROC_CUDA::parse(FILE* fin) {
    char buf[1024], buf2[256];

    clear();
    while (fgets(buf, sizeof(buf), fin)) {
        if (strstr(buf, "</coproc_cuda>")) {
            return 0;
        }
        if (parse_int(buf, "<count>", count)) continue;
        if (parse_double(buf, "<req_secs>", req_secs)) continue;
        if (parse_int(buf, "<req_instances>", req_instances)) continue;
        if (parse_str(buf, "<name>", prop.name, sizeof(prop.name))) continue;
        if (parse_double(buf, "<totalGlobalMem>", prop.dtotalGlobalMem)) continue;
        if (parse_int(buf, "<sharedMemPerBlock>", (int&)prop.sharedMemPerBlock)) continue;
        if (parse_int(buf, "<regsPerBlock>", prop.regsPerBlock)) continue;
        if (parse_int(buf, "<warpSize>", prop.warpSize)) continue;
        if (parse_int(buf, "<memPitch>", (int&)prop.memPitch)) continue;
        if (parse_int(buf, "<maxThreadsPerBlock>", prop.maxThreadsPerBlock)) continue;
        if (parse_str(buf, "<maxThreadsDim>", buf2, sizeof(buf2))) {
            // can't use sscanf here (FCGI)
            //
            prop.maxThreadsDim[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                p++;
                prop.maxThreadsDim[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    p++;
                    prop.maxThreadsDim[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_str(buf, "<maxGridSize>", buf2, sizeof(buf2))) {
            prop.maxGridSize[0] = atoi(buf2);
            char* p = strchr(buf2, ' ');
            if (p) {
                p++;
                prop.maxGridSize[1] = atoi(p);
                p = strchr(p, ' ');
                if (p) {
                    p++;
                    prop.maxGridSize[2] = atoi(p);
                }
            }
            continue;
        }
        if (parse_int(buf, "<clockRate>", prop.clockRate)) continue;
        if (parse_int(buf, "<totalConstMem>", (int&)prop.totalConstMem)) continue;
        if (parse_int(buf, "<major>", prop.major)) continue;
        if (parse_int(buf, "<minor>", prop.minor)) continue;
        if (parse_int(buf, "<textureAlignment>", (int&)prop.textureAlignment)) continue;
        if (parse_int(buf, "<deviceOverlap>", prop.deviceOverlap)) continue;
        if (parse_int(buf, "<multiProcessorCount>", prop.multiProcessorCount)) continue;
    }
    return ERR_XML_PARSE;
}

string COPROC_CELL_SPE::get(COPROCS&) {
    return "";
}
