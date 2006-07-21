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
#pragma implementation "sg_DlgPreferences.h"
#endif

#include "stdwx.h"
#include "sg_DlgPreferences.h"
#include "sg_SkinClass.h"

enum 
{ 
    ID_OPENBUTTON = 10001, 
    ID_SAVEBUTTON = 10002, 
    ID_CANCELBUTTON = 10003,
	ID_SKINPICKERCMBBOX = 10004, 
	ID_DOWORKONLYBGNCMBBOX = 10005,
	ID_DOCONNECTONLYBGNCMBBOX = 10006,
}; 

BEGIN_EVENT_TABLE( CDlgPreferences,wxDialog)
  EVT_COMBOBOX(-1,CDlgPreferences::OnCmbSelected)
  EVT_BUTTON(-1,CDlgPreferences::OnBtnClick)
  EVT_ERASE_BACKGROUND(CDlgPreferences::OnEraseBackground)
END_EVENT_TABLE()
// end events

CDlgPreferences::CDlgPreferences(wxWindow* parent, wxString dirPref,wxWindowID id,const wxString& title,const wxPoint& pos,const wxSize& size,long style,const wxString& name)
{
 m_SkinDirPrefix = dirPref;
 OnPreCreate();
 Create(parent,id,title,pos,size,style,name);

 if((pos==wxDefaultPosition)&&(size==wxDefaultSize)){
     SetSize(0,0,400,450);
 }

 if((pos!=wxDefaultPosition)&&(size==wxDefaultSize)){
     SetSize(400,450);
 }
 initBefore();
 // load images from skin file
 LoadSkinImages();
 //Create dialog
 InitDialog();
 initAfter();
}
CDlgPreferences::~CDlgPreferences()
{

}


void CDlgPreferences::InitDialog()
{
	SetBackgroundColour(appSkin->GetAppBgCol());
	wxString itmsHourIntervals[]={wxT("Always"),wxT("12:00 AM"),wxT("1:00 AM"),wxT("2:00 AM"),wxT("3:00 AM"),wxT("4:00 AM"),wxT("5:00 AM"),wxT("6:00 AM"),wxT("7:00 AM"),wxT("8:00 AM"),wxT("9:00 AM"),wxT("10:00 AM"),wxT("11:00 AM"),wxT("12:00 PM"),
		wxT("13:00 PM"),wxT("14:00 PM"),wxT("15:00 PM"),wxT("16:00 PM"),wxT("17:00 PM"),wxT("18:00 PM"),wxT("19:00 PM"),wxT("20:00 PM"),wxT("21:00 PM"),wxT("22:00 PM"),wxT("23:00 PM")};
	wxString itmsHourIntervalsNoAlways[]={wxT("12:00 AM"),wxT("1:00 AM"),wxT("2:00 AM"),wxT("3:00 AM"),wxT("4:00 AM"),wxT("5:00 AM"),wxT("6:00 AM"),wxT("7:00 AM"),wxT("8:00 AM"),wxT("9:00 AM"),wxT("10:00 AM"),wxT("11:00 AM"),wxT("12:00 PM"),
		wxT("13:00 PM"),wxT("14:00 PM"),wxT("15:00 PM"),wxT("16:00 PM"),wxT("17:00 PM"),wxT("18:00 PM"),wxT("19:00 PM"),wxT("20:00 PM"),wxT("21:00 PM"),wxT("22:00 PM"),wxT("23:00 PM")};
	

	lblPref=new wxStaticText(this,-1,wxT(""),wxPoint(10,10),wxSize(84,18),wxST_NO_AUTORESIZE);
	lblPref->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblPref->SetLabel(wxT("Preferences"));
	lblPref->SetFont(wxFont(12,74,90,90,0,wxT("Tahoma")));
	// Modify settings for this computer
	lblModifySett=new wxStaticText(this,-1,wxT(""),wxPoint(15,60),wxSize(164,13),wxST_NO_AUTORESIZE);
	lblModifySett->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblModifySett->SetLabel(wxT("Modify settings for this computer:"));
	// Do work only between
	lblDoWorkBtwn=new wxStaticText(this,-1,wxT(""),wxPoint(65,95),wxSize(114,13),wxST_NO_AUTORESIZE);
	lblDoWorkBtwn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblDoWorkBtwn->SetLabel(wxT("Do work only between:"));
    cmbDWBtwnBgn=new wxComboBox(this,ID_DOWORKONLYBGNCMBBOX,wxT(""),wxPoint(180,90),wxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbDWBtwnBgn->SetValue(wxT("Always"));
	lblAnd1=new wxStaticText(this,-1,wxT(""),wxPoint(270,95),wxSize(19,13),wxST_NO_AUTORESIZE);
	lblAnd1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblAnd1->SetLabel(wxT("and"));
	lblAnd1->Show(false);
	cmbDWBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(295,90),wxSize(85,21),24,itmsHourIntervalsNoAlways,wxNO_BORDER | wxCB_READONLY);
	cmbDWBtwnEnd->SetValue(wxT("12:00 AM"));
	cmbDWBtwnEnd->Show(false);
	///Connect to internet only between
	lblConnToIntBtwn=new wxStaticText(this,-1,wxT(""),wxPoint(10,130),wxSize(169,13),wxST_NO_AUTORESIZE);
	lblConnToIntBtwn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblConnToIntBtwn->SetLabel(wxT("Connect to internet only between:"));
	cmbCTIBtwnBgn=new wxComboBox(this,ID_DOCONNECTONLYBGNCMBBOX,wxT(""),wxPoint(180,125),wxSize(85,21),24,itmsHourIntervals,wxNO_BORDER | wxCB_READONLY);
	cmbCTIBtwnBgn->SetValue(wxT("Always"));
	lblAnd2=new wxStaticText(this,-1,wxT(""),wxPoint(270,130),wxSize(19,13),wxST_NO_AUTORESIZE);
	lblAnd2->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblAnd2->SetLabel(wxT("and"));
	lblAnd2->Show(false);
	cmbCTIBtwnEnd=new wxComboBox(this,-1,wxT(""),wxPoint(295,125),wxSize(85,21),24,itmsHourIntervalsNoAlways,wxNO_BORDER | wxCB_READONLY);
	cmbCTIBtwnEnd->SetValue(wxT("12:00 AM"));
	cmbCTIBtwnEnd->Show(false);
	///Use no more than
	lblUseNoMoreGB=new wxStaticText(this,-1,wxT(""),wxPoint(85,165),wxSize(94,13),wxST_NO_AUTORESIZE);
	lblUseNoMoreGB->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblUseNoMoreGB->SetLabel(wxT("Use no more than:"));
	wxString itmsUseNoMoreGB[]={wxT("100MB"),wxT("200MB"),wxT("500MB"),wxT("1GB"),wxT("2GB"),wxT("5GB")};
	cmbUseNoMoreGB=new wxComboBox(this,-1,wxT(""),wxPoint(180,160),wxSize(85,21),5,itmsUseNoMoreGB,wxNO_BORDER | wxCB_READONLY);
	cmbUseNoMoreGB->SetValue(wxT("500MB"));
	lblGB=new wxStaticText(this,-1,wxT(""),wxPoint(270,165),wxSize(14,13),wxST_NO_AUTORESIZE);
	lblGB->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblGB->SetLabel(wxT("GB"));
	/// Do work while computer is in use
	lblDWWCInUse=new wxStaticText(this,-1,wxT(""),wxPoint(15,200),wxSize(164,13),wxST_NO_AUTORESIZE);
	lblDWWCInUse->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblDWWCInUse->SetLabel(wxT("Do work while computer is in use?"));
	wxString itmsDWWCInUse[]={wxT("Yes"),wxT("No")};
	cmbDWWCInUse=new wxComboBox(this,-1,wxT(""),wxPoint(180,195),wxSize(45,21),2,itmsDWWCInUse,wxNO_BORDER | wxCB_READONLY);
	cmbDWWCInUse->SetValue(wxT("Yes"));
	///Do work after computer is idle for
	lblDWACIdleFor=new wxStaticText(this,-1,wxT(""),wxPoint(15,235),wxSize(164,13),wxST_NO_AUTORESIZE);
	lblDWACIdleFor->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblDWACIdleFor->SetLabel(wxT("Do work after computer is idle for:"));
	wxString itmsDWACIdleFor[]={wxT("1"),wxT("5"),wxT("10"),wxT("15"),wxT("30"),wxT("60")};
	cmbDWACIdleFor=new wxComboBox(this,-1,wxT(""),wxPoint(180,230),wxSize(85,21),5,itmsDWACIdleFor,wxNO_BORDER | wxCB_READONLY);
	cmbDWACIdleFor->SetValue(wxT("5"));
	lblMinutes=new wxStaticText(this,-1,wxT(""),wxPoint(270,235),wxSize(39,13),wxST_NO_AUTORESIZE);
	lblMinutes->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblMinutes->SetLabel(wxT("minutes"));
	////Skin XML file
	lblSkinXML=new wxStaticText(this,-1,wxT(""),wxPoint(115,270),wxSize(64,13),wxST_NO_AUTORESIZE);
	lblSkinXML->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	lblSkinXML->SetLabel(wxT("Skin XML file:"));
	//skin picker control
    wxString itmsSkinPicker[]={wxT("Default"),wxT("WorldCommunityGrid")};
	cmbSkinPicker=new wxComboBox(this,ID_SKINPICKERCMBBOX,wxT(""),wxPoint(180,265),wxSize(140,21),2,itmsSkinPicker,wxNO_BORDER | wxCB_READONLY);
	cmbSkinPicker->SetValue(wxT("Default"));
	//tx30c=new wxTextCtrl(this,-1,wxT(""),wxPoint(180,265),wxSize(148,21),wxSIMPLE_BORDER);
	//btnOpen=new wxBitmapButton(this,ID_OPENBUTTON,*btmpBtnAttProjL,wxPoint(331,265),wxSize(59,20));
	// Btn Save and Cancel
	btnSave=new wxBitmapButton(this,ID_SAVEBUTTON,*bti26cImg1,wxPoint(115,325),wxSize(59,20));
	btnCancel=new wxBitmapButton(this,ID_CANCELBUTTON,*bti27cImg1,wxPoint(190,325),wxSize(59,20));
	 
	Refresh();
}
void CDlgPreferences::LoadSkinImages(){
	//get skin class
	appSkin = SkinClass::Instance();
    wxString str1 = m_SkinDirPrefix + appSkin->GetDlgPrefBg();
	fileImgBuf[0].LoadFile(m_SkinDirPrefix + appSkin->GetDlgPrefBg(),wxBITMAP_TYPE_BMP);
	fileImgBuf[1].LoadFile(m_SkinDirPrefix + appSkin->GetBtnSave(),wxBITMAP_TYPE_BMP);
	fileImgBuf[2].LoadFile(m_SkinDirPrefix + appSkin->GetBtnCancel(),wxBITMAP_TYPE_BMP);
	fileImgBuf[3].LoadFile(m_SkinDirPrefix + appSkin->GetBtnOpen(),wxBITMAP_TYPE_BMP);
	dlg10484fImg0=&fileImgBuf[0];
	bti26cImg1=&fileImgBuf[1];
	bti27cImg1=&fileImgBuf[2];
	btmpBtnAttProjL=&fileImgBuf[3];
}
void CDlgPreferences::VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz){
 event.Skip(false);wxDC *dc;
 dc=event.GetDC();
 dc->SetBackground(wxBrush(win->GetBackgroundColour(),wxSOLID));
 dc->Clear();
 switch (opz) {
  case 0:{
         dc->DrawBitmap(bitMap, 0, 0);
         break;}
  case 1:{
         wxRect rec=win->GetClientRect();
         rec.SetLeft((rec.GetWidth()-bitMap.GetWidth())   / 2);
         rec.SetTop ((rec.GetHeight()-bitMap.GetHeight()) / 2);
         dc->DrawBitmap(bitMap,rec.GetLeft(),rec.GetTop(),0);
         break;}
  case 2:{
         wxRect rec=win->GetClientRect();
         for(int y=0;y < rec.GetHeight();y+=bitMap.GetHeight()){
           for(int x=0;x < rec.GetWidth();x+=bitMap.GetWidth()){
             dc->DrawBitmap(bitMap,x,y,0);
           }
         }
         break;}
 }
}
void CDlgPreferences::OnEraseBackground(wxEraseEvent& event){
 wxObject *m_wxWin = event.GetEventObject();
 if(m_wxWin==this){event.Skip(true);VwXDrawBackImg(event,this,*dlg10484fImg0,0);VwXEvOnEraseBackground(event) ;return;}
 event.Skip(true);
}

void CDlgPreferences::OnBtnClick(wxCommandEvent& event){ //init function
    wxObject *m_wxDlgBtnObj = event.GetEventObject();
	int btnID =  event.GetId();
	if(btnID==ID_SAVEBUTTON){
		//wxMessageBox("OnBtnClick - btnSave");
		EndModal(wxID_CANCEL);
	}
	else if(btnID==ID_OPENBUTTON){
		wxString fileName = wxFileSelector(_("Choose a file to open"), _(""), _(""), _("*.xml*"), _("*.xml*"), wxOPEN); 
		if(!fileName.IsEmpty()){
			//tx30c->SetLabel(fileName);
			//tx30c->Disable();
			//tx30c->SetEditable(false);
			//set the path value
			//this->SetSkinPath(fileName);  REPLACED WITH DROPDOWN
		}
	}else{
		//wxMessageBox("OnBtnClick - btnCancel");
		EndModal(wxID_CANCEL);
	}
} //end function

void CDlgPreferences::OnCmbSelected(wxCommandEvent& event){ //init function
    wxObject *m_wxDlgCmbObj = event.GetEventObject();
	int cmbID =  event.GetId();
	if(cmbID==ID_SKINPICKERCMBBOX){
		m_SkinName = event.GetString();
	}else if(cmbID==ID_DOWORKONLYBGNCMBBOX){
		if(event.GetString() == wxT("Always")){
			cmbDWBtwnEnd->Show(false);
		}else{
			cmbDWBtwnEnd->Show(true);
		}
	}else if(cmbID==ID_DOCONNECTONLYBGNCMBBOX){
		if(event.GetString() == wxT("Always")){
			cmbCTIBtwnEnd->Show(false);
		}else{
			cmbCTIBtwnEnd->Show(true);
		}
	}
} //end function

void CDlgPreferences::VwXEvOnEraseBackground(wxEraseEvent& WXUNUSED(event)){ //init function
 //[29b]Code event VwX...Don't modify[29b]//
 //add your code here

} //end function

void CDlgPreferences::OnPreCreate(){
 //add your code here

}

void CDlgPreferences::initBefore(){
 //add your code here

}

void CDlgPreferences::initAfter(){
 //add your code here
    Centre();
}

//[evtFunc]end your code
