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
#pragma implementation "ProxyPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "ProxyPage.h"


/*!
 * CErrProxyPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrProxyPage, wxWizardPageEx )
 
/*!
 * CErrProxyPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrProxyPage, wxWizardPageEx )
 
////@begin CErrProxyPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrProxyPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CErrProxyPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CErrProxyPage::OnCancel )

////@end CErrProxyPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrProxyPage constructors
 */
 
CErrProxyPage::CErrProxyPage( )
{
}
 
CErrProxyPage::CErrProxyPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrProxyComplationPage creator
 */
 
bool CErrProxyPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrProxyPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pProxyHTTPDescriptionCtrl = NULL; 
    m_pProxyHTTPServerStaticCtrl = NULL;
    m_pProxyHTTPServerCtrl = NULL;
    m_pProxyHTTPPortStaticCtrl = NULL;
    m_pProxyHTTPPortCtrl = NULL;
    m_pProxyHTTPUsernameStaticCtrl = NULL;
    m_pProxyHTTPUsernameCtrl = NULL;
    m_pProxyHTTPPasswordStaticCtrl = NULL;
    m_pProxyHTTPPasswordCtrl = NULL;
#if 0
    m_pProxyHTTPAutodetectCtrl = NULL;
#endif
    m_pProxySOCKSDescriptionCtrl = NULL; 
    m_pProxySOCKSServerStaticCtrl = NULL;
    m_pProxySOCKSServerCtrl = NULL;
    m_pProxySOCKSPortStaticCtrl = NULL;
    m_pProxySOCKSPortCtrl = NULL;
    m_pProxySOCKSUsernameStaticCtrl = NULL;
    m_pProxySOCKSUsernameCtrl = NULL;
    m_pProxySOCKSPasswordStaticCtrl = NULL;
    m_pProxySOCKSPasswordCtrl = NULL;
////@end CErrProxyPage member initialisation
 
////@begin CErrProxyPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrProxyPage creation
 
    return TRUE;
}
 
/*!
 * Control creation for CErrProxyComplationPage
 */
 
void CErrProxyPage::CreateControls()
{    
////@begin CErrProxyPage content construction
    CErrProxyPage* itemWizardPage121 = this;

    wxBoxSizer* itemBoxSizer122 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage121->SetSizer(itemBoxSizer122);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage121, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer122->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer122->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pProxyHTTPDescriptionCtrl = new wxStaticBox(itemWizardPage121, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer125 = new wxStaticBoxSizer(m_pProxyHTTPDescriptionCtrl, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer125, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer126 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer126->AddGrowableCol(1);
    itemStaticBoxSizer125->Add(itemFlexGridSizer126, 0, wxGROW|wxALL, 2);

    m_pProxyHTTPServerStaticCtrl = new wxStaticText;
    m_pProxyHTTPServerStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer128 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer128->AddGrowableCol(0);
    itemFlexGridSizer126->Add(itemFlexGridSizer128, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProxyHTTPServerCtrl = new wxTextCtrl;
    m_pProxyHTTPServerCtrl->Create( itemWizardPage121, ID_PROXYHTTPSERVERCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_pProxyHTTPServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPortStaticCtrl = new wxStaticText;
    m_pProxyHTTPPortStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer128->Add(m_pProxyHTTPPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPortCtrl = new wxTextCtrl;
    m_pProxyHTTPPortCtrl->Create( itemWizardPage121, ID_PROXYHTTPPORTCTRL, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer128->Add(m_pProxyHTTPPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPUsernameStaticCtrl = new wxStaticText;
    m_pProxyHTTPUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMESTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPUsernameCtrl = new wxTextCtrl;
    m_pProxyHTTPUsernameCtrl->Create( itemWizardPage121, ID_PROXYHTTPUSERNAMECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPasswordStaticCtrl = new wxStaticText;
    m_pProxyHTTPPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer126->Add(m_pProxyHTTPPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxyHTTPPasswordCtrl = new wxTextCtrl;
    m_pProxyHTTPPasswordCtrl->Create( itemWizardPage121, ID_PROXYHTTPPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer126->Add(m_pProxyHTTPPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

#if 0
    m_pProxyHTTPAutodetectCtrl = new wxButton;
    m_pProxyHTTPAutodetectCtrl->Create( itemWizardPage121, ID_PROXYHTTPAUTODETECTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer125->Add(m_pProxyHTTPAutodetectCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
#endif

    m_pProxySOCKSDescriptionCtrl = new wxStaticBox(itemWizardPage121, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer137 = new wxStaticBoxSizer(m_pProxySOCKSDescriptionCtrl, wxVERTICAL);
    itemBoxSizer122->Add(itemStaticBoxSizer137, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer138 = new wxFlexGridSizer(3, 2, 0, 0);
    itemFlexGridSizer138->AddGrowableCol(1);
    itemStaticBoxSizer137->Add(itemFlexGridSizer138, 0, wxGROW|wxALL, 2);

    m_pProxySOCKSServerStaticCtrl = new wxStaticText;
    m_pProxySOCKSServerStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSServerStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxFlexGridSizer* itemFlexGridSizer140 = new wxFlexGridSizer(1, 3, 0, 0);
    itemFlexGridSizer140->AddGrowableCol(0);
    itemFlexGridSizer138->Add(itemFlexGridSizer140, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProxySOCKSServerCtrl = new wxTextCtrl;
    m_pProxySOCKSServerCtrl->Create( itemWizardPage121, ID_PROXYSOCKSSERVERCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_pProxySOCKSServerCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPortStaticCtrl = new wxStaticText;
    m_pProxySOCKSPortStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer140->Add(m_pProxySOCKSPortStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPortCtrl = new wxTextCtrl;
    m_pProxySOCKSPortCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPORTCTRL, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
    itemFlexGridSizer140->Add(m_pProxySOCKSPortCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSUsernameStaticCtrl = new wxStaticText;
    m_pProxySOCKSUsernameStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMESTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSUsernameStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSUsernameCtrl = new wxTextCtrl;
    m_pProxySOCKSUsernameCtrl->Create( itemWizardPage121, ID_PROXYSOCKSUSERNAMECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSUsernameCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPasswordStaticCtrl = new wxStaticText;
    m_pProxySOCKSPasswordStaticCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer138->Add(m_pProxySOCKSPasswordStaticCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_pProxySOCKSPasswordCtrl = new wxTextCtrl;
    m_pProxySOCKSPasswordCtrl->Create( itemWizardPage121, ID_PROXYSOCKSPASSWORDCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer138->Add(m_pProxySOCKSPasswordCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    // Set validators
    m_pProxyHTTPServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPServer) );
    m_pProxyHTTPPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxyHTTPPort) );
    m_pProxyHTTPUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPUsername) );
    m_pProxyHTTPPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxyHTTPPassword) );
    m_pProxySOCKSServerCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSServer) );
    m_pProxySOCKSPortCtrl->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_strProxySOCKSPort) );
    m_pProxySOCKSUsernameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSUsername) );
    m_pProxySOCKSPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strProxySOCKSPassword) );
////@end CErrProxyPage content construction
}
  
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrProxyPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
  
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CErrProxyPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (IS_ATTACHTOPROJECTWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    } else if (IS_ACCOUNTMANAGERWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERPROPERTIESPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrProxyPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrProxyPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CErrProxyPage bitmap retrieval
    return wxNullBitmap;
////@end CErrProxyPage bitmap retrieval
}
  
/*!
 * Get icon resources
 */
 
wxIcon CErrProxyPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CErrProxyPage icon retrieval
    return wxNullIcon;
////@end CErrProxyPage icon retrieval
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRPROXYCOMPLETIONPAGE
 */
 
void CErrProxyPage::OnPageChanged( wxWizardExEvent& event ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pProxyHTTPDescriptionCtrl);
    wxASSERT(m_pProxyHTTPServerStaticCtrl);
    wxASSERT(m_pProxyHTTPServerCtrl);
    wxASSERT(m_pProxyHTTPPortStaticCtrl);
    wxASSERT(m_pProxyHTTPPortCtrl);
    wxASSERT(m_pProxyHTTPUsernameStaticCtrl);
    wxASSERT(m_pProxyHTTPUsernameCtrl);
    wxASSERT(m_pProxyHTTPPasswordStaticCtrl);
    wxASSERT(m_pProxyHTTPPasswordCtrl);
#if 0
    wxASSERT(m_pProxyHTTPAutodetectCtrl);
#endif
    wxASSERT(m_pProxySOCKSDescriptionCtrl);
    wxASSERT(m_pProxySOCKSServerStaticCtrl);
    wxASSERT(m_pProxySOCKSServerCtrl);
    wxASSERT(m_pProxySOCKSPortStaticCtrl);
    wxASSERT(m_pProxySOCKSPortCtrl);
    wxASSERT(m_pProxySOCKSUsernameStaticCtrl);
    wxASSERT(m_pProxySOCKSUsernameCtrl);
    wxASSERT(m_pProxySOCKSPasswordStaticCtrl);
    wxASSERT(m_pProxySOCKSPasswordCtrl);

    if (event.GetDirection() == true) {
        // Moving from the previous page, update text and get state

        m_pTitleStaticCtrl->SetLabel(
            _("Proxy configuration")
        );
        m_pProxyHTTPDescriptionCtrl->SetLabel(
            _("HTTP proxy")
        );
        m_pProxyHTTPServerStaticCtrl->SetLabel(
            _("Server:")
        );
        m_pProxyHTTPPortStaticCtrl->SetLabel(
            _("Port:")
        );
        m_pProxyHTTPUsernameStaticCtrl->SetLabel(
            _("User Name:")
        );
        m_pProxyHTTPPasswordStaticCtrl->SetLabel(
            _("Password:")
        );
#if 0
        m_pProxyHTTPAutodetectCtrl->SetLabel(
            _("Autodetect")
        );
#endif
        m_pProxySOCKSDescriptionCtrl->SetLabel(
            _("SOCKS proxy")
        );
        m_pProxySOCKSServerStaticCtrl->SetLabel(
            _("Server:")
        );
        m_pProxySOCKSPortStaticCtrl->SetLabel(
            _("Port:")
        );
        m_pProxySOCKSUsernameStaticCtrl->SetLabel(
            _("User Name:")
        );
        m_pProxySOCKSPasswordStaticCtrl->SetLabel(
            _("Password:")
        );

        pDoc->GetProxyConfiguration();
        m_pProxyHTTPServerCtrl->SetValue(pDoc->proxy_info.http_server_name.c_str());
        m_pProxyHTTPUsernameCtrl->SetValue(pDoc->proxy_info.http_user_name.c_str());
        m_pProxyHTTPPasswordCtrl->SetValue(pDoc->proxy_info.http_user_passwd.c_str());

        strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
        m_pProxyHTTPPortCtrl->SetValue(strBuffer);

        m_pProxySOCKSServerCtrl->SetValue(pDoc->proxy_info.socks_server_name.c_str());
        m_pProxySOCKSUsernameCtrl->SetValue(pDoc->proxy_info.socks5_user_name.c_str());
        m_pProxySOCKSPasswordCtrl->SetValue(pDoc->proxy_info.socks5_user_passwd.c_str());

        strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
        m_pProxySOCKSPortCtrl->SetValue(strBuffer);

        m_pProxyHTTPServerCtrl->SetFocus();
    }

    Fit();
}
 
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ERRPROXYPAGE
 */

void CErrProxyPage::OnPageChanging( wxWizardExEvent& event ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString       strBuffer = wxEmptyString;
    int            iBuffer = 0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (event.GetDirection() == true) {
        // Moving to the next page, save state
        pDoc->proxy_info.use_http_proxy = (m_pProxyHTTPServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.http_server_name = m_pProxyHTTPServerCtrl->GetValue().c_str();
        pDoc->proxy_info.http_user_name = m_pProxyHTTPUsernameCtrl->GetValue().c_str();
        pDoc->proxy_info.http_user_passwd = m_pProxyHTTPPasswordCtrl->GetValue().c_str();

        strBuffer = m_pProxyHTTPPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->proxy_info.http_server_port = iBuffer;

        pDoc->proxy_info.use_socks_proxy = (m_pProxySOCKSServerCtrl->GetValue().Length() > 0);
        pDoc->proxy_info.socks_server_name = m_pProxySOCKSServerCtrl->GetValue().c_str();
        pDoc->proxy_info.socks5_user_name = m_pProxySOCKSUsernameCtrl->GetValue().c_str();
        pDoc->proxy_info.socks5_user_passwd = m_pProxySOCKSPasswordCtrl->GetValue().c_str();

        strBuffer = m_pProxySOCKSPortCtrl->GetValue();
        strBuffer.ToLong((long*)&iBuffer);
        pDoc->proxy_info.socks_server_port = iBuffer;

        pDoc->SetProxyConfiguration();
    }
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRPROXYCOMPLETIONPAGE
 */
 
void CErrProxyPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

