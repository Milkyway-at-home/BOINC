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
// Contributor(s):
//     DirectX 8.1 Screen Saver Framework from Microsoft.
//     Microsoft Knowledge Base Article - 79212
//

#include "boinc_win.h"

#include <windowsx.h>
#include <mmsystem.h>
#include <regstr.h>
#include <strsafe.h>
#include <mmsystem.h>
#define COMPILE_MULTIMON_STUBS

#include "boinc_ss.h"
#include "diagnostics.h"
#include "common_defs.h"
#include "util.h"
#include "gui_rpc_client.h"
#include "screensaver.h"
#include "screensaver_win.h"

#ifdef _DEBUG
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif

static CScreensaver*            gspScreensaver = NULL;

const UINT                      WM_SETTIMER = RegisterWindowMessage(TEXT("BOINCSetTimer"));
const UINT                      WM_INTERRUPTSAVER = RegisterWindowMessage(TEXT("BOINCInterruptScreensaver"));
const UINT                      WM_BOINCSFW = RegisterWindowMessage(TEXT("BOINCSetForegroundWindow"));


INT WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE UNUSED(hPrevInstance), LPSTR UNUSED(lpCmdLine), int UNUSED(nCmdShow)
) {
    HRESULT      hr;
    CScreensaver BOINCSS;
    int          retval;
    WSADATA      wsdata;
    DWORD        dwVal;
    DWORD        dwSize = sizeof(dwVal); 


#ifdef _DEBUG
    // Initialize Diagnostics
    retval = diagnostics_init (
        BOINC_DIAG_DUMPCALLSTACKENABLED | 
        BOINC_DIAG_HEAPCHECKENABLED |
        BOINC_DIAG_MEMORYLEAKCHECKENABLED |
        BOINC_DIAG_ARCHIVESTDOUT |
        BOINC_DIAG_REDIRECTSTDOUTOVERWRITE |
        BOINC_DIAG_REDIRECTSTDERROVERWRITE |
        BOINC_DIAG_TRACETOSTDOUT,
        "stdoutscr",
        "stderrscr"
    );
    if (retval) {
        BOINCTRACE("WinMain - BOINC Screensaver Diagnostic Error '%d'\n", retval);
        MessageBox(NULL, NULL, "BOINC Screensaver Diagnostic Error", MB_OK);
    }
#endif

    // Initialize the CRT random number generator.
    srand((unsigned int)time(0));

    // Initialize the Windows sockets interface.
    retval = WSAStartup(MAKEWORD(1, 1), &wsdata);
    if (retval) {
        BOINCTRACE("WinMain - Winsock Initialization Failure '%d'\n", retval);
        return retval;
    }

    if (FAILED(hr = BOINCSS.Create(hInstance))) {
        BOINCSS.DisplayErrorMsg(hr);
        WSACleanup();
        return 0;
    }

    retval = BOINCSS.Run();

    // Cleanup any existing screensaver objects and handles
    BOINCSS.Cleanup();

    // Cleanup the Windows sockets interface.
    WSACleanup();

    // Instruct the OS to terminate the screensaver by any
    //   means nessassary.
    TerminateProcess(GetCurrentProcess(), retval);

    return retval;
}


CScreensaver::CScreensaver() {
    gspScreensaver = this;

    m_dwSaverMouseMoveCount = 0;
    m_hWnd = NULL;
    m_hWndParent = NULL;
    
    m_bAllScreensSame = FALSE;
    m_bWindowed = FALSE;
    m_bWaitForInputIdle = FALSE;

    m_bErrorMode = FALSE;
    m_hrError = S_OK;
    m_szError[0] = _T('\0');
    m_strBOINCInstallDirectory.clear();
    m_strBOINCDataDirectory.clear();

    LoadString(NULL, IDS_DESCRIPTION, m_strWindowTitle, 200);

    m_bPaintingInitialized = FALSE;
    m_dwBlankScreen = 0;
    m_dwBlankTime = 0;

    rpc = NULL;
    m_bConnected = false;
    m_hDataManagementThread = NULL;
    m_hGraphicsApplication = NULL;
    m_bResetCoreState = TRUE;
    m_QuitDataManagementProc = FALSE;
    memset(&m_running_result, 0, sizeof(m_running_result));

    ZeroMemory(m_Monitors, sizeof(m_Monitors));
    m_dwNumMonitors = 0;

    m_dwLastInputTimeAtStartup = 0;
    m_tThreadCreateTime = 0;
}


// Have the client program call this function before calling Run().
//
HRESULT CScreensaver::Create(HINSTANCE hInstance) {
    HRESULT hr;
    BOOL    bReturnValue;

    m_hInstance = hInstance;

    // Parse the command line and do the appropriate thing
    m_SaverMode = ParseCommandLine(GetCommandLine());


    // Store last input value if it exists
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);
    GetLastInputInfo(&lii);

    m_dwLastInputTimeAtStartup = lii.dwTime;


    // Enumerate Monitors
    EnumMonitors();


    // Retrieve the locations of the install directory and data directory
	bReturnValue = UtilGetRegDirectoryStr(_T("DATADIR"), m_strBOINCDataDirectory);
    BOINCTRACE("CScreensaver::Create - BOINC Data Directory '%s'\n", m_strBOINCDataDirectory.c_str());
	bReturnValue = UtilGetRegDirectoryStr(_T("INSTALLDIR"), m_strBOINCInstallDirectory);
    BOINCTRACE("CScreensaver::Create - BOINC Install Directory '%s'\n", m_strBOINCInstallDirectory.c_str());


    // Retrieve the blank screen flag so we can determine if we are
    // suppose to actually blank the screen at some point.
	bReturnValue = UtilGetRegKey(REG_BLANK_NAME, m_dwBlankScreen);
    BOINCTRACE("CScreensaver::Create - Get Reg Key REG_BLANK_NAME return value '%d'\n", bReturnValue);
	if (!bReturnValue) m_dwBlankScreen = 0;

    // Retrieve the blank screen timeout
	// make sure you check return value of registry queries
	// in case the item in question doesn't happen to exist.
	bReturnValue = UtilGetRegKey(REG_BLANK_TIME, m_dwBlankTime);
    BOINCTRACE("CScreensaver::Create - Get Reg Key REG_BLANK_TIME return value '%d'\n", bReturnValue);
	if (!bReturnValue) m_dwBlankTime = 5;

    // Save the value back to the registry in case this is the first
    // execution and so we need the default value later.
	bReturnValue = UtilSetRegKey(REG_BLANK_NAME, m_dwBlankScreen);
    BOINCTRACE("CScreensaver::Create - Set Reg Key REG_BLANK_NAME return value '%d'\n", bReturnValue);

	bReturnValue = UtilSetRegKey(REG_BLANK_TIME, m_dwBlankTime);
    BOINCTRACE("CScreensaver::Create - Set Reg Key REG_BLANK_TIME return value '%d'\n", bReturnValue);


    // Calculate the estimated blank time by adding the current time
    //   and and the user specified time which is in minutes
    m_dwBlankTime = (DWORD)time(0) + (m_dwBlankTime * 60);

    // Create the infrastructure mutexes so we can properly aquire them to report
    //   errors
    if (!CreateInfrastructureMutexes()) {
        return E_FAIL;
    }

	if (rpc == NULL) rpc = new RPC_CLIENT;

			// Create the screen saver window(s)
    if (m_SaverMode == sm_preview || 
        m_SaverMode == sm_full
    ) {
        if (FAILED(hr = CreateSaverWindow())) {
            SetError(TRUE, hr);
        }
    }

    if (m_SaverMode == sm_preview) {
        // In preview mode, "pause" (enter a limited message loop) briefly 
        // before proceeding, so the display control panel knows to update itself.
        m_bWaitForInputIdle = TRUE;

        // Post a message to mark the end of the initial group of window messages
        PostMessage(m_hWnd, WM_SETTIMER, 0, 0);

        MSG msg;
        while(m_bWaitForInputIdle) {
            // If GetMessage returns FALSE, it's quitting time.
            if (!GetMessage(&msg, m_hWnd, 0, 0)) {
                // Post the quit message to handle it later
                PostQuitMessage(0);
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return S_OK;
}




// Starts main execution of the screen saver.
//
HRESULT CScreensaver::Run() {
    HOST_INFO hostinfo;
    HRESULT hr;

    // Parse the command line and do the appropriate thing
    switch (m_SaverMode) {
        case sm_config:
            if (m_bErrorMode) {
                DisplayErrorMsg(m_hrError);
            } else {
                DoConfig();
            }
            break;
        case sm_test:
            rpc->init(NULL);
            rpc->get_host_info(hostinfo);
            rpc->close();
            break;
        case sm_preview:
            // In Windows, preview mode is for the mini-view of the screensaver.
            //   For BOINC we just display the icon, so there is no need to
            //   startup the data management thread which in turn will
            //   launch a graphics application.
            if (FAILED(hr = DoSaver())) {
                DisplayErrorMsg(hr);
            }
            break;
        case sm_full:
            // Create the various required threads
            if (!CreateInputActivityThread()) return E_FAIL;
            if (!CreateGraphicsWindowPromotionThread()) {
                DestroyDataManagementThread();
                return E_FAIL;
            }
            if (!CreateDataManagementThread()) {
                DestroyDataManagementThread();
                DestroyGraphicsWindowPromotionThread();
                return E_FAIL;
            }

            if (FAILED(hr = DoSaver())) {
                DisplayErrorMsg(hr);
            }

            // Destroy the various required threads
            //
            DestroyDataManagementThread();
            DestroyGraphicsWindowPromotionThread();
            DestroyInputActivityThread();
            break;
        case sm_passwordchange:
            ChangePassword();
            break;
    }
    return S_OK;
}




// Cleanup anything that needs cleaning.
//
HRESULT CScreensaver::Cleanup() {
    if (m_hGraphicsApplication) {
        TerminateProcess(m_hGraphicsApplication, 0);
        m_hGraphicsApplication = NULL;
    }

    if (rpc) {
        delete rpc;
        rpc = NULL;
    }
    return S_OK;
}




// Displays error messages in a message box
//
HRESULT CScreensaver::DisplayErrorMsg(HRESULT hr) {
    TCHAR strMsg[512];

    GetTextForError(hr, strMsg, 512);

    MessageBox(m_hWnd, strMsg, m_strWindowTitle, MB_ICONERROR | MB_OK);

    return hr;
}




// Interpret command-line parameters passed to this app.
//
SaverMode CScreensaver::ParseCommandLine(TCHAR* pstrCommandLine) {
    m_hWndParent = NULL;

	BOINCTRACE("ParseCommandLine: '%s'\n", pstrCommandLine);

    // Skip the first part of the command line, which is the full path 
    // to the exe.  If it contains spaces, it will be contained in quotes.
    if (*pstrCommandLine == _T('\"')) {
        pstrCommandLine++;
        while (*pstrCommandLine != _T('\0') && *pstrCommandLine != _T('\"')) {
            pstrCommandLine++;
        }
        if (*pstrCommandLine == _T('\"')) {
            pstrCommandLine++;
        }
    } else {
        while (*pstrCommandLine != _T('\0') && *pstrCommandLine != _T(' ')) {
            pstrCommandLine++;
        }
        if (*pstrCommandLine == _T(' ')) {
            pstrCommandLine++;
        }
    }

    // Skip along to the first option delimiter "/" or "-"
    while (*pstrCommandLine != _T('\0') && *pstrCommandLine != _T('/') && *pstrCommandLine != _T('-')) {
        pstrCommandLine++;
    }

    // If there wasn't one, then must be config mode
    if (*pstrCommandLine == _T('\0')) {
        return sm_config;
    }

    // Otherwise see what the option was
    switch (*(++pstrCommandLine)) {
        case 'c':
        case 'C':
            pstrCommandLine++;
            while (*pstrCommandLine && !isdigit(*pstrCommandLine)) {
                pstrCommandLine++;
            }
            if (isdigit(*pstrCommandLine)) {
#ifdef _WIN64
                m_hWndParent = (HWND)_atoi64(pstrCommandLine);
#else
                m_hWndParent = (HWND)_ttol(pstrCommandLine);
#endif
            } else {
                m_hWndParent = NULL;
            }
            return sm_config;

        case 't':
        case 'T':
            return sm_test;

        case 'p':
        case 'P':
            // Preview-mode, so option is followed by the parent HWND in decimal
            pstrCommandLine++;
            while (*pstrCommandLine && !isdigit(*pstrCommandLine)) {
                pstrCommandLine++;
            }
            if (isdigit(*pstrCommandLine)) {
#ifdef _WIN64
                m_hWndParent = (HWND)_atoi64(pstrCommandLine);
#else
                m_hWndParent = (HWND)_ttol(pstrCommandLine);
#endif
            }
            return sm_preview;

        case 'a':
        case 'A':
            // Password change mode, so option is followed by parent HWND in decimal
            pstrCommandLine++;
            while (*pstrCommandLine && !isdigit(*pstrCommandLine)) {
                pstrCommandLine++;
            }
            if (isdigit(*pstrCommandLine)) {
#ifdef _WIN64
                m_hWndParent = (HWND)_atoi64(pstrCommandLine);
#else
                m_hWndParent = (HWND)_ttol(pstrCommandLine);
#endif
            }
            return sm_passwordchange;

        default:
            // All other options => run the screensaver (typically this is "/s")
            return sm_full;
    }
}




// Determine HMONITOR, desktop rect, and other info for each monitor.  
//       Note that EnumDisplayDevices enumerates monitors in the order 
//       indicated on the Settings page of the Display control panel, which 
//       is the order we want to list monitors in, as opposed to the order 
//       used by D3D's GetAdapterInfo.
//
VOID CScreensaver::EnumMonitors(VOID) {
    DWORD iDevice = 0;
    DISPLAY_DEVICE_FULL dispdev;
    DISPLAY_DEVICE_FULL dispdev2;
    DEVMODE devmode;
    dispdev.cb = sizeof(dispdev);
    dispdev2.cb = sizeof(dispdev2);
    devmode.dmSize = sizeof(devmode);
    devmode.dmDriverExtra = 0;
    INTERNALMONITORINFO* pMonitorInfoNew;
    while(EnumDisplayDevices(NULL, iDevice, (DISPLAY_DEVICE*)&dispdev, 0)) {
        // Ignore NetMeeting's mirrored displays
        if ((dispdev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0) {
            // To get monitor info for a display device, call EnumDisplayDevices
            // a second time, passing dispdev.DeviceName (from the first call) as
            // the first parameter.
            EnumDisplayDevices(dispdev.DeviceName, 0, (DISPLAY_DEVICE*)&dispdev2, 0);

            pMonitorInfoNew = &m_Monitors[m_dwNumMonitors];
            ZeroMemory(pMonitorInfoNew, sizeof(INTERNALMONITORINFO));
            StringCchCopy(pMonitorInfoNew->strDeviceName, 128, dispdev.DeviceString);
            StringCchCopy(pMonitorInfoNew->strMonitorName, 128, dispdev2.DeviceString);
            
            if (dispdev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                EnumDisplaySettings(dispdev.DeviceName, ENUM_CURRENT_SETTINGS, &devmode);
                if (dispdev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
                    // For some reason devmode.dmPosition is not always (0, 0)
                    // for the primary display, so force it.
                    pMonitorInfoNew->rcScreen.left = 0;
                    pMonitorInfoNew->rcScreen.top = 0;
                } else {
                    pMonitorInfoNew->rcScreen.left = devmode.dmPosition.x;
                    pMonitorInfoNew->rcScreen.top = devmode.dmPosition.y;
                }
                pMonitorInfoNew->rcScreen.right = pMonitorInfoNew->rcScreen.left + devmode.dmPelsWidth;
                pMonitorInfoNew->rcScreen.bottom = pMonitorInfoNew->rcScreen.top + devmode.dmPelsHeight;
                pMonitorInfoNew->hMonitor = MonitorFromRect(&pMonitorInfoNew->rcScreen, MONITOR_DEFAULTTONULL);
            }
            m_dwNumMonitors++;
            if (m_dwNumMonitors == MAX_DISPLAYS) {
                break;
            }
        }
        iDevice++;
    }
}




BOOL CScreensaver::UtilGetRegKey(LPCTSTR name, DWORD& keyval) {
	LONG  error;
	DWORD type = REG_DWORD;
	DWORD size = sizeof(DWORD);
	DWORD value;
	HKEY  boinc_key;

	error = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Screensaver"),  
		0,
        KEY_ALL_ACCESS,
        &boinc_key
    );
	if (error != ERROR_SUCCESS) return FALSE;

	error = RegQueryValueEx(boinc_key, name, NULL, &type, (BYTE *)&value, &size);

	keyval = value;

	RegCloseKey(boinc_key);

	if (error != ERROR_SUCCESS) return FALSE;

	return TRUE;
}




BOOL CScreensaver::UtilSetRegKey(LPCTSTR name, DWORD value) {
	LONG error;
	HKEY boinc_key;

	error = RegCreateKeyEx(
        HKEY_CURRENT_USER,
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Screensaver"),  
		0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        NULL,
        &boinc_key,
        NULL
    );
	if (error != ERROR_SUCCESS) return FALSE;

	error = RegSetValueEx(boinc_key, name, 0, REG_DWORD, (CONST BYTE *)&value, 4);

	RegCloseKey(boinc_key);

	return TRUE;
}




BOOL CScreensaver::UtilGetRegDirectoryStr(LPCTSTR szTargetName, std::string& strDirectory) {
	LONG    lReturnValue;
	HKEY    hkSetupHive;
    LPTSTR  lpszRegistryValue = NULL;
	DWORD   dwSize = 0;

    // change the current directory to the boinc data directory if it exists
	lReturnValue = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        _T("SOFTWARE\\Space Sciences Laboratory, U.C. Berkeley\\BOINC Setup"),
		0, 
        KEY_READ,
        &hkSetupHive
    );
    if (lReturnValue == ERROR_SUCCESS) {
        // How large does our buffer need to be?
        lReturnValue = RegQueryValueEx(
            hkSetupHive,
            szTargetName,
            NULL,
            NULL,
            NULL,
            &dwSize
        );
        if (lReturnValue != ERROR_FILE_NOT_FOUND) {
            // Allocate the buffer space.
            lpszRegistryValue = (LPTSTR) malloc(dwSize);
            (*lpszRegistryValue) = NULL;

            // Now get the data
            lReturnValue = RegQueryValueEx( 
                hkSetupHive,
                szTargetName,
                NULL,
                NULL,
                (LPBYTE)lpszRegistryValue,
                &dwSize
            );

            // Store the directory for later use.
            strDirectory = lpszRegistryValue;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    // Cleanup
	if (hkSetupHive) RegCloseKey(hkSetupHive);
	return TRUE;
}




// Desc: Create the infrastructure for thread safe acccess to the infrastructure
//       layer of the screen saver.
//
BOOL CScreensaver::CreateInfrastructureMutexes() {
    m_hErrorManagementMutex = CreateMutex(NULL, FALSE, NULL);
    if (NULL == m_hErrorManagementMutex) {
    	BOINCTRACE(_T("CScreensaver::CreateInfrastructureMutexes: Failed to create m_hErrorManagementMutex '%d'\n"), GetLastError());
        return FALSE;
    }
    return TRUE;
}




// Provide a thread-safe implementation for retrieving the current
//       error condition.
//
BOOL CScreensaver::GetError(
    BOOL& bErrorMode, HRESULT& hrError, TCHAR* pszError, size_t iErrorSize
) {
    DWORD dwWaitResult;
    BOOL  bRetVal = FALSE;

    // Request ownership of mutex.
    dwWaitResult = WaitForSingleObject(
        m_hErrorManagementMutex,   // handle to mutex
        5000L);                    // five-second time-out interval
 
    switch (dwWaitResult) {
        // WAIT_OBJECT_0 - The thread got mutex ownership.
        case WAIT_OBJECT_0:
            bErrorMode = m_bErrorMode;
            hrError = m_hrError;

            if (NULL != pszError) {
                StringCbCopyN(pszError, iErrorSize, m_szError, sizeof(m_szError) * sizeof(TCHAR));
            }

            bRetVal = TRUE;
            break; 

        // WAIT_TIMEOUT - Cannot get mutex ownership due to time-out.
        // WAIT_ABANDONED - Got ownership of the abandoned mutex object.
        case WAIT_TIMEOUT: 
        case WAIT_ABANDONED: 
            break; 
    }
    ReleaseMutex(m_hErrorManagementMutex);

    return bRetVal; 
}




// Provide a thread-safe implementation for setting the current
//       error condition.  This API should only be called in the data management
//       thread, any other thread may cause a race condition.
//
BOOL CScreensaver::SetError(BOOL bErrorMode, HRESULT hrError) {
    DWORD dwWaitResult;
    BOOL  bRetVal = FALSE;

    // Request ownership of mutex.
    dwWaitResult = WaitForSingleObject(
        m_hErrorManagementMutex,   // handle to mutex
        5000L                // five-second time-out interval
    );
 
    switch (dwWaitResult) {
        // WAIT_OBJECT_0 - The thread got mutex ownership.
        case WAIT_OBJECT_0:
            m_bErrorMode = bErrorMode;
            m_hrError = hrError;

            // Update the error text, including a possible RPC call
            //   to the daemon.
            UpdateErrorBoxText();

            bRetVal = TRUE;
            break; 

        // WAIT_TIMEOUT - Cannot get mutex ownership due to time-out.
        // WAIT_ABANDONED - Got ownership of the abandoned mutex object.
        case WAIT_TIMEOUT: 
        case WAIT_ABANDONED: 
            break; 
    }
    ReleaseMutex(m_hErrorManagementMutex);
    return bRetVal; 
}



// Update the error message
//
VOID CScreensaver::UpdateErrorBoxText() {
    // Load error string
    GetTextForError(m_hrError, m_szError, sizeof(m_szError) / sizeof(TCHAR));
    BOINCTRACE(_T("CScreensaver::UpdateErrorBoxText - Updated Text '%s'\n"), m_szError);
}




// Translate an HRESULT error code into a string that can be displayed
//       to explain the error.  A class derived from CD3DScreensaver can 
//       provide its own version of this function that provides app-specific
//       error translation instead of or in addition to calling this function.
//       This function returns TRUE if a specific error was translated, or
//       FALSE if no specific translation for the HRESULT was found (though
//       it still puts a generic string into pszError).
//
BOOL CScreensaver::GetTextForError(
    HRESULT hr, TCHAR* pszError, DWORD dwNumChars
) {
    const DWORD dwErrorMap[][2] = {
    //  HRESULT, stringID
        E_FAIL, IDS_ERR_GENERIC,
        E_OUTOFMEMORY, IDS_ERR_OUTOFMEMORY,
		SCRAPPERR_NOPREVIEW, IDS_ERR_NOPREVIEW,
		SCRAPPERR_BOINCSCREENSAVERLOADING, IDS_ERR_BOINCSCREENSAVERLOADING,
		SCRAPPERR_BOINCSHUTDOWNEVENT, IDS_ERR_BOINCSHUTDOWNEVENT,
		SCRAPPERR_BOINCAPPFOUNDGRAPHICSLOADING, IDS_ERR_BOINCAPPFOUNDGRAPHICSLOADING,

        SCRAPPERR_BOINCNOTDETECTED, IDS_ERR_BOINCNOTDETECTED,
		SCRAPPERR_BOINCSUSPENDED, IDS_ERR_BOINCSUSPENDED,
		SCRAPPERR_BOINCNOAPPSEXECUTING, IDS_ERR_BOINCNOAPPSEXECUTING,
        SCRAPPERR_BOINCNOPROJECTSDETECTED, IDS_ERR_BOINCNOAPPSEXECUTINGNOPROJECTSDETECTED,
		SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING, IDS_ERR_BOINCNOGRAPHICSAPPSEXECUTING,
        SCRAPPERR_DAEMONALLOWSNOGRAPHICS, IDS_ERR_DAEMONALLOWSNOGRAPHICS
    };
    const DWORD dwErrorMapSize = sizeof(dwErrorMap) / sizeof(DWORD[2]);

    DWORD iError;
    DWORD resid = 0;

    for(iError = 0; iError < dwErrorMapSize; iError++) {
        if (hr == (HRESULT)dwErrorMap[iError][0]) {
            resid = dwErrorMap[iError][1];
        }
    }
    if (resid == 0) {
        resid = IDS_ERR_GENERIC;
    }

    LoadString(NULL, resid, pszError, dwNumChars);

    if (resid == IDS_ERR_GENERIC) {
        return FALSE;
    } else {
        return TRUE;
    }
}




// Create the thread that is used to monitor input activity.
//
BOOL CScreensaver::CreateInputActivityThread() {
    DWORD dwThreadID = 0;
    BOINCTRACE(_T("CScreensaver::CreateInputActivityThread Start\n"));
    m_hInputActivityThread = CreateThread(
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        InputActivityProcStub,       // thread function 
        NULL,                        // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadID );               // returns the thread identifier 
 
   if (m_hInputActivityThread == NULL) {
    	BOINCTRACE(_T("CScreensaver::CreateInputActivityThread: Failed to create input activity thread '%d'\n"), GetLastError());
        return FALSE;
   }
   
   m_tThreadCreateTime = time(0);
   return TRUE;
}




// Terminate the thread that is used to monitor input activity.
//
BOOL CScreensaver::DestroyInputActivityThread() {
    if (!TerminateThread(m_hInputActivityThread, 0)) {
    	BOINCTRACE(_T("CScreensaver::DestroyInputActivityThread: Failed to terminate input activity thread '%d'\n"), GetLastError());
        return FALSE;
    }
    return TRUE;
}




// This function forwards to InputActivityProc, which has access to the
//   "this" pointer.
//
DWORD WINAPI CScreensaver::InputActivityProcStub(LPVOID UNUSED(lpParam)) {
    return gspScreensaver->InputActivityProc();
}




// Some graphics applications take a really long time to display something on their
// window, during this time the window will appear to eat keyboard and mouse event
// messages and not respond to other system events.  These windows are considered
// ghost windows, normally they have an outline and can be moved around and resized.
// In the graphic applications case where the borders are hidden from view, the
// window just takes on the background of the previous window which happens to be
// the black screensaver window owned by this process.
//
// Verify that their hasn't been any keyboard or mouse activity.  If there has,
// we should hide the window from this process and exit out of the screensaver to
// return control back to the user as quickly as possible.
//
DWORD WINAPI CScreensaver::InputActivityProc() {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);

    while(true) {
        GetLastInputInfo(&lii);
        if (m_dwLastInputTimeAtStartup != lii.dwTime) {
            BOINCTRACE(_T("CScreensaver::InputActivityProc - Activity Detected.\n"));
            SetError(TRUE, SCRAPPERR_BOINCSHUTDOWNEVENT);
            FireInterruptSaverEvent();
        }
        boinc_sleep(0.25);
    }
}




// Create the thread that is used to promote the graphics window.
//
BOOL CScreensaver::CreateGraphicsWindowPromotionThread() {
    DWORD dwThreadID = 0;
    BOINCTRACE(_T("CScreensaver::CreateGraphicsWindowPromotionThread Start\n"));
    m_hGraphicsWindowPromotionThread = CreateThread(
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        GraphicsWindowPromotionProcStub,       // thread function 
        NULL,                        // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadID );               // returns the thread identifier 
 
   if (m_hGraphicsWindowPromotionThread == NULL) {
    	BOINCTRACE(_T("CScreensaver::CreateGraphicsWindowPromotionThread: Failed to create graphics window promotion thread '%d'\n"), GetLastError());
        return FALSE;
   }
   return TRUE;
}




// Terminate the thread that is used to promote the graphics window.
//
BOOL CScreensaver::DestroyGraphicsWindowPromotionThread() {
    if (!TerminateThread(m_hGraphicsWindowPromotionThread, 0)) {
    	BOINCTRACE(_T("CScreensaver::DestroyGraphicsWindowPromotionThread: Failed to terminate graphics window promotion thread '%d'\n"), GetLastError());
        return FALSE;
    }
    return TRUE;
}




// This function forwards to GraphicsWindowPromotionProc, which has access to the
//   "this" pointer.
//
DWORD WINAPI CScreensaver::GraphicsWindowPromotionProcStub(LPVOID UNUSED(lpParam)) {
    return gspScreensaver->GraphicsWindowPromotionProc();
}




// When running in screensaver mode the only two valid conditions for z-order
//   is that either the screensaver or graphics application is the foreground
//   application.  If this is not true, then blow out of the screensaver.
//
DWORD WINAPI CScreensaver::GraphicsWindowPromotionProc() {
    HWND    hwndBOINCGraphicsWindow = NULL;
    HWND    hwndForeWindow = NULL;
    HWND    hwndForeParent = NULL;
    DWORD   iMonitor = 0;
    INTERNALMONITORINFO* pMonitorInfo = NULL;
    BOOL    bForegroundWindowIsScreensaver;

    while(true) {
        hwndBOINCGraphicsWindow = FindWindow(BOINC_WINDOW_CLASS_NAME, NULL);
        if (hwndBOINCGraphicsWindow) {
            // Graphics Application found.

            // If the graphics application is not the top most window try and force it
            //   to the top.
            hwndForeWindow = GetForegroundWindow();
            if (hwndForeWindow != hwndBOINCGraphicsWindow) {
                BOINCTRACE(_T("CScreensaver::GraphicsWindowPromotionProc - Graphics Window Detected but NOT the foreground window, bringing window to foreground.\n"));
                SetForegroundWindow(hwndBOINCGraphicsWindow);
                hwndForeWindow = GetForegroundWindow();
                if (hwndForeWindow != hwndBOINCGraphicsWindow) {
                    BOINCTRACE(_T("CScreensaver::GraphicsWindowPromotionProc - Graphics Window Detected but NOT the foreground window, bringing window to foreground. (Final Try)\n"));

                    // This may be needed on Windows 2000 or better machines
                    //
                    // NOTE: This API appears to be a SendMessage() variant and as such
                    //   can lock up this thread if a graphics application deadlocks.
                    //
                    DWORD dwComponents = BSM_APPLICATIONS;
                    BroadcastSystemMessage(
                        BSF_ALLOWSFW, 
                        &dwComponents,
                        WM_BOINCSFW,
                        NULL,
                        NULL
                    );
                }
            }
        } else {
            // Graphics application does not exist. So check that one of the windows
            //   assigned to each monitor is the foreground window.
            bForegroundWindowIsScreensaver = FALSE;
            hwndForeWindow = GetForegroundWindow();
            hwndForeParent = GetParent(hwndForeWindow);
            for(iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++) {
                pMonitorInfo = &m_Monitors[iMonitor];
                if ((pMonitorInfo->hWnd == hwndForeWindow) || (pMonitorInfo->hWnd == hwndForeParent))
                {
                    bForegroundWindowIsScreensaver = TRUE;
                }
            }
            if (!bForegroundWindowIsScreensaver) {
                // This can happen because of a personal firewall notifications or some
                //   funky IM client that thinks it has to notify the user even when in
                //   screensaver mode.
                BOINCTRACE(_T("CScreensaver::CheckForNotificationWindow - Unknown window detected\n"));
                SetError(TRUE, SCRAPPERR_BOINCSHUTDOWNEVENT);
                FireInterruptSaverEvent();
            }
        }
        boinc_sleep(1.0);
    }
}




// Create the thread that is used to talk to the daemon.
//
BOOL CScreensaver::CreateDataManagementThread() {
    DWORD dwThreadID = 0;
    BOINCTRACE(_T("CScreensaver::CreateDataManagementThread Start\n"));
    m_hDataManagementThread = CreateThread(
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        DataManagementProcStub,      // thread function 
        NULL,                        // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadID );               // returns the thread identifier 
 
   if (m_hDataManagementThread == NULL) {
    	BOINCTRACE(_T("CScreensaver::CreateDataManagementThread: Failed to create data management thread '%d'\n"), GetLastError());
        return FALSE;
   }
   return TRUE;
}




// Terminate the thread that is used to talk to the daemon.
//
BOOL CScreensaver::DestroyDataManagementThread() {
    if (!TerminateThread(m_hDataManagementThread, 0)) {
    	BOINCTRACE(_T("CScreensaver::DestoryDataManagementThread: Failed to terminate data management thread '%d'\n"), GetLastError());
        return FALSE;
    }
    return TRUE;
}




// This function forwards to DataManagementProc, which has access to the
//       "this" pointer.
//
DWORD WINAPI CScreensaver::DataManagementProcStub(LPVOID UNUSED(lpParam)) {
    return gspScreensaver->DataManagementProc();
}




void CScreensaver::HandleRPCError()
{
    rpc->close();
    m_bConnected = false;

    // Attempt to reinitialize the RPC client and state
    if (!rpc->init(NULL)) {
        m_bConnected = true;
        m_bResetCoreState = TRUE;
        return;
    }

    if ((time(0) - m_tThreadCreateTime) > 3) {
        SetError(TRUE, SCRAPPERR_BOINCNOTDETECTED);
    }
}




// Register and create the appropriate window(s)
//
HRESULT CScreensaver::CreateSaverWindow() {
    // Register an appropriate window class for the primary display
    WNDCLASS    cls;
    cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
    cls.hIcon          = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON)); 
    cls.lpszMenuName   = NULL;
    cls.lpszClassName  = _T("BOINCPrimarySaverWndClass");
    cls.hbrBackground  = (HBRUSH) GetStockObject(BLACK_BRUSH);
    cls.hInstance      = m_hInstance; 
    cls.style          = CS_VREDRAW|CS_HREDRAW;
    cls.lpfnWndProc    = SaverProcStub;
    cls.cbWndExtra     = 0; 
    cls.cbClsExtra     = 0; 
    RegisterClass(&cls);

    // Register an appropriate window class for the secondary display(s)
    WNDCLASS    cls2;
    cls2.hCursor        = LoadCursor(NULL, IDC_ARROW);
    cls2.hIcon          = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON)); 
    cls2.lpszMenuName   = NULL;
    cls2.lpszClassName  = _T("BOINCGenericSaverWndClass");
    cls2.hbrBackground  = (HBRUSH) GetStockObject(BLACK_BRUSH);
    cls2.hInstance      = m_hInstance; 
    cls2.style          = CS_VREDRAW|CS_HREDRAW;
    cls2.lpfnWndProc    = SaverProcStub;
    cls2.cbWndExtra     = 0; 
    cls2.cbClsExtra     = 0; 
    RegisterClass(&cls2);

    // Create the window
    RECT rc;
    DWORD dwStyle;
    switch (m_SaverMode) {
        case sm_preview:
            GetClientRect(m_hWndParent, &rc);
            dwStyle = WS_VISIBLE | WS_CHILD;
            AdjustWindowRect(&rc, dwStyle, FALSE);
            m_hWnd = CreateWindow(_T("BOINCPrimarySaverWndClass"),
                m_strWindowTitle, dwStyle, rc.left, rc.top, rc.right-rc.left,
                rc.bottom-rc.top, m_hWndParent, NULL, m_hInstance, this
            );
            m_Monitors[0].hWnd = m_hWnd;
            GetClientRect(m_hWnd, &m_rcRenderTotal);
            GetClientRect(m_hWnd, &m_rcRenderCurDevice);
            break;

        case sm_test:
            rc.left = rc.top = 50;
            rc.right = rc.left+600;
            rc.bottom = rc.top+400;
            dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
            AdjustWindowRect(&rc, dwStyle, FALSE);
            m_hWnd = CreateWindow(_T("BOINCPrimarySaverWndClass"),
                m_strWindowTitle, dwStyle, rc.left, rc.top, rc.right-rc.left,
                rc.bottom-rc.top, NULL, NULL, m_hInstance, this
            );
            m_Monitors[0].hWnd = m_hWnd;
            GetClientRect(m_hWnd, &m_rcRenderTotal);
            GetClientRect(m_hWnd, &m_rcRenderCurDevice);
			SetTimer(m_hWnd, 2, 60000, NULL);
            break;

        case sm_full:
            dwStyle = WS_VISIBLE | WS_POPUP;
            m_hWnd = NULL;
            for(DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++) {
                INTERNALMONITORINFO* pMonitorInfo;
                pMonitorInfo = &m_Monitors[iMonitor];
				if (pMonitorInfo->hWnd == NULL) {
					if (pMonitorInfo->hMonitor == NULL)
						continue;
					rc = pMonitorInfo->rcScreen;
					if (0 == iMonitor) {
						pMonitorInfo->hWnd = CreateWindowEx(NULL, _T("BOINCPrimarySaverWndClass"), 
							m_strWindowTitle, dwStyle, rc.left, rc.top, rc.right - rc.left, 
							rc.bottom - rc.top, NULL, NULL, m_hInstance, this);
					} else {
						pMonitorInfo->hWnd = CreateWindowEx(NULL, _T("BOINCGenericSaverWndClass"), 
							m_strWindowTitle, dwStyle, rc.left, rc.top, rc.right - rc.left, 
							rc.bottom - rc.top, NULL, NULL, m_hInstance, this);
					}
					if (pMonitorInfo->hWnd == NULL) {
						return E_FAIL;
                    }
					
                    if (m_hWnd == NULL) {
						m_hWnd = pMonitorInfo->hWnd;
                    }

					SetTimer(pMonitorInfo->hWnd, 2, 250, NULL);
				}
            }
    }
    if (m_hWnd == NULL) {
        return E_FAIL;
    }

    return S_OK;
}



// Run the screensaver graphics - may be preview, test or full-on mode
//
HRESULT CScreensaver::DoSaver() {
    // Flag as screensaver running if in full on mode
    if (m_SaverMode == sm_full) {
        BOOL bUnused;
        SystemParametersInfo(SPI_SCREENSAVERRUNNING, TRUE, &bUnused, 0);
    }


    // Message pump
    BOOL bGotMsg;
    MSG msg;
    msg.message = WM_NULL;
    while (msg.message != WM_QUIT) {
        bGotMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (bGotMsg) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            Sleep(10);
        }
    }

    return S_OK;
}




VOID CScreensaver::DoConfig() {
    DialogBox(NULL, MAKEINTRESOURCE(DLG_CONFIG), m_hWndParent, ConfigureDialogProcStub);
}



// Handle window messages for main screensaver windows.
//
LRESULT CScreensaver::SaverProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
    DWORD dwMonitor = 0;
#ifdef _DEBUG
    for(DWORD iIndex = 0; iIndex < m_dwNumMonitors; iIndex++) {
		if (hWnd == m_Monitors[iIndex].hWnd ) {
            dwMonitor = iIndex;
        }
    }
#endif

    BOINCTRACE(_T("CScreensaver::SaverProc [%d] hWnd '%d' uMsg '%X' wParam '%d' lParam '%d'\n"), dwMonitor, hWnd, uMsg, wParam, lParam);

    switch (uMsg) {
        case WM_TIMER:
            BOINCTRACE(_T("CScreensaver::SaverProc Received WM_TIMER\n"));
			switch (wParam) { 
				case 1: 
					// Initial idle time is done, proceed with initialization.
					m_bWaitForInputIdle = FALSE;
					KillTimer(hWnd, 1);
                    return 0;
                    break;
				case 2:
                    // Create a screen saver window on the primary display if 
                    //   the boinc client crashes
	                CreateSaverWindow();

                    // Update the position of the box every second so that it
                    //   does not end up off the visible area of the screen.
				    UpdateErrorBox();
                    return 0;
                    break;
            }
            break;
        case WM_PAINT:
            {
				BOOL    bErrorMode;
				HRESULT hrError;
				TCHAR	szError[400];
				GetError(bErrorMode, hrError, szError, sizeof(szError)/sizeof(TCHAR));

				// Show error message, if there is one
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);

                // In preview mode, just fill 
                // the preview window with black, and the BOINC icon. 
                if (!bErrorMode && m_SaverMode == sm_preview) {
                    RECT rc;
                    GetClientRect(hWnd,&rc);
				    FillRect(ps.hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
				    DrawIcon(ps.hdc, (rc.right / 2) - 16, (rc.bottom / 2) - 16,
					    LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON)));
                } else {
                    DoPaint(hWnd, ps.hdc, &ps);
                }

                EndPaint(hWnd, &ps);
            }

            return 0;
            break;

        case WM_MOUSEMOVE:
            if (m_SaverMode != sm_test) {
                static INT xPrev = -1;
                static INT yPrev = -1;
                INT xCur = GET_X_LPARAM(lParam);
                INT yCur = GET_Y_LPARAM(lParam);
                if (xCur != xPrev || yCur != yPrev) {
                    xPrev = xCur;
                    yPrev = yCur;
                    m_dwSaverMouseMoveCount++;
                    if (m_dwSaverMouseMoveCount > 5) {
                        BOINCTRACE(_T("CScreensaver::SaverProc Received WM_MOUSEMOVE and time to InterruptSaver()\n"));
                        InterruptSaver();
                    }
                }
            }
            return 0;
            break;

        case WM_KEYDOWN:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            BOINCTRACE(_T("CScreensaver::SaverProc Received WM_KEYDOWN | WM_LBUTTONDOWN | WM_RBUTTONDOWN | WM_MBUTTONDOWN\n"));
            if (m_SaverMode != sm_test) {
                InterruptSaver();
            }
            return 0;
            break;

        case WM_CLOSE:
        case WM_DESTROY:
            BOINCTRACE(_T("CScreensaver::SaverProc Received WM_CLOSE or WM_DESTROY\n"));
            if (m_SaverMode == sm_preview || m_SaverMode == sm_test) {
                ShutdownSaver();
            }
            return 0;
            break;

        case WM_SYSCOMMAND: 
            BOINCTRACE(_T("CScreensaver::SaverProc Received WM_SYSCOMMAND\n"));
            if (m_SaverMode == sm_full) {
                switch (wParam) {
                    case SC_NEXTWINDOW:
                    case SC_PREVWINDOW:
                    case SC_SCREENSAVE:
                    case SC_CLOSE:
                        return 0;
                }
            }
            break;

        case WM_SETCURSOR:
            BOINCTRACE(_T("CScreensaver::SaverProc Received WM_SETCURSOR\n"));
            if (m_SaverMode == sm_full) {
                // Hide cursor
                SetCursor(NULL);
                return TRUE;
            }
            break;

        case WM_POWERBROADCAST:
            BOINCTRACE(_T("CScreensaver::SaverProc Received WM_POWERBROADCAST\n"));
            if (wParam == PBT_APMQUERYSUSPEND)
                InterruptSaver();
            break;
    }

    if (WM_SETTIMER == uMsg) {

        BOINCTRACE(_T("CScreensaver::SaverProc Received WM_SETTIMER\n"));
        // All initialization messages have gone through.  Allow
        // 500ms of idle time, then proceed with initialization.
        SetTimer(hWnd, 1, 500, NULL);

    } else if (WM_INTERRUPTSAVER == uMsg) {

        BOINCTRACE(_T("CScreensaver::SaverProc Received WM_INTERRUPTSAVER\n"));
        InterruptSaver();

    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




INT_PTR CScreensaver::ConfigureDialogProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM UNUSED(lParam)) {
	DWORD screen_blank=0, blank_time=0;
	char buf[256];
	int retval;

	switch (msg) {
		case WM_INITDIALOG:
			// make sure you check return value of registry queries
			// in case the item in question doesn't happen to exist.
			retval = UtilGetRegKey(REG_BLANK_NAME, screen_blank);
			if (retval < 0) {
                screen_blank=0;
            }
			CheckDlgButton(hwnd, IDC_BLANK, screen_blank);

			retval = UtilGetRegKey(REG_BLANK_TIME, blank_time);
			if (retval < 0) { blank_time=0; }
			_ltot(blank_time, buf, 10);
			SetDlgItemText(hwnd, IDC_BLANK_TIME, buf);

			return TRUE;
		case WM_COMMAND:
			int id=LOWORD(wParam);
			if (id==IDOK) {

				screen_blank = (IsDlgButtonChecked(hwnd, IDC_BLANK) == BST_CHECKED);
				UtilSetRegKey(REG_BLANK_NAME, screen_blank);

				GetDlgItemText(hwnd, IDC_BLANK_TIME, buf, 256);
				blank_time = atoi(buf);
				UtilSetRegKey(REG_BLANK_TIME, blank_time);

			}
			if (id == IDOK || id == IDCANCEL) {
				EndDialog(hwnd, id);
            }
			break;
	}
	return FALSE;
}




// This function forwards all window messages to SaverProc, which has
//       access to the "this" pointer.
//
LRESULT CALLBACK CScreensaver::SaverProcStub(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
    return gspScreensaver->SaverProc(hWnd, uMsg, wParam, lParam);
}




INT_PTR CALLBACK CScreensaver::ConfigureDialogProcStub(
    HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
    return gspScreensaver->ConfigureDialogProc(hwndDlg, uMsg, wParam, lParam);
}




VOID CScreensaver::ShutdownSaver() {
    BOINCTRACE(_T("CScreensaver::ShutdownSaver Function Begin\n"));

    // Unflag screensaver running if in full on mode
    if (m_SaverMode == sm_full) {
        BOOL bUnused;
        SystemParametersInfo(SPI_SCREENSAVERRUNNING, FALSE, &bUnused, 0);
    }

    // Kill the currently executing graphics application
    terminate_screensaver(m_hGraphicsApplication, &m_running_result);

    // Post message to drop out of message loop
    // This can be called from the data management thread, so specifically
    // lookup and post to the primary window instead of calling PostQuitMessage
    // since PostQuitMessage posts to the current threads message pump if it
    // exists.
    for(DWORD iIndex = 0; iIndex < m_dwNumMonitors; iIndex++) {
		if ( m_Monitors[iIndex].hWnd ) {
            PostMessage(m_Monitors[iIndex].hWnd, WM_QUIT, NULL, NULL);
        }
    }

    BOINCTRACE(_T("CScreensaver::ShutdownSaver Function End\n"));
}




VOID CScreensaver::FireInterruptSaverEvent() {
    BOINCTRACE(_T("CScreensaver::FireInterruptSaverEvent Function Begin\n"));

    for(DWORD iIndex = 0; iIndex < m_dwNumMonitors; iIndex++) {
		if ( m_Monitors[iIndex].hWnd ) {
            PostMessage(m_Monitors[iIndex].hWnd, WM_INTERRUPTSAVER, NULL, NULL);
        }
    }

    BOINCTRACE(_T("CScreensaver::FireInterruptSaverEvent Function End\n"));
}




// A message was received (mouse move, keydown, etc.) that may mean
//     the screen saver should show the password dialog and/or shut down.
//
VOID CScreensaver::InterruptSaver() {
    BOINCTRACE(_T("CScreensaver::InterruptSaver Function Begin\n"));
    ShutdownSaver();
    BOINCTRACE(_T("CScreensaver::InterruptSaver Function End\n"));
}




// Update the box that shows the error message
//
VOID CScreensaver::UpdateErrorBox() {
    INTERNALMONITORINFO* pMonitorInfo;
    HWND hwnd;
    RECT rcBounds;
    static DWORD dwTimeLast = 0;
    DWORD dwTimeNow;
    FLOAT fTimeDelta;


    // Update timing to determine how much to move error box
    if (dwTimeLast == 0) {
        dwTimeLast = timeGetTime();
    }

    dwTimeNow = timeGetTime();
    fTimeDelta = (FLOAT)(dwTimeNow - dwTimeLast) / 10000.0f;
    dwTimeLast = dwTimeNow;

    for(DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++) {
        pMonitorInfo = &m_Monitors[iMonitor];
        hwnd = pMonitorInfo->hWnd;
        if (hwnd == NULL)
            continue;
        if (m_SaverMode == sm_full) {
            rcBounds = pMonitorInfo->rcScreen;
            ScreenToClient(hwnd, (POINT*)&rcBounds.left);
            ScreenToClient(hwnd, (POINT*)&rcBounds.right);
        } else {
            rcBounds = m_rcRenderTotal;
        }

        if (pMonitorInfo->widthError == 0) {
            if (m_SaverMode == sm_preview)                {
                pMonitorInfo->widthError = (float) (rcBounds.right - rcBounds.left);
                pMonitorInfo->heightError = (float) (rcBounds.bottom - rcBounds.top);
                pMonitorInfo->xError = 0.0f;
                pMonitorInfo->yError = 0.0f;
                pMonitorInfo->xVelError = 0.0f;
                pMonitorInfo->yVelError = 0.0f;
                InvalidateRect(hwnd, NULL, FALSE);    // Invalidate the hwnd so it gets drawn
                UpdateWindow(hwnd);
            } else {
                pMonitorInfo->widthError = 454;
                pMonitorInfo->heightError = 320;
                pMonitorInfo->xError = (rcBounds.right + rcBounds.left - pMonitorInfo->widthError) / 2.0f;
                pMonitorInfo->yError = (rcBounds.bottom + rcBounds.top - pMonitorInfo->heightError) / 2.0f;
                pMonitorInfo->xVelError = (rcBounds.right - rcBounds.left) / 10.0f;
                pMonitorInfo->yVelError = (rcBounds.bottom - rcBounds.top) / 20.0f;
            }
        } else {
            if (m_SaverMode != sm_preview) {
                RECT rcOld;
                RECT rcNew;

                SetRect(&rcOld, (INT)pMonitorInfo->xError, (INT)pMonitorInfo->yError,
                    (INT)(pMonitorInfo->xError + pMonitorInfo->widthError),
                    (INT)(pMonitorInfo->yError + pMonitorInfo->heightError));

                // Update rect velocity
                if ((pMonitorInfo->xError + pMonitorInfo->xVelError * fTimeDelta + 
                    pMonitorInfo->widthError > rcBounds.right && pMonitorInfo->xVelError > 0.0f) ||
                    (pMonitorInfo->xError + pMonitorInfo->xVelError * fTimeDelta < 
                    rcBounds.left && pMonitorInfo->xVelError < 0.0f)
                ) {
                    pMonitorInfo->xVelError = -pMonitorInfo->xVelError;
                }
                if ((pMonitorInfo->yError + pMonitorInfo->yVelError * fTimeDelta + 
                    pMonitorInfo->heightError > rcBounds.bottom && pMonitorInfo->yVelError > 0.0f) ||
                    (pMonitorInfo->yError + pMonitorInfo->yVelError * fTimeDelta < 
                    rcBounds.top && pMonitorInfo->yVelError < 0.0f)
                ) {
                    pMonitorInfo->yVelError = -pMonitorInfo->yVelError;
                }
                // Update rect position
                pMonitorInfo->xError += pMonitorInfo->xVelError * fTimeDelta;
                pMonitorInfo->yError += pMonitorInfo->yVelError * fTimeDelta;
            
                SetRect(&rcNew, (INT)pMonitorInfo->xError, (INT)pMonitorInfo->yError,
                    (INT)(pMonitorInfo->xError + pMonitorInfo->widthError),
                    (INT)(pMonitorInfo->yError + pMonitorInfo->heightError));

				if ((dwTimeNow - pMonitorInfo->dwTimeLastUpdate) > 1000)
				{
					pMonitorInfo->dwTimeLastUpdate = dwTimeNow;

                    InvalidateRect(hwnd, NULL, TRUE);
                    UpdateWindow(hwnd);
				}
            }
        }
    }
}




VOID CScreensaver::DoPaint(HWND hwnd, HDC hdc, LPPAINTSTRUCT lpps) {
    DWORD iMonitor = 0;
    INTERNALMONITORINFO* pMonitorInfo = NULL;
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    for(iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++) {
        pMonitorInfo = &m_Monitors[iMonitor];
        if (pMonitorInfo->hMonitor == hMonitor)
            break;
    }

    if (iMonitor == m_dwNumMonitors) {
        return;
    }

    // Retrieve the latest piece of error information 
    BOOL    bErrorMode;
    HRESULT hrError;
    TCHAR	szError[400];
    GetError(bErrorMode, hrError, szError, sizeof(szError)/sizeof(TCHAR));


    // Start drawing the goods
    RECT    rc;
    RECT    rc2;
    RECT    rcOrginal;
	int		iTextHeight;

	static HBRUSH	hbrushBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
	static HBRUSH	hbrushRed = (HBRUSH)CreateSolidBrush(RGB(255,0,0));
	static HBITMAP  hbmp = LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_BOINCSPLAT));


	// Start off with a black screen and then draw on top of it.
	FillRect(hdc, &lpps->rcPaint, hbrushBlack);


    // If the screensaver has switched to a blanked state or not in an error mode,
    // we should exit here so the screen has been erased to black.
    if (!bErrorMode) {
        return;
    }

    
    SetRect(&rc, (INT)pMonitorInfo->xError, (INT)pMonitorInfo->yError,
        (INT)(pMonitorInfo->xError + pMonitorInfo->widthError),
        (INT)(pMonitorInfo->yError + pMonitorInfo->heightError)
    );

    // This fill rect is useful when testing
    //FillRect(hdc, &rc, hbrushRed);

    rcOrginal = rc;


    // Draw the bitmap rectangle and copy the bitmap into 
    // it. the bitmap is centered in the rectangle by adding 2
	// to the left and top coordinates of the bitmap rectangle,
	// and subtracting 4 from the right and bottom coordinates.
    BITMAP     bm;
    GetObject(hbmp, sizeof(BITMAP), (LPSTR)&bm);

	long left = rc.left + (pMonitorInfo->widthError - 4 - bm.bmWidth)/2;
	long top = rc.top + 2;
    DrawTransparentBitmap(hdc, hbmp, left, top, RGB(255, 0, 255));

	// Draw text in the center of the frame
	SetBkColor(hdc, RGB(0,0,0));           // Black
	SetTextColor(hdc, RGB(255,255,255));   // Red
   
	// Set font
	HFONT hf;
    hf = CreateFont(0, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial Narrow");
	
    if(hf)
    {
        SelectObject(hdc, hf);
    }

    // Try using the "Arial Narrow" font, if that fails use whatever
    // the system default font is.  Something is better than nothing.
    rc2 = rc;
    iTextHeight = DrawText(hdc, szError, -1, &rc, DT_CENTER | DT_CALCRECT);
	rc = rc2;
	rc2.top += bm.bmHeight+20;
    DrawText(hdc, szError, -1, &rc2, DT_CENTER);

    if(hf)
    {
        DeleteObject(hf);
    }
}




// Draws a bitmap on the screen with a transparent background.
//         Code orginally from Microsoft Knowledge Base Article - 79212
//
void CScreensaver::DrawTransparentBitmap(
    HDC hdc, HBITMAP hBitmap, LONG xStart, LONG yStart,
    COLORREF cTransparentColor
){
    BITMAP     bm;
    COLORREF   cColor;
    HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
    HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
    HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT      ptSize;

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, hBitmap);   // Select the bitmap

    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
    ptSize.x = bm.bmWidth;            // Get width of bitmap
    ptSize.y = bm.bmHeight;           // Get height of bitmap
    DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device

                                     // to logical points

    // Create some DCs to hold temporary data.
    hdcBack   = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem    = CreateCompatibleDC(hdc);
    hdcSave   = CreateCompatibleDC(hdc);

    // Create a bitmap for each DC. DCs are required for a number of
    // GDI functions.

    // Monochrome DC
    bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    // Monochrome DC
    bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

    // Each DC must select a bitmap object to store pixel data.
    bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack);
    bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
    bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem);
    bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave);

    // Set proper mapping mode.
    SetMapMode(hdcTemp, GetMapMode(hdc));

    // Save the bitmap sent here, because it will be overwritten.
    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC to the color.
    // contained in the parts of the bitmap that should be transparent
    cColor = SetBkColor(hdcTemp, cTransparentColor);

    // Create the object mask for the bitmap by performing a BitBlt
    // from the source bitmap to a monochrome bitmap.
    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC back to the original
    // color.
    SetBkColor(hdcTemp, cColor);

    // Create the inverse of the object mask.
    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

    // Copy the background of the main DC to the destination.
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart, SRCCOPY);

    // Mask out the places where the bitmap will be placed.
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

    // Mask out the transparent colored pixels on the bitmap.
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

    // XOR the bitmap with the background on the destination DC.
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

    // Copy the destination to the screen.
    BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);

    // Place the original bitmap back into the bitmap sent here.
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

    // Delete the memory bitmaps.
    DeleteObject(SelectObject(hdcBack, bmBackOld));
    DeleteObject(SelectObject(hdcObject, bmObjectOld));
    DeleteObject(SelectObject(hdcMem, bmMemOld));
    DeleteObject(SelectObject(hdcSave, bmSaveOld));

    // Delete the memory DCs.
    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
}




VOID CScreensaver::ChangePassword() {
    // Load the password change DLL
    HINSTANCE mpr = LoadLibrary(_T("MPR.DLL"));

    if (mpr != NULL) {
        // Grab the password change function from it
        typedef DWORD (PASCAL *PWCHGPROC)(LPCSTR, HWND, DWORD, LPVOID);
        PWCHGPROC pwd = (PWCHGPROC)GetProcAddress(mpr, "PwdChangePasswordA");

        // Do the password change
        if (pwd != NULL) {
            pwd("SCRSAVE", m_hWndParent, 0, NULL);
        }

        // Free the library
        FreeLibrary(mpr);
    }
}




const char *BOINC_RCSID_116268c72f = "$Id$";
