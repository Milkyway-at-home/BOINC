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

#ifndef _STATIMAGELOADER_H_
#define _STATIMAGELOADER_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_StatImageLoader.cpp"
#endif


class StatImageLoader : public wxWindow 
{ 
public: 
	//members
    wxMenu *statPopUpMenu;
    //Skin Class
	std::string m_prjUrl;
	/// Constructors
	StatImageLoader(wxWindow* parent, std::string url); 
	~StatImageLoader(); 
	void LoadImage();
    void OnEraseBackground(wxEraseEvent& /*event*/) {};
	void OnMenuLinkClicked(wxCommandEvent& event);
	void OnProjectDetach();
	void PopUpMenu(wxMouseEvent& event); 
    void OnPaint(wxPaintEvent& event);
	void RebuildMenu();
	void UpdateInterface();

private: 

    //private memb 
    wxBitmap Bitmap; 
	std::string projectIcon;
	int numReloadTries;
	size_t urlCount;
	double project_files_downloaded_time;
	double project_last_rpc_time;
	void LoadStatIcon(wxBitmap& image);
	void ReloadProjectSpecificIcon();
	void BuildUserStatToolTip();
	void AddMenuItems();
    DECLARE_EVENT_TABLE() 
}; 

#endif 

