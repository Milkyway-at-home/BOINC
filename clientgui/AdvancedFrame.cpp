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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AdvancedFrame.h"
#endif

#ifdef __APPLE__
#include "mac/MacGUI.pch"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "Events.h"
#include "BOINCBaseFrame.h"
#include "AdvancedFrame.h"
#include "BOINCBaseView.h"
#include "BOINCDialupManager.h"
#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewStatistics.h"
#include "ViewResources.h"
#include "DlgAbout.h"
#include "DlgGenericMessage.h"
#include "DlgOptions.h"
#include "DlgSelectComputer.h"
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#ifdef __WXMAC__
#include "mac/MacGUI.pch"
#endif

#include "res/connect.xpm"
#include "res/disconnect.xpm"


enum STATUSBARFIELDS {
    STATUS_TEXT,
    STATUS_CONNECTION_STATUS
};


IMPLEMENT_DYNAMIC_CLASS(CStatusBar, wxStatusBar)

BEGIN_EVENT_TABLE(CStatusBar, wxStatusBar)
    EVT_SIZE(CStatusBar::OnSize)
END_EVENT_TABLE()


CStatusBar::CStatusBar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Default Constructor Function End"));
}


CStatusBar::CStatusBar(wxWindow *parent) :
    wxStatusBar(parent, ID_STATUSBAR, wxST_SIZEGRIP, _T("statusBar"))
{
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Function Begin"));

    const int widths[] = {-1, 200, 20};
    SetFieldsCount(WXSIZEOF(widths), widths);

    m_pbmpConnected = new wxStaticBitmap(this, -1, wxIcon(connect_xpm));
    wxASSERT(m_pbmpConnected);
    m_pbmpConnected->Hide();

    m_ptxtConnected = new wxStaticText(this, -1, _("Connected"), wxPoint(0, 0), wxDefaultSize, wxALIGN_LEFT);
    wxASSERT(m_ptxtConnected);
    m_ptxtConnected->Hide();

    m_pbmpDisconnect = new wxStaticBitmap(this, -1, wxIcon(disconnect_xpm));
    wxASSERT(m_pbmpDisconnect);
    m_pbmpDisconnect->Hide();

    m_ptxtDisconnect = new wxStaticText(this, -1, _("Disconnected"), wxPoint(0, 0), wxDefaultSize, wxALIGN_LEFT);
    wxASSERT(m_ptxtDisconnect);
    m_ptxtDisconnect->Hide();

    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Function End"));
}


CStatusBar::~CStatusBar()
{

}


void CStatusBar::OnSize(wxSizeEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::OnSize - Function Begin"));

    if (IsShown()) {
        wxRect rect;
        wxSize size;

        GetFieldRect(STATUS_CONNECTION_STATUS, rect);

        if (m_pbmpConnected) {
            size = m_pbmpConnected->GetSize();
            m_pbmpConnected->Move(rect.x + 1,
                                  rect.y + (rect.height - size.y) / 2);
        }

        if (m_ptxtConnected) {
            m_ptxtConnected->Move((rect.x + size.x) + 2,
                                  (rect.y + (rect.height - size.y) / 2) + 1);
        }

        if (m_pbmpDisconnect) {
            size = m_pbmpConnected->GetSize();
            m_pbmpDisconnect->Move(rect.x + 1,
                                   rect.y + (rect.height - size.y) / 2);
        }

        if (m_ptxtDisconnect) {
            m_ptxtDisconnect->Move((rect.x + size.x) + 2,
                                   (rect.y + (rect.height - size.y) / 2) + 1);
        }
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::OnSize - Function End"));
}


IMPLEMENT_DYNAMIC_CLASS(CAdvancedFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE (CAdvancedFrame, CBOINCBaseFrame)
    EVT_MENU(ID_FILERUNBENCHMARKS, CAdvancedFrame::OnRunBenchmarks)
    EVT_MENU(ID_FILESELECTCOMPUTER, CAdvancedFrame::OnSelectComputer)
    EVT_MENU(ID_FILESWITCHGUI, CAdvancedFrame::OnSwitchGUI)
    EVT_MENU(wxID_EXIT, CAdvancedFrame::OnExit)
    EVT_MENU_RANGE(ID_FILEACTIVITYRUNALWAYS, ID_FILEACTIVITYSUSPEND, CAdvancedFrame::OnActivitySelection)
    EVT_MENU_RANGE(ID_FILENETWORKRUNALWAYS, ID_FILENETWORKSUSPEND, CAdvancedFrame::OnNetworkSelection)
    EVT_MENU(ID_PROJECTSATTACHACCOUNTMANAGER, CAdvancedFrame::OnProjectsAttachToAccountManager)
    EVT_MENU(ID_TOOLSAMUPDATENOW, CAdvancedFrame::OnAccountManagerUpdate)
    EVT_MENU(ID_ADVANCEDAMDEFECT, CAdvancedFrame::OnAccountManagerDetach)
    EVT_MENU(ID_PROJECTSATTACHPROJECT, CAdvancedFrame::OnProjectsAttachToProject)
    EVT_MENU(ID_COMMANDSRETRYCOMMUNICATIONS, CAdvancedFrame::OnCommandsRetryCommunications)
    EVT_MENU(ID_OPTIONSOPTIONS, CAdvancedFrame::OnOptionsOptions)
    EVT_HELP(ID_ADVANCEDFRAME, CAdvancedFrame::OnHelp)
    EVT_MENU(ID_HELPBOINCMANAGER, CAdvancedFrame::OnHelpBOINCManager)
    EVT_MENU(ID_HELPBOINC, CAdvancedFrame::OnHelpBOINCWebsite)
    EVT_MENU(wxID_ABOUT, CAdvancedFrame::OnHelpAbout)
    EVT_SHOW(CAdvancedFrame::OnShow)
    EVT_FRAME_REFRESH(CAdvancedFrame::OnRefreshView)
    EVT_FRAME_CONNECT(CAdvancedFrame::OnConnect)
    EVT_FRAME_UPDATESTATUS(CAdvancedFrame::OnUpdateStatus)
    EVT_TIMER(ID_REFRESHSTATETIMER, CAdvancedFrame::OnRefreshState)
    EVT_TIMER(ID_FRAMERENDERTIMER, CAdvancedFrame::OnFrameRender)
    EVT_TIMER(ID_FRAMELISTRENDERTIMER, CAdvancedFrame::OnListPanelRender)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_FRAMENOTEBOOK, CAdvancedFrame::OnNotebookSelectionChanged)
END_EVENT_TABLE ()


CAdvancedFrame::CAdvancedFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Default Constructor Function End"));
}


CAdvancedFrame::CAdvancedFrame(wxString title, wxIcon* icon) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_ADVANCEDFRAME, title, wxDefaultPosition, wxDefaultSize,
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Function Begin"));
    
    m_pMenubar = NULL;
    m_pNotebook = NULL;
    m_pStatusbar = NULL;

    // Configuration Settings
    m_Top = 30;
    m_Left = 30;
    m_Width = 800;
    m_Height = 600;


    // Working Variables
    m_strBaseTitle = title;


    // Initialize Application
    SetIcon(*icon);

    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));

    RestoreState();

    SetStatusBarPane(0);


    m_pRefreshStateTimer = new wxTimer(this, ID_REFRESHSTATETIMER);
    wxASSERT(m_pRefreshStateTimer);

    m_pFrameRenderTimer = new wxTimer(this, ID_FRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);

    m_pFrameListPanelRenderTimer = new wxTimer(this, ID_FRAMELISTRENDERTIMER);
    wxASSERT(m_pFrameListPanelRenderTimer);

    m_pRefreshStateTimer->Start(300000);             // Send event every 5 minutes
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second
    m_pFrameListPanelRenderTimer->Start(1000);       // Send event every 1 second

    // Limit the number of times the UI can update itself to two times a second
    //   NOTE: Linux and Mac were updating several times a second and eating
    //         CPU time
    wxUpdateUIEvent::SetUpdateInterval(500);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Function End"));
}


CAdvancedFrame::~CAdvancedFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::~CAdvancedFrame - Function Begin"));

    wxASSERT(m_pRefreshStateTimer);
    wxASSERT(m_pFrameRenderTimer);
    wxASSERT(m_pFrameListPanelRenderTimer);
    wxASSERT(m_pMenubar);
    wxASSERT(m_pNotebook);
    wxASSERT(m_pStatusbar);

    SaveState();

    if (m_pRefreshStateTimer) {
        m_pRefreshStateTimer->Stop();
        delete m_pRefreshStateTimer;
    }

    if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }

    if (m_pFrameListPanelRenderTimer) {
        m_pFrameListPanelRenderTimer->Stop();
        delete m_pFrameListPanelRenderTimer;
    }

    if (m_pStatusbar)
        wxCHECK_RET(DeleteStatusbar(), _T("Failed to delete status bar."));

    if (m_pNotebook)
        wxCHECK_RET(DeleteNotebook(), _T("Failed to delete notebook."));

    if (m_pMenubar)
        wxCHECK_RET(DeleteMenu(), _T("Failed to delete menu bar."));

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::~CAdvancedFrame - Function End"));
}


bool CAdvancedFrame::CreateMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateMenu - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    ACCT_MGR_INFO      ami;
    bool               is_acct_mgr_detected = false;
    wxString           strMenuName;
    wxString           strMenuDescription;
    
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Account managers have a different menu arrangement
    pDoc->rpc.acct_mgr_info(ami);
    is_acct_mgr_detected = ami.acct_mgr_url.size() ? true : false;

    // File menu
    wxMenu *menuFile = new wxMenu;

#ifdef SIMPLEGUI
    menuFile->Append(
        ID_FILESWITCHGUI,
        _("&Switch to Simple GUI"),
        _("Display the simple BOINC graphical interface.")
    );
#endif

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Exit the %s"), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuFile->Append(
        wxID_EXIT,
        _("E&xit"),
        strMenuDescription
    );

    // Tools menu
    wxMenu *menuTools = new wxMenu;

    if (!is_acct_mgr_detected) {
        menuTools->Append(
            ID_PROJECTSATTACHPROJECT, 
            _("Attach to &project"),
            _("Attach to a project")
        );
        menuTools->Append(
            ID_PROJECTSATTACHACCOUNTMANAGER, 
            _("Attach to &account manager"),
            _("Attach to an account manager")
        );
    } else {
        strMenuName.Printf(
            _("&Synchronize with %s"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        strMenuDescription.Printf(
            _("Get current settings from %s"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        menuTools->Append(
            ID_TOOLSAMUPDATENOW, 
            strMenuName,
            strMenuDescription
        );
    }

    // Activity menu
    wxMenu *menuActivity = new wxMenu;

    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYRUNALWAYS,
        _("&Run always"),
        _("Allow work regardless of preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYRUNBASEDONPREPERENCES,
        _("Run based on &preferences"),
        _("Allow work according to your preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYSUSPEND,
        _("&Suspend"),
        _("Stop work regardless of preferences")
    );

    menuActivity->AppendSeparator();

    menuActivity->AppendRadioItem(
        ID_FILENETWORKRUNALWAYS,
        _("&Network activity always available"),
        _("Allow network activity regardless of preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILENETWORKRUNBASEDONPREPERENCES,
        _("Network activity based on &preferences"),
        _("Allow network activity according to your preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILENETWORKSUSPEND,
        _("&Network activity suspended"),
        _("Stop BOINC network activity")
    );

    // Advanced menu
    wxMenu *menuAdvanced = new wxMenu;
    menuAdvanced->Append(
        ID_OPTIONSOPTIONS, 
        _("&Options"),
        _("Configure GUI options and proxy settings")
    );
    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strMenuDescription.Printf(
        _("Connect to another computer running %s"), 
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    menuAdvanced->Append(
        ID_FILESELECTCOMPUTER, 
        _("Select computer..."),
        strMenuDescription
    );
    menuAdvanced->Append(
        ID_FILERUNBENCHMARKS, 
        _("Run CPU &benchmarks"),
        _("Runs BOINC CPU benchmarks")
    );
    menuAdvanced->Append(
        ID_COMMANDSRETRYCOMMUNICATIONS, 
        _("Retry &communications"),
        _("Report completed work, get latest credit, "
          "get latest preferences, and possibly get more work.")
    );
    if (is_acct_mgr_detected) {
        strMenuName.Printf(
            _("&Defect from %s"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        menuAdvanced->Append(
            ID_ADVANCEDAMDEFECT, 
            strMenuName,
            _("Remove client from account manager control.")
        );
        menuAdvanced->Append(
            ID_PROJECTSATTACHPROJECT, 
            _("Attach to &project"),
            _("Attach to a project to begin processing work")
        );
    }


    // Help menu
    wxMenu *menuHelp = new wxMenu;

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&%s\tF1"), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about the %s"), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINCMANAGER,
        strMenuName, 
        strMenuDescription
    );

    // %s is the project name
    //    i.e. 'BOINC', 'GridRepublic'
    strMenuName.Printf(
        _("%s &website"), 
        wxGetApp().GetBrand()->GetProjectName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about BOINC and %s"),
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINC,
        strMenuName, 
        strMenuDescription
    );

    menuHelp->AppendSeparator();

    // %s is the project name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&About %s..."), 
        wxGetApp().GetBrand()->GetApplicationName().c_str()
    );
    menuHelp->Append(
        wxID_ABOUT,
        strMenuName, 
        _("Licensing and copyright information.")
    );

    // construct menu
    m_pMenubar = new wxMenuBar;
#ifdef __WXMAC__
// WxWidgets automatically prevents the Exit item from showing in the File menu
// because Mac OSX has its Quit item in "BOINCManager" menu, not in File menu.  
// Don't show File menu on the Mac unless it has additional items.
    if (menuFile->GetMenuItemCount() > 1)
#endif
        m_pMenubar->Append(
            menuFile,
            _("&File")
        );
    m_pMenubar->Append(
        menuTools,
        _("&Tools")
    );
    m_pMenubar->Append(
        menuActivity,
        _("&Activity")
    );
    m_pMenubar->Append(
        menuAdvanced,
        _("A&dvanced")
    );
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );

    wxMenuBar* m_pOldMenubar = GetMenuBar();
    SetMenuBar(m_pMenubar);
#ifdef __WXMAC__
    m_pMenubar->MacInstallMenuBar();
#endif
    if (m_pOldMenubar) {
        delete m_pOldMenubar;
    }
    
#ifdef __WXMAC__
    MenuRef prefsMenuRef;
    MenuItemIndex prefsMenuItemIndex;

    // Hide Mac OS X's standard Preferences menu ite, since we don't use it
    if (GetIndMenuItemWithCommandID(NULL, kHICommandPreferences, 1, &prefsMenuRef, &prefsMenuItemIndex) == noErr)
        ChangeMenuItemAttributes(prefsMenuRef, prefsMenuItemIndex, kMenuItemAttrHidden, 0);
 #endif

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateMenu - Function End"));
    return true;
}


bool CAdvancedFrame::CreateNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebook - Function Begin"));

    // create frame panel
    wxPanel *pPanel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);
    pPanel->SetAutoLayout(TRUE);

    // initialize notebook
    m_pNotebook = new wxNotebook(pPanel, ID_FRAMENOTEBOOK, wxDefaultPosition, wxDefaultSize,
                                wxNB_FIXEDWIDTH|wxCLIP_CHILDREN);

    // layout frame panel
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    pPanelSizer->Add(new wxStaticLine(pPanel, -1), 0, wxEXPAND);
	pPanelSizer->Add(0, 5);
    pPanelSizer->Add(m_pNotebook, 1, wxEXPAND);


    // create the various notebook pages
    CreateNotebookPage(new CViewProjects(m_pNotebook));
    CreateNotebookPage(new CViewWork(m_pNotebook));
    CreateNotebookPage(new CViewTransfers(m_pNotebook));
    CreateNotebookPage(new CViewMessages(m_pNotebook));
	CreateNotebookPage(new CViewStatistics(m_pNotebook));
    CreateNotebookPage(new CViewResources(m_pNotebook));


    pPanel->SetSizer(pPanelSizer);
    pPanel->Layout();


    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebook - Function End"));
    return true;
}


template < class T >
bool CAdvancedFrame::CreateNotebookPage(T pwndNewNotebookPage) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebookPage - Function Begin"));

    wxImageList*    pImageList;
    int         iImageIndex = 0;

    wxASSERT(pwndNewNotebookPage);
    wxASSERT(m_pNotebook);
    wxASSERT(wxDynamicCast(pwndNewNotebookPage, CBOINCBaseView));


    pImageList = m_pNotebook->GetImageList();
    if (!pImageList) {
        pImageList = new wxImageList(16, 16, true, 0);
        wxASSERT(pImageList != NULL);
        m_pNotebook->SetImageList(pImageList);
    }
    
    iImageIndex = pImageList->Add(wxBitmap(pwndNewNotebookPage->GetViewIcon()));
    m_pNotebook->AddPage(pwndNewNotebookPage, pwndNewNotebookPage->GetViewName(), TRUE, iImageIndex);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebookPage - Function End"));
    return true;
}


bool CAdvancedFrame::CreateStatusbar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateStatusbar - Function Begin"));

    if (m_pStatusbar)
        return true;

    m_pStatusbar = new CStatusBar(this);
    wxASSERT(m_pStatusbar);

    SetStatusBar(m_pStatusbar);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateStatusbar - Function End"));
    return true;
}


bool CAdvancedFrame::DeleteMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteMenu - Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteMenu - Function End"));
    return true;
}


bool CAdvancedFrame::DeleteNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteNotebook - Function Begin"));

    wxImageList*    pImageList;

    wxASSERT(m_pNotebook);

    pImageList = m_pNotebook->GetImageList();

    wxASSERT(pImageList);

    if (pImageList)
        delete pImageList;

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteNotebook - Function End"));
    return true;
}


bool CAdvancedFrame::DeleteStatusbar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteStatusbar - Function Begin"));

    if (!m_pStatusbar)
        return true;

    SetStatusBar(NULL);

    delete m_pStatusbar;
    m_pStatusbar = NULL;

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteStatusbar - Function End"));
    return true;
}


bool CAdvancedFrame::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    int             iIndex = 0;
    int             iItemCount = 0;


    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);


    CBOINCBaseFrame::SaveState();


    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("CurrentPage"), m_pNotebook->GetSelection());

    pConfig->Write(wxT("WindowIconized"), IsIconized());
#if defined(__WXMSW__) || defined(__WXMAC__)
    pConfig->Write(wxT("WindowMaximized"), IsMaximized());
#endif

    if (!IsIconized() && !IsMaximized()) {
        GetWindowDimensions();
    }

    pConfig->Write(wxT("Width"), m_Width);
    pConfig->Write(wxT("Height"), m_Height);
#ifdef __WXMAC__
    pConfig->Write(wxT("XPos"), m_Left);
    pConfig->Write(wxT("YPos"), m_Top);
#endif


    //
    // Save Page(s) State
    //
 
    // Convert to a zero based index
    iItemCount = (int)m_pNotebook->GetPageCount() - 1;

    for (iIndex = 0; iIndex <= iItemCount; iIndex++) {   
        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnSaveState(pConfig);
        pConfig->SetPath(strPreviousLocation);
    }

    pConfig->SetPath(strPreviousLocation);


    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::SaveState - Function End"));
    return true;
}


bool CAdvancedFrame::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    wxString        strValue;
    long            iIndex;
    long            iPageCount;
    long            iCurrentPage;


    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);


    CBOINCBaseFrame::RestoreState();


    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    if (wxGetApp().GetBrand()->IsBranded() && 
        wxGetApp().GetBrand()->IsDefaultTabSpecified()) {
        m_pNotebook->SetSelection(wxGetApp().GetBrand()->GetDefaultTab());
    } else {
        pConfig->Read(wxT("CurrentPage"), &iCurrentPage, (ID_LIST_WORKVIEW - ID_LIST_BASE));
        m_pNotebook->SetSelection(iCurrentPage);
    }

    // Read window dimensions now, so SaveState can write them even if we never open the window
    pConfig->Read(wxT("Width"), &m_Width, 800);
    pConfig->Read(wxT("Height"), &m_Height, 600);

#ifdef __WXMAC__
    pConfig->Read(wxT("YPos"), &m_Top, 30);
    pConfig->Read(wxT("XPos"), &m_Left, 30);

    // If the user has changed the arrangement of multiple 
    // displays, make sure the window title bar is still on-screen.
    Rect titleRect = {m_Top, m_Left, m_Top+22, m_Left+m_Width };
    InsetRect(&titleRect, 5, 5);    // Make sure at least a 5X5 piece visible
    RgnHandle displayRgn = NewRgn();
    CopyRgn(GetGrayRgn(), displayRgn);  // Region encompassing all displays
    Rect menuRect = ((**GetMainDevice())).gdRect;
    menuRect.bottom = GetMBarHeight() + menuRect.top;
    RgnHandle menuRgn = NewRgn();
    RectRgn(menuRgn, &menuRect);                // Region hidden by menu bar
    DiffRgn(displayRgn, menuRgn, displayRgn);   // Subtract menu bar retion
    if (!RectInRgn(&titleRect, displayRgn))
        m_Top = m_Left = 30;
    DisposeRgn(menuRgn);
    DisposeRgn(displayRgn);

    SetSize(m_Left, m_Top, m_Width, m_Height);
#endif

    //
    // Restore Page(s) State
    //

    // Convert to a zero based index
    iPageCount = (long)m_pNotebook->GetPageCount() - 1;

    for (iIndex = 0; iIndex <= iPageCount; iIndex++) {   

        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnRestoreState(pConfig);
        pConfig->SetPath(strPreviousLocation);

    }

    pConfig->SetPath(strPreviousLocation);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RestoreState - Function End"));
    return true;
}


void CAdvancedFrame::OnActivitySelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnActivitySelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CTaskBarIcon*  pTaskbar  = wxGetApp().GetTaskBarIcon();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pTaskbar);
    wxASSERT(wxDynamicCast(pTaskbar, CTaskBarIcon));

    pTaskbar->ResetSnoozeState();

    switch(event.GetId()) {
    case ID_FILEACTIVITYRUNALWAYS:
        pDoc->SetActivityRunMode(RUN_MODE_ALWAYS);
        break;
    case ID_FILEACTIVITYSUSPEND:
        pDoc->SetActivityRunMode(RUN_MODE_NEVER);
        break;
    case ID_FILEACTIVITYRUNBASEDONPREPERENCES:
        pDoc->SetActivityRunMode(RUN_MODE_AUTO);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnActivitySelection - Function End"));
}


void CAdvancedFrame::OnNetworkSelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNetworkSelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();
    CTaskBarIcon*  pTaskbar  = wxGetApp().GetTaskBarIcon();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pTaskbar);
    wxASSERT(wxDynamicCast(pTaskbar, CTaskBarIcon));

    pTaskbar->ResetSnoozeState();

    switch(event.GetId()) {
    case ID_FILENETWORKRUNALWAYS:
        pDoc->SetNetworkRunMode(RUN_MODE_ALWAYS);
        break;
    case ID_FILENETWORKSUSPEND:
        pDoc->SetNetworkRunMode(RUN_MODE_NEVER);
        break;
    case ID_FILENETWORKRUNBASEDONPREPERENCES:
        pDoc->SetNetworkRunMode(RUN_MODE_AUTO);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNetworkSelection - Function End"));
}

   
void CAdvancedFrame::OnRunBenchmarks(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnRunBenchmarks - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
    pDoc->RunBenchmarks();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnRunBenchmarks - Function End"));
}


void CAdvancedFrame::OnSelectComputer(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSelectComputer - Function Begin"));

    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CDlgSelectComputer* pDlg = new CDlgSelectComputer(this);
    long                lAnswer = 0;
    size_t              lIndex = 0;
    long                lRetVal = -1;
    wxArrayString       aComputerNames;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pDlg);


    // Lets copy the template store in the system state
    aComputerNames = m_aSelectedComputerMRU;

    // Lets populate the combo control with the MRU list
    pDlg->m_ComputerNameCtrl->Clear();
    for (lIndex = 0; lIndex < aComputerNames.Count(); lIndex++) {
        pDlg->m_ComputerNameCtrl->Append(aComputerNames.Item(lIndex));
    }

    lAnswer = pDlg->ShowModal();
    if (wxID_OK == lAnswer) {

        // Make a null hostname be the same thing as localhost
        if (wxEmptyString == pDlg->m_ComputerNameCtrl->GetValue()) {
            lRetVal = pDoc->Connect(
                wxT("localhost"),
                wxEmptyString,
                TRUE,
                TRUE
            );
        } else {
            // Connect up to the remote machine
            lRetVal = pDoc->Connect(
                pDlg->m_ComputerNameCtrl->GetValue(), 
                pDlg->m_ComputerPasswordCtrl->GetValue(),
                TRUE,
                FALSE
            );
        }
        if (lRetVal) {
            ShowConnectionFailedAlert();
        }

        // Insert a copy of the current combo box value to the head of the
        //   computer names string array
        if (wxEmptyString != pDlg->m_ComputerNameCtrl->GetValue()) {
            aComputerNames.Insert(pDlg->m_ComputerNameCtrl->GetValue(), 0);
        }

        // Loops through the computer names and remove any duplicates that
        //   might exist with the new head value
        for (lIndex = 1; lIndex < aComputerNames.Count(); lIndex++) {
            if (aComputerNames.Item(lIndex) == aComputerNames.Item(0))
                aComputerNames.RemoveAt(lIndex);
        }

        // Store the modified computer name MRU list back to the system state
        m_aSelectedComputerMRU = aComputerNames;
    }

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSelectComputer - Function End"));
}


void CAdvancedFrame::OnSwitchGUI(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSwitchGUI - Function Begin"));

#ifdef SIMPLEGUI
    wxGetApp().SetActiveGUI(BOINC_SIMPLEGUI, true);
#endif

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSwitchGUI - Function End"));
}


void CAdvancedFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnExit - Function Begin"));

    if (m_iDisplayExitWarning &&
        wxGetApp().GetBrand()->IsBranded() && 
        !wxGetApp().GetBrand()->GetExitMessage().IsEmpty()) {

        CDlgGenericMessage* pDlg = new CDlgGenericMessage(this);
        long                lAnswer = 0;

        wxString strMessage;
        if (wxGetApp().GetBrand()->IsBranded() && 
            !wxGetApp().GetBrand()->GetExitMessage().IsEmpty()) {
            strMessage = wxGetApp().GetBrand()->GetExitMessage();
        } else {
            strMessage = 
                _("This will shut down your tasks until it restarts automatically\n"
                  "following your user preferences. Close window to close the manager\n"
                  "without stopping the tasks.");
        }

        pDlg->SetTitle(_("Close Confirmation"));
        pDlg->m_DialogMessage->SetLabel(strMessage);
        pDlg->Fit();
        pDlg->Centre();

        lAnswer = pDlg->ShowModal();
        if (wxID_OK == lAnswer) {
            if (pDlg->m_DialogDisableMessage->GetValue()) {
                m_iDisplayExitWarning = 0;
            }
            Close(true);
        }

        if (pDlg)
            pDlg->Destroy();

    } else {
        Close(true);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnExit - Function End"));
}


void CAdvancedFrame::OnProjectsAttachToAccountManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToAccountManager - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_ATTACH);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToAccountManager - Function End"));
}


void CAdvancedFrame::OnAccountManagerUpdate(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerUpdate - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_UPDATE);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();
        ResetReminderTimers();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerUpdate - Function End"));
}


void CAdvancedFrame::OnAccountManagerDetach(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerDetach - Function Begin"));

    CMainDocument* pDoc           = wxGetApp().GetDocument();
    wxInt32        iAnswer        = 0; 
    wxString       strTitle       = wxEmptyString;
    wxString       strMessage     = wxEmptyString;
    ACCT_MGR_INFO  ami;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {

        pDoc->rpc.acct_mgr_info(ami);

        strTitle.Printf(
            _("BOINC Manager - Detach from %s"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        strMessage.Printf(
            _("If you defect from %s,\n"
              "you'll keep all your current projects,\n"
              "but you'll have to manage projects manually.\n"
              "\n"
              "Do you want to defect from %s?"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str(),
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );

        iAnswer = ::wxMessageBox(
            strMessage,
            strTitle,
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            std::string url, name, passwd;
            pDoc->rpc.acct_mgr_rpc(
                url.c_str(),
                name.c_str(),
                passwd.c_str(),
                false
            );
        }

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerDetach - Function End"));
}


void CAdvancedFrame::OnProjectsAttachToProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        UpdateStatusText(_("Attaching to project..."));

        m_pRefreshStateTimer->Stop();
        m_pFrameRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Stop();
        m_pDocumentPollTimer->Stop();

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        pWizard->Run( strName, strURL, false );

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();

        m_pRefreshStateTimer->Start();
        m_pFrameRenderTimer->Start();
        m_pFrameListPanelRenderTimer->Start();
        m_pDocumentPollTimer->Start();

        UpdateStatusText(wxT(""));

        FireRefreshView();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function End"));
}


void CAdvancedFrame::OnCommandsRetryCommunications( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnCommandsRetryCommunications - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    UpdateStatusText(_("Retrying communications for project(s)..."));
    pDoc->rpc.network_available();
    UpdateStatusText(wxT(""));

    FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnCommandsRetryCommunications - Function End"));
}


void CAdvancedFrame::OnOptionsOptions(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnOptionsOptions - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CDlgOptions*   pDlg = new CDlgOptions(this);
    int            iAnswer = 0;
    int            iBuffer = 0;
    wxString       strBuffer = wxEmptyString;
    wxArrayString  astrDialupConnections;
    bool           bRetrievedProxyConfiguration = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pDlg);


    // General Tab
    pDlg->m_LanguageSelectionCtrl->Append(wxGetApp().GetSupportedLanguages());

    pDlg->m_LanguageSelectionCtrl->SetSelection(m_iSelectedLanguage);
    pDlg->m_ReminderFrequencyCtrl->SetValue(m_iReminderFrequency);

#ifdef __WXMSW__
    // Connection Tab
    if (m_pDialupManager) {
        m_pDialupManager->GetISPNames(astrDialupConnections);

        pDlg->m_DialupConnectionsCtrl->Append(astrDialupConnections);
        pDlg->SetDefaultDialupConnection(m_strNetworkDialupConnectionName);
    } else {
        pDlg->m_DialupSetDefaultCtrl->Disable();
        pDlg->m_DialupClearDefaultCtrl->Disable();
    }
#endif

    // Proxy Tabs
    bRetrievedProxyConfiguration = (0 == pDoc->GetProxyConfiguration());
    if(!bRetrievedProxyConfiguration) {
        // We were unable to get the proxy configuration, so disable
        //   the controls
        pDlg->m_EnableHTTPProxyCtrl->Enable(false);
        pDlg->m_EnableSOCKSProxyCtrl->Enable(false);
    } else {
        pDlg->m_EnableHTTPProxyCtrl->Enable(true);
        pDlg->m_EnableSOCKSProxyCtrl->Enable(true);
    }

    pDlg->m_EnableHTTPProxyCtrl->SetValue(pDoc->proxy_info.use_http_proxy);
    pDlg->m_HTTPAddressCtrl->SetValue(wxString(pDoc->proxy_info.http_server_name.c_str(), wxConvUTF8));
    pDlg->m_HTTPUsernameCtrl->SetValue(wxString(pDoc->proxy_info.http_user_name.c_str(), wxConvUTF8));
    pDlg->m_HTTPPasswordCtrl->SetValue(wxString(pDoc->proxy_info.http_user_passwd.c_str(), wxConvUTF8));

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
    pDlg->m_HTTPPortCtrl->SetValue(strBuffer);

    pDlg->m_EnableSOCKSProxyCtrl->SetValue(pDoc->proxy_info.use_socks_proxy);
    pDlg->m_SOCKSAddressCtrl->SetValue(wxString(pDoc->proxy_info.socks_server_name.c_str(), wxConvUTF8));
    pDlg->m_SOCKSUsernameCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_name.c_str(), wxConvUTF8));
    pDlg->m_SOCKSPasswordCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_passwd.c_str(), wxConvUTF8));

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
    pDlg->m_SOCKSPortCtrl->SetValue(strBuffer);

    iAnswer = pDlg->ShowModal();
    if (wxID_OK == iAnswer) {
        // General Tab
        if (m_iSelectedLanguage != pDlg->m_LanguageSelectionCtrl->GetSelection()) {
            wxString strDialogTitle;
            wxString strDialogMessage;

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogTitle.Printf(
                _("%s - Language Selection"),
                wxGetApp().GetBrand()->GetApplicationName().c_str()
            );

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogMessage.Printf(
                _("The %s's default language has been changed, in order for this "
                  "change to take affect you must restart the %s."),
                wxGetApp().GetBrand()->GetApplicationName().c_str(),
                wxGetApp().GetBrand()->GetApplicationName().c_str()
            );

            ShowAlert(
                strDialogTitle,
                strDialogMessage,
                wxOK | wxICON_INFORMATION
           );
        }

        m_iSelectedLanguage = pDlg->m_LanguageSelectionCtrl->GetSelection();
        m_iReminderFrequency = pDlg->m_ReminderFrequencyCtrl->GetValue();

#ifdef __WXMSW__
        // Connection Tab
        m_strNetworkDialupConnectionName = pDlg->GetDefaultDialupConnection();
#endif

        // Proxy Tabs
        if (bRetrievedProxyConfiguration) {
            pDoc->proxy_info.use_http_proxy = pDlg->m_EnableHTTPProxyCtrl->GetValue();
            pDoc->proxy_info.http_server_name = (const char*)pDlg->m_HTTPAddressCtrl->GetValue().mb_str();
            pDoc->proxy_info.http_user_name = (const char*)pDlg->m_HTTPUsernameCtrl->GetValue().mb_str();
            pDoc->proxy_info.http_user_passwd = (const char*)pDlg->m_HTTPPasswordCtrl->GetValue().mb_str();

            strBuffer = pDlg->m_HTTPPortCtrl->GetValue();
            strBuffer.ToLong((long*)&iBuffer);
            pDoc->proxy_info.http_server_port = iBuffer;

            pDoc->proxy_info.use_socks_proxy = pDlg->m_EnableSOCKSProxyCtrl->GetValue();
            pDoc->proxy_info.socks_server_name = (const char*)pDlg->m_SOCKSAddressCtrl->GetValue().mb_str();
            pDoc->proxy_info.socks5_user_name = (const char*)pDlg->m_SOCKSUsernameCtrl->GetValue().mb_str();
            pDoc->proxy_info.socks5_user_passwd = (const char*)pDlg->m_SOCKSPasswordCtrl->GetValue().mb_str();

            strBuffer = pDlg->m_SOCKSPortCtrl->GetValue();
            strBuffer.ToLong((long*)&iBuffer);
            pDoc->proxy_info.socks_server_port = iBuffer;

            pDoc->SetProxyConfiguration();
        }
    }

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnOptionsOptions - Function End"));
}


void CAdvancedFrame::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
        if (ID_ADVANCEDFRAME == event.GetId()) {
            wxString url = wxGetApp().GetBrand()->GetCompanyWebsite().c_str();
            url += wxT("manager.php");
            ExecuteBrowserLink(url);
        } else {
            event.Skip();
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCManager - Function End"));
}


void CAdvancedFrame::OnHelpBOINCManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
        wxString url = wxGetApp().GetBrand()->GetCompanyWebsite().c_str();
        url += wxT("manager.php");
        ExecuteBrowserLink(url);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCManager - Function End"));
}


void CAdvancedFrame::OnHelpBOINCWebsite(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCWebsite - Function Begin"));

    if (IsShown()) {
        wxString url = wxGetApp().GetBrand()->GetCompanyWebsite().c_str();
        ExecuteBrowserLink(url);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCWebsite - Function End"));
}


void CAdvancedFrame::OnHelpAbout(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpAbout - Function Begin"));

    CDlgAbout* pDlg = new CDlgAbout(this);
    wxASSERT(pDlg);

    pDlg->ShowModal();

    if (pDlg)
        pDlg->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpAbout - Function End"));
}


void CAdvancedFrame::OnShow(wxShowEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnShow - Function Begin"));

    if (event.GetShow())
        SetWindowDimensions();
    else
        GetWindowDimensions();
    
    event.Skip();
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnShow - Function End"));
}


void CAdvancedFrame::GetWindowDimensions() {
    if (!IsIconized() && !IsMaximized()) {
        m_Top = GetPosition().y;
        m_Left = GetPosition().x;
        m_Width = GetSize().GetWidth();
        m_Height = GetSize().GetHeight();
    }
}
    

void CAdvancedFrame::SetWindowDimensions() {
#ifndef __WXMAC__
    static bool bFirstTime = true;

    if (bFirstTime) {
        bFirstTime = false;

        wxString        strBaseConfigLocation = wxString(wxT("/"));
        wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
        bool            bWindowIconized = false;
#ifdef __WXMSW__ 
        bool            bWindowMaximized = false;
#endif

        wxASSERT(pConfig);

        //
        // Restore Frame State
        //

        pConfig->SetPath(strBaseConfigLocation);

        pConfig->Read(wxT("WindowIconized"), &bWindowIconized, false);
        pConfig->Read(wxT("Width"), &m_Width, 800);
        pConfig->Read(wxT("Height"), &m_Height, 600);
        SetSize(-1, -1, m_Width, m_Height);

#ifdef __WXMSW__ 
        pConfig->Read(wxT("WindowMaximized"), &bWindowMaximized, false);
        Maximize(bWindowMaximized);
#endif
    }
#endif  // ! __WXMAC__
}


void CAdvancedFrame::OnRefreshView(CFrameEvent& WXUNUSED(event)) {
    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        if (IsShown()) {
            wxWindow*       pwndNotebookPage = NULL;
            CBOINCBaseView* pView = NULL;
            wxTimerEvent    timerEvent;

            wxASSERT(m_pNotebook);

            pwndNotebookPage = m_pNotebook->GetPage(m_pNotebook->GetSelection());
            wxASSERT(pwndNotebookPage);

            pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
            wxASSERT(pView);

            pView->FireOnListRender(timerEvent);
        }

        bAlreadyRunningLoop = false;
    }
}


void CAdvancedFrame::OnConnect(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnConnect - Function Begin"));
    
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CWizardAccountManager* pAMWizard = NULL;
    CWizardAttachProject* pAPWizard = NULL;
    wxString strComputer = wxEmptyString;
    wxString strName = wxEmptyString;
    wxString strURL = wxEmptyString;
    bool bCachedCredentials = false;
    ACCT_MGR_INFO ami;
    PROJECT_INIT_STATUS pis;

    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Update the menus
    DeleteMenu();
    CreateMenu();

    // Only present one of the wizards if no projects are currently
    //   detected.
    m_pRefreshStateTimer->Stop();
    m_pFrameRenderTimer->Stop();
    m_pFrameListPanelRenderTimer->Stop();


    // If we are connected to the localhost, run a really quick screensaver
    //   test to trigger a firewall popup.
    pDoc->GetConnectedComputerName(strComputer);
    if (pDoc->IsComputerNameLocal(strComputer)) {
        wxGetApp().StartBOINCScreensaverTest();
    }


    pDoc->rpc.acct_mgr_info(ami);
    if (ami.acct_mgr_url.size() && !ami.have_credentials) {
        pAMWizard = new CWizardAccountManager(this);

        if (!IsShown()) {
            Show();
        }

        if (pAMWizard->Run()) {
            // If successful, hide the main window
            Hide();
        } else {
            // If failure, display the messages tab
            m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
        }
    } else if (0 >= pDoc->GetProjectCount()) {
        pAPWizard = new CWizardAttachProject(this);

        if (!IsShown()) {
            Show();
        }

        pDoc->rpc.get_project_init_status(pis);
        strName = wxString(pis.name.c_str(), wxConvUTF8);
        strURL = wxString(pis.url.c_str(), wxConvUTF8);
        bCachedCredentials = pis.url.length() && pis.has_account_key;

        if (pAPWizard->Run(strName, strURL, bCachedCredentials)) {
            // If successful, display the work tab
            m_pNotebook->SetSelection(ID_LIST_WORKVIEW - ID_LIST_BASE);
        } else {
            // If failure, display the messages tab
            m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
        }
    }

    m_pRefreshStateTimer->Start();
    m_pFrameRenderTimer->Start();
    m_pFrameListPanelRenderTimer->Start();

    if (pAMWizard)
        pAMWizard->Destroy();
    if (pAPWizard)
        pAPWizard->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnConnect - Function End"));
}


void CAdvancedFrame::OnUpdateStatus(CFrameEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnUpdateStatus - Function Begin"));

    m_pStatusbar->SetStatusText(event.m_message);
    ::wxSleep(0);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnUpdateStatus - Function End"));
}


void CAdvancedFrame::OnRefreshState(wxTimerEvent &event) {
    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        // Write a snapshot of the current state to the config
        //   module, on Win9x systems we don't always shutdown
        //   in a nice way, if we are terminated by the user
        //   we still want the UI state to have been stored
        //   for their next use
        SaveState();

        bAlreadyRunningLoop = false;
    }

    event.Skip();
}


void CAdvancedFrame::OnFrameRender(wxTimerEvent &event) {
    static bool       bAlreadyRunningLoop = false;
    static wxString   strCachedStatusText = wxEmptyString;

    CMainDocument*    pDoc = wxGetApp().GetDocument();

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        if (IsShown()) {
            if (pDoc) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));

                // Update the menu bar
                wxMenuBar* pMenuBar      = GetMenuBar();
                int        iActivityMode = -1;
                int        iNetworkMode  = -1;

                wxASSERT(pMenuBar);
                wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

                pMenuBar->Check(ID_FILEACTIVITYRUNALWAYS, false);
                pMenuBar->Check(ID_FILEACTIVITYSUSPEND, false);
                pMenuBar->Check(ID_FILEACTIVITYRUNBASEDONPREPERENCES, false);
                if ((pDoc->IsConnected()) && (0 == pDoc->GetActivityRunMode(iActivityMode))) {
                    if (iActivityMode == RUN_MODE_ALWAYS)
                        pMenuBar->Check(ID_FILEACTIVITYRUNALWAYS, true);
                    if (iActivityMode == RUN_MODE_NEVER)
                        pMenuBar->Check(ID_FILEACTIVITYSUSPEND, true);
                    if (iActivityMode == RUN_MODE_AUTO)
                        pMenuBar->Check(ID_FILEACTIVITYRUNBASEDONPREPERENCES, true);
                }

                pMenuBar->Check(ID_FILENETWORKRUNALWAYS, false);
                pMenuBar->Check(ID_FILENETWORKSUSPEND, false);
                pMenuBar->Check(ID_FILENETWORKRUNBASEDONPREPERENCES, false);
                if ((pDoc->IsConnected()) && (0 == pDoc->GetNetworkRunMode(iNetworkMode))) {
                    if (RUN_MODE_ALWAYS == iNetworkMode)
                        pMenuBar->Check(ID_FILENETWORKRUNALWAYS, true);
                    if (RUN_MODE_NEVER == iNetworkMode)
                        pMenuBar->Check(ID_FILENETWORKSUSPEND, true);
                    if (RUN_MODE_AUTO == iNetworkMode)
                        pMenuBar->Check(ID_FILENETWORKRUNBASEDONPREPERENCES, true);
                }

                // Update the statusbar
                wxASSERT(wxDynamicCast(m_pStatusbar, CStatusBar));
                if (pDoc->IsConnected() || pDoc->IsReconnecting()) {
                    m_pStatusbar->m_pbmpConnected->Show();
                    m_pStatusbar->m_ptxtConnected->Show();
                    m_pStatusbar->m_pbmpDisconnect->Hide();
                    m_pStatusbar->m_ptxtDisconnect->Hide();

                    wxString strBuffer = wxEmptyString;
                    wxString strComputerName = wxEmptyString;
                    wxString strStatusText = wxEmptyString;
                    wxString strTitle = m_strBaseTitle;
                    wxString strLocale = wxString(setlocale(LC_NUMERIC, NULL), wxConvUTF8);
     
                    if (pDoc->IsReconnecting())
                        pDoc->GetConnectingComputerName(strComputerName);
                    else
                        pDoc->GetConnectedComputerName(strComputerName);

                    if (pDoc->IsComputerNameLocal(strComputerName)) {
                        strTitle += wxT(" - (localhost)");
                    } else {
                        strStatusText += strComputerName;
                    }

                    if (pDoc->IsReconnecting()) {
                        strTitle.Printf(_("%s - (%s)"), m_strBaseTitle.c_str(), strComputerName.c_str());
                        strStatusText.Printf(_("Connecting to %s"), strComputerName.c_str());
                    } else {
                        strTitle.Printf(_("%s - (%s)"), m_strBaseTitle.c_str(), strComputerName.c_str());
                        strStatusText.Printf(_("Connected to %s"), strComputerName.c_str());
                    }

                    // The Mac takes a huge performance hit redrawing this window, 
                    //   window, so don't change the text unless we really have too.
                    if (GetTitle() != strTitle)
                        SetTitle(strTitle);
                        
                    if (strStatusText != strCachedStatusText) {
                        strCachedStatusText = strStatusText;
                        m_pStatusbar->m_ptxtConnected->SetLabel(strStatusText);
                    }
                } else {
                    m_pStatusbar->m_pbmpConnected->Hide();
                    m_pStatusbar->m_ptxtConnected->Hide();
                    m_pStatusbar->m_pbmpDisconnect->Show();
                    m_pStatusbar->m_ptxtDisconnect->Show();

                    if (GetTitle() != m_strBaseTitle)
                        SetTitle(m_strBaseTitle);
                }
            }
        }

        bAlreadyRunningLoop = false;
    }

    event.Skip();
}


void CAdvancedFrame::OnListPanelRender(wxTimerEvent&) {
    FireRefreshView();
    SetFrameListPanelRenderTimerRate();   // Set to refresh every 5 or 60 seconds
}


void CAdvancedFrame::OnNotebookSelectionChanged(wxNotebookEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNotebookSelectionChanged - Function Begin"));

    if ((-1 != event.GetSelection()) && IsShown()) {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;
        wxTimerEvent    timerEvent;

        wxASSERT(m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage(event.GetSelection());
        wxASSERT(pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        FireRefreshView();

        SetFrameListPanelRenderTimerRate();
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNotebookSelectionChanged - Function End"));
}


void CAdvancedFrame::ResetReminderTimers() {
#ifdef __WXMSW__
    wxASSERT(m_pDialupManager);
    wxASSERT(wxDynamicCast(m_pDialupManager, CBOINCDialUpManager));

    m_pDialupManager->ResetReminderTimers();
#endif
}


void CAdvancedFrame::SetFrameListPanelRenderTimerRate() {
    static wxWindowID   previousPane = -1;
    static int          connectedCount = 0;
    wxWindowID          currentPane;

    CMainDocument*      pDoc = wxGetApp().GetDocument();
   
    wxASSERT(m_pNotebook);
    wxASSERT(m_pFrameListPanelRenderTimer);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Keep timer at faster rate until we have been connected > 10 seconds
    if (!pDoc->IsConnected())
        connectedCount = 0;
        
    if (connectedCount < 3) {
        connectedCount++;
        previousPane = -1;  // Ensure an update when connectedCount reaches 3
        
        if (m_pFrameListPanelRenderTimer->IsRunning())
            m_pFrameListPanelRenderTimer->Stop();
        m_pFrameListPanelRenderTimer->Start(1000);  // Refresh every 1 seconds
        return;
    }
    
    currentPane = m_pNotebook->GetSelection() + ID_TASK_BASE;
    if (currentPane == previousPane) 
        return;
        
    previousPane = currentPane;
    if (m_pFrameListPanelRenderTimer->IsRunning())
        m_pFrameListPanelRenderTimer->Stop();

    switch (currentPane) {
    case ID_TASK_STATISTICSVIEW: 
        m_pFrameListPanelRenderTimer->Start(60000); // Refresh every 1 minute
        break;
    default:
        m_pFrameListPanelRenderTimer->Start(1000);  // Refresh every 1 seconds
        break;
    }
}


#ifdef __WXMAC__

bool CAdvancedFrame::Show(bool show) {
    ProcessSerialNumber psn;

    GetCurrentProcess(&psn);
    if (show) {
        SetFrontProcess(&psn);  // Shows process if hidden
    } else {
        GetWindowDimensions();
        if (IsProcessVisible(&psn))
            ShowHideProcess(&psn, false);
    }
    
    return wxFrame::Show(show);
}

#endif // __WXMAC__


const char *BOINC_RCSID_d881a56dc5 = "$Id$";
