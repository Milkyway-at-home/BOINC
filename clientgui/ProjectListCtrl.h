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
//
#ifndef _WIZ_PROJECTLISTCTRL_H_
#define _WIZ_PROJECTLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ProjectListCtrl.cpp"
#endif


class CProjectListItemCtrl;
class CProjectListItemStaticCtrl;
class wxHyperLink;
class ProjectListCtrlEvent;
class ProjectListItemCtrlEvent;


/*!
 * CProjectListCtrl class declaration
 */

class CProjectListCtrl: public wxScrolledWindow
{    
    DECLARE_DYNAMIC_CLASS( CProjectListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListCtrl( );

    CProjectListCtrl( wxWindow* parent );

    /// Creation
    bool Create( wxWindow* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectListCtrl event handler declarations

    /// wxEVT_PROJECTLISTITEMCTRL_CLICKED event handler for window
    void OnItemClicked( ProjectListItemCtrlEvent& event );

////@end CProjectListCtrl event handler declarations

    /// Methods
    bool Append(
        wxString strTitle,
        wxString strURL
    );

private:
    wxBoxSizer* m_pMainSizer;
};


/*!
 * ProjectListCtrlEvent class declaration
 */

class ProjectListCtrlEvent : public wxNotifyEvent
{
public:
    ProjectListCtrlEvent( wxEventType evtType = wxEVT_NULL, wxString strName = wxEmptyString, wxString strURL = wxEmptyString ) :
      wxNotifyEvent( evtType, wxID_ANY )
    {
        m_strName = strName;
        m_strURL = strURL;
    } 

    wxString GetName() { return m_strName; };
    wxString GetURL() { return m_strURL; };

    virtual wxNotifyEvent* Clone() const { return new ProjectListCtrlEvent(*this); }

private:
    wxString m_strName;
    wxString m_strURL;

    DECLARE_DYNAMIC_CLASS(ProjectListCtrlEvent)
};

// ----------------------------------------------------------------------------
// macros for handling ProjectListCtrlEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED, 100000 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*ProjectListCtrlEventFunction)(ProjectListCtrlEvent&);

#define ProjectListCtrlEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(ProjectListCtrlEventFunction, &func)

#define wx__DECLARE_PROJECTLISTCTRLEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_PROJECTLISTCTRL_ ## evt, ProjectListCtrlEventHandler(fn))

#define EVT_PROJECTLISTCTRL_SELECTION_CHANGED(fn) wx__DECLARE_PROJECTLISTCTRLEVT(SELECTION_CHANGED, fn)


/*!
 * CProjectListItemCtrl class declaration
 */

class CProjectListItemCtrl: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CProjectListItemCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListItemCtrl( );

    CProjectListItemCtrl( wxWindow* parent );

    /// Creation
    bool Create( wxWindow* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProjectListItemCtrl event handler declarations

    /// wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
    void OnMouseEnterLeave( wxMouseEvent& event );

    /// wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
    void OnMouseClick( wxMouseEvent& event );

////@end CProjectListItemCtrl event handler declarations

    /// Methods
    wxString GetTitle() { return m_strTitle; };
    wxString GetURL() { return m_strURL; };

    bool SetTitle( wxString strTitle );
    bool SetURL( wxString strURL );

private:
    wxHyperLink*                m_pWebsiteCtrl;
    CProjectListItemStaticCtrl* m_pTitleStaticCtrl;
    wxString                    m_strTitle;
    wxString                    m_strURL;
    bool                        m_bLeftButtonDownDetected;
};


/*!
 * ProjectListItemCtrlEvent class declaration
 */

class ProjectListItemCtrlEvent : public wxNotifyEvent
{
public:
    ProjectListItemCtrlEvent( wxEventType evtType = wxEVT_NULL, int iControlId = 0 ) :
      wxNotifyEvent( evtType, wxID_ANY )
    {
        m_iControlId = iControlId;
    } 

    int GetControlId() { return m_iControlId; };

    virtual wxNotifyEvent* Clone() const { return new ProjectListItemCtrlEvent(*this); }

private:
    int m_iControlId;

    DECLARE_DYNAMIC_CLASS(ProjectListItemCtrlEvent)
};

// ----------------------------------------------------------------------------
// macros for handling ProjectListItemCtrlEvent
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE( wxEVT_PROJECTLISTITEMCTRL_CLICKED, 110000 )
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*ProjectListItemCtrlEventFunction)(ProjectListItemCtrlEvent&);

#define ProjectListItemCtrlEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(ProjectListItemCtrlEventFunction, &func)

#define wx__DECLARE_PROJECTLISTITEMCTRLEVT(evt, fn) \
    wx__DECLARE_EVT0(wxEVT_PROJECTLISTITEMCTRL_ ## evt, ProjectListItemCtrlEventHandler(fn))

#define EVT_PROJECTLISTITEMCTRL_CLICKED(fn) wx__DECLARE_PROJECTLISTITEMCTRLEVT(CLICKED, fn)


/*!
 * CProjectListItemStaticCtrl class declaration
 */

class CProjectListItemStaticCtrl: public wxStaticText
{    
    DECLARE_DYNAMIC_CLASS( CProjectListItemCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectListItemStaticCtrl();

    CProjectListItemStaticCtrl(
        wxWindow *parent,
        wxWindowID id,
        const wxString &label = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = 0,
        const wxString &name = _T("ProjectListItemStaticCtrl")
    );

    /// Creation
    bool Create (
        wxWindow *parent,
        wxWindowID id,
        const wxString &label = wxEmptyString,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long style = 0,
        const wxString &name = _T("ProjectListItemStaticCtrl")
    );

////@begin CProjectListItemCtrl event handler declarations

    /// wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
    void OnMouseEnterLeave( wxMouseEvent& event );

    /// wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
    void OnMouseClick( wxMouseEvent& event );

////@end CProjectListItemCtrl event handler declarations
};


#endif // _WIZ_PROJECTLISTCTRL_H_
