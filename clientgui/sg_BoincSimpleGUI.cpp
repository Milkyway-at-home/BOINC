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
#pragma implementation "sg_BoincSimpleGUI.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "str_util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "common/wxFlatNotebook.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "Events.h"
#include "BOINCBaseFrame.h"
#include "wizardex.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "error_numbers.h"

#include "sg_BoincSimpleGUI.h"
#include "sg_ImageLoader.h"
#include "sg_ProjectsComponent.h"
#include "sg_ClientStateIndicator.h"
#include "sg_StatImageLoader.h"
#include "sg_ViewTabPage.h"


IMPLEMENT_DYNAMIC_CLASS(CSimpleFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE(CSimpleFrame, CBOINCBaseFrame)
    EVT_SIZE(CSimpleFrame::OnSize)
    EVT_MENU(wxID_EXIT, CSimpleFrame::OnExit)
    EVT_FRAME_CONNECT(CSimpleFrame::OnConnect)
    EVT_HELP(wxID_ANY, CSimpleFrame::OnHelp)
    EVT_FRAME_RELOADSKIN(CSimpleFrame::OnReloadSkin)
    // We can't eliminate the Mac Help menu, so we might as well make it useful.
    EVT_MENU(ID_HELPBOINCMANAGER, CSimpleFrame::OnHelpBOINCManager)
    EVT_MENU(ID_HELPBOINC, CSimpleFrame::OnHelpBOINCWebsite)
END_EVENT_TABLE()

CSimpleFrame::CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Default Constructor Function End"));
}


CSimpleFrame::CSimpleFrame(wxString title, wxIcon* icon) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_SIMPLEFRAME, title, wxDefaultPosition, 
#ifdef __WXMAC__
                    wxSize(409, 561),
#else
                    wxSize(416, 570),
#endif
                    wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Overloaded Constructor Function Begin"));

    RestoreState();

    // Initialize Application
    SetIcon(*icon);
    
#ifdef __WXMAC__
    // We can't eliminate the Mac menu bar or the Help menu, so we 
    // might as well make them useful.
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString           strMenuName;
    wxString           strMenuDescription;

    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    // File menu
    wxMenu *menuFile = new wxMenu;

    menuFile->Append(
        ID_FILECLOSEWINDOW,
        _("&Close Window\tCTRL+W"),
		_("Close BOINC Manager Window.")
    );

    // Help menu
    wxMenu *menuHelp = new wxMenu;

    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuName.Printf(
        _("&%s"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about the %s"), 
        pSkinAdvanced->GetApplicationName().c_str()
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
        pSkinAdvanced->GetProjectName().c_str()
    );
    // %s is the application name
    //    i.e. 'BOINC Manager', 'GridRepublic Manager'
    strMenuDescription.Printf(
        _("Show information about BOINC and %s"),
        pSkinAdvanced->GetApplicationName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINC,
        strMenuName, 
        strMenuDescription
    );

    // Clear menubar
    m_pMenubar = new wxMenuBar;
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );
    
    SetMenuBar(m_pMenubar);
    m_pMenubar->MacInstallMenuBar();
    ::ClearMenuBar();       // Remove everything we can from the Mac menu bar
    
    m_pMenubar->Insert(     // Must be done after ClearMenuBar()
        0,
        menuFile,
        _("&File")
    );
    
    // wxMac maps Command key to wxACCEL_ALT for wxAcceleratorTable but CTRL for wxMenu.
    m_Shortcuts[0].Set(wxACCEL_NORMAL, WXK_HELP, ID_HELPBOINCMANAGER);
#else
    m_Shortcuts[0].Set(wxACCEL_CTRL, (int)'W', ID_FILECLOSEWINDOW);
#endif

    m_pAccelTable = new wxAcceleratorTable(1, m_Shortcuts);
    SetAcceleratorTable(*m_pAccelTable);

    m_pBackgroundPanel = new CSimplePanel(this);
}


CSimpleFrame::~CSimpleFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function Begin"));

	SaveState();

    if (m_pAccelTable)
        delete m_pAccelTable;

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::CSimpleFrame - Destructor Function End"));
}


bool CSimpleFrame::RestoreState() {
    CBOINCBaseFrame::RestoreState();
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

	// Read the last coordinates of the BSG
	int x = pConfig->Read(wxT("X_Position"), ((wxPoint) wxDefaultPosition).x);
	int y = pConfig->Read(wxT("Y_Position"), ((wxPoint) wxDefaultPosition).y);
	
	// Read the size of the BSG
	int width, height;
	GetSize(&width, &height);

#ifdef __WXMAC__

        // If the user has changed the arrangement of multiple 
        // displays, make sure the window title bar is still on-screen.
        Rect titleRect = {y, x, y+22, x+width };
        InsetRect(&titleRect, 5, 5);    // Make sure at least a 5X5 piece visible
        RgnHandle displayRgn = NewRgn();
        CopyRgn(GetGrayRgn(), displayRgn);  // Region encompassing all displays
        Rect menuRect = ((**GetMainDevice())).gdRect;
        menuRect.bottom = GetMBarHeight() + menuRect.top;
        RgnHandle menuRgn = NewRgn();
        RectRgn(menuRgn, &menuRect);                // Region hidden by menu bar
        DiffRgn(displayRgn, menuRgn, displayRgn);   // Subtract menu bar retion
        if (!RectInRgn(&titleRect, displayRgn))
            x = y = 30;
        DisposeRgn(menuRgn);
        DisposeRgn(displayRgn);
    
#else

	// If either co-ordinate is less then 0 then set it equal to 0 to ensure
	// it displays on the screen.
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;

	// Read the size of the screen
	int maxX = wxSystemSettings::GetMetric( wxSYS_SCREEN_X );
	int maxY = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );

	// Max sure that it doesn't go off to the right or bottom
	if ( x + width > maxX ) x=maxX-width;
	if ( y + height > maxY ) y=maxY-height;
#endif

	Move(x,y);

	return true;
}


bool CSimpleFrame::SaveState() {
	CBOINCBaseFrame::SaveState();
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
	wxString        strBaseConfigLocation = wxString(wxT("/"));

    wxASSERT(pConfig);

    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

	int x,y;
	GetPosition(&x, &y);
	pConfig->Write(wxT("X_Position"), x);
	pConfig->Write(wxT("Y_Position"), y);
	return true;
}


void CSimpleFrame::OnHelp(wxHelpEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelp - Function Begin"));

    if (IsShown()) {
		std::string url;
		url = wxGetApp().GetSkinManager()->GetAdvanced()->GetCompanyWebsite().mb_str();
		canonicalize_master_url(url);

		wxString wxurl;
		wxurl.Printf(wxT("%smanager_links.php?target=simple&controlid=%d"), url.c_str(), event.GetId());
        ExecuteBrowserLink(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnHelp - Function End"));
}


void CSimpleFrame::OnHelpBOINCManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCManager - Function Begin"));

    if (IsShown()) {
		std::string url;
		url = wxGetApp().GetSkinManager()->GetAdvanced()->GetCompanyWebsite().mb_str();
		canonicalize_master_url(url);

		wxString wxurl;
		wxurl.Printf(wxT("%smanager_links.php?target=simple"),url.c_str());
		ExecuteBrowserLink(wxurl);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCManager - Function End"));
}


void CSimpleFrame::OnHelpBOINCWebsite(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCWebsite - Function Begin"));

    if (IsShown()) {
        wxString url = wxGetApp().GetSkinManager()->GetAdvanced()->GetCompanyWebsite().c_str();
        ExecuteBrowserLink(url);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpBOINCWebsite - Function End"));
}


void CSimpleFrame::OnReloadSkin(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnReloadSkin - Function Start"));
    
    m_pBackgroundPanel->ReskinInterface();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnReloadSkin - Function End"));
}


void CSimpleFrame::OnProjectsAttachToProject() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        
        m_pBackgroundPanel->m_pFrameRenderTimer->Stop();

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        pWizard->Run( strName, strURL, false );

        if (pWizard)
            pWizard->Destroy();

        m_pBackgroundPanel->m_pFrameRenderTimer->Start();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function End"));
}


void CSimpleFrame::OnConnect(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function Begin"));
    
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CWizardAccountManager* pAMWizard = NULL;
    CWizardAttachProject* pAPWizard = NULL;
    wxString strComputer = wxEmptyString;
    wxString strName = wxEmptyString;
    wxString strURL = wxEmptyString;
    bool bCachedCredentials = false;
    ACCT_MGR_INFO ami;
    PROJECT_INIT_STATUS pis;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

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

        pAPWizard->Run(strName, strURL, bCachedCredentials);
    }

    if (pAMWizard)
        pAMWizard->Destroy();
	if (pAPWizard){
            pAPWizard->Destroy();
            //update Project Component
            m_pBackgroundPanel->UpdateProjectView();
	}

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnConnect - Function End"));
}


IMPLEMENT_DYNAMIC_CLASS(CSimplePanel, wxPanel)

BEGIN_EVENT_TABLE(CSimplePanel, wxPanel)
    EVT_SIZE(CSimplePanel::OnSize)
    EVT_ERASE_BACKGROUND(CSimplePanel::OnEraseBackground)
    EVT_TIMER(ID_SIMPLEFRAMERENDERTIMER, CSimplePanel::OnFrameRender)
END_EVENT_TABLE()


CSimplePanel::CSimplePanel() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanels::CSimplePanel - Default Constructor Function End"));
}


CSimplePanel::CSimplePanel(wxWindow* parent) : 
    wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Overloaded Constructor Function Begin"));

    wrkUnitNB = NULL;
    clientState = NULL;
    projComponent = NULL;

	projectViewInitialized = false;
	emptyViewInitialized = false;
	notebookViewInitialized = false;
	dlgOpen = false;

	//set polling timer for interface
	m_pFrameRenderTimer = new wxTimer(this, ID_SIMPLEFRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second

	InitEmptyView();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Overloaded Constructor Function End"));
}


CSimplePanel::~CSimplePanel()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Destructor Function Begin"));
    wxASSERT(m_pFrameRenderTimer);

	if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::CSimplePanel - Destructor Function End"));
}


void CSimplePanel::ReskinInterface() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::ReskinInterface - Function Start"));

    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    Freeze();
    //bg color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

    if(notebookViewInitialized){
		if (wrkUnitNB) wrkUnitNB->ReskinAppGUI();
	} else {
		if (clientState) clientState->ReskinInterface();
	}

    //reskin component 
	if (projComponent) projComponent->ReskinInterface();

    Thaw();
    Refresh();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::ReskinInterface - Function End"));
}


void CSimplePanel::OnProjectsAttachToProject() {
    wxLogTrace(wxT("Function Start/End"), wxT("CSimplePanel::OnProjectsAttachToProject - Function Begin"));
	
    CSimpleFrame* pFrame = wxDynamicCast(GetParent(), CSimpleFrame);
    wxASSERT(pFrame);

    pFrame->OnProjectsAttachToProject();

    wxLogTrace(wxT("Function Start/End"), wxT("CSimpleFrame::OnProjectsAttachToProject - Function End"));
}


void CSimplePanel::OnFrameRender(wxTimerEvent& WXUNUSED(event)) {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    if (IsShown()) {

	    if (!projectViewInitialized) {
		    InitProjectView();
		    return;
	    } else if ( pDoc->IsConnected() ) {
		    UpdateProjectView();
	    }

	    // Now check to see if we show the empty state or results
	    if ( pDoc->GetSimpleGUIWorkCount() > 0 ) {
		    // State changes can cause the BSG to crash if a dialogue is open.
		    // Defer state change until after the dialogue is closed
		    if ( (emptyViewInitialized || !notebookViewInitialized) && dlgOpen ) {
			    return;
		    }

		    // If empty was displayed, remove
		    if ( emptyViewInitialized ) {
			    DestroyEmptyView();
		    }
		    // If we hadn't previously shown the notebook, create it.
		    if ( !notebookViewInitialized ) {
			    InitNotebook();
		    }
		    wrkUnitNB->Update();
	    } else {
		    // State changes can cause the BSG to crash if a dialogue is open.
		    // Defer state change until after the dialogue is closed
		    if ( (!emptyViewInitialized || notebookViewInitialized) && dlgOpen ) {
			    return;
		    }

		    if ( notebookViewInitialized ) {
			    DestroyNotebook();
		    }
		    if ( !emptyViewInitialized ) {
			    InitEmptyView();
		    }
		    UpdateEmptyView();
	    }
    }
}


void CSimplePanel::UpdateEmptyView() {
	clientState->DisplayState();
}


void CSimplePanel::DestroyEmptyView() {
	delete clientState;
	clientState = NULL;
	emptyViewInitialized = false;
}


void CSimplePanel::InitEmptyView()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	//Set Background color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());

	// Flex Grid Sizer
	mainSizer = new wxFlexGridSizer(3,2);
	SetSizer(mainSizer);

	clientState = new ClientStateIndicator(this,wxPoint(31,94));
	clientState->DisplayState();
	
	emptyViewInitialized = true;
}


void CSimplePanel::UpdateProjectView()
{
	//update Project Component
    projComponent->UpdateInterface();
}


void CSimplePanel::DestroyNotebook() {
	mainSizer->Detach(wrkUnitNB);
	delete wrkUnitNB;
	wrkUnitNB = NULL;
	notebookViewInitialized = false;
}


void CSimplePanel::InitProjectView()
{
	// Do not update screen at this point
	/////////////// MY PROJECTS COMPONENT /////////////////////
    projComponent = new CProjectsComponent(this,wxPoint(31,414));
	///////////////////////////////////////////////////////////
	projectViewInitialized = true;
}


void CSimplePanel::InitNotebook()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function Start"));
	// FlatNotebook
	wrkUnitNB = new WorkunitNotebook(this, -1, wxDefaultPosition, wxSize(370,330), wxFNB_TABS_BORDER_SIMPLE | wxFNB_NO_X_BUTTON | wxFNB_NO_NAV_BUTTONS | wxFNB_FANCY_TABS | wxFNB_NODRAG );
    SetSizer(mainSizer);
	mainSizer->Add(31, 68,0);
	mainSizer->Add(343, 68,0);
	mainSizer->Add(31, 68,0);
	mainSizer->Add(0, 0,1);
	mainSizer->Add(wrkUnitNB);
	mainSizer->Layout();
	notebookViewInitialized = true;
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::InitNotebook - Function End"));
}


void CSimplePanel::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxDC *dc = event.GetDC();
    dc->DrawBitmap(*pSkinSimple->GetBackgroundImage()->GetBitmap(), 0, 0);
}

