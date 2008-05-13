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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewResources.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewResources.h"
#include "Events.h"
#include <wx/arrimpl.cpp>
#include "res/usage.xpm"

WX_DEFINE_OBJARRAY(wxArrayColour);

IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewResources, CBOINCBaseView)
END_EVENT_TABLE ()


CViewResources::CViewResources()
{}

#define COLOR(c) wxColour(c>>16, (c>>8)&0xff, c&0xff)
CViewResources::CViewResources(wxNotebook* pNotebook) :
	CBOINCBaseView(pNotebook)
{
	m_BOINCwasEmpty=false;

	wxGridSizer* itemGridSizer = new wxGridSizer(2, 0, 3);
    wxASSERT(itemGridSizer);

	// create pie chart ctrl for total disk usage
	m_pieCtrlTotal = new wxPieCtrl(this, ID_LIST_RESOURCEUTILIZATIONVIEWTOTAL, wxDefaultPosition, wxSize(-1,-1));
	wxASSERT(m_pieCtrlTotal);
	// setup the legend
	m_pieCtrlTotal->SetTransparent(true);
	m_pieCtrlTotal->SetHorLegendBorder(10);
	m_pieCtrlTotal->SetLabelFont(*wxSWISS_FONT);
	m_pieCtrlTotal->SetLabelColour(wxColour(0,0,0));
	m_pieCtrlTotal->SetLabel(_("total disk usage"));

	// create pie chart ctrl for BOINC disk usage
	m_pieCtrlBOINC = new wxPieCtrl(this, ID_LIST_RESOURCEUTILIZATIONVIEW, wxDefaultPosition, wxSize(-1,-1));
	wxASSERT(m_pieCtrlBOINC);
	//setup the legend
	m_pieCtrlBOINC->SetTransparent(true);
	m_pieCtrlBOINC->SetHorLegendBorder(10);
	m_pieCtrlBOINC->SetLabelFont(*wxSWISS_FONT);
	m_pieCtrlBOINC->SetLabelColour(wxColour(0,0,0));
	m_pieCtrlBOINC->SetLabel(_("disk usage by BOINC projects"));
	//init the flexGrid
    itemGridSizer->Add(m_pieCtrlTotal,1,wxGROW|wxALL,1);
    itemGridSizer->Add(m_pieCtrlBOINC,1, wxGROW|wxALL,1);

    SetSizer(itemGridSizer);

    Layout();

    UpdateSelection();
}


CViewResources::~CViewResources() {
    EmptyTasks();
}


wxString& CViewResources::GetViewName() {
    static wxString strViewName(_("Disk"));
    return strViewName;
}


wxString& CViewResources::GetViewDisplayName() {
    static wxString strViewName(_("Disk"));
    return strViewName;
}


const char** CViewResources::GetViewIcon() {
    return usage_xpm;
}

void CViewResources::UpdateSelection() {
	//TODO: is this needed ? no task buttons
    CBOINCBaseView::PreUpdateSelection();
    //CBOINCBaseView::PostUpdateSelection();
}


#ifdef __WXMAC__
const int CViewResources::GetViewRefreshRate() {
    return 10;
}
#endif


wxInt32 CViewResources::FormatProjectName(PROJECT* project, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    std::string project_name;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (project) {
        PROJECT* state_project = doc->state.lookup_project(project->master_url);
        if (state_project) {
            state_project->get_name(project_name);
            strBuffer = HtmlEntityDecode(wxString(project_name.c_str(), wxConvUTF8));
        }
    }

    return 0;
}


bool CViewResources::OnSaveState(wxConfigBase* /*pConfig*/) {
    return true;/*bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;*/
}

bool CViewResources::OnRestoreState(wxConfigBase* /*pConfig*/) {
    return true;/*wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    return true;*/
}

void CViewResources::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString diskspace;
	double boinctotal=0.0;
	unsigned int i;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	//get data for BOINC projects disk usage
    pDoc->CachedDiskUsageUpdate();
    pDoc->CachedStateUpdate();
	bool refreshBOINC=false;
	if (pDoc->disk_usage.projects.size()>0) {
		m_BOINCwasEmpty=false;
		//check for changes worth a refresh
		if(pDoc->disk_usage.projects.size() != m_pieCtrlBOINC->m_Series.size()) {
			refreshBOINC=true;
		} else {
			for (i=0; i<pDoc->disk_usage.projects.size(); i++) {
				wxString oldValue;
				wxString newValue;
				FormatDiskSpace(pDoc->DiskUsageProject(i)->disk_usage, newValue);
				FormatDiskSpace(m_pieCtrlBOINC->m_Series.Item(i).GetValue(), oldValue);
				if(newValue.Cmp(oldValue)!=0) {
					refreshBOINC=true;
					break;
				}
			}
		}
		//only refresh when worthy changes
		if(refreshBOINC) {
			m_pieCtrlBOINC->m_Series.Clear();
			for (i=0; i<pDoc->disk_usage.projects.size(); i++) {
				//update data for boinc projects pie chart
				PROJECT* project = pDoc->DiskUsageProject(i);
				wxString projectname;
				FormatProjectName(project, projectname);
				FormatDiskSpace(project->disk_usage, diskspace);
				double usage = project->disk_usage;
				boinctotal += usage;
				wxPiePart part;
				part.SetLabel(projectname + wxT(" - ") + diskspace);
				part.SetValue(usage);
                unsigned char r=128+(rand()&127);
                unsigned char g=128+(rand()&127);
                unsigned char b=128+(rand()&127);
                part.SetColour(wxColour(r, g, b));
				m_pieCtrlBOINC->m_Series.Add(part);
			}
			m_pieCtrlBOINC->Refresh();
		}
	} else {
		if(!m_BOINCwasEmpty) {
			//paint an empty black pie
			m_pieCtrlBOINC->m_Series.Clear();
			wxPiePart part;
			part.SetLabel(_("not attached to any BOINC project - 0 bytes"));
			part.SetValue(boinctotal);
			part.SetColour(wxColour(0,0,0));
			m_pieCtrlBOINC->m_Series.Add(part);
			m_pieCtrlBOINC->Refresh();
			m_BOINCwasEmpty=true;
			refreshBOINC=true;
		}
	}
	//data for pie chart 2 (total disk usage)
	//
	// good source of color palettes:
	// http://www.siteprocentral.com/cgi-bin/feed/feed.cgi
	//
	bool refreshTotal=false;
	double free = pDoc->disk_usage.d_free;
	double total = pDoc->disk_usage.d_total;
	if(m_pieCtrlTotal->m_Series.size()>0) {
		wxString oldFree;
		wxString newFree;
		FormatDiskSpace(free, newFree);
		FormatDiskSpace(m_pieCtrlTotal->m_Series.Item(0).GetValue(), oldFree);
		if(oldFree.Cmp(newFree)!=0) {
			refreshTotal=true;
		}
	} else {
		refreshTotal=true;
	}
	if(refreshBOINC || refreshTotal) {
		m_pieCtrlTotal->m_Series.Clear();
		wxPiePart part;

		//free disk space
		FormatDiskSpace(free,diskspace);
		part.SetLabel(_("free disk space - ") + diskspace);
		part.SetValue(free);
		part.SetColour(wxColour(238,238,238));
		m_pieCtrlTotal->m_Series.Add(part);

		//used by boinc projects
		boinctotal += pDoc->disk_usage.d_boinc;
		FormatDiskSpace(boinctotal,diskspace);
		part.SetLabel(_("used by BOINC - ") + diskspace);
		part.SetValue(boinctotal);
		part.SetColour(wxColour(0,0,0));
		m_pieCtrlTotal->m_Series.Add(part);

		//used by others
		FormatDiskSpace(total-boinctotal-free,diskspace);
		part.SetLabel(_("used by other programs - ") + diskspace);
		part.SetValue(total-boinctotal-free);
		part.SetColour(wxColour(192,192,192));
		m_pieCtrlTotal->m_Series.Add(part);
		m_pieCtrlTotal->Refresh();
	}
}

wxInt32 CViewResources::FormatDiskSpace(double bytes, wxString& strBuffer) const {
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;

    if (bytes >= xTera) {
        strBuffer.Printf(wxT("%0.2f TB"), bytes/xTera);
    } else if (bytes >= xGiga) {
        strBuffer.Printf(wxT("%0.2f GB"), bytes/xGiga);
    } else if (bytes >= xMega) {
        strBuffer.Printf(wxT("%0.2f MB"), bytes/xMega);
    } else if (bytes >= xKilo) {
        strBuffer.Printf(wxT("%0.2f KB"), bytes/xKilo);
    } else {
        strBuffer.Printf(wxT("%0.0f bytes"), bytes);
    }

    return 0;
}

const char *BOINC_RCSID_5a37b46a6e = "$Id$";
