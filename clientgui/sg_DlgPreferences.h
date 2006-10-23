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


#ifndef _DLG_PREFERENCES_H_ 
#define _DLG_PREFERENCES_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_DlgPreferences.cpp"
#endif


class DirTraverserSkins : public wxDirTraverser
{
public:
    DirTraverserSkins(wxArrayString& skins) : m_skins(skins) { }

    virtual wxDirTraverseResult OnFile(const wxString& filename);
    virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname));

private:
    wxArrayString& m_skins;
};

class CDlgPreferences:public wxDialog
{
public:
    CDlgPreferences(wxWindow* parent, wxWindowID id = -1, const wxString& title = wxT(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("dialogBox"));

	wxBitmapButton *btnSave;
	wxBitmapButton *btnSaveSkin;
	wxBitmapButton *btnCancel;
	wxBitmapButton *btnClear;
	// Pointer control
	wxStaticText *lblPref;
	wxStaticText *lblModifySett;
	wxStaticText *lblDoWorkBtwn;
	wxStaticText *lblAnd1;
	wxStaticText *lblConnToIntBtwn;
	wxComboBox *cmbDWBtwnBgn;
	wxComboBox *cmbDWBtwnEnd;
	wxComboBox *cmbCTIBtwnBgn;
	wxComboBox *cmbCTIBtwnEnd;
	wxStaticText *lblAnd2;
	wxStaticText *lblUseNoMoreGB;
	wxComboBox *cmbUseNoMoreGB;
	wxStaticText *lblGB;
	wxStaticText *lblDWWCInUse;
	wxStaticText *lblDWACIdleFor;
	wxComboBox *cmbDWACIdleFor;
	wxStaticText *lblMinutes;
	wxComboBox *cmbDWWCInUse;
	
	wxStaticText *lblSkinXML;
	wxComboBox *cmbSkinPicker;
	//wxTextCtrl *tx30c;

	virtual ~CDlgPreferences();
	void initBefore();
	void WriteSettings();
	bool CheckIfInArray(wxString valArray[],wxString value,int size);
	void ReadSettings(GLOBAL_PREFS prefs);
	void CreateDialog();
	int ConvertToNumber(wxString num);
	wxString GetSkinName() const { return m_SkinName; }
	void SetSkinName(const wxString& skinName) { m_SkinName = skinName; }
	void OnPaint(wxPaintEvent& event); 
	void initAfter();

	DECLARE_EVENT_TABLE()

protected:
	wxString m_SkinName;
	wxString m_PrefIndicator;
	wxArrayString m_skinNames;
	GLOBAL_PREFS m_prefs;
	void OnEraseBackground(wxEraseEvent& event);
	void OnBtnClick(wxCommandEvent& event);
	void OnCmbSelected(wxCommandEvent& event);
	void VwXEvOnEraseBackground(wxEraseEvent& event);
	void VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap* bitMap,int opz);

private: 
	CStaticLine *lnMyTop;

//[win]end your code 
};

#endif  // end CDlgPreferences
