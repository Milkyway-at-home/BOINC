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
#pragma implementation "NotDetectedPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "NotDetectedPage.h"


/*!
 * CErrNotDetectedPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrNotDetectedPage, wxWizardPageEx )
 
/*!
 * CErrNotDetectedPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrNotDetectedPage, wxWizardPageEx )
 
////@begin CErrNotDetectedPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrNotDetectedPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrNotDetectedPage::OnCancel )

////@end CErrNotDetectedPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrNotDetectedPage constructors
 */
 
CErrNotDetectedPage::CErrNotDetectedPage( )
{
}
 
CErrNotDetectedPage::CErrNotDetectedPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProjectUnavailable creator
 */
 
bool CErrNotDetectedPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrNotDetectedPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
////@end CErrNotDetectedPage member initialisation
 
////@begin CErrNotDetectedPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrNotDetectedPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrProjectUnavailable
 */
 
void CErrNotDetectedPage::CreateControls()
{    
////@begin CErrNotDetectedPage content construction
    CErrNotDetectedPage* itemWizardPage96 = this;

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
////@end CErrNotDetectedPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrNotDetectedPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrNotDetectedPage::GetNext() const
{
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrNotDetectedPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrNotDetectedPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrNotDetectedPage bitmap retrieval
    return wxNullBitmap;
////@end CErrNotDetectedPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CErrNotDetectedPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrNotDetectedPage icon retrieval
    return wxNullIcon;
////@end CErrNotDetectedPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROJECTNOTDETECTEDPAGE
 */
 
void CErrNotDetectedPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;

    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    if (IS_ATTACHTOPROJECTWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Project not found")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("The URL you supplied is not that of a BOINC-based project.\n"
              "\n"
              "Please check the URL and try again.")
        );
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        m_pTitleStaticCtrl->SetLabel(
            _("Account manager not found")
        );
        m_pDirectionsStaticCtrl->SetLabel(
            _("The URL you supplied is not that of a BOINC-based account\n"
              "manager.\n"
              "\n"
              "Please check the URL and try again.")
        );
    } else {
        wxASSERT(FALSE);
    }

    Fit();
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROJECTNOTDETECTEDPAGE
 */
 
void CErrNotDetectedPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

