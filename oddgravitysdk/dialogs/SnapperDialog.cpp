// SnapperDialog.cpp: implementation of the CSnapperDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SnapperDialog.h"
#include "../system/MultiMon/MultiMonitor.h"
#include "../system/MultiMon/Monitors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)    ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)    ((int)(short)HIWORD(lParam))
#endif

// ================================================================
//  helper
// ----------------------------------------------------------------

bool absless(long a, long b)
{
    return abs(a) < abs(b);
}


// ================================================================
//  SnapInfo::Init
// ----------------------------------------------------------------

void SNAPINFO::Init(RECT const & oldRect, RECT const & newRect, DWORD snapwidth)
{
    rold = oldRect;
    rnew = newRect;

    rout.left     = rnew.left   + MAXDELTA;
    rout.top      = rnew.top    + MAXDELTA;
    rout.right    = rnew.right  + MAXDELTA;
    rout.bottom   = rnew.bottom + MAXDELTA;

    rdelta.left   = rold.left   - rnew.left;
    rdelta.top    = rold.top    - rnew.top;
    rdelta.right  = rold.right  - rnew.right;
    rdelta.bottom = rold.bottom - rnew.bottom;

    snapWidth = snapwidth;
}


// ================================================================
//  SnapInfo::Init(r, snapwidth)
// ----------------------------------------------------------------

void SNAPINFO::Init(RECT const & r, DWORD snapwidth, bool moveOnly)
{
    RECT rnew = r;
    if (!moveOnly) {    // dirty little trick: "size & move" is tested in SnapLine
        ++rnew.left;    // based on rold and rnew
        ++rnew.top;     // rnew has no other influence
    }

    Init(r, rnew, snapWidth);
}


// ================================================================
//  SnapInfo::SnapVLine
// ----------------------------------------------------------------

void SNAPINFO::SnapVLine(long x)    
{
    if (!rdelta.left && !rdelta.right)  // if none of the coords changed we can't snap
        return;

    long dleft  = rnew.left - x;        // sign must match rdelta calculation
    long dright = rnew.right - x;

    if (!rdelta.left)   dleft  = MAXDELTA;  // "SameSign": did move in this direction - and did move at all
    if (!rdelta.right)  dright = MAXDELTA;

    if (rdelta.left == rdelta.right) {      // keep width
        if (absless(dleft,dright)) dright = MAXDELTA;
        else if (absless(dright, dleft)) dleft = MAXDELTA;

        if (absless(dleft, rout.left-rnew.left)) {
            rout.right = x+(rnew.right-rnew.left);
            rout.left = x;
        }
        else if (absless(dright, rnew.right-rout.right)) {
            rout.left = x - (rnew.right-rnew.left);
            rout.right = x;
        }
    }
    else {  // can change width

        if (absless(dleft, rout.left-rnew.left)) 
            rout.left = x;
        else if (absless(dright, rout.right-rnew.right))
            rout.right = x;
    }
}


// ================================================================
//  SnapInfo::SnapHLine
// ----------------------------------------------------------------

void SNAPINFO::SnapHLine(long y)    
{
    if (!rdelta.top && !rdelta.bottom)  // if none of the coords changed we can't snap
        return;

    long dtop  = rnew.top - y;      // sign must match rdelta calculation
    long dbottom = rnew.bottom - y;

    if (!rdelta.top)   dtop  = MAXDELTA;    // "SameSign": did move in this direction - and did move at all
    if (!rdelta.bottom)  dbottom = MAXDELTA;

    if (rdelta.top == rdelta.bottom) {      // keep width
        if (absless(dtop,dbottom)) dbottom = MAXDELTA;
        else if (absless(dbottom, dtop)) dtop = MAXDELTA;

        if (absless(dtop, rout.top-rnew.top)) {
            rout.bottom = y+(rnew.bottom-rnew.top);
            rout.top = y;
        }
        else if (absless(dbottom, rnew.bottom-rout.bottom)) {
            rout.top = y - (rnew.bottom-rnew.top);
            rout.bottom = y;
        }
    }
    else {  // can change width

        if (absless(dtop, rout.top-rnew.top)) 
            rout.top = y;
        else if (absless(dbottom, rout.bottom-rnew.bottom))
            rout.bottom = y;
    }
}


// ================================================================
//  SnapInfo::EndSnap
// ----------------------------------------------------------------

RECT &SNAPINFO::EndSnap()
{
    if (abs(rout.left  - rnew.left  ) > (long)snapWidth) rout.left  = rnew.left;
    if (abs(rout.top   - rnew.top   ) > (long)snapWidth) rout.top   = rnew.top;
    if (abs(rout.right - rnew.right ) > (long)snapWidth) rout.right = rnew.right;
    if (abs(rout.bottom- rnew.bottom) > (long)snapWidth) rout.bottom= rnew.bottom;

    return rout;
}


// ========================================================
//  CTor/DTor
// --------------------------------------------------------

CSnapperDialog::CSnapperDialog(DWORD snapWidth)
{
    m_bSizeMoveIsSysCommand = false;
    SetSnapWidth(snapWidth);
    SetSnapToAllWindows(false);
}


CSnapperDialog::~CSnapperDialog()
{

}


// ==================================================================
//  CSnapperDialog::AllowSnap
// ------------------------------------------------------------------
///
/// decides whether snapping is enabled.
/// 
/// \par Default Implementation
///     The default implementation returns false when either Shift is pressed, 
///     or \c m_bSizeMoveIsSysCommand is true. \c m_bSizeMoveIsSysCommand indicates
///     that the current sizing / moving command was initiated through the menu.
/// 
/// \par Override
///     A derived class can override this function to modify the conditions that disable
///     snapping. 
/// 
bool CSnapperDialog::AllowSnap()
{
    if (m_bSizeMoveIsSysCommand)
        return false;

    bool shiftPressed = GetAsyncKeyState(VK_SHIFT) < 0; 
    return !shiftPressed;
}


// ========================================================
//  Init/Set/Get
// --------------------------------------------------------

void CSnapperDialog::SetSnapWidth(DWORD snapWidth)
{
    m_snapWidth = snapWidth;
}


// ========================================================
//  Sizing : Calculate new rect
// --------------------------------------------------------

void CSnapperDialog::Sizing(CWnd* wnd, RECT& rnew)
{
    RECT rold;
    wnd->GetWindowRect(&rold);

    //CWnd* parent = wnd->GetParent();
    CWnd* parent = CWnd::FromHandle(::GetDesktopWindow());
    _ASSERTE(parent);
    //parent->ScreenToClient(&rold);
    //parent->ScreenToClient(&rnew);

    SNAPINFO sni;
    sni.Init(rold, rnew, m_snapWidth);

    // get working area (multi-monitor aware)
    // note: use the passed rect for getting the nearest monitor because when the
    //       option "Show window contents while dragging" is off, the actual
    //       window remains on its place until moving is completed.
    RECT r;
    CMonitor monitor = CMonitors::GetNearestMonitor(&rnew);
    monitor.GetWorkAreaRect(&r);
    //parent->GetClientRect(&r);

    // use the outer rect
    sni.SnapVLine(r.left);
    sni.SnapVLine(r.right);
    sni.SnapHLine(r.top);
    sni.SnapHLine(r.bottom);

    // check whether snapping to all windows
    if(m_bSnapToAllWindows)
    {
        // iterate through all other visible children
        CWnd* child = parent->GetWindow(GW_CHILD);
        while(child)
        {
            if(child->IsWindowVisible() && child->m_hWnd != wnd->m_hWnd)
            {
                RECT r;
                child->GetWindowRect(&r);
                parent->ScreenToClient(&r);
                sni.SnapHLine(r.top);
                sni.SnapHLine(r.bottom);
                sni.SnapVLine(r.left);
                sni.SnapVLine(r.right);
            }
            child = child->GetNextWindow();
        }
    }
    sni.EndSnap();

    rnew = sni.rout; 
    //parent->ClientToScreen(&rnew);
}


// ================================================================
//  OnMessage
// ----------------------------------------------------------------

LRESULT CSnapperDialog::OnMessage(CWnd* wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg)
    {
        case WM_MOVING          :   if (AllowSnap()) OnMoving(wnd, wp, (LPRECT) lp);    break;
        case WM_SIZING          :   if (AllowSnap()) OnSizing(wnd, wp, (LPRECT) lp);    break;
        case WM_ENTERSIZEMOVE   :   if (AllowSnap()) OnEnterSizeMove(wnd);              break;
        case WM_EXITSIZEMOVE    :   OnExitSizeMove();                                   break;

        case WM_SYSCOMMAND      :   
            if (wp == SC_MOVE || wp == SC_SIZE)
                m_bSizeMoveIsSysCommand = true;
            break;
    };

    return 0;
}


// ================================================================
//  OnEnterSizeMove
// ----------------------------------------------------------------

void CSnapperDialog::OnEnterSizeMove(CWnd* wnd)
{
    if(m_wndMoving)
        OnExitSizeMove();

    m_wndMoving = wnd;
    DWORD dw = GetMessagePos();
    m_mouseStart.x = GET_X_LPARAM(dw);
    m_mouseStart.y = GET_Y_LPARAM(dw);

    wnd->GetWindowRect(&m_rectStart);
}


// ================================================================
//  OnExitSizeMove
// ----------------------------------------------------------------

void CSnapperDialog::OnExitSizeMove()
{
    m_bSizeMoveIsSysCommand = false;
    m_wndMoving = NULL;
    //m_swapOther = NULL;
}


// ================================================================
//  OnSizing
// ----------------------------------------------------------------

void CSnapperDialog::OnSizing(CWnd* wnd, WPARAM edge, LPRECT newrect)
{
    Sizing(wnd, *newrect);
}


//CWnd * CSnapperDialog::PickSwapWindow(CWnd * current, POINT mouse)
//{
//    // implementation: find the first sibling the mouse is in, ignoring peer itself
//    // but make sure to use original coordinates on swapped windows
//
//    RECT r;
//
//    CWnd * sibling = current->GetWindow(GW_HWNDFIRST);
//    _ASSERTE(sibling); // we should at least find ourselves!
//    do
//    {
//        // skip the current window
//        if (sibling->m_hWnd == current->m_hWnd)
//            continue;
//
//        if (m_swapOther && sibling->m_hWnd == m_swapOther->m_hWnd)
//            r = m_swapOtherRect;
//        else
//            sibling->GetWindowRect(&r);
//
//        if (PtInRect(&r, mouse))
//            return sibling;
//    } while ( (sibling = sibling->GetWindow(GW_HWNDNEXT)) != NULL);
//
//    return NULL;
//}


// ================================================================
//  OnMoving
// ----------------------------------------------------------------

void CSnapperDialog::OnMoving(CWnd* wnd, WPARAM edge, LPRECT r)
{
    DWORD dw = GetMessagePos();
    POINT mouse = { GET_X_LPARAM(dw), GET_Y_LPARAM(dw) };

    //if(GetAsyncKeyState(VK_CONTROL) < 0)
    //{
    //    CWnd * swapWith = PickSwapWindow(wnd, mouse);

    //    // first, swap back if closest window has changed
    //    if (swapWith != m_swapOther && m_swapOther != NULL)
    //    {
    //        RECT r = m_swapOtherRect;
    //        wnd->GetParent()->ScreenToClient(&r);
    //        m_swapOther->MoveWindow(&r);
    //        m_swapOther = NULL; 
    //    }

    //    //wnd->GetParent()->ScreenToClient(r);

    //    // swap with other window:
    //    if (swapWith && swapWith != m_swapOther)
    //    {
    //        m_swapOther = swapWith;
    //        m_swapOther->GetWindowRect(&m_swapOtherRect);

    //        RECT rnpOther = m_rectStart; 
    //        wnd->GetParent()->ScreenToClient(&rnpOther);
    //        swapWith->MoveWindow(&rnpOther);

    //        *r = m_swapOtherRect;
    //        wnd->GetParent()->UpdateWindow();
    //    }

    //    *r = m_swapOther ? m_swapOtherRect : m_rectStart;
    //}
    //else
    {
        POINT offs1 = { mouse.x - m_mouseStart.x, mouse.y - m_mouseStart.y  }; 
        POINT org   = { offs1.x + m_rectStart.left, offs1.y + m_rectStart.top };

        if (offs1.x || offs1.y) {
            r->right  = org.x + (r->right - r->left);
            r->bottom = org.y + (r->bottom - r->top);
            r->left   = org.x;
            r->top    = org.y;
        }

        Sizing(wnd, *r);
    }
}
