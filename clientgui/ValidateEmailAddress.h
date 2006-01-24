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

#ifndef _VALIDATEEMAILADDRESS_H_
#define _VALIDATEEMAILADDRESS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ValidateEmailAddress.cpp"
#endif


class CValidateEmailAddress : public wxValidator
{
    DECLARE_DYNAMIC_CLASS( CValidateEmailAddress )

public:

    CValidateEmailAddress( wxString *val = 0 );
    CValidateEmailAddress( const CValidateEmailAddress& val );

    ~CValidateEmailAddress();

    virtual wxObject* Clone() const { return new CValidateEmailAddress(*this); }
    virtual bool      Copy( const CValidateEmailAddress& val );

    virtual bool      Validate(wxWindow *parent);
    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();

protected:
    wxString*         m_stringValue;
    wxString          m_errormsg;

    bool              wxIsAlphaNumeric(const wxString& val);
    virtual bool      CheckValidator() const;

};


#endif

