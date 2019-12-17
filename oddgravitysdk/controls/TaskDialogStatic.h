/*----------------------------------------------------------------------------
| File:    TaskDialogStatic.h
| Project: oddgravitysdk
|
| Description:
|   Implementation of the CTaskDialogStatic class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 184 $
| $Id: TaskDialogStatic.h 184 2007-06-02 16:22:38Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once


// callback for moving dialog controls downwards if controls above need extra space.
// if top of control is below threshold (y ofs greater), adjust it.
struct MoveInfoType {int m_yCoord, m_yAdjust;};
static BOOL CALLBACK EnumChildProc_AdjustClientRect(HWND hWnd, LPARAM lParam)
{
    CRect windowRect;
    ::GetWindowRect(hWnd, &windowRect);

    HWND hWndParent = ::GetParent(hWnd);
    MoveInfoType* info = (MoveInfoType*)lParam;

    if((windowRect.top >= info->m_yCoord) && hWndParent)
    {
        ::ScreenToClient(hWndParent, &windowRect.TopLeft());
        ::SetWindowPos(hWnd, NULL, windowRect.left, windowRect.top + info->m_yAdjust, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE);
    }
    return TRUE;
}


// CTaskDialogStatic

class CTaskDialogStatic : public CStatic
{
  CFont *m_bigfont;
  BOOL m_fgcolor_valid, m_bgcolor_valid;
  COLORREF m_fgcolor, m_bgcolor;

public:
  // Constructor/Destructor...
  CTaskDialogStatic() : CStatic()
  {
    m_bigfont = NULL; 
    SetFontColor(-1);
    SetBackgroundColor(-1);
  }

  virtual ~CTaskDialogStatic()
  {
    if (m_bigfont) delete m_bigfont; 
    m_bigfont = NULL;
  }

  // Operations...
  void SetFontScale(double ratio)
  {
    if (m_bigfont) delete m_bigfont;
    // Get system message font...
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
    LOGFONT lf = ncm.lfMessageFont;
    // Scale it & create font...
    lf.lfHeight = (DWORD)(lf.lfHeight*ratio);
    m_bigfont = new CFont();
    m_bigfont->CreateFontIndirect(&lf);
    if (m_hWnd) if (::IsWindow(m_hWnd)) Invalidate();
  }

  void SetFontColor(COLORREF fgcolor)
  {
    m_fgcolor_valid = (fgcolor != -1);
    m_fgcolor = (m_fgcolor_valid) ? fgcolor : ::GetSysColor(COLOR_BTNTEXT);
  }

  void SetBackgroundColor(COLORREF bgcolor)
  {
    m_bgcolor_valid = (bgcolor != -1);
    m_bgcolor = (m_bgcolor_valid) ? bgcolor : ::GetSysColor(COLOR_BTNFACE);
  }

  // Adjust size of client rect if text too large...
  int AdjustClientRect (CDC *pDC, CRect *oldrw=NULL)
  {
    if (m_bigfont==NULL) SetFontScale(1.0);
    CFont *old_font = pDC->SelectObject(m_bigfont);
    CRect rc; GetClientRect(rc);
    CString stext; GetWindowText(stext);
    CRect use_rc(rc);
    use_rc.bottom = 9999;
    pDC->DrawText(stext, use_rc, DT_CALCRECT|DT_LEFT|DT_TOP|DT_WORDBREAK); 
    pDC->SelectObject(old_font);
    if (use_rc.bottom > rc.bottom) {
      int yadjust = use_rc.bottom - rc.bottom;
      CRect rw; GetWindowRect(rw);
      if (oldrw) *oldrw = rw;
      SetWindowPos(NULL, 0, 0, rw.Width(), rw.Height()+yadjust, SWP_NOZORDER|SWP_NOMOVE); 
      Invalidate();
      return yadjust;
    }
    return 0;
  }

  // Message Handlers...
  afx_msg void OnPaint()
  {
    if (m_bigfont==NULL) SetFontScale(1.0);
    CPaintDC dc(this);
    CRect rc; GetClientRect(rc);
    CWnd *parent = GetParent();
    // Query parent for background color with WM_CTLCOLOR message...
    HBRUSH hBrush = NULL;
    if (parent) hBrush = (HBRUSH)(parent->SendMessage(
      WM_CTLCOLORSTATIC, (WPARAM)dc.GetSafeHdc(), (LPARAM)GetSafeHwnd()));
    if (!m_bgcolor_valid && hBrush)
      dc.FillRect(rc, CBrush::FromHandle(hBrush));
    else dc.FillSolidRect(rc, m_bgcolor);
    // Select font...
    CFont *old_font = dc.SelectObject(m_bigfont);
    // Remove extrenal/internal leading so actual text position better controled...
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    rc.top -= (tm.tmExternalLeading+tm.tmInternalLeading)/2;
    // Draw text...
    if (m_fgcolor_valid) dc.SetTextColor(m_fgcolor);
    if (m_bgcolor_valid) dc.SetBkColor(m_bgcolor);
    CString stext; GetWindowText(stext);
    // Vertically center text...
    CRect use_rc(rc);
    dc.DrawText(stext, use_rc, DT_CALCRECT|DT_LEFT|DT_TOP|DT_WORDBREAK);
    if (use_rc.Height()<rc.Height()) rc.top += (rc.Height()-use_rc.Height())/2;
    dc.DrawText(stext, rc, DT_LEFT|DT_TOP|DT_WORDBREAK);
    // Cleanup...
    dc.SelectObject(old_font);
  }

  afx_msg void OnSysColorChange()
  {
    if (!m_bgcolor_valid) m_bgcolor = ::GetSysColor(COLOR_BTNFACE);
  }

  DECLARE_MESSAGE_MAP()
};

