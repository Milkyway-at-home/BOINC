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

#ifndef _SKIN_CLASS
#define _SKIN_CLASS

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_SkinClass.cpp"
#endif

class SkinClass
{ 
public: 
	    static SkinClass* Instance();

        // Getters
		wxString GetAppBg() { return m_appBg; }
		wxColour GetAppBgCol() { return GetColorFromStr(m_appBgCol); }
		wxString GetBtnPrefer() const { return m_btnPrefer; }
		wxString GetBtnAddProj() const { return m_btnAddProj; }
		wxString GetBtnAdvView() const { return m_btnAdvView; }
        wxString GetBtnPlay() const { return m_btnPlay; }
        wxString GetBtnPause() const { return m_btnPause; }
		wxString GetBtnMessages() const { return m_btnMessages; }
		wxString GetBtnOpen() const { return m_btnOpen; }
        wxString GetBtnSave() const { return m_btnSave; }
		wxString GetBtnCancel() const { return m_btnCancel; }
		wxString GetBtnLeftArr() const { return m_btnLeftArr; }
		wxString GetBtnRightArr() const { return m_btnRightArr; }
		wxString GetBtnLeftArrClick() const { return m_btnLeftArrClick; }
		wxString GetBtnRightArrClick() const { return m_btnRightArrClick; }
        wxString GetBtnExpand() const { return m_btnExpand; }
		wxString GetBtnCollapse() const { return m_btnCollapse; }
		wxString GetBtnShowGraphic() const { return m_btnShowGraphic; }
		wxString GetBtnExpandClick() const { return m_btnExpandClick; }
		wxString GetBtnCollapseClick() const { return m_btnCollapseClick; }
        wxString GetBtnShowGraphicClick() const { return m_btnShowGraphicClick; }
        
		wxString GetDlgPrefBg() { return m_dlgPrefBg; }
		wxString GetDlgMessBg() { return m_dlgMessBg; }

		wxColour GetGaugeFgCol() { return GetColorFromStr(m_gaugeFgCol); }
		wxColour GetGaugeBgCol() { return GetColorFromStr(m_gaugeBgCol); }
	

		wxColour GetTabFromColAc() { return GetColorFromStr(m_tabFromColAc); }
		wxColour GetTabToColAc() { return GetColorFromStr(m_tabToColAc); }
		wxColour GetTabBrdColAc() { return GetColorFromStr(m_tabBrdColAc); }
		wxColour GetTabFromColIn() { return GetColorFromStr(m_tabFromColIn); }
		wxColour GetTabToColIn() { return GetColorFromStr(m_tabToColIn); }
		wxColour GetTabBrdColIn() { return GetColorFromStr(m_tabBrdColIn); }
        
		wxString GetIcnWorking() const { return m_icnWorking; }
        wxString GetIcnSleeping() const { return m_icnSleeping; }
        wxString GetIcnWorkingWkUnit() const { return m_icnWorkingWkUnit; }
        wxString GetIcnSleepingWkUnit() const { return m_icnSleepingWkUnit; }
        wxString GetDefaultPrjIcn() const { return m_defaultPrjIcn; }
        wxString GetDefaultStatIcn() const { return m_defaultStatIcn; }
        wxString GetAnimationBG() const { return m_animBg; }
        wxString GetAnimationFile() const { return m_animFile; }
        
		wxString GetSkinName() const { return m_skinName; }
		wxString GetSkinsFolder() const { return m_skinsFolder; }

        // Setters
		void SetAppBg(const wxString imgsrc) { m_appBg = imgsrc; }
		void SetAppBgCol(const wxString& clr) { m_appBgCol = clr; }
		void SetBtnPrefer(const wxString& imgsrc) { m_btnPrefer = imgsrc; }
		void SetBtnAddProj(const wxString& imgsrc) { m_btnAddProj = imgsrc; }
		void SetBtnAdvView(const wxString& imgsrc) { m_btnAdvView = imgsrc; }
		void SetBtnPlay(const wxString& imgsrc) { m_btnPlay = imgsrc; }
		void SetBtnPause(const wxString& imgsrc) { m_btnPause = imgsrc; }
		void SetBtnMessages(const wxString& imgsrc) { m_btnMessages = imgsrc; }
		void SetBtnOpen(const wxString& imgsrc) { m_btnOpen = imgsrc; }
		void SetBtnSave(const wxString& imgsrc) { m_btnSave = imgsrc; }
		void SetBtnCancel(const wxString& imgsrc) { m_btnCancel = imgsrc; }
		void SetBtnLeftArr(const wxString& imgsrc) { m_btnLeftArr = imgsrc; }
		void SetBtnRightArr(const wxString& imgsrc) { m_btnRightArr = imgsrc; }
		void SetBtnLeftArrClick(const wxString& imgsrc) { m_btnLeftArrClick = imgsrc; }
		void SetBtnRightArrClick(const wxString& imgsrc) { m_btnRightArrClick = imgsrc; }
		void SetBtnExpand(const wxString& imgsrc) { m_btnExpand = imgsrc; }
		void SetBtnCollapse(const wxString& imgsrc)  { m_btnCollapse = imgsrc; }
		void SetBtnShowGraphic(const wxString& imgsrc)  { m_btnShowGraphic = imgsrc; }
		void SetBtnExpandClick(const wxString& imgsrc)  { m_btnExpandClick = imgsrc; }
		void SetBtnCollapseClick(const wxString& imgsrc)  { m_btnCollapseClick = imgsrc; }
        void SetBtnShowGraphicClick(const wxString& imgsrc)  { m_btnShowGraphicClick = imgsrc; }
		
		void SetDlgPrefBg(const wxString& imgsrc) { m_dlgPrefBg = imgsrc; }
		void SetDlgMessBg(const wxString& imgsrc) { m_dlgMessBg = imgsrc; }

		void SetGaugeFgCol(const wxString& clr) { m_gaugeFgCol = clr; }
		void SetGaugeBgCol(const wxString& clr) { m_gaugeBgCol = clr; }

		void SetTabFromColAc(const wxString& clr) { m_tabFromColAc = clr; }
		void SetTabToColAc(const wxString& clr) { m_tabToColAc = clr; }
		void SetTabBrdColAc(const wxString& clr) { m_tabBrdColAc = clr; }
		void SetTabFromColIn(const wxString& clr) { m_tabFromColIn = clr; }
		void SetTabToColIn(const wxString& clr) { m_tabToColIn = clr; }
		void SetTabBrdColIn(const wxString& clr) { m_tabBrdColIn = clr; }

		void SetIcnWorking(const wxString& imgsrc) { m_icnWorking = imgsrc; }
		void SetIcnSleeping(const wxString& imgsrc) { m_icnSleeping = imgsrc; }
		void SetIcnWorkingWkUnit(const wxString& imgsrc) { m_icnWorkingWkUnit = imgsrc; }
		void SetIcnSleepingWkUnit(const wxString& imgsrc) { m_icnSleepingWkUnit = imgsrc; }
		void SetDefaultPrjIcn(const wxString& imgsrc) { m_defaultPrjIcn = imgsrc; }
		void SetDefaultStatIcn(const wxString& imgsrc) { m_defaultStatIcn = imgsrc; }
		void SetAnimationBg(const wxString& imgsrc) { m_animBg = imgsrc; }
		void SetAnimationFile(const wxString& imgsrc) { m_animFile = imgsrc; }
		
		void SetSkinName(const wxString& name) { m_skinName = name; }
		void SetSkinsFolder(const wxString& fldr) { m_skinsFolder = fldr; }

private: 
	    /// Constructors
	    SkinClass();
		wxColour GetColorFromStr(wxString col);
	    // Bg
	    wxString m_appBg; 
		wxString m_appBgCol;
        //Dialogs
		wxString m_dlgPrefBg;
		wxString m_dlgMessBg;
		//gauge colors
        wxString m_gaugeFgCol; 
		wxString m_gaugeBgCol;
		//notebook colors
		//active tab
        wxString m_tabFromColAc; 
		wxString m_tabToColAc; 
		wxString m_tabBrdColAc; 
		//inactive tab
		wxString m_tabFromColIn; 
		wxString m_tabToColIn; 
		wxString m_tabBrdColIn; 
		// Btns
		wxString m_btnPrefer;
		wxString m_btnAddProj;
		wxString m_btnAdvView;
		wxString m_btnPlay;
        wxString m_btnPause;
        wxString m_btnMessages;
		wxString m_btnOpen;
        wxString m_btnSave;
		wxString m_btnCancel;
		wxString m_btnLeftArr;
		wxString m_btnRightArr;
		wxString m_btnLeftArrClick;
		wxString m_btnRightArrClick;
		wxString m_btnExpand;
		wxString m_btnCollapse;
		wxString m_btnExpandClick;
		wxString m_btnCollapseClick;
		wxString m_btnShowGraphic;
		wxString m_btnShowGraphicClick;
		// Icons
		wxString m_icnWorking;
		wxString m_icnSleeping;	
		wxString m_icnWorkingWkUnit;
		wxString m_icnSleepingWkUnit;
		wxString m_defaultPrjIcn;// default proj icon
		wxString m_defaultStatIcn;// default stat icon
		// animation
		wxString m_animBg;
		wxString m_animFile;
		//skin info
		wxString m_skinName;
		wxString m_skinsFolder;
};
#endif  /* _SKIN_CLASS */
