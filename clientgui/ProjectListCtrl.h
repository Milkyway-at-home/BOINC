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
#ifndef _WIZ_PROJECTLISTCTRL_H_
#define _WIZ_PROJECTLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ProjectListCtrl.cpp"
#endif


/*!
 * CProjectListItem class declaration
 */

class CProjectListItem: public wxObject
{    
    DECLARE_DYNAMIC_CLASS( CProjectListItem )
public:

    wxString GetURL() const { return m_strURL ; }
    void SetURL(wxString value) { m_strURL = value ; }

    wxString GetTitle() const { return m_strTitle ; }
    void SetTitle(wxString value) { m_strTitle = value ; }

    wxString GetOrganization() const { return m_strOrganization ; }
    void SetOrganization(wxString value) { m_strOrganization = value ; }

    wxString GetDescription() const { return m_strDescription ; }
    void SetDescription(wxString value) { m_strDescription = value ; }

    wxString GetCategory() const { return m_strCategory ; }
    void SetCategory(wxString value) { m_strCategory = value ; }

    bool IsPlatformSupported() const { return m_bSupported ; }
    void SetPlatformSupported(bool value) { m_bSupported = value ; }

private:
    wxString m_strURL;
    wxString m_strTitle;
    wxString m_strOrganization;
    wxString m_strDescription;
    wxString m_strCategory;
    bool m_bSupported;
};


#ifdef wxUSE_ACCESSIBILITY

class CProjectListCtrlAccessible: public wxWindowAccessible
{
public:
    CProjectListCtrlAccessible(wxWindow* win): wxWindowAccessible(win) {}

    virtual wxAccStatus GetName(int childId, wxString* name);
    virtual wxAccStatus HitTest(const wxPoint& pt, int* childId, wxAccessible** childObject);
    virtual wxAccStatus GetLocation(wxRect& rect, int elementId);
    virtual wxAccStatus Navigate(wxNavDir navDir, int fromId, int* toId, wxAccessible** toObject);
    virtual wxAccStatus GetChildCount(int* childCount);
    virtual wxAccStatus DoDefaultAction(int childId);
    virtual wxAccStatus GetDefaultAction(int childId, wxString* actionName);
    virtual wxAccStatus GetDescription(int childId, wxString* description);
    virtual wxAccStatus GetRole(int childId, wxAccRole* role);
    virtual wxAccStatus GetState(int childId, long* state);
    virtual wxAccStatus Select(int childId, wxAccSelectionFlags selectFlags);
    virtual wxAccStatus GetSelections(wxVariant* selections);
};

#endif


/*!
 * CProjectListCtrl class declaration
 */

class CProjectListCtrl: public wxHtmlListBox
{    
    DECLARE_DYNAMIC_CLASS( CProjectListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListCtrl( );

    CProjectListCtrl( wxWindow* parent );

    /// Creation
    bool Create( wxWindow* parent );

////@begin CProjectListCtrl event handler declarations

    void OnSelected( wxCommandEvent& event );
    void OnClicked( wxHtmlCellEvent& event );
    void OnDClicked( wxCommandEvent& event );
    void OnLinkClicked( wxHtmlLinkEvent& event );
    void OnHover( wxHtmlCellEvent& event );

////@end CProjectListCtrl event handler declarations

    virtual wxString OnGetItem(size_t i) const;

    /// Methods
    bool Append(
        wxString strURL,
        wxString strTitle,
        wxString strDescription,
        bool bSupported
    );

    CProjectListItem* GetItem( 
        int iIndex
    );

    wxCoord GetTotalClientHeight();

private:
    std::vector<CProjectListItem*> m_Items;
};


/*!
 * ProjectListCtrlEvent class declaration
 */

class ProjectListCtrlEvent : public wxNotifyEvent
{
public:
    ProjectListCtrlEvent( wxEventType evtType = wxEVT_NULL, wxString strName = wxEmptyString, wxString strURL = wxEmptyString, bool bSupported = false ) :
      wxNotifyEvent( evtType, wxID_ANY )
    {
        m_strName = strName;
        m_strURL = strURL;
        m_bSupported = bSupported;
    } 

    wxString GetName() { return m_strName; };
    wxString GetURL() { return m_strURL; };
    bool IsSupported() { return m_bSupported; };

    virtual wxNotifyEvent* Clone() const { return new ProjectListCtrlEvent(*this); }

private:
    wxString m_strName;
    wxString m_strURL;
    bool m_bSupported;

    DECLARE_DYNAMIC_CLASS(ProjectListCtrlEvent)
};

// ----------------------------------------------------------------------------
// macros for handling ProjectListCtrlEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_CHANGE, 100000 )
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLIST_ITEM_DISPLAY, 100001 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*ProjectListCtrlEventFunction)(ProjectListCtrlEvent&);

#define ProjectListCtrlEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(ProjectListCtrlEventFunction, &func)

#define wx__DECLARE_PROJECTLISTEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_PROJECTLIST_ ## evt, ProjectListCtrlEventHandler(fn))

#define EVT_PROJECTLIST_ITEM_CHANGE(fn) wx__DECLARE_PROJECTLISTEVT(ITEM_CHANGE, fn)
#define EVT_PROJECTLIST_ITEM_DISPLAY(fn) wx__DECLARE_PROJECTLISTEVT(ITEM_DISPLAY, fn)


#endif // _WIZ_PROJECTLISTCTRL_H_
