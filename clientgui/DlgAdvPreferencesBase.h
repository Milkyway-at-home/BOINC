///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 13 2006)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DlgAdvPreferencesBase__
#define __DlgAdvPreferencesBase__

// Define WX_GCH in order to support precompiled headers with GCC compiler.
// You have to create the header "wx_pch.h" and include all files needed
// for compile your gui inside it.
// Then, compile it and place the file "wx_pch.h.gch" into the same
// directory that "wx_pch.h".
#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#include <wx/button.h>
#include <wx/notebook.h> 
#include <wx/panel.h>
#include <wx/radiobut.h>

///////////////////////////////////////////////////////////////////////////

#define ID_DEFAULT wxID_ANY // Default
#define ID_CHKMEMORYWHILESUSPENDED 20000
#define ID_CHKNETCONFIRMBEFORECONNECT 20001
#define ID_CHKNETDISCONNECTWHENDONE 20002
#define ID_CHKNETFRIDAY 20003
#define ID_CHKNETMONDAY 20004
#define ID_CHKNETSATURDAY 20005
#define ID_CHKNETSKIPIMAGEVERIFICATION 20006
#define ID_CHKNETSUNDAY 20007
#define ID_CHKNETTHURSDAY 20008
#define ID_CHKNETTUESDAY 20009
#define ID_CHKNETWEDNESDAY 20010
#define ID_CHKPROCFRIDAY 20011
#define ID_CHKPROCINUSE 20012
#define ID_CHKPROCMONDAY 20013
#define ID_CHKPROCONBATTERIES 20014
#define ID_CHKPROCSATURDAY 20015
#define ID_CHKPROCSUNDAY 20016
#define ID_CHKPROCTHURSDAY 20017
#define ID_CHKPROCTUESDAY 20018
#define ID_CHKPROCWEDNESDAY 20019
#define ID_DLGADVPREFS 20020
#define ID_RBTNETEVERYDAY 20021
#define ID_RBTNETSPECIALTIMES 20022
#define ID_RBTPROCEVERYDAY 20023
#define ID_RBTPROCSPECIALTIMES 20024
#define ID_TABPAGE_DISK 20025
#define ID_TABPAGE_NET 20026
#define ID_TABPAGE_PROC 20027
#define ID_TXTDISKLEASTFREE 20028
#define ID_TXTDISKMAXOFTOTAL 20029
#define ID_TXTDISKMAXSPACE 20030
#define ID_TXTDISKWRITETODISK 20031
#define ID_TXTMEMORYMAXINUSE 20032
#define ID_TXTMEMORYMAXONIDLE 20033
#define ID_TXTNETCONNECTINTERVAL 20034
#define ID_TXTNETDOWNLOADRATE 20035
#define ID_TXTNETEVERYDAYSTART 20036
#define ID_TXTNETEVERYDAYSTOP 20037
#define ID_TXTNETFRIDAY 20038
#define ID_TXTNETMONDAY 20039
#define ID_TXTNETSATURDAY 20040
#define ID_TXTNETSUNDAY 20041
#define ID_TXTNETTHURSDAY 20042
#define ID_TXTNETTUESDAY 20043
#define ID_TXTNETUPLOADRATE 20044
#define ID_TXTNETWEDNESDAY 20045
#define ID_TXTPOCUSECPUTIME 20046
#define ID_TXTPROCEVERYDAYSTART 20047
#define ID_TXTPROCEVERYDAYSTOP 20048
#define ID_TXTPROCFRIDAY 20049
#define ID_TXTPROCIDLEFOR 20050
#define ID_TXTPROCMONDAY 20051
#define ID_TXTPROCSATURDAY 20052
#define ID_TXTPROCSUNDAY 20053
#define ID_TXTPROCSWITCHEVERY 20054
#define ID_TXTPROCTHURSDAY 20055
#define ID_TXTPROCTUESDAY 20056
#define ID_TXTPROCUSEPROCESSORS 20057
#define ID_TXTPROCWEDNESDAY 20058

/**
 * Class CDlgAdvPreferencesBase
 */
class CDlgAdvPreferencesBase : public wxDialog 
{
	private:
	
	protected:
		wxPanel* m_panelControls;
		wxNotebook* m_Notebook;
		wxPanel* m_panelProcessor;
		wxCheckBox* m_chkProcOnBatteries;
		wxCheckBox* m_chkProcInUse;
		wxStaticText* m_staticText26;
		wxTextCtrl* m_txtProcIdleFor;
		wxStaticText* m_staticText27;
		wxStaticText* m_staticText28;
		wxRadioButton* m_rbtProcEveryDay;
		wxTextCtrl* m_txtProcEveryDayStart;
		wxStaticText* m_staticText25;
		wxTextCtrl* m_txtProcEveryDayStop;
		wxRadioButton* m_rbtProcSpecialTimes;
		wxPanel* m_panelProcSpecialTimes;
		wxCheckBox* m_chkProcMonday;
		wxTextCtrl* m_txtProcMonday;
		wxCheckBox* m_chkProcTuesday;
		wxTextCtrl* m_txtProcTuesday;
		wxCheckBox* m_chkProcWednesday;
		wxTextCtrl* m_txtProcWednesday;
		wxCheckBox* m_chkProcThursday;
		wxTextCtrl* m_txtProcThursday;
		wxCheckBox* m_chkProcFriday;
		wxTextCtrl* m_txtProcFriday;
		wxCheckBox* m_chkProcSaturday;
		wxTextCtrl* m_txtProcSaturday;
		wxCheckBox* m_chkProcSunday;
		wxTextCtrl* m_txtProcSunday;
		wxStaticText* m_staticText18;
		wxTextCtrl* m_txtProcSwitchEvery;
		wxStaticText* m_staticText19;
		wxStaticText* m_staticText20;
		wxTextCtrl* m_txtProcUseProcessors;
		wxStaticText* m_staticText21;
		wxStaticText* m_staticText22;
		wxTextCtrl* m_txtProcUseCPUTime;
		wxStaticText* m_staticText23;
		wxPanel* m_panelNetwork;
		wxStaticText* m_staticText30;
		wxTextCtrl* m_txtNetConnectInterval;
		wxStaticText* m_staticText31;
		wxStaticText* m_staticText32;
		wxTextCtrl* m_txtNetDownloadRate;
		wxStaticText* m_staticText33;
		wxStaticText* m_staticText34;
		wxTextCtrl* m_txtNetUploadRate;
		wxStaticText* m_staticText35;
		wxCheckBox* m_chkNetSkipImageVerification;
		wxCheckBox* m_chkNetConfirmBeforeConnect;
		wxCheckBox* m_chkNetDisconnectWhenDone;
		wxRadioButton* m_rbtNetEveryDay;
		wxTextCtrl* m_txtNetEveryDayStart;
		wxStaticText* m_staticText37;
		wxTextCtrl* m_txtNetEveryDayStop;
		wxRadioButton* m_rbtNetSpecialTimes;
		wxPanel* m_panelNetSpecialTimes;
		wxCheckBox* m_chkNetMonday;
		wxTextCtrl* m_txtNetMonday;
		wxCheckBox* m_chkNetTuesday;
		wxTextCtrl* m_txtNetTuesday;
		wxCheckBox* m_chkNetWednesday;
		wxTextCtrl* m_txtNetWednesday;
		wxCheckBox* m_chkNetThursday;
		wxTextCtrl* m_txtNetThursday;
		wxCheckBox* m_chkNetFriday;
		wxTextCtrl* m_txtNetFriday;
		wxCheckBox* m_chkNetSaturday;
		wxTextCtrl* m_txtNetSaturday;
		wxCheckBox* m_chkNetSunday;
		wxTextCtrl* m_txtNetSunday;
		wxPanel* m_panelDiskAndMemory;
		wxStaticText* m_staticText40;
		wxTextCtrl* m_txtDiskMaxSpace;
		wxStaticText* m_staticText41;
		wxStaticText* m_staticText42;
		wxTextCtrl* m_txtDiskLeastFree;
		wxStaticText* m_staticText43;
		wxStaticText* m_staticText44;
		wxTextCtrl* m_txtDiskMaxOfTotal;
		wxStaticText* m_staticText45;
		wxStaticText* m_staticText46;
		wxTextCtrl* m_txtDiskWriteToDisk;
		wxStaticText* m_staticText47;
		wxStaticText* m_staticText48;
		wxTextCtrl* m_txtDiskMaxSwap;
		wxStaticText* m_staticText49;
		wxStaticText* m_staticText50;
		wxTextCtrl* m_txtMemoryMaxInUse;
		wxStaticText* m_staticText51;
		wxStaticText* m_staticText52;
		wxTextCtrl* m_txtMemoryMaxOnIdle;
		wxStaticText* m_staticText53;
		wxCheckBox* m_chkMemoryWhileSuspended;
		wxPanel* m_panelButtons;
		wxButton* m_btnOK;
		wxButton* m_btnCancel;
		wxButton* m_btnHelp;
	
	public:
		CDlgAdvPreferencesBase( wxWindow* parent, int id = -1, wxString title = wxT("BOINC Manager - Preferences"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 603,561 ), int style = wxDEFAULT_DIALOG_STYLE );
	
};

#endif //__DlgAdvPreferencesBase__
