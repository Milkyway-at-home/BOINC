static volatile const char *BOINCrcsid="$Id$";
// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewMessages.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewMessages.h"
#include "Events.h"

#include "wx/arrimpl.cpp" 


#include "res/mess.xpm"
#include "res/task.xpm"
#include "res/tips.xpm"


#define VIEW_HEADER                 wxT("mess")

#define SECTION_TASK                wxT(VIEW_HEADER "task")
#define SECTION_TIPS                wxT(VIEW_HEADER "tips")

#define BITMAP_MESSAGE              wxT(VIEW_HEADER ".xpm")
#define BITMAP_TASKHEADER           wxT(SECTION_TASK ".xpm")
#define BITMAP_TIPSHEADER           wxT(SECTION_TIPS ".xpm")

#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

#define PRIORITY_INFO               1
#define PRIORITY_ERROR              2


const wxString LINKDESC_DEFAULT         = 
     _("Please click a message to see additional options.");

const wxString LINK_TASKCOPYALL         = wxT(SECTION_TASK "copyall");
const wxString LINKDESC_TASKCOPYALL     = 
     _("<b>Copy all</b><br>"
       "Copy all the messages to the system clipboard.");

const wxString LINK_TASKCOPYMESSAGE     = wxT(SECTION_TASK "copymessage");
const wxString LINKDESC_TASKCOPYMESSAGE = 
     _("<b>Copy selection</b><br>"
       "Copy the selected message(s) to the system clipboard.  "
       "You can select multiple items by holding down the shift key "
       " or control key while clicking on the next desired message.");


WX_DEFINE_OBJARRAY( CMessageCache );


CMessage::CMessage()
{
	m_strPriority = wxEmptyString;
	m_strProjectName = wxEmptyString;
	m_strTime = wxEmptyString;
	m_strMessage = wxEmptyString;
}


CMessage::~CMessage()
{
	m_strPriority.Clear();
	m_strProjectName.Clear();
	m_strTime.Clear();
	m_strMessage.Clear();
}


wxInt32 CMessage::GetProjectName( wxString& strProjectName )
{
    strProjectName = m_strProjectName;	
	return 0;
}


wxInt32 CMessage::GetPriority( wxString& strPriority )
{
    strPriority = m_strPriority;	
	return 0;
}


wxInt32 CMessage::GetTime( wxString& strTime )
{
    strTime = m_strTime;	
	return 0;
}


wxInt32 CMessage::GetMessage( wxString& strMessage )
{
    strMessage =  m_strMessage;	
	return 0;
}


wxInt32 CMessage::SetProjectName( wxString& strProjectName )
{
    m_strProjectName = strProjectName;	
	return 0;
}


wxInt32 CMessage::SetPriority( wxString& strPriority )
{
	m_strPriority = strPriority;
	return 0;
}


wxInt32 CMessage::SetTime( wxString& strTime )
{
	m_strTime = strTime;
	return 0;
}


wxInt32 CMessage::SetMessage( wxString& strMessage )
{
	m_strMessage = strMessage;
	return 0;
}


IMPLEMENT_DYNAMIC_CLASS( CViewMessages, CBOINCBaseView )


CViewMessages::CViewMessages()
{
}


CViewMessages::CViewMessages(wxNotebook* pNotebook) :
    CBOINCBaseView( pNotebook, ID_HTML_MESSAGESVIEW, DEFAULT_HTML_FLAGS, ID_LIST_MESSAGESVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS )
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    wxBitmap bmpMessage(mess_xpm);
    wxBitmap bmpTask(task_xpm);
    wxBitmap bmpTips(tips_xpm);

    bmpMessage.SetMask(new wxMask(bmpMessage, wxColour(255, 0, 255)));
    bmpTask.SetMask(new wxMask(bmpTask, wxColour(255, 0, 255)));
    bmpTips.SetMask(new wxMask(bmpTips, wxColour(255, 0, 255)));

    m_pTaskPane->AddVirtualFile(BITMAP_MESSAGE, bmpMessage, wxBITMAP_TYPE_XPM);

    m_pTaskPane->CreateTaskHeader(BITMAP_TASKHEADER, bmpTask, _("Tasks"));
    m_pTaskPane->CreateTaskHeader(BITMAP_TIPSHEADER, bmpTips, _("Quick Tips"));

    m_pListPane->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 115);
    m_pListPane->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 145);
    m_pListPane->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 550);

    m_pMessageInfoAttr = new wxListItemAttr( *wxBLACK, *wxWHITE, wxNullFont );
    m_pMessageErrorAttr = new wxListItemAttr( *wxRED, *wxWHITE, wxNullFont );

    m_bTipsHeaderHidden = false;
    m_bItemSelected = false;

    SetCurrentQuickTip(
        LINK_DEFAULT, 
        LINKDESC_DEFAULT
    );

    UpdateSelection();
}


CViewMessages::~CViewMessages()
{
    if ( m_pMessageInfoAttr )
    {
        delete m_pMessageInfoAttr;
        m_pMessageInfoAttr = NULL;
    }

    if ( m_pMessageErrorAttr )
    {
        delete m_pMessageErrorAttr;
        m_pMessageErrorAttr = NULL;
    }
}


wxString CViewMessages::GetViewName()
{
    return wxString(_("Messages"));
}


char** CViewMessages::GetViewIcon()
{
    return mess_xpm;
}


wxInt32 CViewMessages::GetDocCount()
{
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    return pDoc->GetMessageCount();
}


wxString CViewMessages::OnListGetItemText( long item, long column ) const
{
    CMessage&       message     = m_MessageCache.Item( item );
    wxString        strBuffer   = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            message.GetProjectName( strBuffer );
            break;
        case COLUMN_TIME:
            message.GetTime( strBuffer );
            break;
        case COLUMN_MESSAGE:
            message.GetMessage( strBuffer );
            break;
    }

    return strBuffer;
}


wxListItemAttr* CViewMessages::OnListGetItemAttr( long item ) const
{
    wxListItemAttr* pAttribute  = NULL;
    CMessage&       message     = m_MessageCache.Item( item );
    wxString        strBuffer   = wxEmptyString;

    message.GetPriority( strBuffer );

    if ( wxT("E") == strBuffer )
    {
        pAttribute = m_pMessageErrorAttr;
    }
    else
    {
        pAttribute = m_pMessageInfoAttr;
    }

    return pAttribute;
}


wxString CViewMessages::OnDocGetItemText( long item, long column ) const
{
    wxString       strBuffer = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            FormatProjectName( item, strBuffer );
            break;
        case COLUMN_TIME:
            FormatTime( item, strBuffer );
            break;
        case COLUMN_MESSAGE:
            FormatMessage( item, strBuffer );
            break;
    }

    return strBuffer;
}


void CViewMessages::OnTaskLinkClicked( const wxHtmlLinkInfo& link )
{
    wxInt32 iIndex = -1;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if      ( link.GetHref() == SECTION_TASK )
        m_bTaskHeaderHidden ? m_bTaskHeaderHidden = false : m_bTaskHeaderHidden = true;

#ifndef NOCLIPBOARD
    else if ( link.GetHref() == LINK_TASKCOPYALL )
    {
        wxInt32 iRowCount = 0;

        OpenClipboard();

        iRowCount = m_pListPane->GetItemCount();
        for ( iIndex = 0; iIndex < iRowCount; iIndex++ )
        {
            CopyToClipboard( iIndex );            
        }

        CloseClipboard();
    }
    else if ( link.GetHref() == LINK_TASKCOPYMESSAGE )
    {
        OpenClipboard();

        for ( ;; )
        {
            iIndex = m_pListPane->GetNextItem(iIndex,
                                        wxLIST_NEXT_ALL,
                                        wxLIST_STATE_SELECTED);
            if ( iIndex == -1 )
                break;

            CopyToClipboard( iIndex );            
        }

        CloseClipboard();
    }
#endif

    else if ( link.GetHref() == SECTION_TIPS )
        m_bTipsHeaderHidden ? m_bTipsHeaderHidden = false : m_bTipsHeaderHidden = true;


    UpdateSelection();
}


void CViewMessages::OnTaskCellMouseHover( wxHtmlCell* cell, wxCoord WXUNUSED(x), wxCoord WXUNUSED(y) )
{
    if ( NULL != cell->GetLink() )
    {
        bool        bUpdateSelection = false;
        wxString    strLink;

        strLink = cell->GetLink()->GetHref();

        if      ( UpdateQuickTip( strLink, LINK_TASKCOPYALL, LINKDESC_TASKCOPYALL ) )
            bUpdateSelection = true;
        else if ( UpdateQuickTip( strLink, LINK_TASKCOPYMESSAGE, LINKDESC_TASKCOPYMESSAGE ) )
            bUpdateSelection = true;
        else
        {
            if ( 0 == m_pListPane->GetSelectedItemCount() )
            {
                if  ( LINK_DEFAULT != GetCurrentQuickTip() )
                {
                    SetCurrentQuickTip(
                        LINK_DEFAULT, 
                        LINKDESC_DEFAULT
                    );

                    bUpdateSelection = true;
                }
            }
        }

        if ( bUpdateSelection )
        {
            UpdateSelection();
        }
    }
}


wxInt32 CViewMessages::AddCacheElement()
{
    CMessage* pItem = new CMessage();
    wxASSERT( NULL != pItem );
    if ( NULL != pItem )
    {
        m_MessageCache.Add( pItem );
        return 0;
    }
    return -1;
}


wxInt32 CViewMessages::EmptyCache()
{
    m_MessageCache.Empty();
    return 0;
}


wxInt32 CViewMessages::GetCacheCount()
{
    return m_MessageCache.GetCount();
}


wxInt32 CViewMessages::RemoveCacheElement()
{
    m_MessageCache.RemoveAt( GetCacheCount() - 1 );
    return 0;
}

    
wxInt32 CViewMessages::UpdateCache( long item, long column, wxString& strNewData )
{
    CMessage& message     = m_MessageCache.Item( item );
    wxString  strPriority = wxEmptyString;

    switch(column)
    {
        case COLUMN_PROJECT:
            message.SetProjectName( strNewData );
            break;
        case COLUMN_TIME:
            message.SetTime( strNewData );
            break;
        case COLUMN_MESSAGE:
            message.SetMessage( strNewData );
            break;
    }

    FormatPriority( item, strPriority );
    message.SetPriority( strPriority );

    return 0;
}


bool CViewMessages::EnsureLastItemVisible()
{
    return true;
}


void CViewMessages::UpdateSelection()
{
    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

    if ( 0 == m_pListPane->GetSelectedItemCount() )
    {
        m_bTaskHeaderHidden = false;
        m_bTaskCopyAllHidden = false;
        m_bTaskCopyMessageHidden = true;

        if ( m_bItemSelected )
        {
            SetCurrentQuickTip(
                LINK_DEFAULT, 
                wxT("")
            );
        }
        m_bItemSelected = false;
    }
    else
    {
        m_bTaskHeaderHidden = false;
        m_bTaskCopyAllHidden = false;
        m_bTaskCopyMessageHidden = false;

        m_bItemSelected = true;
    }
    UpdateTaskPane();
}


void CViewMessages::UpdateTaskPane()
{
    wxASSERT(NULL != m_pTaskPane);

    m_pTaskPane->BeginTaskPage();

    m_pTaskPane->BeginTaskSection( SECTION_TASK, BITMAP_TASKHEADER, m_bTaskHeaderHidden );
    if (!m_bTaskHeaderHidden)
    {
        m_pTaskPane->CreateTask( LINK_TASKCOPYALL, BITMAP_MESSAGE, _("Copy all"), m_bTaskCopyAllHidden );
        m_pTaskPane->CreateTask( LINK_TASKCOPYMESSAGE, BITMAP_MESSAGE, _("Copy selection"), m_bTaskCopyMessageHidden );
    }
    m_pTaskPane->EndTaskSection( m_bTaskHeaderHidden );

    m_pTaskPane->UpdateQuickTip( SECTION_TIPS, BITMAP_TIPSHEADER, GetCurrentQuickTipText(), m_bTipsHeaderHidden );

    m_pTaskPane->EndTaskPage();
}


wxInt32 CViewMessages::FormatProjectName( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessageProjectName(item, strBuffer);

    return 0;
}


wxInt32 CViewMessages::FormatPriority( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument*  pDoc = wxGetApp().GetDocument();
    wxInt32         iBuffer = 0;

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessagePriority(item, iBuffer);

    switch(iBuffer)
    {
        case PRIORITY_INFO:
            strBuffer = wxT("I");
            break;
        case PRIORITY_ERROR:
            strBuffer = wxT("E");
            break;
    }

    return 0;
}


wxInt32 CViewMessages::FormatTime( wxInt32 item, wxString& strBuffer ) const
{
    wxDateTime     dtBuffer(wxDateTime::Now());
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessageTime(item, dtBuffer);
    strBuffer = dtBuffer.Format();

    return 0;
}


wxInt32 CViewMessages::FormatMessage( wxInt32 item, wxString& strBuffer ) const
{
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(NULL != pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    strBuffer.Clear();

    pDoc->GetMessageMessage(item, strBuffer);

    strBuffer.Replace( wxT("\n"), wxT(""), true );

    return 0;
}


#ifndef NOCLIPBOARD
bool CViewMessages::OpenClipboard()
{
    bool bRetVal = false;

    bRetVal = wxTheClipboard->Open();
    if ( bRetVal )
    {
        m_bClipboardOpen = true;
        m_strClipboardData = wxEmptyString;
        wxTheClipboard->Clear();
    }

    return bRetVal;
}


wxInt32 CViewMessages::CopyToClipboard( wxInt32 item )
{
    wxInt32        iRetVal = -1;

    if ( m_bClipboardOpen )
    {
        wxString       strBuffer = wxEmptyString;
        wxString       strTimeStamp = wxEmptyString;
        wxString       strProject = wxEmptyString;
        wxString       strMessage = wxEmptyString;

        FormatTime( item, strTimeStamp );
        FormatProjectName( item, strProject );
        FormatMessage( item, strMessage );

#ifdef __WXMSW__
        strBuffer.Printf( wxT("%s|%s|%s\r\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str() );
#else
        strBuffer.Printf( wxT("%s|%s|%s\n"), strTimeStamp.c_str(), strProject.c_str(), strMessage.c_str() );
#endif

        m_strClipboardData += strBuffer;

        iRetVal = 0;
    }

    return iRetVal;
}


bool CViewMessages::CloseClipboard()
{
    bool bRetVal = false;

    if ( m_bClipboardOpen )
    {
        wxTheClipboard->SetData( new wxTextDataObject( m_strClipboardData ) );
        wxTheClipboard->Close();

        m_bClipboardOpen = false;
        m_strClipboardData = wxEmptyString;
    }

    return bRetVal;
}

#endif
