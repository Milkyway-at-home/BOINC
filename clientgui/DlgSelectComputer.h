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
#ifndef _DLGSELECTCOMPUTER_H_
#define _DLGSELECTCOMPUTER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "DlgSelectComputer.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_CDLGSELECTCOMPUTER_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_CDLGSELECTCOMPUTER_TITLE _("Select Computer")
#define SYMBOL_CDLGSELECTCOMPUTER_IDNAME ID_DIALOG
#define SYMBOL_CDLGSELECTCOMPUTER_SIZE wxSize(400, 300)
#define SYMBOL_CDLGSELECTCOMPUTER_POSITION wxDefaultPosition
#define ID_SELECTCOMPUTERNAME 10001
#define ID_SELECTCOMPUTERPASSWORD 10002
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CDlgSelectComputer class declaration
 */

class CDlgSelectComputer: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CDlgSelectComputer )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDlgSelectComputer( );
    CDlgSelectComputer( wxWindow* parent, wxWindowID id = SYMBOL_CDLGSELECTCOMPUTER_IDNAME, const wxString& caption = SYMBOL_CDLGSELECTCOMPUTER_TITLE, const wxPoint& pos = SYMBOL_CDLGSELECTCOMPUTER_POSITION, const wxSize& size = SYMBOL_CDLGSELECTCOMPUTER_SIZE, long style = SYMBOL_CDLGSELECTCOMPUTER_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDLGSELECTCOMPUTER_IDNAME, const wxString& caption = SYMBOL_CDLGSELECTCOMPUTER_TITLE, const wxPoint& pos = SYMBOL_CDLGSELECTCOMPUTER_POSITION, const wxSize& size = SYMBOL_CDLGSELECTCOMPUTER_SIZE, long style = SYMBOL_CDLGSELECTCOMPUTER_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDlgSelectComputer event handler declarations

////@end CDlgSelectComputer event handler declarations

////@begin CDlgSelectComputer member function declarations

    wxString GetComputerName() const { return m_strComputerName ; }
    void SetComputerName(wxString value) { m_strComputerName = value ; }

    wxString GetStrComputerPassword() const { return m_strComputerPassword ; }
    void SetStrComputerPassword(wxString value) { m_strComputerPassword = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDlgSelectComputer member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDlgSelectComputer member variables
    wxComboBox* m_ComputerNameCtrl;
    wxTextCtrl* m_ComputerPasswordCtrl;
    wxString m_strComputerName;
    wxString m_strComputerPassword;
////@end CDlgSelectComputer member variables
};

#endif
    // _DLGSELECTCOMPUTER_H_
