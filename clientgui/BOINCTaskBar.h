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

#ifndef _BOINCTASKBAR_H_
#define _BOINCTASKBAR_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskBar.cpp"
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef __WXMSW__
#include "msw/taskbarex.h"
#else
#define wxTaskBarIconEx     wxTaskBarIcon
#endif

class CTaskbarEvent;


class CTaskBarIcon : public wxTaskBarIconEx {
public:
    CTaskBarIcon(wxString title, wxIcon* icon, wxIcon* iconDisconnected, wxIcon* iconSnooze);
    ~CTaskBarIcon();

    void OnOpenWebsite(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSuspend(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

#ifdef __WXMSW__
    void OnShutdown(wxTaskBarIconExEvent& event);
#endif

    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnRefresh(wxTimerEvent& event);

#ifdef __WXMSW__
    void OnMouseMove(wxTaskBarIconEvent& event);
#endif
    void OnLButtonDClick(wxTaskBarIconEvent& event);

#ifdef __WXMSW__
    void OnContextMenu(wxTaskBarIconExEvent& event);
#else
    void OnContextMenu(wxTaskBarIconEvent& event);
#endif

    void OnRButtonDown(wxTaskBarIconEvent& event);
    void OnRButtonUp(wxTaskBarIconEvent& event);

    void OnReloadSkin(CTaskbarEvent& event);

    void FireReloadSkin();

    wxMenu *BuildContextMenu();
    void AdjustMenuItems(wxMenu* menu);
    void ResetSnoozeState();

#ifdef __APPLE__
    wxMenu *CreatePopupMenu();
    bool SetIcon(const wxIcon& icon, const wxString& tooltip);
#endif

#ifndef __WXMSW__
    inline bool IsBalloonsSupported() {
        return false;
    }
#endif

    wxIcon     m_iconTaskBarNormal;
    wxIcon     m_iconTaskBarDisconnected;
    wxIcon     m_iconTaskBarSnooze;
    wxString   m_strDefaultTitle;

private:
    wxDateTime m_dtLastHoverDetected;
    wxDateTime m_dtLastBalloonDisplayed;
    wxDateTime m_dtSnoozeStartTime;

    wxTimer*   m_pRefreshTimer;

    bool       m_bMouseButtonPressed;

    int        m_iPreviousActivityMode;
    int        m_iPreviousNetworkMode;

    void       ResetTaskBar();

    void       CreateContextMenu();
    
    DECLARE_EVENT_TABLE()

};


class CTaskbarEvent : public wxEvent
{
public:
    CTaskbarEvent(wxEventType evtType, CTaskBarIcon *taskbar)
        : wxEvent(-1, evtType)
        {
            SetEventObject(taskbar);
        }

    CTaskbarEvent(wxEventType evtType, CTaskBarIcon *taskbar, wxString message)
        : wxEvent(-1, evtType), m_message(message)
        {
            SetEventObject(taskbar);
        }

    virtual wxEvent *Clone() const { return new CTaskbarEvent(*this); }

    wxString                m_message;
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE( wxEVT_TASKBAR_RELOADSKIN, 10100 )
END_DECLARE_EVENT_TYPES()

#define EVT_TASKBAR_RELOADSKIN(fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASKBAR_RELOADSKIN, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


#endif

