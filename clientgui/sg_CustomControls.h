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

#ifndef _CUSTOMCONTROLS_H_
#define _CUSTOMCONTROLS_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_CustomControls.cpp"
#endif


class CTransparentStaticLine : public wxPanel 
{ 
    DECLARE_DYNAMIC_CLASS (CTransparentStaticLine)

public: 
    CTransparentStaticLine();
    CTransparentStaticLine(
        wxWindow* parent, 
        wxWindowID id, 
        const wxPoint& pos = wxDefaultPosition, 
        const wxSize& size = wxDefaultSize,
        long style = 0, 
        const wxString& name= wxStaticTextNameStr
    );

    bool Create(
        wxWindow* parent, 
        wxWindowID id, 
        const wxPoint& pos = wxDefaultPosition, 
        const wxSize& size = wxDefaultSize,
        long style = 0, 
        const wxString& name= wxStaticTextNameStr
    );

    wxColour GetLineColor() const { return m_LineColor ; }
    void     SetLineColor(wxColour value) { m_LineColor = value ; }

    virtual bool HasTransparentBackground() { return true; };

    void     OnPaint(wxPaintEvent& event);

    DECLARE_EVENT_TABLE()

private:
    wxColour m_LineColor;
}; 


class CTransparentStaticText : public wxStaticText
{ 
    DECLARE_DYNAMIC_CLASS (CTransparentStaticText)

public:
    CTransparentStaticText();
    CTransparentStaticText(
        wxWindow* parent, 
        wxWindowID id, 
        const wxString& label, 
        const wxPoint& pos = wxDefaultPosition, 
        const wxSize& size = wxDefaultSize,
        long style = 0, 
        const wxString& name= wxStaticTextNameStr
    );

    bool Create(
        wxWindow* parent, 
        wxWindowID id, 
        const wxString& label, 
        const wxPoint& pos = wxDefaultPosition, 
        const wxSize& size = wxDefaultSize,
        long style = 0, 
        const wxString& name= wxStaticTextNameStr
    );

    virtual bool HasTransparentBackground() { return true; };
    virtual bool AssociateWindow(wxWindow* pWnd);

    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnMouse(wxMouseEvent& event);
    virtual void OnFocus(wxFocusEvent& event);
    // event handlers used by owner-drawn checkbox

    DECLARE_EVENT_TABLE()
private:
    wxWindow*   m_pWnd;
}; 


#endif 

