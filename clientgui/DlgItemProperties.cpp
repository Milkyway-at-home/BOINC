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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgItemProperties.h"
#endif

#include "stdwx.h"
#include "util.h"
#include "DlgItemProperties.h"
#include "BOINCGUIApp.h"
#include "Events.h"
#include "error_numbers.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgItemProperties, wxDialog)

/* Constructor */
CDlgItemProperties::CDlgItemProperties(wxWindow* parent) : 
    wxDialog( parent, ID_ANYDIALOG, wxEmptyString, wxDefaultPosition, 
                wxSize( 503,480 ), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ) {
	SetSizeHints( wxDefaultSize, wxDefaultSize );
	SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	m_bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_scrolledWindow = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledWindow->SetScrollRate( 5, 5 );
	wxBoxSizer* m_bSizer2;
	m_bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_gbSizer = new wxGridBagSizer( 0, 0 );
	m_gbSizer->AddGrowableCol( 1 );
	m_gbSizer->SetFlexibleDirection( wxBOTH );
	m_gbSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_bSizer2->Add( m_gbSizer, 1, wxEXPAND, 5 );
	
	m_scrolledWindow->SetSizer( m_bSizer2 );
	m_scrolledWindow->Layout();
	m_bSizer2->Fit( m_scrolledWindow );
	m_bSizer1->Add( m_scrolledWindow, 1, wxEXPAND | wxALL, 5 );
	
	m_btnClose = new wxButton( this, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
	m_btnClose->SetDefault(); 
	m_bSizer1->Add( m_btnClose, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	SetSizer( m_bSizer1 );
	Layout();
	
	Centre( wxBOTH );

	m_current_row=0;
}

// destructor
CDlgItemProperties::~CDlgItemProperties() {
}

// show project properties
//
void CDlgItemProperties::renderInfos(PROJECT* project_in) {
	std::string projectname;
	//collecting infos
	project_in->get_name(projectname);
	//disk usage needs additional lookups
	CMainDocument* pDoc = wxGetApp().GetDocument();
	pDoc->CachedDiskUsageUpdate();
    
	// CachedDiskUsageUpdate() may have invalidated our project 
	// pointer, so get an updated pointer to this project
	PROJECT* project = pDoc->project(wxString(projectname.c_str(),wxConvUTF8));
	if(!project) return;     // TODO: display some sort of error alert?

	std::vector<PROJECT*> dp = pDoc->disk_usage.projects;
	double diskusage=0.0;	
	for (unsigned int i=0; i< dp.size(); i++) {
		PROJECT* tp = dp[i];		
		std::string tname;		
		tp->get_name(tname);
		wxString t1(wxString(tname.c_str(),wxConvUTF8));
		if(t1.IsSameAs(wxString(projectname.c_str(),wxConvUTF8)) || t1.IsSameAs(wxString(project->master_url.c_str(),wxConvUTF8))) {
			diskusage =tp->disk_usage;
			break;
		}
	}
	//set dialog title
	wxString wxTitle = _("Properties of project ");
	wxTitle.append(wxString(projectname.c_str(),wxConvUTF8));
	SetTitle(wxTitle);
	//layout controls
	addSection(_("General"));
	addProperty(_("Master URL"),wxString(project->master_url.c_str(),wxConvUTF8));
	addProperty(_("User name"),wxString(project->user_name.c_str(),wxConvUTF8));
	addProperty(_("Team name"),wxString(project->team_name.c_str(),wxConvUTF8));
	addProperty(_("Resource share"),wxString::Format(wxT("%0.0f"),project->resource_share));
    if (project->min_rpc_time > dtime()) {
		addProperty(_("Scheduler RPC deferred for"), FormatTime(project->min_rpc_time - dtime()));
    }
    if (project->download_backoff) {
		addProperty(_("File downloads deferred for"), FormatTime(project->download_backoff));
    }
    if (project->upload_backoff) {
		addProperty(_("File uploads deferred for"), FormatTime(project->upload_backoff));
    }
	addProperty(_("Disk usage"),FormatDiskSpace(diskusage));
    addProperty(_("Computer ID"), wxString::Format(wxT("%d"), project->hostid));
	addProperty(_("Non CPU intensive"),project->non_cpu_intensive ? _("yes") : _("no"));
	addProperty(_("Suspended via GUI"),project->suspended_via_gui ? _("yes") : _("no"));
	addProperty(_("Don't request more work"),project->dont_request_more_work ? _("yes") : _("no"));
	addProperty(_("Scheduler call in progress"),project->scheduler_rpc_in_progress ? _("yes") : _("no"));
	addProperty(_("Attached via account manager"),project->attached_via_acct_mgr ? _("yes") : _("no"));
	addProperty(_("Detach when done"),project->detach_when_done ? _("yes") : _("no"));
	addProperty(_("Ended"),project->ended ? _("yes") : _("no"));
	addSection(_("Credit"));
	addProperty(_("User"),
        wxString::Format(
            wxT("%0.2f total, %0.2f average"),
            project->user_total_credit,
            project->user_expavg_credit
        )
    );
	addProperty(_("Host"),
        wxString::Format(
            wxT("%0.2f total, %0.2f average"),
            project->host_total_credit,
            project->host_expavg_credit
        )
    );
	
    if (!project->non_cpu_intensive) {
	    addSection(_("Scheduling"));
	    addProperty(_("CPU scheduling priority"),wxString::Format(wxT("%0.2f"), project->short_term_debt));
	    addProperty(_("CPU work fetch priority"),wxString::Format(wxT("%0.2f"), project->cpu_long_term_debt));
        double x = project->cpu_backoff_time - dtime();
        if (x<0) x = 0;
        addProperty(_("CPU work fetch deferred for"), FormatTime(x));
        addProperty(_("CPU work fetch deferral interval"), FormatTime(project->cpu_backoff_interval));
        if (pDoc->state.have_cuda) {
            addProperty(_("NVIDIA GPU work fetch priority"),wxString::Format(wxT("%0.2f"), project->cuda_debt));
            x = project->cuda_backoff_time - dtime();
            if (x<0) x = 0;
            addProperty(_("NVIDIA GPU work fetch deferred for"), FormatTime(x));
            addProperty(_("NVIDIA GPU work fetch deferral interval"), FormatTime(project->cuda_backoff_interval));
        }
        if (pDoc->state.have_ati) {
            addProperty(_("ATI GPU work fetch priority"),wxString::Format(wxT("%0.2f"), project->ati_debt));
            x = project->ati_backoff_time - dtime();
            if (x<0) x = 0;
            addProperty(_("ATI GPU work fetch deferred for"), FormatTime(x));
            addProperty(_("ATI GPU work fetch deferral interval"), FormatTime(project->ati_backoff_interval));
        }
	    addProperty(_("Duration correction factor"),wxString::Format(wxT("%0.4f"), project->duration_correction_factor));
    }
    m_gbSizer->Layout();
	m_scrolledWindow->FitInside();
}

// show task properties
//
void CDlgItemProperties::renderInfos(RESULT* result) {
    wxDateTime dt;
	wxString wxTitle = _("Properties of task ");
	wxTitle.append(wxString(result->name.c_str(),wxConvUTF8));
	SetTitle(wxTitle);

	addProperty(_("Application"), FormatApplicationName(result));
	addProperty(_("Workunit name"),wxString(result->wu_name.c_str(),wxConvUTF8));
	addProperty(_("State"), FormatStatus(result));
    if (result->received_time) {
        dt.Set((time_t)result->received_time);
	    addProperty(_("Received"), dt.Format());
    }
    dt.Set((time_t)result->report_deadline);
	addProperty(_("Report deadline"), dt.Format());
	if (result->resources.size()) {
		addProperty(_("Resources"), wxString(result->resources.c_str(), wxConvUTF8));
	}
    if (result->active_task) {
		addProperty(_("CPU time at last checkpoint"), FormatTime(result->checkpoint_cpu_time));
		addProperty(_("CPU time"), FormatTime(result->current_cpu_time));
        if (result->elapsed_time >= 0) {
		    addProperty(_("Elapsed time"), FormatTime(result->elapsed_time));
        }
		addProperty(_("Estimated time remaining"), FormatTime(result->estimated_cpu_time_remaining));
		addProperty(_("Fraction done"), wxString::Format(wxT("%.3f %%"), result->fraction_done*100));
		addProperty(_("Virtual memory size"), FormatDiskSpace(result->swap_size));
		addProperty(_("Working set size"), FormatDiskSpace(result->working_set_size_smoothed));
        if (result->slot >= 0) {
            addProperty(_("Directory"), wxString::Format(wxT("slots/%d"), result->slot));
        }
	} else if (result->state >= RESULT_COMPUTE_ERROR) {
		addProperty(_("CPU time"), FormatTime(result->final_cpu_time));
		addProperty(_("Elapsed time"), FormatTime(result->final_elapsed_time));
    }
	m_gbSizer->Layout();
	m_scrolledWindow->FitInside();
}

//
wxString CDlgItemProperties::FormatDiskSpace(double bytes) {	
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;
	wxString strBuffer= wxEmptyString;

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
    return strBuffer;
}

//
wxString CDlgItemProperties::FormatApplicationName(RESULT* result ) {
	wxString       strBuffer = wxEmptyString;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    RESULT*        state_result = NULL;
    wxString       strAppBuffer = wxEmptyString;
    wxString       strClassBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (result) {
        state_result = pDoc->state.lookup_result(result->project_url, result->name);
        if (!state_result) {
            pDoc->ForceCacheUpdate();
            state_result = pDoc->state.lookup_result(result->project_url, result->name);
        }
        wxASSERT(state_result);

        if (!state_result) return strBuffer;
        WORKUNIT* wup = state_result->wup;
        if (!wup) return strBuffer;
        APP* app = wup->app;
        if (!app) return strBuffer;
        APP_VERSION* avp = state_result->avp;
        if (!avp) return strBuffer;

        if (app->user_friendly_name.size()) {
            strAppBuffer = wxString(state_result->app->user_friendly_name.c_str(), wxConvUTF8);
        } else {
            strAppBuffer = wxString(state_result->avp->app_name.c_str(), wxConvUTF8);
        }

        if (avp->plan_class.size()) {
            strClassBuffer.Printf(
                wxT(" (%s)"),
                wxString(avp->plan_class.c_str(), wxConvUTF8).c_str()
            );
        }

        strBuffer.Printf(
            wxT("%s %d.%02d %s"), 
            strAppBuffer.c_str(),
            state_result->avp->version_num / 100,
            state_result->avp->version_num % 100,
            strClassBuffer.c_str()
        );
    }
    return strBuffer;
}


//
wxString CDlgItemProperties::FormatStatus(RESULT* result) {
	wxString strBuffer= wxEmptyString;
    CMainDocument* doc = wxGetApp().GetDocument();    
    CC_STATUS      status;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    doc->GetCoreClientStatus(status);
    
	int throttled = status.task_suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT;
    switch(result->state) {
    case RESULT_NEW:
        strBuffer = _("New"); 
        break;
    case RESULT_FILES_DOWNLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Download failed");
        } else {
            strBuffer = _("Downloading");
        }
        break;
    case RESULT_FILES_DOWNLOADED:
        if (result->project_suspended_via_gui) {
            strBuffer = _("Project suspended by user");
        } else if (result->suspended_via_gui) {
            strBuffer = _("Task suspended by user");
        } else if (status.task_suspend_reason && !throttled) {
            strBuffer = _("Suspended");
            if (status.task_suspend_reason & SUSPEND_REASON_BATTERIES) {
                strBuffer += _(" - on batteries");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_ACTIVE) {
                strBuffer += _(" - user active");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_USER_REQ) {
                strBuffer += _(" - computation suspended");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_TIME_OF_DAY) {
                strBuffer += _(" - time of day");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_BENCHMARKS) {
                strBuffer += _(" - CPU benchmarks");
            }
            if (status.task_suspend_reason & SUSPEND_REASON_DISK_SIZE) {
                strBuffer += _(" - need disk space");
            }
        } else if (result->active_task) {
            if (result->too_large) {
                strBuffer = _("Waiting for memory");
            } else if (result->needs_shmem) {
                strBuffer = _("Waiting for shared memory");
            } else if (result->scheduler_state == CPU_SCHED_SCHEDULED) {
                if (result->edf_scheduled) {
                    strBuffer = _("Running, high priority");
                } else {
                    strBuffer = _("Running");
                }
            } else if (result->scheduler_state == CPU_SCHED_PREEMPTED) {
                strBuffer = _("Waiting to run");
            } else if (result->scheduler_state == CPU_SCHED_UNINITIALIZED) {
                strBuffer = _("Ready to start");
            }
        } else {
            strBuffer = _("Ready to start");
        }
        break;
    case RESULT_COMPUTE_ERROR:
        strBuffer = _("Computation error");
        break;
    case RESULT_FILES_UPLOADING:
        if (result->ready_to_report) {
            strBuffer = _("Upload failed");
        } else {
            strBuffer = _("Uploading");
        }
        break;
    case RESULT_ABORTED:
        switch(result->exit_status) {
        case ERR_ABORTED_VIA_GUI:
            strBuffer = _("Aborted by user");
            break;
        case ERR_ABORTED_BY_PROJECT:
            strBuffer = _("Aborted by project");
            break;
        default:
            strBuffer = _("Aborted");
        }
        break;
    default:
        if (result->got_server_ack) {
            strBuffer = _("Acknowledged");
        } else if (result->ready_to_report) {
            strBuffer = _("Ready to report");
        } else {
            strBuffer.Format(_("Error: invalid state '%d'"), result->state);
        }
        break;
    }
    return strBuffer;
}

// 
wxString CDlgItemProperties::FormatTime(float fBuffer) {
    wxInt32        iHour = 0;
    wxInt32        iMin = 0;
    wxInt32        iSec = 0;
    wxTimeSpan     ts;
	wxString strBuffer= wxEmptyString;

    if (0 >= fBuffer) {
        strBuffer = wxT("---");
    } else {
        iHour = (wxInt32)(fBuffer / (60 * 60));
        iMin  = (wxInt32)(fBuffer / 60) % 60;
        iSec  = (wxInt32)(fBuffer) % 60;

        ts = wxTimeSpan(iHour, iMin, iSec);

        strBuffer = ts.Format();
    }

    return strBuffer;
}

// adds a title section label to the dialog 
void CDlgItemProperties::addSection(const wxString& title) {
	wxStaticText* staticText = new wxStaticText( m_scrolledWindow, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, 0);
	staticText->Wrap( -1 );
	staticText->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );	
	m_gbSizer->Add( staticText, wxGBPosition( m_current_row, 0 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 3);
	m_current_row++;
}

// adds a property row to the dialog 
void CDlgItemProperties::addProperty(const wxString& name, const wxString& value) {
	
	wxStaticText* staticText = new wxStaticText( m_scrolledWindow, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, 0 );
	staticText->Wrap( -1 );
	m_gbSizer->Add( staticText, wxGBPosition( m_current_row, 0 ), wxGBSpan( 1, 1 ), wxALL, 3 );
	
	staticText = new wxStaticText( m_scrolledWindow, wxID_ANY, value, wxDefaultPosition, wxDefaultSize, 0 );
	staticText->Wrap( -1 );
	m_gbSizer->Add( staticText, wxGBPosition( m_current_row, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 3 );
	m_current_row++;
}
