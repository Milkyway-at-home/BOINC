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
#pragma implementation "sg_ImageButton.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "common/wxFlatNotebook.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_ImageButton.h"
#include "sg_ImageLoader.h"
#include "sg_ViewTabPage.h"

BEGIN_EVENT_TABLE(CImageButton, wxPanel) 
        EVT_PAINT(CImageButton::OnPaint) 
		EVT_LEFT_UP(CImageButton::OnLeftUp)
		EVT_ERASE_BACKGROUND(CImageButton::OnEraseBackground)
END_EVENT_TABLE() 

CImageButton::CImageButton(wxWindow* parent,wxBitmap bg, wxPoint coord,wxSize size,bool enableShowGraphics, int initStatus) : wxPanel(parent, wxID_ANY, coord, size, wxNO_BORDER) 
{ 
	btnBG = bg;
	m_enableShowGraphics = enableShowGraphics;
	status = initStatus;
	SetToolTip();
}

CImageButton::~CImageButton() {
}

void CImageButton::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
        wxPaintDC dc(this); 
		dc.SetTextForeground(wxColour(*wxWHITE));
#ifdef __WXMAC__
		dc.SetFont(wxFont(10,74,90,90,0,wxT("Arial")));
#else
		dc.SetFont(wxFont(7,74,90,90,0,wxT("Arial")));
#endif
		int x, y;
		GetSize(&x,&y);
		wxCoord width, height;
		dc.GetTextExtent(GetStatusText(), &width, &height);
		dc.DrawText(GetStatusText(), wxPoint(7, y-height-4));
		// display show graphics text (if available)
		if(m_enableShowGraphics) {
			wxString text = wxString(_T("Graphics Available"));
			dc.GetTextExtent(text, &width, &height);
			dc.DrawText(text, wxPoint(x-width-7,y-height-4));
		}
} 
void CImageButton::SetImage(wxBitmap bg) 
{ 
      btnBG = bg;  
} 
void CImageButton::SetStatus(int newStatus) {
	status = newStatus;
}
int CImageButton::GetStatus() {
	return status;
}

wxString CImageButton::GetStatusText() {
	if ( status == TAB_STATUS_RUNNING ) {
		return wxString(_("Running"));
	} else if ( status == TAB_STATUS_PREEMPTED )  {
		return wxString(_("Paused: Other work running"));
	} else if ( status == TAB_STATUS_PAUSED_USER_REQ )  {
		return wxString(_("Paused: User initiatied.  Click 'Resume' to continue"));
	} else if ( status == TAB_STATUS_PAUSED_USER_ACTIVE )  {
		return wxString(_("Paused: User active"));
	} else if ( status == TAB_STATUS_PAUSED_POWER )  {
		return wxString(_("Paused: Computer on battery"));
	} else if ( status == TAB_STATUS_PAUSED_TIME_OF_DAY )  {
		return wxString(_("Paused: Time of Day"));
	} else if ( status == TAB_STATUS_PAUSED_BENCHMARKS )  {
		return wxString(_("Paused: Benchmarks running"));
	} else if ( status == TAB_STATUS_PAUSED )  {
		return wxString(_("Paused"));
	} else {
		return wxEmptyString;
	}
}
void CImageButton::OnEraseBackground(wxEraseEvent& event){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	if(btnBG.Ok()) 
    { 
		dc->DrawBitmap(btnBG, 0, 0); 
    } 
	
}
void CImageButton::OnLeftUp(wxMouseEvent& WXUNUSED(event))
{
	CViewTabPage* pView = wxDynamicCast(GetParent(), CViewTabPage);
    wxASSERT(pView);
	pView->OnImageButton();
	
}

void CImageButton::SetToolTip() {
	if ( m_enableShowGraphics ) {
		wxPanel::SetToolTip(new wxToolTip(_("Click to show project graphics")));
	} else {
		wxPanel::SetToolTip(NULL);
	}
}

void CImageButton::SetEnableShowGraphics(bool show) {
	if ( show != m_enableShowGraphics )  {
		m_enableShowGraphics = show;
		SetToolTip();
		Refresh();
	}
}
 