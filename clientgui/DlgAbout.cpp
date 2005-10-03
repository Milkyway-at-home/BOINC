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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgAbout.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"

////@begin includes
////@end includes

#include "DlgAbout.h"

#ifdef __WXMSW__
#include "../version.h"
#else
#include "config.h"
#endif


////@begin XPM images
#include "res/boincsm.xpm"
////@end XPM images

/*!
 * CDlgAbout type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CDlgAbout, wxDialog)

/*!
 * CDlgAbout event table definition
 */

BEGIN_EVENT_TABLE(CDlgAbout, wxDialog)

////@begin CDlgAbout event table entries
////@end CDlgAbout event table entries

END_EVENT_TABLE()

/*!
 * CDlgAbout constructors
 */

CDlgAbout::CDlgAbout() {}

CDlgAbout::CDlgAbout(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgHelpAbout creator
 */

bool CDlgAbout::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
////@begin CDlgAbout member initialisation
    m_strVersion = BOINC_VERSION_STRING;
////@end CDlgAbout member initialisation

////@begin CDlgAbout creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgAbout creation
    return TRUE;
}

/*!
 * Control creation for CDlgHelpAbout
 */

void CDlgAbout::CreateControls() {    
////@begin CDlgAbout content construction
    CDlgAbout* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText;
    itemStaticText3->Create( itemDialog1, wxID_STATIC, _("BOINC Manager"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    itemStaticText3->SetFont(wxFont(24, wxDEFAULT, wxNORMAL, wxBOLD, FALSE, _T("")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmap itemStaticBitmap6Bitmap(itemDialog1->GetBitmapResource(wxT("res/boincsm.xpm")));
    wxStaticBitmap* itemStaticBitmap6 = new wxStaticBitmap;
    itemStaticBitmap6->Create( itemDialog1, wxID_STATIC, itemStaticBitmap6Bitmap, wxDefaultPosition, wxSize(50, 50), 0 );
    itemBoxSizer5->Add(itemStaticBitmap6, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Version:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText9 = new wxStaticText;
    itemStaticText9->Create( itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText10 = new wxStaticText;
    itemStaticText10->Create( itemDialog1, wxID_STATIC, _("Copyright:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText11 = new wxStaticText;
    itemStaticText11->Create( itemDialog1, wxID_STATIC, _("(C) 2005 University of California at Berkeley.\nAll Rights Reserved."), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer7->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText12 = new wxStaticText;
    itemStaticText12->Create( itemDialog1, wxID_STATIC, _("Berkeley Open Infrastructure for Network Computing"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText;
    itemStaticText13->Create( itemDialog1, wxID_STATIC, _("A software platform for distributed computing using volunteered computer resources"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperLink* itemHyperLink14 = new wxHyperLink;
    itemHyperLink14->Create( itemDialog1, ID_ABOUTBOINCLINK, wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
    itemBoxSizer2->Add(itemHyperLink14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine* itemStaticLine15 = new wxStaticLine;
    itemStaticLine15->Create( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine15, 0, wxGROW|wxALL, 5);

    wxButton* itemButton16 = new wxButton;
    itemButton16->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetDefault();
    itemBoxSizer2->Add(itemButton16, 0, wxALIGN_RIGHT|wxALL, 5);

    // Set validators
    itemStaticText9->SetValidator( wxGenericValidator(& m_strVersion) );
////@end CDlgAbout content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgAbout::ShowToolTips() {
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgAbout::GetBitmapResource(const wxString& name) {
    // Bitmap retrieval
////@begin CDlgAbout bitmap retrieval
    if (name == wxT("res/boincsm.xpm"))
    {
        wxBitmap bitmap(boincsm_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CDlgAbout bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgAbout::GetIconResource(const wxString&) {
    // Icon retrieval
////@begin CDlgAbout icon retrieval
    return wxNullIcon;
////@end CDlgAbout icon retrieval
}
const char *BOINC_RCSID_b40c2996e6="$Id$";
