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
#pragma implementation "BOINCBaseView.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"

#include "res/boinc.xpm"


IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseView, wxPanel)


CBOINCBaseView::CBOINCBaseView() {}

CBOINCBaseView::CBOINCBaseView(wxNotebook* pNotebook) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    wxASSERT(pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bForceUpdateSelection = true;

    //
    // Setup View
    //
    m_pTaskPane = NULL;
    m_pListPane = NULL;

    SetName(GetViewName());

    SetAutoLayout(TRUE);
}


CBOINCBaseView::CBOINCBaseView(
    wxNotebook* pNotebook, wxWindowID iTaskWindowID, int iTaskWindowFlags, wxWindowID iListWindowID, int iListWindowFlags) :
    wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    wxASSERT(pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bForceUpdateSelection = true;

    //
    // Setup View
    //
    m_pTaskPane = NULL;
    m_pListPane = NULL;

    SetName(GetViewName());

    SetAutoLayout(TRUE);

    wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(2, 0, 0);
    wxASSERT(itemFlexGridSizer);

    itemFlexGridSizer->AddGrowableRow(0);
    itemFlexGridSizer->AddGrowableCol(1);
    
    m_pTaskPane = new CBOINCTaskCtrl(this, iTaskWindowID, iTaskWindowFlags);
    wxASSERT(m_pTaskPane);

    m_pListPane = new CBOINCListCtrl(this, iListWindowID, iListWindowFlags);
    wxASSERT(m_pListPane);

    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pListPane, 1, wxGROW|wxALL, 1);

    SetSizer(itemFlexGridSizer);

    Layout();
}


CBOINCBaseView::~CBOINCBaseView() {}


// The name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString& CBOINCBaseView::GetViewName() {
    static wxString strViewName(wxT("Undefined"));
    return strViewName;
}


// The user friendly name of the view.
//   If it has not been defined by the view "Undefined" is returned.
//
wxString& CBOINCBaseView::GetViewDisplayName() {
    static wxString strViewName(wxT("Undefined"));
    return strViewName;
}


// The user friendly icon of the view.
//   If it has not been defined by the view the BOINC icon is returned.
//
const char** CBOINCBaseView::GetViewIcon() {
    wxASSERT(boinc_xpm);
    return boinc_xpm;
}


// The rate at which the view is refreshed.
//   If it has not been defined by the view 1 second is retrned.
//
const int CBOINCBaseView::GetViewRefreshRate() {
    return 1;
}


bool CBOINCBaseView::FireOnSaveState(wxConfigBase* pConfig) {
    return OnSaveState(pConfig);
}


bool CBOINCBaseView::FireOnRestoreState(wxConfigBase* pConfig) {
    return OnRestoreState(pConfig);
}


int CBOINCBaseView::GetListRowCount() {
    wxASSERT(m_pListPane);
    return m_pListPane->GetItemCount();
}


void CBOINCBaseView::FireOnListRender(wxTimerEvent& event) {
    OnListRender(event);
}


void CBOINCBaseView::FireOnListSelected(wxListEvent& event) {
    OnListSelected(event);
}


void CBOINCBaseView::FireOnListDeselected(wxListEvent& event) {
    OnListDeselected(event);
}


wxString CBOINCBaseView::FireOnListGetItemText(long item, long column) const {
    return OnListGetItemText(item, column);
}


int CBOINCBaseView::FireOnListGetItemImage(long item) const {
    return OnListGetItemImage(item);
}


wxListItemAttr* CBOINCBaseView::FireOnListGetItemAttr(long item) const {
    return OnListGetItemAttr(item);
}


void CBOINCBaseView::OnListRender(wxTimerEvent& event) {
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pListPane);

        int iDocCount = GetDocCount();
        int iCacheCount = GetCacheCount();
        if (iDocCount != iCacheCount) {
            if (0 >= iDocCount) {
                EmptyCache();
                m_pListPane->DeleteAllItems();
            } else {
                int iIndex = 0;
                int iReturnValue = -1;
                if (iDocCount > iCacheCount) {
                    for (iIndex = 0; iIndex < (iDocCount - iCacheCount); iIndex++
                    ) {
                        iReturnValue = AddCacheElement();
                        wxASSERT(!iReturnValue);
                    }
                    wxASSERT(GetDocCount() == GetCacheCount());
                } else {
                    for (iIndex = 0; iIndex < (iCacheCount - iDocCount); iIndex++
                    ) {
                        iReturnValue = RemoveCacheElement();
                        wxASSERT(!iReturnValue);
                    }
                    wxASSERT(GetDocCount() == GetCacheCount());
                }

                m_pListPane->SetItemCount(iDocCount);
            }
        }

        if (iDocCount) {
            SyncronizeCache();
        }

        if (_EnsureLastItemVisible() && (iDocCount != iCacheCount)) {
            m_pListPane->EnsureVisible(iDocCount - 1);
        }

        // If no item has been selected yet, select the first item.
#ifdef __WXMSW__
         if ((m_pListPane->GetSelectedItemCount() == 0) &&
            (m_pListPane->GetItemCount() >= 1)) {

            long desiredstate = wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED;
            m_pListPane->SetItemState(0, desiredstate, desiredstate);
        }
#else
         if ((m_pListPane->GetFirstSelected() < 0) &&
            (m_pListPane->GetItemCount() >= 1))
            m_pListPane->SetItemState(0, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED, 
                                            wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED);
#endif

        UpdateSelection();

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}


bool CBOINCBaseView::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    if (!m_pListPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}


bool CBOINCBaseView::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    if (!m_pListPane->OnRestoreState(pConfig)) {
        return false;
    }
    return true;
}


void CBOINCBaseView::OnListSelected(wxListEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListSelected - Function Begin"));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListSelected - Function End"));
}


void CBOINCBaseView::OnListDeselected(wxListEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListDeselected - Function Begin"));

    m_bForceUpdateSelection = true;
    UpdateSelection();
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListDeselected - Function End"));
}


wxString CBOINCBaseView::OnListGetItemText(long WXUNUSED(item), long WXUNUSED(column)) const {
    return wxString(wxT("Undefined"));
}


int CBOINCBaseView::OnListGetItemImage(long WXUNUSED(item)) const {
    return -1;
}


wxListItemAttr* CBOINCBaseView::OnListGetItemAttr(long WXUNUSED(item)) const {
    return NULL;
}


void CBOINCBaseView::OnGridSelectCell( wxGridEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectCell - Function Begin"));

	m_bForceUpdateSelection = true;
    UpdateSelection();
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectCell - Function End"));
}

void CBOINCBaseView::OnGridSelectRange( wxGridRangeSelectEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectRange - Function Begin"));

    m_bForceUpdateSelection = true;
    UpdateSelection();
	event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectRange - Function End"));
}


int CBOINCBaseView::GetDocCount() {
    return 0;
}


wxString CBOINCBaseView::OnDocGetItemText(long WXUNUSED(item), long WXUNUSED(column)) const {
    return wxString(wxT("Undefined"));
}


wxString CBOINCBaseView::OnDocGetItemImage(long WXUNUSED(item)) const {
    return wxString(wxT("Undefined"));
}


wxString CBOINCBaseView::OnDocGetItemAttr(long WXUNUSED(item)) const {
    return wxString(wxT("Undefined"));
}


int CBOINCBaseView::AddCacheElement() {
    return -1;
}

    
int CBOINCBaseView::EmptyCache() {
    return -1;
}


int CBOINCBaseView::GetCacheCount() {
    return -1;
}


int CBOINCBaseView::RemoveCacheElement() {
    return -1;
}


int CBOINCBaseView::SyncronizeCache() {
    int         iRowIndex        = 0;
    int         iRowTotal        = 0;
    int         iColumnIndex     = 0;
    int         iColumnTotal     = 0;
    wxString    strDocumentText  = wxEmptyString;
    wxString    strListPaneText  = wxEmptyString;
    bool        bNeedRefreshData = false;

    iRowTotal = GetDocCount();
    iColumnTotal = m_pListPane->GetColumnCount();

    for (iRowIndex = 0; iRowIndex < iRowTotal; iRowIndex++) {
        bNeedRefreshData = false;

        for (iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++) {
            strDocumentText.Empty();
            strListPaneText.Empty();

            strDocumentText = OnDocGetItemText(iRowIndex, iColumnIndex);
            strListPaneText = OnListGetItemText(iRowIndex, iColumnIndex);

            if (!strDocumentText.IsSameAs(strListPaneText)) {
                if (0 != UpdateCache(iRowIndex, iColumnIndex, strDocumentText)) {
                    wxASSERT(FALSE);
                }
                bNeedRefreshData = true;
            }
        }

        if (bNeedRefreshData) {
            m_pListPane->RefreshItem(iRowIndex);
        }
    }

    return 0;
}


int CBOINCBaseView::UpdateCache(
    long WXUNUSED(item), long WXUNUSED(column), wxString& WXUNUSED(strNewData)
) {
    return -1;
}


void CBOINCBaseView::EmptyTasks() {
    unsigned int i;
    unsigned int j;
    for (i=0; i<m_TaskGroups.size(); i++) {
        for (j=0; j<m_TaskGroups[i]->m_Tasks.size(); j++) {
            delete m_TaskGroups[i]->m_Tasks[j];
        }
        m_TaskGroups[i]->m_Tasks.clear();
        delete m_TaskGroups[i];
    }
    m_TaskGroups.clear();
}

    
void CBOINCBaseView::PreUpdateSelection(){
}


void CBOINCBaseView::UpdateSelection(){
}


void CBOINCBaseView::PostUpdateSelection(){
    wxASSERT(m_pTaskPane);
    m_pTaskPane->UpdateControls();
    Layout();
}


void CBOINCBaseView::UpdateWebsiteSelection(long lControlGroup, PROJECT* project){
    unsigned int        i;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;

    wxASSERT(m_pTaskPane);
    wxASSERT(m_pListPane);

    // Update the websites list
    //
    if (m_bForceUpdateSelection) {
        if (m_TaskGroups.size() > 1) {

            // Delete task group, objects, and controls.
            pGroup = m_TaskGroups[lControlGroup];

            m_pTaskPane->DeleteTaskGroupAndTasks(pGroup);
            for (i=0; i<pGroup->m_Tasks.size(); i++) {
                delete pGroup->m_Tasks[i];
            }
            pGroup->m_Tasks.clear();
            delete pGroup;

            pGroup = NULL;

            m_TaskGroups.erase( m_TaskGroups.begin() + 1 );
        }

        // If something is selected create the tasks and controls
        if (m_pListPane->GetSelectedItemCount()) {
            if (project) {
                // Create the web sites task group
  	            pGroup = new CTaskItemGroup( _("Web sites") );
	            m_TaskGroups.push_back( pGroup );

                // Default project url
                pItem = new CTaskItem(
                    wxString(project->project_name.c_str(), wxConvUTF8), 
                    wxT(""), 
                    wxString(project->master_url.c_str(), wxConvUTF8),
                    ID_TASK_PROJECT_WEB_PROJDEF_MIN
                );
                pGroup->m_Tasks.push_back(pItem);


                // Project defined urls
                for (i=0;(i<project->gui_urls.size())&&(i<=ID_TASK_PROJECT_WEB_PROJDEF_MAX);i++) {
                    pItem = new CTaskItem(
                        wxGetTranslation(wxString(project->gui_urls[i].name.c_str(), wxConvUTF8)),
                        wxGetTranslation(wxString(project->gui_urls[i].description.c_str(), wxConvUTF8)),
                        wxString(project->gui_urls[i].url.c_str(), wxConvUTF8),
                        ID_TASK_PROJECT_WEB_PROJDEF_MIN + 1 + i
                    );
                    pGroup->m_Tasks.push_back(pItem);
                }
            }
        }

        m_bForceUpdateSelection = false;
    }

}


bool CBOINCBaseView::_EnsureLastItemVisible() {
    return EnsureLastItemVisible();
}


bool CBOINCBaseView::EnsureLastItemVisible() {
    return false;
}


void CBOINCBaseView::append_to_status(wxString& existing, const wxChar* additional) {
    if (existing.size() == 0) {
        existing = additional;
    } else {
        existing = existing + wxT(", ") + additional;
    }
}


// HTML Entity Conversion:
// http://www.webreference.com/html/reference/character/
// Completed: The ISO Latin 1 Character Set
//
wxString CBOINCBaseView::HtmlEntityEncode(wxString strRaw) {
	wxString strEncodedHtml(strRaw);

    strEncodedHtml.Replace(wxT("&"),  wxT("&amp;"),    true);
    strEncodedHtml.Replace(wxT("\""), wxT("&quot;"),   true);
    strEncodedHtml.Replace(wxT("<"),  wxT("&lt;"),     true);
    strEncodedHtml.Replace(wxT(">"),  wxT("&gt;"),     true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sbquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&fnof;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&bdquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&hellip;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&dagger;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Dagger;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Scaron;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&OElig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&lsquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&rsquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ldquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&rdquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&bull;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ndash;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&mdash;"),  true);
    strEncodedHtml.Replace(wxT("��~"),  wxT("&tilde;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&trade;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&scaron;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&oelig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Yuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iexcl;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&cent;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&pound;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&curren;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&yen;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&brvbar;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sect;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&uml;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&copy;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ordf;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&laquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&not;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&reg;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&macr;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&deg;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&plusmn;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sup2;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sup3;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&acute;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&micro;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&para;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&middot;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&cedil;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sup1;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ordm;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&raquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&frac14;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&frac12;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&frac34;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iquest;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Agrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Aacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Acirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Atilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Auml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Aring;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&AElig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ccedil;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Egrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Eacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ecirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Euml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Igrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Iacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Icirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Iuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ETH;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ntilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ograve;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Oacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ocirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Otilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ouml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&times;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Oslash;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ugrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Uacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ucirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Uuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Yacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&THORN;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&szlig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&agrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&aacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&acirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&atilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&auml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&aring;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&aelig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ccedil;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&egrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&eacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ecirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&euml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&igrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&icirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&eth;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ntilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ograve;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&oacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ocirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&otilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ouml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&divide;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&oslash;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ugrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&uacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ucirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&uuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&yacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&thorn;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&yuml;"),   true);

    return strEncodedHtml;
}

wxString CBOINCBaseView::HtmlEntityDecode(wxString strRaw) {
	wxString strDecodedHtml(strRaw);

    if (0 <= strDecodedHtml.Find(wxT("&"))) {
        strDecodedHtml.Replace(wxT("&amp;"),    wxT("&"),  true);
        strDecodedHtml.Replace(wxT("&quot;"),   wxT("\""), true);
        strDecodedHtml.Replace(wxT("&lt;"),     wxT("<"),  true);
        strDecodedHtml.Replace(wxT("&gt;"),     wxT(">"),  true);
        strDecodedHtml.Replace(wxT("&sbquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&fnof;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&bdquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&hellip;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&dagger;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Dagger;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Scaron;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&OElig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&lsquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&rsquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ldquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&rdquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&bull;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ndash;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&mdash;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&tilde;"),  wxT("��~"),  true);
        strDecodedHtml.Replace(wxT("&trade;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&scaron;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&oelig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Yuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iexcl;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&cent;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&pound;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&curren;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&yen;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&brvbar;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sect;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&uml;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&copy;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ordf;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&laquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&not;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&reg;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&macr;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&deg;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&plusmn;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sup2;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sup3;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&acute;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&micro;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&para;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&middot;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&cedil;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sup1;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ordm;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&raquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&frac14;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&frac12;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&frac34;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iquest;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Agrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Aacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Acirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Atilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Auml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Aring;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&AElig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ccedil;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Egrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Eacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ecirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Euml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Igrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Iacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Icirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Iuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ETH;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ntilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ograve;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Oacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ocirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Otilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ouml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&times;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Oslash;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ugrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Uacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ucirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Uuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Yacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&THORN;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&szlig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&agrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&aacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&acirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&atilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&auml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&aring;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&aelig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ccedil;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&egrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&eacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ecirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&euml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&igrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&icirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&eth;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ntilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ograve;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&oacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ocirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&otilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ouml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&divide;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&oslash;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ugrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&uacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ucirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&uuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&yacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&thorn;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&yuml;"),   wxT("�"),  true);
    }

	return strDecodedHtml;
}


const char *BOINC_RCSID_0a1bd38a5a = "$Id$";
