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
#pragma implementation "sg_StatImageLoader.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "sg_StatImageLoader.h" 
#include "sg_ProjectsComponent.h" 
#include "app_ipc.h"

enum{
	WEBSITE_URL_MENU_ID = 34500,
	WEBSITE_URL_MENU_ID_REMOVE_PROJECT = 34550,
	WEBSITE_URL_MENU_ID_HOMEPAGE = 34551,
};


BEGIN_EVENT_TABLE(StatImageLoader, wxWindow) 
    EVT_PAINT(StatImageLoader::OnPaint) 
    EVT_LEFT_DOWN(StatImageLoader::PopUpMenu)
	EVT_MENU(WEBSITE_URL_MENU_ID,StatImageLoader::OnMenuLinkClicked)
	EVT_MENU(WEBSITE_URL_MENU_ID_REMOVE_PROJECT,StatImageLoader::OnMenuLinkClicked)
END_EVENT_TABLE() 

StatImageLoader::StatImageLoader(wxWindow* parent, std::string url) : 
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(40,40), wxNO_BORDER) 
{
    m_prjUrl = url;
	project_files_downloaded_time = 1;
	project_last_rpc_time = 1;
	BuildUserStatToolTip();
	statPopUpMenu = new wxMenu(wxSIMPLE_BORDER);
    AddMenuItems();
}


StatImageLoader::~StatImageLoader() {
	delete statPopUpMenu;
}


#ifdef __WXMAC__
void StatImageLoader::PopUpMenu(wxMouseEvent& event) 
{ 
	// pop up menu: move menu down a bit so tooltip does not obscure it
	PopupMenu(statPopUpMenu, event.m_x-10, event.m_y+40);
}
#else
void StatImageLoader::PopUpMenu(wxMouseEvent& WXUNUSED(event)) 
{ 
	// pop up menu
	PopupMenu(statPopUpMenu);
}
#endif

void StatImageLoader::RebuildMenu() {
	for(int i=(int) statPopUpMenu->GetMenuItemCount()-1; i>=0;i--){
		wxMenuItem* item = statPopUpMenu->FindItemByPosition(i);
		statPopUpMenu->Delete(item);
	}
	AddMenuItems();
}


void StatImageLoader::BuildUserStatToolTip() {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
	//user credit text
	wxString userCredit;
	userCredit.Printf(wxT("%0.2f"), project->user_total_credit);
	wxString toolTipTxt = wxString(project->project_name.c_str(), wxConvUTF8 ) +wxT(". User ") + wxString(project->user_name.c_str(), wxConvUTF8) + wxT(" has ") + userCredit + wxT(" points."); 
	wxToolTip* userStatToolTip = new wxToolTip(toolTipTxt);
	SetToolTip(userStatToolTip);
}


void StatImageLoader::AddMenuItems() 
{ 
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
	urlCount = project->gui_urls.size();

	// Add the home page link
    wxMenuItem *urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID_HOMEPAGE,wxString(project->project_name.c_str(), wxConvUTF8));
#ifdef __WXMSW__
    urlItem->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
#endif
	Connect( WEBSITE_URL_MENU_ID_HOMEPAGE,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
	statPopUpMenu->Append(urlItem);


	// Add any GUI urls
	for(unsigned int i = 0; i < urlCount; i++){
		urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID + i,wxString(project->gui_urls[i].name.c_str(), wxConvUTF8));
#ifdef __WXMSW__
		urlItem->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
#endif
	    Connect( WEBSITE_URL_MENU_ID + i,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
 
		statPopUpMenu->Append(urlItem);
	}
    
	//  Add the 'remove project' option
	statPopUpMenu->AppendSeparator();
	wxMenuItemList menuList = statPopUpMenu->GetMenuItems();
#ifdef __WXMSW__
	menuList[statPopUpMenu->GetMenuItemCount()-1]->SetBackgroundColour(wxColour("RED"));
#endif

	urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID_REMOVE_PROJECT,wxT("Remove Project"));
#ifdef __WXMSW__
	urlItem->SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
#endif
	Connect( WEBSITE_URL_MENU_ID_REMOVE_PROJECT,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
	statPopUpMenu->Append(urlItem);

}


void StatImageLoader::OnMenuLinkClicked(wxCommandEvent& event) 
{ 
	 CMainDocument* pDoc = wxGetApp().GetDocument();
     wxASSERT(pDoc);
	 int menuIDevt =  event.GetId();

	 if(menuIDevt == WEBSITE_URL_MENU_ID_REMOVE_PROJECT){
		 //call detach project function	
         OnProjectDetach();
	 } else if (menuIDevt == WEBSITE_URL_MENU_ID_HOMEPAGE ) {
	     CBOINCBaseFrame* pFrame = wxDynamicCast(m_parent->GetParent(),CBOINCBaseFrame);
         wxASSERT(pFrame);
         wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
	     pFrame->ExecuteBrowserLink(m_prjUrl.c_str());
	 } else{
         int menuId = menuIDevt - WEBSITE_URL_MENU_ID;
	     PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
		 project->gui_urls[menuId].name.c_str();
     
	     CBOINCBaseFrame* pFrame = wxDynamicCast(m_parent->GetParent(),CBOINCBaseFrame);
         wxASSERT(pFrame);
         wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
	     pFrame->ExecuteBrowserLink(project->gui_urls[menuId].url.c_str());
	 }
} 


void StatImageLoader::OnProjectDetach() {
    wxInt32  iAnswer        = 0; 
    std::string strProjectName;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    CProjectsComponent* pComp      = wxDynamicCast(GetParent(), CProjectsComponent);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pComp);

    if (!pDoc->IsUserAuthorized())
        return;

	int indexOfProj = -1;
	int prjCount = pDoc->GetProjectCount();
	for(int m = 0; m < prjCount; m++){
		PROJECT* project = pDoc->project(m);
		project->get_name(strProjectName);
		if(project->master_url == m_prjUrl){
			indexOfProj = m;
			break;
		}
	}
    strMessage.Printf(
        _("Are you sure you want to detach from project '%s'?"), 
        strProjectName.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Detach from Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(indexOfProj);
		pComp->UpdateProjectArray();
    }
}


void StatImageLoader::LoadStatIcon(wxBitmap& image) {
	int width = image.GetWidth(); 
	int height = image.GetHeight(); 
	Bitmap = wxBitmap(image); 
	SetSize(width, height); 
}

std::string StatImageLoader::GetProjectIconLoc() {
	char urlDirectory[256];
	CMainDocument* pDoc = wxGetApp().GetDocument();
	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
	url_to_project_dir((char*)project->master_url.c_str() ,urlDirectory);
	return (std::string)urlDirectory + "/" + wxT("stat_icon");
}


void StatImageLoader::LoadImage() {
	std::string dirProjectGraphic;

    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

	char defaultIcnPath[256];
	if(boinc_resolve_filename(GetProjectIconLoc().c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
		wxBitmap* btmpStatIcn = new wxBitmap();
		if ( btmpStatIcn->LoadFile(defaultIcnPath, wxBITMAP_TYPE_ANY) ) {
			LoadStatIcon(*btmpStatIcn);
		} else {
			LoadStatIcon(*pSkinSimple->GetProjectImage()->GetBitmap());
		}
		delete btmpStatIcn;
	}else{
		LoadStatIcon(*pSkinSimple->GetProjectImage()->GetBitmap());
	}
}


void StatImageLoader::ReloadProjectSpecificIcon() {
	char defaultIcnPath[256];
	// Only update if it is project specific is found
	if(boinc_resolve_filename(GetProjectIconLoc().c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
		wxBitmap* btmpStatIcn = new wxBitmap();
		if ( btmpStatIcn->LoadFile(defaultIcnPath, wxBITMAP_TYPE_ANY) ) {
			LoadStatIcon(*btmpStatIcn);
			RebuildMenu();
			Refresh();
			Update();
		}
		delete btmpStatIcn;
	}
}


void StatImageLoader::UpdateInterface() {
	CMainDocument* pDoc = wxGetApp().GetDocument();
	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);

	// Check to see if we need to reload the stat icon
	if ( project > NULL && project->project_files_downloaded_time > project_files_downloaded_time ) {
		ReloadProjectSpecificIcon();
		project_files_downloaded_time = project->project_files_downloaded_time;
	}

	// Check to see if we need to rebuild the hoover and menu
	if ( project > NULL && project->last_rpc_time > project_last_rpc_time ) {
		RebuildMenu();
		BuildUserStatToolTip();
		project_last_rpc_time = project->last_rpc_time;
	}
}


void StatImageLoader::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
    dc.SetBackgroundMode(wxTRANSPARENT);
    if(Bitmap.Ok()) { 
        dc.DrawBitmap(Bitmap, 0, 0);
    } 
} 
