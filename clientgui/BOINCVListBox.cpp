// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// Adapted from vlbox.cpp by Vadim Zeitlin <vadim@wxwindows.org>

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if wxUSE_LISTBOX

#include "BOINCVListBox.h"

#include "wx/selstore.h"

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CBOINCVListBox, wxScrolledWindow)
    EVT_PAINT(CBOINCVListBox::OnPaint)
    EVT_SIZE(CBOINCVListBox::OnSize)

#if ALLOW_NOTICES_SELECTION
    EVT_KEY_DOWN(CBOINCVListBox::OnKeyDown)
    EVT_LEFT_DOWN(CBOINCVListBox::OnLeftDown)
    EVT_LEFT_DCLICK(CBOINCVListBox::OnLeftDClick)
#endif      // ALLOW_NOTICES_SELECTION
END_EVENT_TABLE()

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_ABSTRACT_CLASS(CBOINCVListBox, wxScrolledWindow)

// ----------------------------------------------------------------------------
// CBOINCVListBox creation
// ----------------------------------------------------------------------------

void CBOINCVListBox::Init()
{
    m_current =
    m_anchor = wxNOT_FOUND;
    m_selStore = NULL;
    m_iItemCount = 0;
}

bool CBOINCVListBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name)
{
    style |= wxWANTS_CHARS | wxFULL_REPAINT_ON_RESIZE;
    if ( !wxScrolledWindow::Create(parent, id, pos, size, style, name) )
        return false;

    if ( style & wxLB_MULTIPLE )
        m_selStore = new wxSelectionStore;

    // make sure the native widget has the right colour since we do
    // transparent drawing by default
    SetBackgroundColour(GetBackgroundColour());
    m_colBgSel = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

    // flicker-free drawing requires this
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    return true;
}

CBOINCVListBox::~CBOINCVListBox()
{
    delete m_selStore;
}

void CBOINCVListBox::SetItemCount(size_t count)
{
    // don't leave the current index invalid
    if ( m_current != wxNOT_FOUND && (size_t)m_current >= count )
        m_current = count - 1; // also ok when count == 0 as wxNOT_FOUND == -1

    if ( m_selStore )
    {
        // tell the selection store that our number of items has changed
        m_selStore->SetItemCount(count);
    }

    m_iItemCount = count;
    
    UpdateScrollbar();
}


void CBOINCVListBox::UpdateScrollbar() {
    // do sum up their heights
    int height = 0, width = GetMaxItemWidth();
    for ( int i = 0; i < m_iItemCount; ++i )
    {
        height += OnGetItemHeight(i);
    }
    
    SetVirtualSize(width, height);
    SetScrollRate(0, PIXELS_PER_VERTICAL_SCROLL_UNIT);

}


#if ALLOW_NOTICES_SELECTION

// ----------------------------------------------------------------------------
// selection handling
// ----------------------------------------------------------------------------

bool CBOINCVListBox::IsSelected(size_t line) const
{
    return m_selStore ? m_selStore->IsSelected(line) : (int)line == m_current;
}

bool CBOINCVListBox::Select(size_t item, bool select)
{
    wxCHECK_MSG( m_selStore, false,
                 _T("Select() may only be used with multiselection listbox") );

    wxCHECK_MSG( item < GetItemCount(), false,
                 _T("Select(): invalid item index") );

    bool changed = m_selStore->SelectItem(item, select);
    if ( changed )
    {
        // selection really changed
        RefreshItem(item);
    }

    DoSetCurrent(item);

    return changed;
}

bool CBOINCVListBox::SelectRange(size_t from, size_t to)
{
    wxCHECK_MSG( m_selStore, false,
                 _T("SelectRange() may only be used with multiselection listbox") );

    // make sure items are in correct order
    if ( from > to )
    {
        size_t tmp = from;
        from = to;
        to = tmp;
    }

    wxCHECK_MSG( to < GetItemCount(), false,
                    _T("SelectRange(): invalid item index") );

    wxArrayInt changed;
    if ( !m_selStore->SelectRange(from, to, true, &changed) )
    {
        // too many items have changed, we didn't record them in changed array
        // so we have no choice but to refresh everything between from and to
        RefreshItems(from, to);
    }
    else // we've got the indices of the changed items
    {
        const size_t count = changed.GetCount();
        if ( !count )
        {
            // nothing changed
            return false;
        }

        // refresh just the lines which have really changed
        for ( size_t n = 0; n < count; n++ )
        {
            RefreshItem(changed[n]);
        }
    }

    // something changed
    return true;
}

bool CBOINCVListBox::DoSelectAll(bool select)
{
    wxCHECK_MSG( m_selStore, false,
                 _T("SelectAll may only be used with multiselection listbox") );

    size_t count = GetItemCount();
    if ( count )
    {
        wxArrayInt changed;
        if ( !m_selStore->SelectRange(0, count - 1, select) ||
                !changed.IsEmpty() )
        {
            Refresh();

            // something changed
            return true;
        }
    }

    return false;
}

bool CBOINCVListBox::DoSetCurrent(int current)
{
    wxASSERT_MSG( current == wxNOT_FOUND ||
                    (current >= 0 && (size_t)current < GetItemCount()),
                  _T("CBOINCVListBox::DoSetCurrent(): invalid item index") );

    if ( current == m_current )
    {
        // nothing to do
        return false;
    }

    if ( m_current != wxNOT_FOUND )
        RefreshItem(m_current);

    m_current = current;

    if ( m_current != wxNOT_FOUND )
    {
        // if the line is not visible at all, we scroll it into view but we
        // don't need to refresh it -- it will be redrawn anyhow
        if ( !IsVisible(m_current) )
        {
            ScrollToLine(m_current);
        }
        else // line is at least partly visible
        {
            // it is, indeed, only partly visible, so scroll it into view to
            // make it entirely visible
            while ( (size_t)m_current == GetLastVisibleLine() &&
                    ScrollToLine(GetVisibleBegin()+1) ) ;

            // but in any case refresh it as even if it was only partly visible
            // before we need to redraw it entirely as its background changed
            RefreshItem(m_current);
        }
    }

    return true;
}

void CBOINCVListBox::SendSelectedEvent()
{
    wxASSERT_MSG( m_current != wxNOT_FOUND,
                    _T("SendSelectedEvent() shouldn't be called") );

    wxCommandEvent event(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
    event.SetEventObject(this);
    event.SetInt(m_current);

    (void)GetEventHandler()->ProcessEvent(event);
}

void CBOINCVListBox::SetSelection(int selection)
{
    wxCHECK_RET( selection == wxNOT_FOUND ||
                  (selection >= 0 && (size_t)selection < GetItemCount()),
                  _T("CBOINCVListBox::SetSelection(): invalid item index") );

    if ( HasMultipleSelection() )
    {
        if (selection != wxNOT_FOUND)
            Select(selection);
        else
            DeselectAll();
        m_anchor = selection;
    }

    DoSetCurrent(selection);
}

size_t CBOINCVListBox::GetSelectedCount() const
{
    return m_selStore ? m_selStore->GetSelectedCount()
                      : m_current == wxNOT_FOUND ? 0 : 1;
}

int CBOINCVListBox::GetFirstSelected(unsigned long& cookie) const
{
    cookie = 0;

    return GetNextSelected(cookie);
}

int CBOINCVListBox::GetNextSelected(unsigned long& cookie) const
{
    wxCHECK_MSG( m_selStore, wxNOT_FOUND,
                  _T("GetFirst/NextSelected() may only be used with multiselection listboxes") );

    while ( cookie < GetItemCount() )
    {
        if ( IsSelected(cookie++) )
            return cookie - 1;
    }

    return wxNOT_FOUND;
}

#endif  // ALLOW_NOTICES_SELECTION
// ----------------------------------------------------------------------------
// CBOINCVListBox appearance parameters
// ----------------------------------------------------------------------------

void CBOINCVListBox::SetMargins(const wxPoint& pt)
{
    if ( pt != m_ptMargins )
    {
        m_ptMargins = pt;

        Refresh();
    }
}

void CBOINCVListBox::SetSelectionBackground(const wxColour& col)
{
    m_colBgSel = col;
}

// ----------------------------------------------------------------------------
// CBOINCVListBox painting
// ----------------------------------------------------------------------------

wxCoord CBOINCVListBox::OnGetItemHeight(size_t item) const
{
    return OnMeasureItem(item) + (2 * m_ptMargins.y);
}

void CBOINCVListBox::OnDrawSeparator(wxDC& WXUNUSED(dc),
                                 wxRect& WXUNUSED(rect),
                                 size_t WXUNUSED(n)) const
{
}

void CBOINCVListBox::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{
#if ALLOW_NOTICES_SELECTION
    // we need to render selected and current items differently
    const bool isSelected = IsSelected(n),
               isCurrent = IsCurrent(n);
    if ( isSelected || isCurrent )
    {
        if ( isSelected )
        {
            dc.SetBrush(wxBrush(m_colBgSel, wxSOLID));
        }
        else // !selected
        {
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
        }

        dc.SetPen(*(isCurrent ? wxBLACK_PEN : wxTRANSPARENT_PEN));

        dc.DrawRectangle(rect);
    }
    //else: do nothing for the normal items
#endif  // ALLOW_NOTICES_SELECTION
}

void CBOINCVListBox::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    int x, y;
    int itemMax;
    wxRect rectItem;
    wxSize clientSize = GetClientSize();

    wxAutoBufferedPaintDC dc(this);
    DoPrepareDC(dc);

    // the update rectangle
    wxRect rectUpdate = GetUpdateClientRect();
    GetViewStart(&x, &y);
    rectUpdate.x += x*PIXELS_PER_HORIZONTAL_SCROLL_UNIT;
    rectUpdate.y += y*PIXELS_PER_VERTICAL_SCROLL_UNIT;

    // fill it with background colour
    dc.SetBackground(GetBackgroundColour());
    dc.Clear();

    // the bounding rectangle of the current item
    rectItem.width = clientSize.x;

    // iterate over all items
    itemMax = GetItemCount();
    for ( int item = 0; item < itemMax; item++ )
    {
        const wxCoord hItem = OnGetItemHeight(item);

        rectItem.height = hItem;

        // and draw the ones which intersect the update rect
        if ( rectItem.Intersects(rectUpdate) )
        {
            // don't allow drawing outside of the items rectangle
            wxDCClipper clip(dc, rectItem);

            wxRect rect = rectItem;
            OnDrawBackground(dc, rect, item);

            OnDrawSeparator(dc, rect, item);

            rect.Deflate(m_ptMargins.x, m_ptMargins.y);
            OnDrawItem(dc, rect, item);
        }
        else // no intersection
        {
            if ( rectItem.GetTop() > rectUpdate.GetBottom() )
            {
                // we are already below the update rect, no need to continue
                // further
                break;
            }
            //else: the next item may intersect the update rect
        }

        rectItem.y += hItem;
    }
}

void CBOINCVListBox::OnSize(wxSizeEvent& event)
{
    UpdateScrollbar();

    event.Skip();
}


#if ALLOW_NOTICES_SELECTION

// ============================================================================
// CBOINCVListBox keyboard/mouse handling
// ============================================================================

void CBOINCVListBox::DoHandleItemClick(int item, int flags)
{
    // has anything worth telling the client code about happened?
    bool notify = false;

    if ( HasMultipleSelection() )
    {
        // select the iteem clicked?
        bool select = true;

        // NB: the keyboard interface we implement here corresponds to
        //     wxLB_EXTENDED rather than wxLB_MULTIPLE but this one makes more
        //     sense IMHO
        if ( flags & ItemClick_Shift )
        {
            if ( m_current != wxNOT_FOUND )
            {
                if ( m_anchor == wxNOT_FOUND )
                    m_anchor = m_current;

                select = false;

                // only the range from the selection anchor to new m_current
                // must be selected
                if ( DeselectAll() )
                    notify = true;

                if ( SelectRange(m_anchor, item) )
                    notify = true;
            }
            //else: treat it as ordinary click/keypress
        }
        else // Shift not pressed
        {
            m_anchor = item;

            if ( flags & ItemClick_Ctrl )
            {
                select = false;

                if ( !(flags & ItemClick_Kbd) )
                {
                    Toggle(item);

                    // the status of the item has definitely changed
                    notify = true;
                }
                //else: Ctrl-arrow pressed, don't change selection
            }
            //else: behave as in single selection case
        }

        if ( select )
        {
            // make the clicked item the only selection
            if ( DeselectAll() )
                notify = true;

            if ( Select(item) )
                notify = true;
        }
    }

    // in any case the item should become the current one
    if ( DoSetCurrent(item) )
    {
        if ( !HasMultipleSelection() )
        {
            // this has also changed the selection for single selection case
            notify = true;
        }
    }

    if ( notify )
    {
        // notify the user about the selection change
        SendSelectedEvent();
    }
    //else: nothing changed at all
}

// ----------------------------------------------------------------------------
// keyboard handling
// ----------------------------------------------------------------------------

void CBOINCVListBox::OnKeyDown(wxKeyEvent& event)
{
    // flags for DoHandleItemClick()
    int flags = ItemClick_Kbd;

    int current;
    switch ( event.GetKeyCode() )
    {
        case WXK_HOME:
            current = 0;
            break;

        case WXK_END:
            current = GetLineCount() - 1;
            break;

        case WXK_DOWN:
            if ( m_current == (int)GetLineCount() - 1 )
                return;

            current = m_current + 1;
            break;

        case WXK_UP:
            if ( m_current == wxNOT_FOUND )
                current = GetLineCount() - 1;
            else if ( m_current != 0 )
                current = m_current - 1;
            else // m_current == 0
                return;
            break;

        case WXK_PAGEDOWN:
            PageDown();
            current = GetFirstVisibleLine();
            break;

        case WXK_PAGEUP:
            if ( m_current == (int)GetFirstVisibleLine() )
            {
                PageUp();
            }

            current = GetFirstVisibleLine();
            break;

        case WXK_SPACE:
            // hack: pressing space should work like a mouse click rather than
            // like a keyboard arrow press, so trick DoHandleItemClick() in
            // thinking we were clicked
            flags &= ~ItemClick_Kbd;
            current = m_current;
            break;

#ifdef __WXMSW__
        case WXK_TAB:
            // Since we are using wxWANTS_CHARS we need to send navigation
            // events for the tabs on MSW
            {
                wxNavigationKeyEvent ne;
                ne.SetDirection(!event.ShiftDown());
                ne.SetCurrentFocus(this);
                ne.SetEventObject(this);
                GetParent()->GetEventHandler()->ProcessEvent(ne);
            }
            // fall through to default
#endif
        default:
            event.Skip();
            current = 0; // just to silent the stupid compiler warnings
            wxUnusedVar(current);
            return;
    }

    if ( event.ShiftDown() )
       flags |= ItemClick_Shift;
    if ( event.ControlDown() )
        flags |= ItemClick_Ctrl;

    DoHandleItemClick(current, flags);
}

// ----------------------------------------------------------------------------
// CBOINCVListBox mouse handling
// ----------------------------------------------------------------------------

void CBOINCVListBox::OnLeftDown(wxMouseEvent& event)
{
    SetFocus();

    int item = HitTest(event.GetPosition());

    if ( item != wxNOT_FOUND )
    {
        int flags = 0;
        if ( event.ShiftDown() )
           flags |= ItemClick_Shift;

        // under Mac Apple-click is used in the same way as Ctrl-click
        // elsewhere
#ifdef __WXMAC__
        if ( event.MetaDown() )
#else
        if ( event.ControlDown() )
#endif
            flags |= ItemClick_Ctrl;

        DoHandleItemClick(item, flags);
    }
}

void CBOINCVListBox::OnLeftDClick(wxMouseEvent& eventMouse)
{
    int item = HitTest(eventMouse.GetPosition());
    if ( item != wxNOT_FOUND )
    {

        // if item double-clicked was not yet selected, then treat
        // this event as a left-click instead
        if ( item == m_current )
        {
            wxCommandEvent event(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, GetId());
            event.SetEventObject(this);
            event.SetInt(item);

            (void)GetEventHandler()->ProcessEvent(event);
        }
        else
        {
            OnLeftDown(eventMouse);
        }

    }
}

#endif      // ALLOW_NOTICES_SELECTION

// ----------------------------------------------------------------------------
// use the same default attributes as wxListBox
// ----------------------------------------------------------------------------

//static
wxVisualAttributes
CBOINCVListBox::GetClassDefaultAttributes(wxWindowVariant variant)
{
    return wxListBox::GetClassDefaultAttributes(variant);
}

#endif