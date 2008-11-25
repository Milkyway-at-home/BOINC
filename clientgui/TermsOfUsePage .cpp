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
#pragma implementation "TermsOfUsePage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "TermsOfUsePage.h"


/*!
 * CTermsOfUsePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CTermsOfUsePage, wxWizardPageEx )
 
/*!
 * CTermsOfUsePage event table definition
 */
 
BEGIN_EVENT_TABLE( CTermsOfUsePage, wxWizardPageEx )
 
////@begin CTermsOfUsePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CTermsOfUsePage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CTermsOfUsePage::OnCancel )
    EVT_RADIOBUTTON( ID_TERMSOFUSEAGREECTRL, CTermsOfUsePage::OnAgree )
    EVT_RADIOBUTTON( ID_TERMSOFUSEDISAGREECTRL, CTermsOfUsePage::OnDisagree )
////@end CTermsOfUsePage event table entries
 
END_EVENT_TABLE()
  
/*!
 * CTermsOfUsePage constructors
 */
 
CTermsOfUsePage::CTermsOfUsePage( )
{
}
  
CTermsOfUsePage::CTermsOfUsePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CTermsOfUsePage creator
 */
 
bool CTermsOfUsePage::Create( CBOINCBaseWizard* parent )
{
////@begin CTermsOfUsePage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
    m_pTermsOfUseCtrl = NULL;
    m_pAgreeCtrl = NULL;
    m_pDisagreeCtrl = NULL;
////@end CTermsOfUsePage member initialisation
 
////@begin CTermsOfUsePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, ID_TERMSOFUSEPAGE, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CTermsOfUsePage creation
 
    return TRUE;
}
  
/*!
 * Control creation for CTermsOfUsePage
 */
 
void CTermsOfUsePage::CreateControls()
{    
////@begin CTermsOfUsePage content construction
    CTermsOfUsePage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer97->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer97->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pTermsOfUseCtrl = new wxTextCtrl;
    m_pTermsOfUseCtrl->Create( itemWizardPage96, ID_TERMSOFUSECTRL, wxEmptyString, wxDefaultPosition, wxSize(300, 125), wxTE_MULTILINE );
    itemBoxSizer97->Add(m_pTermsOfUseCtrl, 0, wxGROW|wxALL, 5);

    m_pAgreeCtrl = new wxRadioButton;
    m_pAgreeCtrl->Create( itemWizardPage96, ID_TERMSOFUSEAGREECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pAgreeCtrl->SetValue(false);
    itemBoxSizer97->Add(m_pAgreeCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDisagreeCtrl = new wxRadioButton;
    m_pDisagreeCtrl->Create( itemWizardPage96, ID_TERMSOFUSEDISAGREECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pDisagreeCtrl->SetValue(true);
    itemBoxSizer97->Add(m_pDisagreeCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CTermsOfUsePage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CTermsOfUsePage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CTermsOfUsePage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (GetUserAgrees()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */

bool CTermsOfUsePage::ShowToolTips()
{
    return TRUE;
}
  
/*!
 * Get bitmap resources
 */
 
wxBitmap CTermsOfUsePage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
 
////@begin CTermsOfUsePage bitmap retrieval
    return wxNullBitmap;
////@end CTermsOfUsePage bitmap retrieval
}
  
/*!
 * Get icon resources
 */

wxIcon CTermsOfUsePage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval

////@begin CTermsOfUsePage icon retrieval
    return wxNullIcon;
////@end CTermsOfUsePage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_TERMSOFUSEPAGE
 */

void CTermsOfUsePage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    PROJECT_CONFIG&        pc = ((CBOINCBaseWizard*)GetParent())->project_config;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Terms of Use")
    );

    m_pDirectionsStaticCtrl->SetLabel(
        _("Please read the following terms of use:")
    );

    m_pTermsOfUseCtrl->SetValue(
        wxString(pc.terms_of_use.c_str(), wxConvUTF8)
    );

    m_pAgreeCtrl->SetLabel(
        _("I agree to the terms of use.")
    );

    m_pDisagreeCtrl->SetLabel(
        _("I disagree with the terms of use.")
    );

    Fit();
}
 
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_TERMSOFUSEPAGE
 */

void CTermsOfUsePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TERMSOFUSEAGREECTRL
 */

void CTermsOfUsePage::OnAgree( wxCommandEvent& /* event */ ) {
    SetUserAgrees(true);
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TERMSOFUSEDISAGREECTRL
 */

void CTermsOfUsePage::OnDisagree( wxCommandEvent& /* event */ ) {
    SetUserAgrees(false);
}

