//********************************************************************************************
//* HYPEREDIT.CPP - Custom window message handler(s) implementation                          *
//*                                                                                          *
//* This module contains implementation code for object initialization and default windows   *
//* messages.                                                                                *
//*                                                                                          *
//* Aug.31.04                                                                                *
//*                                                                                          *
//* Copyright PCSpectra 2004 (Free for any purpose, except to sell indivually)               *
//* Website: www.pcspectra.com                                                               *
//*                                                                                          *
//* Notes:                                                                                   *
//* ======                                                                                   *
//* Search module for 'PROGRAMMERS NOTE'                                                     *
//*                                                                                          *
//* History:                                                                                 *
//* ========                                                                                 *
//* Mon.dd.yy - None so far                                                                  *
//********************************************************************************************

#include "stdafx.h"
#include "HyperEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Object construction

CHyperEdit::CHyperEdit()
{
    m_bTrackMouse = false;
    m_bHoveringHyperText = false;

    m_nTimer = 0;

    m_nLineHeight = 0;

    m_hHandCursor = NULL;

    // default hyperlink colors
    m_clrNormal = RGB(92,92,154);
    m_clrHover  = RGB(168,168,230);

    // font smoothing fix
    m_bFontSmoothingFixEnabled = false;
    m_clrBackground = GetSysColor(COLOR_WINDOW);

    m_csPreviousUrl.Empty();
    m_csLocation.Empty();
}

// Object destruction

CHyperEdit::~CHyperEdit()
{
    if (m_fontOverlay.m_hObject != NULL)
    {
        m_fontOverlay.DeleteObject();
    }

    if (m_hHandCursor != NULL)
    {
        VERIFY(DestroyCursor(m_hHandCursor));
    }
}
          
BEGIN_MESSAGE_MAP(CHyperEdit, CEdit)
    //{{AFX_MSG_MAP(CHyperEdit)
    ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDOWN()
    //}}AFX_MSG_MAP
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// Control initialization

void CHyperEdit::PreSubclassWindow() 
{
    CEdit::PreSubclassWindow();

    ASSERT(GetStyle() & ES_MULTILINE);  // MUST have multiline style
    ASSERT(GetStyle() & WS_VSCROLL);    // MUST have vertical scrollbar
    //ASSERT(GetStyle() & ES_AUTOVSCROLL);

    CDC* pDC = GetDC();
    ASSERT(pDC);

    // initialize URL overlay font
    InitUrlOverlayFont();

    // initialize hand cursor
    SetDefaultCursor();

    // calculate single line height
    m_nLineHeight = pDC->DrawText("Test Line", CRect(0,0,0,0), DT_SINGLELINE | DT_CALCRECT);

    // PROGRAMMERS NOTE:
    // =================
    // If the hyperlinks flicker when changing the selection state
    // of the edit control change the timer value to a lower count
    // 10 is the default and appears to have an almost flicker free
    // transition from selection to hyperlinked colors.
    m_nTimer = SetTimer(IDT_SELCHANGE, 10, NULL);

    // free resources
    ReleaseDC(pDC);
}

// Override mouse movements

void CHyperEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
    // check if mouse tracking needs to be activated
    if(!m_bTrackMouse)
    {
        m_bTrackMouse = true;

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = GetSafeHwnd();
        _TrackMouseEvent(&tme);
    }

    CEdit::OnMouseMove(nFlags, point);                        
    CString csURL = IsHyperlink(point);

    // If not empty, then display hand cursor
    if(!csURL.IsEmpty()) // we are hovering hypertext
    {
        // Get the coordinates of last character in entire buffer
        CPoint pt_lastchar = PosFromChar(GetWindowTextLength() - 1);

        // Don't bother changing mouse cursor if it's below last visible character
        if(point.y <= (pt_lastchar.y + (long)m_nLineHeight))
            ::SetCursor(m_hHandCursor);

        if (!m_bHoveringHyperText || csURL != m_csPreviousUrl) // only redraw if state has changed
        {
            DrawHyperlinks();            
            m_bHoveringHyperText = true; // store new state
            m_csPreviousUrl = csURL; // store current url
        }
    }
    else // we are not hovering hypertext
    {
        if (m_bHoveringHyperText) // only redraw if state has changed
        {
            DrawHyperlinks();
            m_bHoveringHyperText = false; //store new state
        }
    }
}

/*
// deprecated implementation
// (see: http://www.codeproject.com/editctrl/multi_hyperedit.asp?df=100&forumid=99288&select=1158265#xx1158265xx)
void CHyperEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
    CEdit::OnMouseMove(nFlags, point);
                                      
    CString csURL = IsHyperlink(point);

    // If not empty, then display hand cursor
    if(!csURL.IsEmpty()){
    
        // Get the coordinates of last character in entire buffer
        CPoint pt_lastchar = PosFromChar(GetWindowTextLength()-1);

        // Don't bother changing mouse cursor if it's below last visible character
        if(point.y<=(pt_lastchar.y+m_nLineHeight))      
            ::SetCursor(m_hHandCursor);
    }

    DrawHyperlinks();
}
*/

LRESULT CHyperEdit::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    m_bTrackMouse = false;
    Invalidate();

    return S_OK;
}

// Override left mouse button down (Clicking)

void CHyperEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
    m_csLocation = IsHyperlink(point);

    CEdit::OnLButtonDown(nFlags, point);
}

// Override left mouse button up (Clicking)

void CHyperEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
    CEdit::OnLButtonUp(nFlags, point);

    int iSelStart = 0, iSelFinish = 0;
    GetSel(iSelStart, iSelFinish);

    // If there is a selection, just exit now, we don't open URL's
    if(IsSelection(iSelStart, iSelFinish)) return;

    // If were below the last visible character exit again, cuz we don't want 
    // to open any URL's that aren't directly clicked on
    // Get the coordinates of last character in entire buffer
    CPoint pt = PosFromChar(GetWindowTextLength() - 1);

    // Exit if mouse is below last visible character
    if(point.y > (pt.y + (long)m_nLineHeight)) return; 

    CString csURL = IsHyperlink(point);

    // If not empty, then open browser and show web site
    // only if the URL is the same as one clicked on in OnLButtonDown()
    if(!csURL.IsEmpty() && (m_csLocation == csURL))
        GotoURL(csURL, SW_SHOW);
}

// Override low level message handling

LRESULT CHyperEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    // handling OnPaint() didn't work
    if(message == WM_PAINT)
    {
        CEdit::WindowProc(message, wParam, lParam);

        DrawHyperlinks();

        return TRUE;
    }
    
    return CEdit::WindowProc(message, wParam, lParam);
}

// Emulate an OnSelChange() event using a low interval timer (initialized in PreSubclassWindow)

void CHyperEdit::OnTimer(UINT nIDEvent) 
{
    //
    // Emulate a OnSelChange() event
    //

    static int iPrevStart = 0, iPrevFinish = 0;
    
    DWORD dwSel = GetSel(); 

    // Check the previous start/finish of selection range
    // and compare them against the current selection range
    // if there is any difference between them fire off an OnSelChange event
    if(LOWORD(dwSel) != iPrevStart || HIWORD(dwSel) != iPrevFinish)
        OnSelChange();

    // Save current selection state for next call (as previous state)
    iPrevStart = LOWORD(dwSel);
    iPrevFinish = HIWORD(dwSel);

    CEdit::OnTimer(nIDEvent);
}

BOOL CHyperEdit::OnEraseBkgnd(CDC* pDC)
{
    return CEdit::OnEraseBkgnd(pDC);
}

void CHyperEdit::InitUrlOverlayFont()
{
    // destroy old font
    if (m_fontOverlay.m_hObject != NULL)
    {
        m_fontOverlay.DeleteObject();
    }

    // get current font
    CFont* pFont = GetFont();
    if(!pFont)
    {
        ASSERT(0);
        return;
    }

    // set underline style
    LOGFONT lf;
    pFont->GetLogFont(&lf);

    // TODO: Check if there is a system setting to see if hyperlinks should be underlined by default or not
    lf.lfUnderline = TRUE;

    // create overlay font
    m_fontOverlay.CreateFontIndirect(&lf);
}
