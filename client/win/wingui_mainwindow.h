// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#ifndef __WIN_MAINWINDOW_H_
#define __WIN_MAINWINDOW_H_

// includes

#include "wingui.h"
#include "wingui_listctrl.h"
#include "wingui_piectrl.h"
#include "wingui_dialog.h"
#include "wingui_sswindow.h"
#include "gui_titles.h"

// constants

#define WND_TITLE			"BOINC"		// window's title

#define ICON_OFF			0			// remove icon
#define ICON_NORMAL			1			// normal icon
#define ICON_NETWORK		2			// network icon
#define ICON_ERROR			3			// error icon
#define ICON_INFO			4			// info icon
#define ICON_WARNING		5			// warning icon

#define ICON_BLINKCOUNT		10			// Number of times the icon should blink
										//   in the system tray before becoming solid

#define DEF_COL_WIDTH		80			// default width of list columns

#define TOP_BUFFER			5			// buffer pixels around edge of client
#define EDGE_BUFFER			2			// buffer pixels around edge of client

#define GUI_TIMER			104			// gui refresh
#define GUI_WAIT			1000		// timeout sleep time (in milliseconds)

#define STATUS_ICON_ID		(WM_USER + 1)	// id for notifications from status icon

#define STATUS_MENU			0			// submenus for context menus
#define PROJECT_MENU		1
#define RESULT_MENU			2
#define XFER_MENU			3
#define MESSAGE_MENU		4


#define PROJECT_ID			0			// child control ids
#define RESULT_ID			1
#define XFER_ID				2
#define MESSAGE_ID			3
#define USAGE_ID			4
#define TAB_ID				5

#define PROJECT_COLS		5			// number of columns for each control
#define RESULT_COLS			8
#define XFER_COLS			7
#define MESSAGE_COLS		3


//////////
// class:		CMyApp
// parent:		CWinApp
// description:	subclasses CWinApp to create main window.
class CMyApp : public CWinApp 
{
public:
    virtual BOOL			InitInstance();
    virtual int				ExitInstance();
};

//////////
// class:		CMainWindow
// parent:		CWnd
// description:	the main window, organizes child control windows and client
//				state, handles timer updates, some display features.
class CMainWindow : public CWnd
{
public:
							CMainWindow ();
	void					UpdateGUI(CLIENT_STATE*);
	void					MessageUser(char*,char*,int);
    BOOL					IsUserSuspended();
    BOOL					RequestNetConnect();
	void					DetachProject(PROJECT *);
	CSSWindow*				m_pSSWnd;				// default graphics window

protected:
	CMenu					m_MainMenu;				// window's main menu
	CMenu					m_ContextMenu;			// context menu for status icon and lists
	CProgressListCtrl		m_ProjectListCtrl;		// list control
	CProgressListCtrl		m_XferListCtrl;			// list control
	CProgressListCtrl		m_ResultListCtrl;		// list control
	CProgressListCtrl		m_MessageListCtrl;		// list control for messages to user
	CPieChartCtrl			m_UsagePieCtrl;			// pie chart control
	CPieChartCtrl			m_UsageBOINCPieCtrl;	// pie chart control for BOINC
	CFont					m_Font;					// window's font
	CTabCtrl				m_TabCtrl;				// tab control for choosing display
	CImageList				m_TabIL;				// image list for tab control
	CBitmap					m_TabBMP[MAX_TABS];		// bitmaps for tab image list
	HINSTANCE				m_hIdleDll;				// handle to dll for user idle
	int						m_nIconState;			// state of the status icon
	int						m_nDesiredIconState;	// desired state of the status icon
	int						m_nToggleIconState;		// next icon state
	int						m_nIconBlinkCount;		// number of times the icon has blinked
	BOOL					m_bMessage;				// does the user have a new message?
	int						m_nMessage;				// desired message type
	BOOL					m_bRequest;				// does the user have a net connection request?
	int						m_nContextItem;			// item selected for context menu
	UINT					m_nGuiTimerID;			// ID of current GUI timer	


	void UpdateRunRequestMenu(CMenu* pMenu);
	void UpdateRunRequestFileMenu();
	COLORREF				GetPieColor(int);
	void					CheckIdle();
    void					ShowTab(int);
    bool					SetStatusIcon(DWORD);
    void					SaveListControls();
    void					LoadListControls();
    void					SaveUserSettings();
    void					LoadUserSettings();
	void					LoadLanguage();
	PROJECT*				GetProjectFromContextMenu();
	DWORD					GetUserIdleTime();
	void					ClearProjectItems(char *);
	void					Syncronize(CProgressListCtrl*, vector<void*>*);
    virtual void			PostNcDestroy();
	bool					should_request_global_prefs();

    afx_msg void			OnClose();
    afx_msg void			OnDestroy();
    afx_msg int				OnCreate(LPCREATESTRUCT);
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
	afx_msg void			OnRButtonDown(UINT, CPoint);
    afx_msg void			OnSetFocus(CWnd*);
    afx_msg void			OnSize(UINT, int, int);
	afx_msg void			OnTimer(UINT);

    afx_msg void			OnCommandSettingsQuit();
	afx_msg void			OnCommandSettingsLogin();
	afx_msg void			OnCommandSettingsProxyServer();
	afx_msg void			OnCommandHelpAbout();
	afx_msg void			OnCommandProjectWebSite();
	afx_msg void			OnCommandProjectGetPrefs();
	afx_msg void			OnCommandProjectDetach();
	afx_msg void			OnCommandProjectReset();
	afx_msg void			OnCommandWorkShowGraphics();
	afx_msg void            OnCommandTransfersRetryNow();
	afx_msg void			OnCommandFileClearInactive();
	afx_msg void			OnCommandFileClearMessages();
	afx_msg void            OnCommandRunBenchmarks();
	afx_msg void			OnCommandConnectionHangupIfDialed();
	afx_msg void			OnCommandConnectionConfirmBeforeConnecting();
	afx_msg void			OnCommandShow();
	afx_msg void			OnCommandHide();
	afx_msg void            OnCommandRunRequestAlways();
	afx_msg void            OnCommandRunRequestAuto();
	afx_msg void            OnCommandRunRequestNever();

	afx_msg void			OnCommandExit();
	afx_msg void			OnCommandMessageCopyToClip();

    afx_msg LRESULT			OnStatusIcon( WPARAM, LPARAM );

    afx_msg LRESULT         OnShowWindow( WPARAM, LPARAM );
    afx_msg LRESULT         OnNetworkActivity( WPARAM, LPARAM );
    afx_msg LRESULT         OnStartScreensaver( WPARAM, LPARAM );
    afx_msg LRESULT         OnEndScreensaver( WPARAM, LPARAM );
    afx_msg LRESULT         OnTaskbarCreated( WPARAM, LPARAM );

	CString m_MenuLabelRetryNow, m_MenuLabelGetPreferences;
	CString m_DialogResetQuery, m_DialogDetachQuery;

    DECLARE_MESSAGE_MAP()

public:
	void OnBenchmarksBegin();
	void OnBenchmarksEnd();
};

// globals

extern CMyApp g_myApp;
extern CMainWindow* g_myWnd;

#endif
