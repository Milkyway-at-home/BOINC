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
// 

#ifdef _WIN32
#include "boinc_win.h"

#include <windowsx.h>
#include <mmsystem.h>
#include <regstr.h>
#include <strsafe.h>

#include "boinc_ss.h"

#endif

#include "diagnostics.h"
#include "common_defs.h"
#include "util.h"
#include "common_defs.h"
#include "filesys.h"
#include "error_numbers.h"


#ifdef _WIN32
#include "gui_rpc_client.h"
#include "screensaver_win.h"
#include "str_util.h"
// Allow for Unicode wide characters
#define PATH_SEPARATOR (_T("\\"))
#define THE_DEFAULT_SS_EXECUTABLE (_T(DEFAULT_SS_EXECUTABLE))
#define THE_SS_CONFIG_FILE (_T(SS_CONFIG_FILE))
#define DEFAULT_GFX_CANT_CONNECT ERR_CONNECT
#else
// Using (_T()) here causes compiler errors on Mac
#define PATH_SEPARATOR "/"
#define THE_DEFAULT_SS_EXECUTABLE DEFAULT_SS_EXECUTABLE
#define THE_SS_CONFIG_FILE SS_CONFIG_FILE
#define DEFAULT_GFX_CANT_CONNECT (ERR_CONNECT & 0xff)
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <sys/wait.h>
#include "gui_rpc_client.h"
#include "Mac_Saver_Module.h"
#endif
#include "screensaver.h"

// Flags for testing & debugging
#define SIMULATE_NO_GRAPHICS 0


bool CScreensaver::is_same_task(RESULT* taska, RESULT* taskb) {
    if ((taska == NULL) || (taskb == NULL)) return false;
    if (taska->name != taskb->name) return false;
    if (taska->project_url != taskb->project_url) return false;
    return true;
}

int CScreensaver::count_active_graphic_apps(RESULTS& results, RESULT* exclude) {
    unsigned int i = 0;
    unsigned int graphics_app_count = 0;
    m_bV5_GFX_app_is_running = false;

    // Count the number of active graphics-capable apps excluding the specified result.
    // If exclude is NULL, don't exclude any results.
    for (i = 0; i < results.results.size(); i++) {
        BOINCTRACE(_T("get_random_graphics_app -- active task detected\n"));
        BOINCTRACE(
            _T("get_random_graphics_app -- name = '%s', path = '%s'\n"),
            results.results[i]->name.c_str(), results.results[i]->graphics_exec_path.c_str()
        );
        if (results.results[i]->supports_graphics) m_bV5_GFX_app_is_running = true;
        if ((results.results[i]->graphics_exec_path.size() == 0) 
                && (state.executing_as_daemon || !(results.results[i]->supports_graphics))) continue;
        BOINCTRACE(_T("get_random_graphics_app -- active task detected w/graphics\n"));

        if (is_same_task(results.results[i], exclude)) continue;
        graphics_app_count++;
    }
    return graphics_app_count;
}


// Choose a random graphics application out of the vector.
// Exclude the specified result unless it is the only candidate.
// If exclude is NULL or an empty string, don't exclude any results.
//
RESULT* CScreensaver::get_random_graphics_app(RESULTS& results, RESULT* exclude) {
    RESULT*      rp = NULL;
    unsigned int i = 0;
    unsigned int graphics_app_count = 0;
    unsigned int random_selection = 0;
    unsigned int current_counter = 0;
    RESULT *avoid = exclude;

    BOINCTRACE(_T("get_random_graphics_app -- Function Start\n"));

    graphics_app_count = count_active_graphic_apps(results, avoid);
    BOINCTRACE(_T("get_random_graphics_app -- graphics_app_count = '%d'\n"), graphics_app_count);

    // If no graphics app found other than the one excluded, count again without excluding any
    if ((0 == graphics_app_count) && (avoid != NULL)) {
        avoid = NULL;
        graphics_app_count = count_active_graphic_apps(results, avoid);
    }
        
    // If no graphics app was found, return NULL
    if (0 == graphics_app_count) {
        goto CLEANUP;
    }

    // Choose which application to display.
    random_selection = (rand() % graphics_app_count) + 1;
    BOINCTRACE(_T("get_random_graphics_app -- random_selection = '%d'\n"), random_selection);

    // Lets find the chosen graphics application.
    for (i = 0; i < results.results.size(); i++) {
        if ((results.results[i]->graphics_exec_path.size() == 0) 
                && (state.executing_as_daemon || !(results.results[i]->supports_graphics))) continue;
        if (is_same_task(results.results[i], avoid)) continue;

        current_counter++;
        if (current_counter == random_selection) {
            rp = results.results[i];
            break;
        }
    }

CLEANUP:
    BOINCTRACE(_T("get_random_graphics_app -- Function End\n"));

    return rp;
}


// Launch the graphics application
//
#ifdef _WIN32
int CScreensaver::launch_screensaver(RESULT* rp, HANDLE& graphics_application)
#else
int CScreensaver::launch_screensaver(RESULT* rp, int& graphics_application)
#endif
{
    int retval = 0;
    if (!rp->graphics_exec_path.empty()) {
        // V6 Graphics
#ifdef __APPLE__
        // For sandbox security, use gfx_switcher to launch gfx app 
        // as user boinc_project and group boinc_project.
        //
        // For unknown reasons, the graphics application exits with 
        // "RegisterProcess failed (error = -50)" unless we pass its 
        // full path twice in the argument list to execv.
        char* argv[5];
        argv[0] = "gfx_Switcher";
        argv[1] = "-launch_gfx";
        argv[2] = strrchr(rp->slot_path.c_str(), '/');
        if (*argv[2]) argv[2]++;    // Point to the slot number in ascii
        
        argv[3] = "--fullscreen";
        argv[4] = 0;

       retval = run_program(
            rp->slot_path.c_str(),
            m_gfx_Switcher_Path,
            4,
            argv,
            0,
            graphics_application
        );
#else
        char* argv[3];
        argv[0] = "app_graphics";   // not used
        argv[1] = "--fullscreen";
        argv[2] = 0;
        retval = run_program(
            rp->slot_path.c_str(),
            rp->graphics_exec_path.c_str(),
            2,
            argv,
            0,
            graphics_application
        );
#endif
    } else {
        // V5 and Older
        DISPLAY_INFO di;
#ifdef _WIN32
        graphics_application = NULL;

        memset(di.window_station, 0, sizeof(di.window_station));
        memset(di.desktop, 0, sizeof(di.desktop));
        memset(di.display, 0, sizeof(di.display));

        if (!m_bIs9x) {
            // Retrieve the current window station and desktop names
            GetUserObjectInformation(
                GetProcessWindowStation(), 
                UOI_NAME, 
                di.window_station,
                (sizeof(di.window_station)),
                NULL
            );
            GetUserObjectInformation(
                GetThreadDesktop(GetCurrentThreadId()), 
                UOI_NAME, 
                di.desktop,
                sizeof(di.desktop),
                NULL
            );
        }
#else
        char *p = getenv("DISPLAY");
        if (p) strcpy(di.display, p);
        
        graphics_application = 0;
#endif
        retval = rpc->show_graphics(
            rp->project_url.c_str(),
            rp->name.c_str(),
            MODE_FULLSCREEN,
            di
        );
    }
    return retval;
}


// Terminate the graphics application
//
#ifdef _WIN32
int CScreensaver::terminate_screensaver(HANDLE& graphics_application, RESULT *worker_app)
#else
int CScreensaver::terminate_screensaver(int& graphics_application, RESULT *worker_app)
#endif
{
    if (!m_bScience_gfx_running) {
        return 0;
    }
    
    if (graphics_application) {
        // V6 Graphics
        kill_program(graphics_application);
#ifdef __APPLE__
        // For sandbox security, use gfx_switcher to launch gfx app 
        // as user boinc_project and group boinc_project.
        int retval = 0;
        char current_dir[MAXPATHLEN];
        char gfx_pid[16];
        pid_t thePID;
        int i;

        sprintf(gfx_pid, "%d", graphics_application);
        getcwd( current_dir, sizeof(current_dir));

        char* argv[4];
        argv[0] = "gfx_switcher";
        argv[1] = "-kill_gfx";
        argv[2] = gfx_pid;
        argv[3] = 0;

       retval = run_program(
            current_dir,
            m_gfx_Switcher_Path,
            3,
            argv,
            0,
            thePID
        );
        if (retval) return retval;
        
        for (i=0; i<200; i++) {
            boinc_sleep(0.01);      // Wait 2 seconds max
            // Prevent gfx_switcher from becoming a zombie
            if (waitpid(thePID, 0, WNOHANG) == thePID) break;
        }
        return retval;
#endif
        graphics_application = 0;
    } else {
        // V5 and Older
        DISPLAY_INFO di;

        if (worker_app == NULL) return 0;
        if (worker_app->name.empty()) return 0;

        memset(di.window_station, 0, sizeof(di.window_station));
        memset(di.desktop, 0, sizeof(di.desktop));
        memset(di.display, 0, sizeof(di.display));

        rpc->show_graphics(
            worker_app->project_url.c_str(),
            worker_app->name.c_str(),
            MODE_HIDE_GRAPHICS,
            di
        );
    }
    return 0;
}


// Launch the graphics application
//
#ifdef _WIN32
int CScreensaver::launch_default_screensaver(char *dir_path, HANDLE& graphics_application)
#else
int CScreensaver::launch_default_screensaver(char *dir_path, int& graphics_application)
#endif
{
    int retval = 0;
    char full_path[1024];
    int num_args = 2;
    
    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, PATH_SEPARATOR, sizeof(full_path));
    strlcat(full_path, THE_DEFAULT_SS_EXECUTABLE, sizeof(full_path));

    // For unknown reasons, the graphics application exits with 
    // "RegisterProcess failed (error = -50)" unless we pass its 
    // full path twice in the argument list to execv on Macs.

    char* argv[4];
    argv[0] = full_path;   // not used
    argv[1] = "--fullscreen";
    argv[2] = 0;
    argv[3] = 0;
    if (!m_bConnected) {
        BOINCTRACE(_T("launch_default_screensaver using --retry_connect argument\n"));
        argv[2] = "--retry_connect";
        num_args = 3;
    }
    retval = run_program(
        dir_path,
        full_path,
        num_args,
        argv,
        0,
        graphics_application
    );

     BOINCTRACE(_T("launch_default_screensaver %s returned %d\n"), full_path, retval);
     return retval;
}


// If we cannot connect to the core client:
//   - we retry connecting every 10 seconds 
//   - we launch the default graphics application with the argument --retry_connect, so 
//     it will continue running and will also retry connecting every 10 seconds.
//
// If we successfully connected to the core client, launch the default graphics application 
// without the argument --retry_connect.  If it can't connect, it will return immediately 
// with the exit code ERR_CONNECT.  In that case, we assume it was blocked by a firewall 
// and so we run only project (science) graphics.

#ifdef _WIN32
DWORD WINAPI CScreensaver::DataManagementProc()
#else
void *CScreensaver::DataManagementProc()
#endif
{
    int             retval                      = 0;
    int             suspend_reason              = 0;
    RESULT*         theResult                   = NULL;
    RESULT*         graphics_app_result_ptr     = NULL;
    RESULT          previous_result;
    // previous_result_ptr = &previous_result when previous_result is valid, else NULL
    RESULT*         previous_result_ptr         = NULL;
    int             iResultCount                = 0;
    int             iIndex                      = 0;
    double          default_phase_start_time    = 0.0;
    double          science_phase_start_time    = 0.0;
    double          last_change_time            = 0.0;
    // If we run default screensaver during science phase because no science graphics 
    // are available, then shorten next default graphics phase by that much time.
    double          default_saver_start_time_in_science_phase    = 0.0;
    double          default_saver_duration_in_science_phase      = 0.0;

    SS_PHASE        ss_phase                    = DEFAULT_SS_PHASE;
    bool            switch_to_default_gfx       = false;
    int             exit_status                 = 0;
    
    char*           default_ss_dir_path         = NULL;
    char*           default_data_dir_path       = NULL;
    char            full_path[1024];

#ifdef _WIN32
    BOINCTRACE(_T("CScreensaver::DataManagementProc - Display screen saver loading message\n"));
    SetError(TRUE, SCRAPPERR_BOINCSCREENSAVERLOADING);
    m_tThreadCreateTime = time(0);

    // Set the starting point for iterating through the results
    m_iLastResultShown = 0;
    m_tLastResultChangeTime = 0;
#endif

    m_bDefault_ss_exists = false;
    m_bScience_gfx_running = false;
    m_bDefault_gfx_running = false;
    m_fGFXDefaultPeriod = GFX_DEFAULT_PERIOD;
    m_fGFxSciencePeriod = GFX_SCIENCE_PERIOD;
    m_fGFXChangePeriod = GFX_CHANGE_PERIOD;

#ifdef __APPLE__
    default_data_dir_path = "/Library/Application Support/BOINC Data";
    default_ss_dir_path = default_data_dir_path;
#else
    UtilGetRegDirectoryStr(_T("INSTALLDIR"), &default_ss_dir_path);
    BOINCTRACE(_T("CScreensaver::DataManagementProc - default_ss_dir_path = '%s'\n"), default_ss_dir_path);
    retval = UtilGetRegDirectoryStr(_T("DATADIR"), &default_data_dir_path);
    BOINCTRACE(_T("CScreensaver::DataManagementProc - default_data_dir_path = '%s'\n"), default_data_dir_path);
retval = 0;
#endif
    strlcpy(full_path, default_ss_dir_path, sizeof(full_path));
    strlcat(full_path, PATH_SEPARATOR, sizeof(full_path));
    strlcat(full_path, THE_DEFAULT_SS_EXECUTABLE, sizeof(full_path));
    
    if (boinc_file_exists(full_path)) {
        m_bDefault_ss_exists = true;
        ss_phase = DEFAULT_SS_PHASE;
        default_phase_start_time = dtime();
        science_phase_start_time = 0;
        switch_to_default_gfx = true;
    } else {
        ss_phase = SCIENCE_SS_PHASE;
        default_phase_start_time = 0;
        science_phase_start_time = dtime();
    }
    
    GetDisplayPeriods(default_data_dir_path);

    while (true) {
        for (int i = 0; i < 4; i++) {
            // ***
            // *** Things that should be run frequently.
            // ***   4 times per second.
            // ***

            // Are we supposed to exit the screensaver?
            if (m_QuitDataManagementProc) {     // If main thread has requested we exit
                if (m_hGraphicsApplication || graphics_app_result_ptr) {
                    terminate_screensaver(m_hGraphicsApplication, graphics_app_result_ptr);
                    graphics_app_result_ptr = NULL;
                    previous_result_ptr = NULL;
                    m_hGraphicsApplication = 0;
                }
#ifdef _WIN32
                if (default_ss_dir_path) free(default_ss_dir_path);
                default_ss_dir_path = NULL;
                if (default_data_dir_path) free(default_data_dir_path);
                default_data_dir_path = NULL;
#endif
                return 0;       // Exit the thread
            }

#ifdef _WIN32

            // Check for keyboard and mouse activity just in case the
            // user wants to blow out of the screensaver.
            //
            CheckKeyboardMouseActivity();

            // Check to see if there are any notification windows from
            // personal firewalls, virus scanners, or anything else that
            // demands the users attention. If there is blow out of the
            // screensaver
            CheckForNotificationWindow();

#endif

            boinc_sleep(0.25);
        }

        // ***
        // *** Things that should be run frequently.
        // *** 1 time per second.
        // ***

        // Blank screen saver?
        if ((m_dwBlankScreen) && (time(0) > m_dwBlankTime)) {
            BOINCTRACE(_T("CScreensaver::DataManagementProc - Time to blank\n"));
            SetError(FALSE, SCRAPPERR_SCREENSAVERBLANKED);
            m_QuitDataManagementProc = true;
        }

        BOINCTRACE(_T("CScreensaver::DataManagementProc - ErrorMode = '%d', ErrorCode = '%x'\n"), m_bErrorMode, m_hrError);

        if (! m_bConnected) {
            HandleRPCError();
        }
        
        if (m_bConnected) {
            // Do we need to get the core client state?
            if (m_bResetCoreState) {
                // Try and get the current state of the CC
                retval = rpc->get_state(state);
                if (retval) {
                    // CC may not yet be running
                    HandleRPCError();
                } else {
                    m_bResetCoreState = false;
                }
            }
    
            // Update our task list
            m_updating_results = true;
            retval = rpc->get_screensaver_tasks(suspend_reason, results);
            m_updating_results = false;
            if (retval) {
                // rpc call returned error
                HandleRPCError();
                m_bResetCoreState = true;
                continue;
            }
        } else {
            results.clear();
        }
        
        // Time to switch to default graphics phase?
        if (m_bDefault_ss_exists && (ss_phase == SCIENCE_SS_PHASE)) {
            if (science_phase_start_time && ((dtime() - science_phase_start_time) > m_fGFxSciencePeriod)) {
                if (!m_bDefault_gfx_running) {
                    switch_to_default_gfx = true;
                }
                ss_phase = DEFAULT_SS_PHASE;
                default_phase_start_time = dtime();
                science_phase_start_time = 0;
                if (m_bDefault_gfx_running && default_saver_start_time_in_science_phase) {
                    // Remember how long default graphics ran during science phase
                    default_saver_duration_in_science_phase += (dtime() - default_saver_start_time_in_science_phase); 
                }
                default_saver_start_time_in_science_phase = 0;
            }
        }
        
        // Time to switch to science graphics phase?
        if ((ss_phase == DEFAULT_SS_PHASE) && m_bConnected) {
            if (default_phase_start_time && 
                    ((dtime() - default_phase_start_time + default_saver_duration_in_science_phase) 
                    > m_fGFXDefaultPeriod)) {
                // BOINCTRACE(_T("CScreensaver::Ending Default phase: now=%f, default_phase_start_time=%f, default_saver_duration_in_science_phase=%f\n"),
                // dtime(), default_phase_start_time, default_saver_duration_in_science_phase);
                ss_phase = SCIENCE_SS_PHASE;
                default_phase_start_time = 0;
                default_saver_duration_in_science_phase = 0;
                science_phase_start_time = dtime();
                if (m_bDefault_gfx_running) {
                    default_saver_start_time_in_science_phase = science_phase_start_time;
                }
                switch_to_default_gfx = false;
            }
        }

        // Core client suspended?
        if (suspend_reason && !(suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT)) {
            SetError(TRUE, SCRAPPERR_BOINCSUSPENDED);
            if (m_bDefault_ss_exists && !m_bDefault_gfx_running) {
                switch_to_default_gfx = true;
            }
        }
        
        if (switch_to_default_gfx) {
            if (m_bScience_gfx_running) {
                if (m_hGraphicsApplication || previous_result_ptr) {
                    // use previous_result_ptr because graphics_app_result_ptr may no longer be valid
                    terminate_screensaver(m_hGraphicsApplication, previous_result_ptr);
                    if (m_hGraphicsApplication == 0) {
                        graphics_app_result_ptr = NULL;
                        m_bScience_gfx_running = false;
                    } else {
                        // process_exists() test will clear m_hGraphicsApplication and graphics_app_result_ptr
                    }
                    previous_result_ptr = NULL;
                }
            } else {
                if (!m_bDefault_gfx_running) {
                    switch_to_default_gfx = false;
                    retval = launch_default_screensaver(default_ss_dir_path, m_hGraphicsApplication);
                    if (retval) {
                        m_hGraphicsApplication = 0;
                        previous_result_ptr = NULL;
                        graphics_app_result_ptr = NULL;
                        m_bDefault_gfx_running = false;
                    } else {
                        m_bDefault_gfx_running = true;
                        if (ss_phase == SCIENCE_SS_PHASE) {
                            default_saver_start_time_in_science_phase = dtime();
                        }
                    }
                }
            }
        }

        if ((ss_phase == SCIENCE_SS_PHASE) && !switch_to_default_gfx) {
        
#if SIMULATE_NO_GRAPHICS /* FOR TESTING */

        SetError(TRUE, SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING);
        if (m_bDefault_ss_exists && !m_bDefault_gfx_running) {
            switch_to_default_gfx = true;
        }

#else                   /* NORMAL OPERATION */

            if (m_bScience_gfx_running) {
                // Is the current graphics app's associated task still running?
                
                if ((m_hGraphicsApplication) || (graphics_app_result_ptr)) {
                    iResultCount = results.results.size();
                    graphics_app_result_ptr = NULL;

                    // Find the current task in the new results vector (if it still exists)
                    for (iIndex = 0; iIndex < iResultCount; iIndex++) {
                        theResult = results.results.at(iIndex);

                        if (is_same_task(theResult, previous_result_ptr)) {
                            graphics_app_result_ptr = theResult;
                            previous_result = *theResult;
                            previous_result_ptr = &previous_result;
                            break;
                        }
                    }

                    // V6 graphics only: if worker application has stopped running, terminate_screensaver
                    if ((graphics_app_result_ptr == NULL) && (m_hGraphicsApplication != 0)) {
                        if (previous_result_ptr) {
                            BOINCTRACE(_T("CScreensaver::DataManagementProc - %s finished\n"), 
                                    previous_result.graphics_exec_path.c_str());
                        }
                        terminate_screensaver(m_hGraphicsApplication, previous_result_ptr);
                        previous_result_ptr = NULL;
                        if (m_hGraphicsApplication == 0) {
                            graphics_app_result_ptr = NULL;
                            m_bScience_gfx_running = false;
                            // Save previous_result and previous_result_ptr for get_random_graphics_app() call
                        } else {
                            // process_exists() test will clear m_hGraphicsApplication and graphics_app_result_ptr
                        }
                    }

                     if (last_change_time && ((dtime() - last_change_time) > m_fGFXChangePeriod)) {
                        if (count_active_graphic_apps(results, previous_result_ptr) > 0) {
                            if (previous_result_ptr) {
                                BOINCTRACE(_T("CScreensaver::DataManagementProc - time to change: %s / %s\n"), 
                                        previous_result.name.c_str(), previous_result.graphics_exec_path.c_str());
                            }
                            terminate_screensaver(m_hGraphicsApplication, graphics_app_result_ptr);
                            if (m_hGraphicsApplication == 0) {
                                graphics_app_result_ptr = NULL;
                                m_bScience_gfx_running = false;
                                // Save previous_result and previous_result_ptr for get_random_graphics_app() call
                            } else {
                                // process_exists() test will clear m_hGraphicsApplication and graphics_app_result_ptr
                            }
                        }
                        last_change_time = dtime();
                    }
                }
            }       // End if (m_bScience_gfx_running)
        
            // If no current graphics app, pick an active task at random and launch its graphics app
            if ((m_bDefault_gfx_running || (m_hGraphicsApplication == 0)) && (graphics_app_result_ptr == NULL)) {
                graphics_app_result_ptr = get_random_graphics_app(results, previous_result_ptr);
                previous_result_ptr = NULL;
                
                if (graphics_app_result_ptr) {
                    if (m_bDefault_gfx_running) {
                        kill_program(m_hGraphicsApplication);
                        // Remember how long default graphics ran during science phase
                        if (default_saver_start_time_in_science_phase) {
                            default_saver_duration_in_science_phase += (dtime() - default_saver_start_time_in_science_phase); 
                            //BOINCTRACE(_T("CScreensaver::During Science phase: now=%f, default_saver_start_time=%f, default_saver_duration=%f\n"),
                            //    dtime(), default_saver_start_time_in_science_phase, default_saver_duration_in_science_phase);
                        }
                        default_saver_start_time_in_science_phase = 0;
                        // process_exists() test will clear m_hGraphicsApplication and graphics_app_result_ptr
                     } else {
                        retval = launch_screensaver(graphics_app_result_ptr, m_hGraphicsApplication);
                        if (retval) {
                            m_hGraphicsApplication = 0;
                            previous_result_ptr = NULL;
                            graphics_app_result_ptr = NULL;
                            m_bScience_gfx_running = false;
                        } else {
        #ifdef __APPLE__
                            // Show ScreenSaverAppStartingMsg for GFX_STARTING_MSG_DURATION seconds
                            SetError(FALSE, SCRAPPERR_BOINCAPPFOUNDGRAPHICSLOADING);
        #endif
                            SetError(FALSE, SCRAPPERR_SCREENSAVERRUNNING);
                            last_change_time = dtime();
                            m_bScience_gfx_running = true;
                            // Make a local copy of current result, since original pointer 
                            // may have been freed by the time we perform later tests
                            previous_result = *graphics_app_result_ptr;
                            previous_result_ptr = &previous_result;
                            if (previous_result_ptr) {
                                BOINCTRACE(_T("CScreensaver::DataManagementProc - launching %s\n"), 
                                        previous_result.graphics_exec_path.c_str());
                            }
                        }
                    }
                } else {
                    if ((!suspend_reason) || (suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT)) {
                        // No science graphics available
                        if (state.projects.size() == 0) {
                            // We are not attached to any projects
                            SetError(TRUE, SCRAPPERR_BOINCNOPROJECTSDETECTED);
                        } else if (results.results.size() == 0) {
                            // We currently do not have any applications to run
                            SetError(TRUE, SCRAPPERR_BOINCNOAPPSEXECUTING);
                        } else {
                            // We currently do not have any graphics capable application
                            if (m_bV5_GFX_app_is_running) {
                                SetError(TRUE, SCRAPPERR_DAEMONALLOWSNOGRAPHICS);
                            } else {
                                SetError(TRUE, SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING);
                            }
                        }
                    }
                    
                    // We can't run a science graphics app, so run the default graphics if available
                    if (m_bDefault_ss_exists && !m_bDefault_gfx_running) {
                        switch_to_default_gfx = true;
                    }

                }   // End if no science graphics available
            }      // End if no current science graphics app is running

#endif      // ! SIMULATE_NO_GRAPHICS

            if (switch_to_default_gfx) {
                switch_to_default_gfx = false;
                if (!m_bDefault_gfx_running) {
                    retval = launch_default_screensaver(default_ss_dir_path, m_hGraphicsApplication);
                    if (retval) {
                        m_hGraphicsApplication = 0;
                        previous_result_ptr = NULL;
                        graphics_app_result_ptr = NULL;
                        m_bDefault_gfx_running = false;
                    } else {
                        m_bDefault_gfx_running = true;
                        default_saver_start_time_in_science_phase = dtime();
                    }
                }
            }
        }   // End if ((ss_phase == SCIENCE_SS_PHASE) && !switch_to_default_gfx)
        
        
        
        // Is the graphics app still running?
        if (m_hGraphicsApplication) {
            if (HasProcessExited(m_hGraphicsApplication, exit_status)) {
                // Something has happened to the previously selected screensaver
                //   application. Start a different one.
                BOINCTRACE(_T("CScreensaver::DataManagementProc - Graphics application isn't running, start a new one.\n"));
                if (m_bDefault_gfx_running) {
                    // If we were able to connect to core client but gfx app can't, don't use it. 
                    BOINCTRACE(_T("CScreensaver::DataManagementProc - Default graphics application exited with code %d.\n"), exit_status);
                    if (exit_status == DEFAULT_GFX_CANT_CONNECT) {
                        m_bDefault_ss_exists = false;
                        ss_phase = SCIENCE_SS_PHASE;
                    }
                }
                m_hGraphicsApplication = 0;
                graphics_app_result_ptr = NULL;
                m_bDefault_gfx_running = false;
                m_bScience_gfx_running = false;
                continue;
            } else {
#ifdef _WIN32
                CheckForegroundWindow();
#endif
            }
        }
        
    }                           // end while(true)
}


#ifdef _WIN32
BOOL CScreensaver::HasProcessExited(HANDLE pid_handle, int &exitCode) {
    unsigned long status = 1;
    if (GetExitCodeProcess(pid_handle, &status)) {
        if (status == STILL_ACTIVE) {
            exitCode = 0;
            return false;
        }
    }
    exitCode = (int)status;
    return true;
}
#else
bool CScreensaver::HasProcessExited(pid_t pid, int &exitCode) {
    int status;
    pid_t p;
    
    p = waitpid(pid, &status, WNOHANG);
    exitCode = WEXITSTATUS(status);
    if (p == pid) return true;     // process has exited
    if (p == -1) return true;      // PID doesn't exist
    exitCode = 0;
    return false;
}
#endif


void CScreensaver::GetDisplayPeriods(char *dir_path) {
    char buf[1024];
    FILE* f;
    MIOFILE mf;

    strlcpy(buf, dir_path, sizeof(buf));
    strlcat(buf, PATH_SEPARATOR, sizeof(buf));
    strlcat(buf, THE_SS_CONFIG_FILE, sizeof(buf));

    f = boinc_fopen(buf, "r");
    if (!f) return;
    
    mf.init_file(f);
    XML_PARSER xp(&mf);

    while (mf.fgets(buf, sizeof(buf))) {
        if (parse_double(buf, "<default_gfx_duration>", m_fGFXDefaultPeriod)) continue;
        if (parse_double(buf, "<science_gfx_duration>", m_fGFxSciencePeriod)) continue;
        if (parse_double(buf, "<science_gfx_change_interval>", m_fGFXChangePeriod)) continue;
    }
    fclose(f);

    BOINCTRACE(_T("CScreensaver::GetDisplayPeriods: m_fGFXDefaultPeriod=%f, m_fGFxSciencePeriod=%f, m_fGFXChangePeriod=%f\n"),
                    m_fGFXDefaultPeriod, m_fGFxSciencePeriod, m_fGFXChangePeriod);
}