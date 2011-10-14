// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#include <cstring>
#include <cstdlib>
#endif


#ifdef _WIN32
#include "win_util.h"
#else
#ifdef __APPLE__
// Suppress obsolete warning when building for OS 10.3.9
#define DLOPEN_NO_WARN
#include <mach-o/dyld.h>
#endif
#include "config.h"
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "util.h"

#include "coproc.h"

#ifndef _USING_FCGI_
using std::perror;
#endif

int COPROC_REQ::parse(XML_PARSER& xp) {
    strcpy(type, "");
    count = 0;
    while (!xp.get_tag()) {
        if (xp.match_tag("/coproc")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_str("type", type, sizeof(type))) continue;
        if (xp.parse_double("count", count)) continue;
    }
    return ERR_XML_PARSE;
}

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

void COPROC::write_request(MIOFILE& f) {
    f.printf(
        "   <req_secs>%f</req_secs>\n"
        "   <req_instances>%f</req_instances>\n"
        "   <estimated_delay>%f</estimated_delay>\n",
        req_secs,
        req_instances,
        estimated_delay
    );
}

void OPENCL_DEVICE_PROP::write_xml(MIOFILE& f) {
    f.printf(
        "   <coproc_opencl>\n"
        "      <name>%s</name>\n"
        "      <vendor>%s</vendor>\n"
        "      <vendor_id>%lu</vendor_id>\n"
        "      <available>%d</available>\n"
        "      <half_fp_config>%llu</half_fp_config>\n"
        "      <single_fp_config>%llu</single_fp_config>\n"
        "      <double_fp_config>%llu</double_fp_config>\n"
        "      <endian_little>%d</endian_little>\n"
        "      <execution_capabilities>%llu</execution_capabilities>\n"
        "      <extensions>%s</extensions>\n"
        "      <global_mem_size>%llu</global_mem_size>\n"
        "      <local_mem_size>%llu</local_mem_size>\n"
        "      <max_clock_frequency>%lu</max_clock_frequency>\n"
        "      <max_compute_units>%lu</max_compute_units>\n"
        "      <opencl_platform_version>%s</opencl_platform_version>\n"
        "      <opencl_device_version>%s</opencl_device_version>\n"
        "      <opencl_driver_version>%s</opencl_driver_version>\n"
        "   </coproc_opencl>\n",
        name,
        vendor,
        vendor_id,
        available ? 1 : 0,
        half_fp_config,
        single_fp_config,
        double_fp_config,
        endian_little ? 1 : 0,
        execution_capabilities,
        extensions,
        global_mem_size,
        local_mem_size,
        max_clock_frequency,
        max_compute_units,
        opencl_platform_version,
        opencl_device_version,
        opencl_driver_version
    );
}

int COPROC::parse(XML_PARSER& xp) {
    char buf[256];
    strcpy(type, "");
    clear();
    for (int i=0; i<MAX_COPROC_INSTANCES; i++) {
        device_nums[i] = i;
    }
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("/coproc")) {
            if (!strlen(type)) return ERR_XML_PARSE;
            return 0;
        }
        if (xp.parse_str("type", type, sizeof(type))) continue;
        if (xp.parse_int("count", count)) continue;
        if (xp.parse_double("peak_flops", peak_flops)) continue;
        if (xp.parse_str("device_nums", buf, sizeof(buf))) {
            int i=0;
            char* p = strtok(buf, " ");
            while (p && i<MAX_COPROC_INSTANCES) {
                device_nums[i++] = atoi(p);
                p = strtok(NULL, " ");
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

#endif

int OPENCL_DEVICE_PROP::parse(XML_PARSER& xp) {
    int n;
    unsigned long long ull;

    while (!xp.get_tag()) {
        if (xp.match_tag("/coproc_opencl")) {
            get_device_version_int();
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("vendor", vendor, sizeof(vendor))) continue;
        if (xp.parse_int("available", n)) {
            available = n;
            continue;
        }
        if (xp.parse_ulonglong("half_fp_config", ull)) {
                half_fp_config = ull;
                continue; 
        }
        if (xp.parse_ulonglong("single_fp_config", ull)) {
            single_fp_config = ull;
            continue; 
        }
        if (xp.parse_ulonglong("double_fp_config", ull)) {
            double_fp_config = ull;
            continue; 
        }
        if (xp.parse_int("endian_little", n)) {
            endian_little = n;
            continue;
        }
        if (xp.parse_ulonglong("execution_capabilities", ull)) {
            execution_capabilities = ull;
            continue;
        }
        if (xp.parse_str("extensions", 
            extensions, 
            sizeof(extensions)
        )) {
            continue;
        }
        if (xp.parse_ulonglong("global_mem_size", ull)) {
            global_mem_size = ull;
            continue;
        }
        if (xp.parse_ulonglong("local_mem_size", ull)) {
            local_mem_size = ull;
            continue;
        }
        if (xp.parse_int("max_clock_frequency", n)) {
            max_clock_frequency = n;
            continue;
        }
        if (xp.parse_int("max_compute_units", n)) {
            max_compute_units = n;
            continue;
        }
        if (xp.parse_str("opencl_platform_version", 
            opencl_platform_version, 
            sizeof(opencl_platform_version)
        )) {
            continue;
        }
        if (xp.parse_str("opencl_device_version", 
            opencl_device_version, 
            sizeof(opencl_device_version)
        )) {
            continue;
        }
        if (xp.parse_str("opencl_driver_version", 
            opencl_driver_version, 
            sizeof(opencl_driver_version)
        )) {
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int OPENCL_DEVICE_PROP::get_device_version_int() {
    int maj, min;
    int n = sscanf(
        opencl_device_version, "OpenCL %d.%d", &maj, &min
    );
    if (n != 2) {
        return ERR_NOT_FOUND;
    }
    opencl_device_version_int = 100*maj + min;
    return 0;
}

void OPENCL_DEVICE_PROP::description(char* buf, const char* type) {
    char s1[256], s2[256];
    int n;
    
    // openCL_device_version may have a trailing space
    strlcpy(s1, opencl_device_version, sizeof(s1));
    n = strlen(s1) - 1;
    if ((n > 0) && (s1[n] == ' ')) s1[n] = '\0';
    sprintf(s2, "%s (driver version %s, device version %s, %.0fMB)",
        name, opencl_driver_version, s1, global_mem_size/MEGA
    );

    switch(is_used) {
    case COPROC_IGNORED:
        sprintf(buf, "OpenCL: %s GPU %d (ignored by config): %s", type, device_num, s2);
        break;
    case COPROC_USED:
        sprintf(buf, "OpenCL: %s GPU %d: %s", type, device_num, s2);
        break;
    case COPROC_UNUSED:
    default:
        sprintf(buf, "OpenCL: %s GPU %d (not used): %s", type, device_num, s2);
        break;
    }
}

void COPROCS::summary_string(char* buf, int len) {
    char buf2[1024];

    strcpy(buf, "");
    if (nvidia.count) {
        int mem = (int)(nvidia.prop.dtotalGlobalMem/MEGA);
        sprintf(buf2, "[CUDA|%s|%d|%dMB|%d]",
            nvidia.prop.name, nvidia.count, mem, nvidia.display_driver_version
        );
        strlcat(buf, buf2, len);
    }
    if (ati.count) {
        sprintf(buf2,"[CAL|%s|%d|%dMB|%s]",
            ati.name, ati.count, ati.attribs.localRAM, ati.version
        );
        strlcat(buf, buf2, len);
    }
}

int COPROCS::parse(XML_PARSER& xp) {
    char buf[1024];
    int retval;

    clear();
    n_rsc = 1;
    strcpy(coprocs[0].type, "CPU");
    MIOFILE& in = *(xp.f);
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</coprocs>")) {
            return 0;
        }
        if (match_tag(buf, "<coproc_cuda>")) {
            retval = nvidia.parse(xp);
            if (retval) {
                nvidia.clear();
            } else {
                coprocs[n_rsc++] = nvidia;
            }
            continue;
        }
        if (match_tag(buf, "<coproc_ati>")) {
            retval = ati.parse(xp);
            if (retval) {
                ati.clear();
            } else {
                coprocs[n_rsc++] = ati;
            }
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void COPROCS::write_xml(MIOFILE& mf, bool include_request) {
#ifndef _USING_FCGI_
//TODO: Write coprocs[0] through coprocs[n_rsc]
    mf.printf("    <coprocs>\n");
    if (nvidia.count) {
        nvidia.write_xml(mf, include_request);
    }
    if (ati.count) {
        ati.write_xml(mf, include_request);
    }
    mf.printf("    </coprocs>\n");
#endif
}

void COPROC_NVIDIA::description(char* buf) {
    char vers[256], cuda_vers[256];
    if (display_driver_version) {
#ifdef __APPLE__
     	int maj = display_driver_version >> 16;
    	int min = (display_driver_version >> 8) & 0xff;
    	int rev = display_driver_version & 0xff;
        sprintf(vers, "%d.%d.%d", maj, min, rev);
#else
        int maj = display_driver_version/100;
        int min = display_driver_version%100;
        sprintf(vers, "%d.%d", maj, min);
#endif
    } else {
        strcpy(vers, "unknown");
    }
    if (cuda_version) {
        int maj = cuda_version/1000;
        int min = cuda_version%1000;
        sprintf(cuda_vers, "%d.%d", maj, min);
    } else {
        strcpy(cuda_vers, "unknown");
    }
    sprintf(buf, "%s (driver version %s, CUDA version %s, compute capability %d.%d, %.0fMB, %.0fMB available, %.0f GFLOPS peak)",
        prop.name, vers, cuda_vers, prop.major, prop.minor,
        prop.totalGlobalMem/MEGA, available_ram/MEGA, peak_flops/1e9
    );
}

#ifndef _USING_FCGI_
void COPROC_NVIDIA::write_xml(MIOFILE& f, bool include_request) {
    f.printf(
        "<coproc_cuda>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <available_ram>%f</available_ram>\n"
        "   <have_cuda>%d</have_cuda>\n"
        "   <have_opencl>%d</have_opencl>\n",
        count,
        prop.name,
        available_ram,
        have_cuda ? 1 : 0,
        have_opencl ? 1 : 0
    );
    if (include_request) {
        write_request(f);
    }
    f.printf(
        "   <peak_flops>%f</peak_flops>\n"
        "   <cudaVersion>%d</cudaVersion>\n"
        "   <drvVersion>%d</drvVersion>\n"
        "   <deviceHandle>%p</deviceHandle>\n"
        "   <totalGlobalMem>%u</totalGlobalMem>\n"
        "   <sharedMemPerBlock>%u</sharedMemPerBlock>\n"
        "   <regsPerBlock>%d</regsPerBlock>\n"
        "   <warpSize>%d</warpSize>\n"
        "   <memPitch>%u</memPitch>\n"
        "   <maxThreadsPerBlock>%d</maxThreadsPerBlock>\n"
        "   <maxThreadsDim>%d %d %d</maxThreadsDim>\n"
        "   <maxGridSize>%d %d %d</maxGridSize>\n"
        "   <clockRate>%d</clockRate>\n"
        "   <totalConstMem>%u</totalConstMem>\n"
        "   <major>%d</major>\n"
        "   <minor>%d</minor>\n"
        "   <textureAlignment>%u</textureAlignment>\n"
        "   <deviceOverlap>%d</deviceOverlap>\n"
        "   <multiProcessorCount>%d</multiProcessorCount>\n",
        peak_flops,
        cuda_version,
        display_driver_version,
        prop.deviceHandle,
        (unsigned int)prop.totalGlobalMem,
        (unsigned int)prop.sharedMemPerBlock,
        prop.regsPerBlock,
        prop.warpSize,
        (unsigned int)prop.memPitch,
        prop.maxThreadsPerBlock,
        prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2],
        prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2],
        prop.clockRate,
        (unsigned int)prop.totalConstMem,
        prop.major,
        prop.minor,
        (unsigned int)prop.textureAlignment,
        prop.deviceOverlap,
        prop.multiProcessorCount
    );

    if (have_opencl) {
        opencl_prop.write_xml(f);
    }
    
    f.printf("</coproc_cuda>\n");
}
#endif

void COPROC_NVIDIA::clear() {
    COPROC::clear();
    strcpy(type, GPU_TYPE_NVIDIA);
    estimated_delay = -1;   // mark as absent
    cuda_version = 0;
    display_driver_version = 0;
    strcpy(prop.name, "");
    prop.deviceHandle = 0;
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

int COPROC_NVIDIA::parse(XML_PARSER& xp) {
    char buf2[256];
    int retval;

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/coproc_cuda")) {
            if (!peak_flops) {
				set_peak_flops();
            }
            if (!available_ram) {
                available_ram = prop.dtotalGlobalMem;
            }
            return 0;
        }
        if (xp.parse_int("count", count)) continue;
        if (xp.parse_double("peak_flops", peak_flops)) continue;
        if (xp.parse_bool("have_cuda", have_cuda)) continue;
        if (xp.parse_bool("have_opencl", have_opencl)) continue;
        if (xp.parse_double("available_ram", available_ram)) continue;
        if (xp.parse_double("req_secs", req_secs)) continue;
        if (xp.parse_double("req_instances", req_instances)) continue;
        if (xp.parse_double("estimated_delay", estimated_delay)) continue;
        if (xp.parse_int("cudaVersion", cuda_version)) continue;
        if (xp.parse_int("drvVersion", display_driver_version)) continue;
        if (xp.parse_str("name", prop.name, sizeof(prop.name))) continue;
        if (xp.parse_int("deviceHandle", prop.deviceHandle)) continue;
        if (xp.parse_double("totalGlobalMem", prop.dtotalGlobalMem)) {
            prop.totalGlobalMem = (int)prop.dtotalGlobalMem;
            continue;
        }
        if (xp.parse_int("sharedMemPerBlock", (int&)prop.sharedMemPerBlock)) continue;
        if (xp.parse_int("regsPerBlock", prop.regsPerBlock)) continue;
        if (xp.parse_int("warpSize", prop.warpSize)) continue;
        if (xp.parse_int("memPitch", (int&)prop.memPitch)) continue;
        if (xp.parse_int("maxThreadsPerBlock", prop.maxThreadsPerBlock)) continue;
        if (xp.parse_str("maxThreadsDim", buf2, sizeof(buf2))) {
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
        if (xp.parse_str("maxGridSize", buf2, sizeof(buf2))) {
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
        if (xp.parse_int("clockRate", prop.clockRate)) continue;
        if (xp.parse_int("totalConstMem", (int&)prop.totalConstMem)) continue;
        if (xp.parse_int("major", prop.major)) continue;
        if (xp.parse_int("minor", prop.minor)) continue;
        if (xp.parse_int("textureAlignment", (int&)prop.textureAlignment)) continue;
        if (xp.parse_int("deviceOverlap", prop.deviceOverlap)) continue;
        if (xp.parse_int("multiProcessorCount", prop.multiProcessorCount)) continue;
        if (xp.match_tag("coproc_opencl")) {
            retval = opencl_prop.parse(xp);
            if (retval) return retval;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void COPROC_NVIDIA::set_peak_flops() {
    double x=0;
    if (have_cuda) {
        int flops_per_clock=0, cores_per_proc=0;
        switch (prop.major) {
        case 1:
            flops_per_clock = 3;
            cores_per_proc = 8;
            break;
        case 2:
            flops_per_clock = 2;
            switch (prop.minor) {
            case 0:
                cores_per_proc = 32;
                break;
            default:
                cores_per_proc = 48;
                break;
            }
        }
        // clock rate is scaled down by 1000
        //
        x = (1000.*prop.clockRate) * prop.multiProcessorCount * cores_per_proc * flops_per_clock;
    } else if (have_opencl) {
        // OpenCL doesn't give us compute capability.
        // assume cores_per_proc is 8 and flops_per_clock is 2
        //
        x = opencl_prop.max_compute_units * 8 * 2 * opencl_prop.max_clock_frequency * 1e6;
    }
    peak_flops =  (x>0)?x:5e10;
}

////////////////// ATI STARTS HERE /////////////////

#ifndef _USING_FCGI_
void COPROC_ATI::write_xml(MIOFILE& f, bool include_request) {
    f.printf(
        "<coproc_ati>\n"
        "   <count>%d</count>\n"
        "   <name>%s</name>\n"
        "   <available_ram>%f</available_ram>\n"
        "   <have_cal>%d</have_cal>\n"
        "   <have_opencl>%d</have_opencl>\n",
        count,
        name,
        available_ram,
        have_cal ? 1 : 0,
        have_opencl ? 1 : 0
    );
    if (include_request) {
        write_request(f);
    }
    f.printf(
        "   <peak_flops>%f</peak_flops>\n"
        "   <CALVersion>%s</CALVersion>\n"
        "   <target>%d</target>\n"
        "   <localRAM>%d</localRAM>\n"
        "   <uncachedRemoteRAM>%d</uncachedRemoteRAM>\n"
        "   <cachedRemoteRAM>%d</cachedRemoteRAM>\n"
        "   <engineClock>%u</engineClock>\n"
        "   <memoryClock>%d</memoryClock>\n"
        "   <wavefrontSize>%d</wavefrontSize>\n"
        "   <numberOfSIMD>%d</numberOfSIMD>\n"
        "   <doublePrecision>%d</doublePrecision>\n"
        "   <pitch_alignment>%d</pitch_alignment>\n"
        "   <surface_alignment>%d</surface_alignment>\n"
        "   <maxResource1DWidth>%d</maxResource1DWidth>\n"
        "   <maxResource2DWidth>%d</maxResource2DWidth>\n"
        "   <maxResource2DHeight>%d</maxResource2DHeight>\n",
        peak_flops,
        version,
        attribs.target,
        attribs.localRAM,
        attribs.uncachedRemoteRAM,
        attribs.cachedRemoteRAM,
        attribs.engineClock,
        attribs.memoryClock,
        attribs.wavefrontSize,
        attribs.numberOfSIMD,
        attribs.doublePrecision,
        attribs.pitch_alignment,
        attribs.surface_alignment,
        info.maxResource1DWidth,
        info.maxResource2DWidth,
        info.maxResource2DHeight
    );

    if (atirt_detected) {
        f.printf("    <atirt_detected/>\n");
    }

    if (amdrt_detected) {
        f.printf("    <amdrt_detected/>\n");
    }

    if (have_opencl) {
        opencl_prop.write_xml(f);
    }
        
    f.printf("</coproc_ati>\n");
};
#endif

void COPROC_ATI::clear() {
    COPROC::clear();
    strcpy(type, GPU_TYPE_ATI);
    estimated_delay = -1;
    strcpy(name, "");
    strcpy(version, "");
    atirt_detected = false;
    amdrt_detected = false;
    memset(&attribs, 0, sizeof(attribs));
    memset(&info, 0, sizeof(info));
}

int COPROC_ATI::parse(XML_PARSER& xp) {
    int n, retval;

    clear();

    while (!xp.get_tag()) {
        if (xp.match_tag("/coproc_ati")) {
            int major, minor, release;
            sscanf(version, "%d.%d.%d", &major, &minor, &release);
            version_num = major*1000000 + minor*1000 + release;

            if (!peak_flops) {
				set_peak_flops();
            }
            if (!available_ram) {
                available_ram = attribs.localRAM*MEGA;
            }
            return 0;
        }
        if (xp.parse_int("count", count)) continue;
        if (xp.parse_double("peak_flops", peak_flops)) continue;
        if (xp.parse_bool("have_cal", have_cal)) continue;
        if (xp.parse_bool("have_opencl", have_opencl)) continue;
        if (xp.parse_double("available_ram", available_ram)) continue;
        if (xp.parse_double("req_secs", req_secs)) continue;
        if (xp.parse_double("req_instances", req_instances)) continue;
        if (xp.parse_double("estimated_delay", estimated_delay)) continue;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("CALVersion", version, sizeof(version))) continue;
        if (xp.parse_bool("amdrt_detected", amdrt_detected)) continue;
        if (xp.parse_bool("atirt_detected", atirt_detected)) continue;

        if (xp.parse_int("target", n)) {
            attribs.target = (CALtarget)n;
            continue;
        }
        if (xp.parse_int("localRAM", n)) {
            attribs.localRAM = n;
            continue;
        }
        if (xp.parse_int("uncachedRemoteRAM", n)) {
            attribs.uncachedRemoteRAM = n;
            continue;
        }
        if (xp.parse_int("cachedRemoteRAM", n)) {
            attribs.cachedRemoteRAM = n;
            continue;
        }
        if (xp.parse_int("engineClock", n)) {
            attribs.engineClock = n;
            continue;
        }
        if (xp.parse_int("memoryClock", n)) {
            attribs.memoryClock = n;
            continue;
        }
        if (xp.parse_int("wavefrontSize", n)) {
            attribs.wavefrontSize = n;
            continue;
        }
        if (xp.parse_int("numberOfSIMD"  , n)) {
            attribs.numberOfSIMD = n;
            continue;
        }
        if (xp.parse_int("doublePrecision", n)) {
            attribs.doublePrecision = n?CAL_TRUE:CAL_FALSE;
            continue;
        }
        if (xp.parse_int("pitch_alignment", n)) {
            attribs.pitch_alignment = n;
            continue;
        }
        if (xp.parse_int("surface_alignment", n)) {
            attribs.surface_alignment = n;
            continue;
        }
        if (xp.parse_int("maxResource1DWidth", n)) {
            info.maxResource1DWidth = n;
            continue;
        }
        if (xp.parse_int("maxResource2DWidth", n)) {
            info.maxResource2DWidth = n;
            continue;
        }
        if (xp.parse_int("maxResource2DHeight", n)) {
            info.maxResource2DHeight = n;
            continue;
        }
        if (xp.match_tag("coproc_opencl")) {
            retval = opencl_prop.parse(xp);
            if (retval) return retval;
            continue;
        }
    }
    return ERR_XML_PARSE;
}

void COPROC_ATI::description(char* buf) {
    sprintf(buf, "%s (CAL version %s, %dMB, %.0fMB available, %.0f GFLOPS peak)",
        name, version, attribs.localRAM, available_ram/MEGA, peak_flops/1.e9
    );
}

void COPROC_ATI::set_peak_flops() {
    double x = 0;
    if (have_cal) {
        x = attribs.numberOfSIMD * attribs.wavefrontSize * 5 * attribs.engineClock * 1.e6;
        // clock is in MHz
    } else if (have_opencl) {
        // OpenCL gives us only:
        // - max_compute_units
        //   (which I'll assume is the same as attribs.numberOfSIMD)
        // - max_clock_frequency (which I'll assume is the same as engineClock)
        // It doesn't give wavefrontSize, which can be 16/32/64.
        // So let's be conservative and use 16
        //
        x = opencl_prop.max_compute_units * 16 * 5 * opencl_prop.max_clock_frequency * 1e6;
    }
    peak_flops = (x>0)?x:5e10;
}
