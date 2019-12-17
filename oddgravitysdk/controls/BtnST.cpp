#include "stdafx.h"
#include "BtnST.h"

#ifdef	BTNST_USE_SOUND
#pragma comment(lib, "winmm.lib")
#include <Mmsystem.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButtonST

// Mask for control's type
#ifdef BS_TYPEMASK
#undef BS_TYPEMASK
#endif
#define BS_TYPEMASK SS_TYPEMASK

#ifndef	TTM_SETTITLE
#define TTM_SETTITLEA           (WM_USER + 32)  // wParam = TTI_*, lParam = char* szTitle
#define TTM_SETTITLEW           (WM_USER + 33)  // wParam = TTI_*, lParam = wchar* szTitle
#ifdef	UNICODE
#define TTM_SETTITLE            TTM_SETTITLEW
#else
#define TTM_SETTITLE            TTM_SETTITLEA
#endif
#endif

#if WINVER < 0x0500
#define WM_QUERYUISTATE         0x0129
#define UISF_HIDEFOCUS          0x1
#define UISF_HIDEACCEL          0x2
#define DT_HIDEPREFIX           0x00100000
#endif

#ifndef	TTS_BALLOON
#define	TTS_BALLOON		0x40
#endif

#pragma warning (push)
#pragma warning (disable : 4312)


CButtonST::CButtonST()
{
	m_bIsPressed		= FALSE;
	m_bIsFocused		= FALSE;
	m_bIsDisabled		= FALSE;
	m_bMouseOnButton	= FALSE;

	FreeResources(FALSE);

	// Default type is "flat" button
	m_bIsFlat = TRUE;
	// Button will be tracked also if when the window is inactive (like Internet Explorer)
	m_bAlwaysTrack = TRUE;

	// By default draw border in "flat" button 
	m_bDrawBorder = TRUE; 

	// By default icon is aligned horizontally (image on the left, text on the right)
    m_eAlignmentStyle = BTNST_ALIGN_HORZ;

	// By default use pressed style with the usual behavior
	SetPressedStyle(BTNST_PRESSED_LEFTRIGHT, FALSE);

	// By default, for "flat" buttons, draw the focus rect (if activated by the user)
	m_bDrawFlatFocus = TRUE;

	// By default the button is not the default button
	m_bIsDefault = FALSE;
	// Invalid value, since type still unknown
	m_nTypeStyle = BS_TYPEMASK;

	// By default the button is not a checkbox
	m_bIsCheckBox = FALSE;
	m_nCheck = 0;

	// Set default colors
	SetDefaultColors(FALSE);

	// No tooltip created
	m_ToolTip.m_hWnd = NULL;
	m_dwToolTipStyle = 0;
	m_strTooltipText = _T("");

	// Do not draw as a transparent button
	m_bDrawTransparent = FALSE;
	m_pbmpOldBk = NULL;

	// No URL defined
	SetURL(NULL);

	// No cursor defined
	m_hCursor = NULL;

	// No associated menu
#ifndef	BTNST_USE_BCMENU
	m_hMenu = NULL;
#endif
	m_hParentWndMenu = NULL;
	m_bMenuDisplayed = FALSE;

	m_bShowDisabledBitmap = TRUE;

	m_ptImageOrg.x = 3;
	m_ptImageOrg.y = 3;

	// no icon or text offset
	m_iIconOffsetX = 0;
	m_iIconOffsetY = 0;
	m_iTextOffsetX = 0;
	m_iTextOffsetY = 0;

	// No defined callbacks
	::ZeroMemory(&m_csCallbacks, sizeof(m_csCallbacks));

#ifdef	BTNST_USE_SOUND
	// No defined sounds
	::ZeroMemory(&m_csSounds, sizeof(m_csSounds));
#endif

	m_pLF = NULL;	//font structure pointer

}

CButtonST::~CButtonST()
{
	// Restore old bitmap (if any)
	if (m_dcBk.m_hDC && m_pbmpOldBk)
	{
		m_dcBk.SelectObject(m_pbmpOldBk);
	} // if

	FreeResources();

	// Destroy the cursor (if any)
	if (m_hCursor) ::DestroyCursor(m_hCursor);

	// Destroy the menu (if any)
#ifdef	BTNST_USE_BCMENU
	if (m_menuPopup.m_hMenu)	m_menuPopup.DestroyMenu();
#else
	if (m_hMenu)	::DestroyMenu(m_hMenu);
#endif

	if(m_font.m_hObject)
	{
		m_font.DeleteObject();
	}

	if(m_pLF)
	{
		free(m_pLF);
	}

}

IMPLEMENT_DYNAMIC(CButtonST, CButton)

BEGIN_MESSAGE_MAP(CButtonST, CButton)
    //{{AFX_MSG_MAP(CButtonST)
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCOLORCHANGE()
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	ON_WM_ACTIVATE()
	ON_WM_ENABLE()
	ON_WM_CANCELMODE()
	ON_WM_GETDLGCODE()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
#ifdef	BTNST_USE_BCMENU
	ON_WM_MENUCHAR()
	ON_WM_MEASUREITEM()
#endif

	ON_MESSAGE(BM_SETSTYLE, OnSetStyle)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_MESSAGE(BM_GETCHECK, OnGetCheck)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CButtonST::FreeResources(BOOL bCheckForNULL)
{
	if (bCheckForNULL)
	{
		// Destroy icons
		// Note: the following three lines MUST be here! even if
		// BoundsChecker says they are unnecessary!
		if (m_csIcons[0].hIcon)	::DestroyIcon(m_csIcons[0].hIcon);
		if (m_csIcons[1].hIcon)	::DestroyIcon(m_csIcons[1].hIcon);
		if (m_csIcons[2].hIcon)	::DestroyIcon(m_csIcons[2].hIcon);
		if (m_csIcons[3].hIcon)	::DestroyIcon(m_csIcons[3].hIcon);

		// Destroy bitmaps
		if (m_csBitmaps[0].hBitmap)	::DeleteObject(m_csBitmaps[0].hBitmap);
		if (m_csBitmaps[1].hBitmap)	::DeleteObject(m_csBitmaps[1].hBitmap);
		if (m_csBitmaps[2].hBitmap)	::DeleteObject(m_csBitmaps[2].hBitmap);
		if (m_csBitmaps[3].hBitmap)	::DeleteObject(m_csBitmaps[3].hBitmap);

		// Destroy mask bitmaps
		if (m_csBitmaps[0].hMask)	::DeleteObject(m_csBitmaps[0].hMask);
		if (m_csBitmaps[1].hMask)	::DeleteObject(m_csBitmaps[1].hMask);
		if (m_csBitmaps[2].hMask)	::DeleteObject(m_csBitmaps[2].hMask);
		if (m_csBitmaps[3].hMask)	::DeleteObject(m_csBitmaps[3].hMask);
	} // if

	::ZeroMemory(&m_csIcons, sizeof(m_csIcons));
	::ZeroMemory(&m_csBitmaps, sizeof(m_csBitmaps));
}

void CButtonST::PreSubclassWindow() 
{
	UINT nBS;

	nBS = GetButtonStyle();

	// Set initial control type
	m_nTypeStyle = nBS & BS_TYPEMASK;

	// Check if this is a checkbox
	if (nBS & BS_CHECKBOX) m_bIsCheckBox = TRUE;

	// Set initial default state flag
	if (m_nTypeStyle == BS_DEFPUSHBUTTON)
	{
		// Set default state for a default button
		m_bIsDefault = TRUE;

		// Adjust style for default button
		m_nTypeStyle = BS_PUSHBUTTON;
	} // If

	// You should not set the Owner Draw before this call
	// (don't use the resource editor "Owner Draw" or
	// ModifyStyle(0, BS_OWNERDRAW) before calling PreSubclassWindow() )
	ASSERT(m_nTypeStyle != BS_OWNERDRAW);

	// Switch to owner-draw
	ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

UINT CButtonST::OnGetDlgCode() 
{
	UINT nCode = CButton::OnGetDlgCode();

	// Tell the system if we want default state handling
	// (losing default state always allowed)
	nCode |= (m_bIsDefault ? DLGC_DEFPUSHBUTTON : DLGC_UNDEFPUSHBUTTON);

	return nCode;
}

BOOL CButtonST::PreTranslateMessage(MSG* pMsg) 
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);
	
	if (pMsg->message == WM_LBUTTONDBLCLK)
		pMsg->message = WM_LBUTTONDOWN;

	return CButton::PreTranslateMessage(pMsg);
}

HBRUSH CButtonST::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	return (HBRUSH)::GetStockObject(NULL_BRUSH); 
}

void CButtonST::OnSysColorChange() 
{
	CButton::OnSysColorChange();

	m_dcBk.DeleteDC();
	m_bmpBk.DeleteObject();	
	SetDefaultColors();
}

LRESULT CButtonST::OnSetStyle(WPARAM wParam, LPARAM lParam)
{
	UINT nNewType = (UINT)(wParam & BS_TYPEMASK);

	// Update default state flag
	if (nNewType == BS_DEFPUSHBUTTON)
	{
		m_bIsDefault = TRUE;
	} // if
	else if (nNewType == BS_PUSHBUTTON)
	{
		// Losing default state always allowed
		m_bIsDefault = FALSE;
	} // if

	// Can't change control type after owner-draw is set.
	// Let the system process changes to other style bits
	// and redrawing, while keeping owner-draw style
	return DefWindowProc(BM_SETSTYLE,
		(wParam & ~BS_TYPEMASK) | BS_OWNERDRAW, lParam);
}

LRESULT CButtonST::OnSetCheck(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_bIsCheckBox);

	switch (wParam)
	{
		case BST_CHECKED:
		case BST_INDETERMINATE:	// Indeterminate state is handled like checked state
			SetCheck(1);
			break;
		default:
			SetCheck(0);
			break;
	} // switch

	return 0;
}

LRESULT CButtonST::OnGetCheck(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_bIsCheckBox);
	return GetCheck();
}

#ifdef	BTNST_USE_BCMENU
LRESULT CButtonST::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	LRESULT lResult;
	if (BCMenu::IsMenu(pMenu))
		lResult = BCMenu::FindKeyboardShortcut(nChar, nFlags, pMenu);
	else
		lResult = CButton::OnMenuChar(nChar, nFlags, pMenu);
	return lResult;
}
#endif

#ifdef	BTNST_USE_BCMENU
void CButtonST::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	BOOL bSetFlag = FALSE;
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
	{
		if (IsMenu((HMENU)lpMeasureItemStruct->itemID) && BCMenu::IsMenu((HMENU)lpMeasureItemStruct->itemID))
		{
			m_menuPopup.MeasureItem(lpMeasureItemStruct);
			bSetFlag = TRUE;
		} // if
	} // if
	if (!bSetFlag) CButton::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
#endif

void CButtonST::OnEnable(BOOL bEnable) 
{
	CButton::OnEnable(bEnable);
	
	if (bEnable == FALSE)	
	{
		CWnd*	pWnd = GetParent()->GetNextDlgTabItem(this);
		if (pWnd)
			pWnd->SetFocus();
		else
			GetParent()->SetFocus();

		CancelHover();
	} // if

	Invalidate();

}

void CButtonST::OnKillFocus(CWnd * pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
	CancelHover();
}

void CButtonST::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CButton::OnActivate(nState, pWndOther, bMinimized);
	if (nState == WA_INACTIVE)	CancelHover();
}

void CButtonST::OnCancelMode() 
{
	CButton::OnCancelMode();
	CancelHover();
}

BOOL CButtonST::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If a cursor was specified then use it!
	if (m_hCursor != NULL)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	} // if

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}

void CButtonST::CancelHover()
{
	// Only for flat buttons
	if (m_bIsFlat)
	{
		if (m_bMouseOnButton)
		{
			m_bMouseOnButton = FALSE;
			Invalidate();
		} // if
	} // if
}

void CButtonST::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd*				wndUnderMouse = NULL;
	CWnd*				wndActive = this;
	TRACKMOUSEEVENT		csTME;

	CButton::OnMouseMove(nFlags, point);

	ClientToScreen(&point);
	wndUnderMouse = WindowFromPoint(point);

	// If the mouse enter the button with the left button pressed then do nothing
	if (nFlags & MK_LBUTTON && m_bMouseOnButton == FALSE) return;

	// If our button is not flat then do nothing
	if (m_bIsFlat == FALSE) return;

	if (m_bAlwaysTrack == FALSE) wndActive = GetActiveWindow();

	if (wndUnderMouse && wndUnderMouse->m_hWnd == m_hWnd && wndActive)
	{
		if (!m_bMouseOnButton)
		{
			m_bMouseOnButton = TRUE;

			Invalidate();

#ifdef	BTNST_USE_SOUND
			// Play sound ?
			if (m_csSounds[0].lpszSound)
				::PlaySound(m_csSounds[0].lpszSound, m_csSounds[0].hMod, m_csSounds[0].dwFlags);
#endif

			csTME.cbSize = sizeof(csTME);
			csTME.dwFlags = TME_LEAVE;
			csTME.hwndTrack = m_hWnd;
			::_TrackMouseEvent(&csTME);
		} // if
	} else CancelHover();
}

// Handler for WM_MOUSELEAVE
LRESULT CButtonST::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	CancelHover();
	ReActivateTooltip();
	return 0;
}

void CButtonST::ReActivateTooltip()
{
	if(!m_ToolTip.m_hWnd)
		return;

	m_ToolTip.DelTool(this, 1);
	if(!m_strTooltipText.IsEmpty())
	{
		CRect rectBtn; 
		GetClientRect(rectBtn);
		m_ToolTip.AddTool(this, m_strTooltipText, rectBtn, 1);
		m_ToolTip.Activate(TRUE);
	}
}

BOOL CButtonST::OnClicked() 
{	
	SetFocus();

#ifdef	BTNST_USE_SOUND
	// Play sound ?
	if (m_csSounds[1].lpszSound)
		::PlaySound(m_csSounds[1].lpszSound, m_csSounds[1].hMod, m_csSounds[1].dwFlags);
#endif

	if (m_bIsCheckBox)
	{
		m_nCheck = !m_nCheck;
		Invalidate();
	} // if
	else
	{
		// Handle the menu (if any)
#ifdef	BTNST_USE_BCMENU
		if (m_menuPopup.m_hMenu)
#else
		if (m_hMenu)
#endif
		{
			CRect	rWnd;
			GetWindowRect(rWnd);

			m_bMenuDisplayed = TRUE;
			Invalidate();

#ifdef	BTNST_USE_BCMENU
			BCMenu* psub = (BCMenu*)m_menuPopup.GetSubMenu(0);
			if (m_csCallbacks.hWnd)	::SendMessage(m_csCallbacks.hWnd, m_csCallbacks.nMessage, (WPARAM)psub, m_csCallbacks.lParam);
			DWORD dwRetValue = psub->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rWnd.left, rWnd.bottom, this, NULL);
#else
			HMENU hSubMenu = ::GetSubMenu(m_hMenu, 0);
			if (m_csCallbacks.hWnd)	::SendMessage(m_csCallbacks.hWnd, m_csCallbacks.nMessage, (WPARAM)hSubMenu, m_csCallbacks.lParam);
			DWORD dwRetValue = ::TrackPopupMenuEx(hSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rWnd.left, rWnd.bottom, m_hParentWndMenu, NULL);
#endif

			m_bMenuDisplayed = FALSE;
			Invalidate();

			if (dwRetValue)
				::PostMessage(m_hParentWndMenu, WM_COMMAND, MAKEWPARAM(dwRetValue, 0), (LPARAM)NULL);
		} // if
		else
		{
			// Handle the URL (if any)
			if (_tcslen(m_szURL) > 0)
			{
				SHELLEXECUTEINFO	csSEI;

				memset(&csSEI, 0, sizeof(csSEI));
				csSEI.cbSize = sizeof(SHELLEXECUTEINFO);
				csSEI.fMask = SEE_MASK_FLAG_NO_UI;
				csSEI.lpVerb = _T("open");
				csSEI.lpFile = m_szURL;
				csSEI.nShow = SW_SHOWMAXIMIZED;
				::ShellExecuteEx(&csSEI);
			} // if
		} // else
	} // else

	return FALSE;
}

void CButtonST::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	// get dc
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// use a memdc for flicker free drawing
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	int iSaveDc = memDC.SaveDC();
	CRect rect = lpDIS->rcItem;
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	memDC.SelectObject(&bmp);
	CDC* pMemDC = &memDC;

	// Checkbox?
	if (m_bIsCheckBox)
	{
		m_bIsPressed  =  (lpDIS->itemState & ODS_SELECTED) || (m_nCheck != 0);
	}
	else	// Normal button OR other button style ...
	{
		m_bIsPressed = (lpDIS->itemState & ODS_SELECTED);

		// If there is a menu and it's displayed, draw the button as pressed
		if (
#ifdef	BTNST_USE_BCMENU
			m_menuPopup.m_hMenu 
#else
			m_hMenu 
#endif
			&& m_bMenuDisplayed)	m_bIsPressed = TRUE;
	}

	m_bIsFocused  = (lpDIS->itemState & ODS_FOCUS);
	m_bIsDisabled = (lpDIS->itemState & ODS_DISABLED);

	CRect itemRect = lpDIS->rcItem;

	pMemDC->SetBkMode(TRANSPARENT);

	// Prepare draw... paint button background

	// Draw transparent?
	if (m_bDrawTransparent)
		PaintBk(pMemDC);
	else
		OnDrawBackground(pMemDC, &itemRect);

	// Draw button border
	OnDrawBorder(pMemDC, &itemRect);

	// Read the button's title
	CString sTitle;
	GetWindowText(sTitle);

	CRect captionRect = lpDIS->rcItem;

	// Draw the icon
	if (m_csIcons[0].hIcon)
	{
		DrawTheIcon(pMemDC, !sTitle.IsEmpty(), &lpDIS->rcItem, &captionRect, m_bIsPressed, m_bIsDisabled);
	}

	if (m_csBitmaps[0].hBitmap)
	{
		pMemDC->SetBkColor(RGB(255,255,255));
		DrawTheBitmap(pMemDC, !sTitle.IsEmpty(), &lpDIS->rcItem, &captionRect, m_bIsPressed, m_bIsDisabled);
	}

	// Write the button title (if any)
	if (sTitle.IsEmpty() == FALSE)
	{
		DrawTheText(pMemDC, (LPCTSTR)sTitle, &lpDIS->rcItem, &captionRect, m_bIsPressed, m_bIsDisabled);
	}

	if (!m_bIsFlat || (m_bIsFlat && m_bDrawFlatFocus))
	{
		// Draw the focus rect
		if (m_bIsFocused && !(GetParent()->SendMessage(WM_QUERYUISTATE) & UISF_HIDEFOCUS))
		{
			CRect focusRect = itemRect;
			focusRect.DeflateRect(3, 3);
			pMemDC->DrawFocusRect(&focusRect);
		}
	}

	// blit memdc, which contains the complete item, into visible dc
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), pMemDC, 0, 0, SRCCOPY);

	// free resources
	memDC.RestoreDC(iSaveDc);
	memDC.DeleteDC();

}

void CButtonST::PaintBk(CDC* pDC)
{
	CClientDC clDC(GetParent());
	CRect rect;
	CRect rect1;

	GetClientRect(rect);

	GetWindowRect(rect1);
	GetParent()->ScreenToClient(rect1);

    //=============================================================================================
    // note: this is a quick hack for solving the DrawTransparent bug,
    //       when e.g. a topmost window is initially before the button.
    //       watch closely if this causes problems or decreases performance
    //       significantly!

	// Restore old bitmap (if any)
	if (m_dcBk.m_hDC != NULL && m_pbmpOldBk != NULL)
	{
		m_dcBk.SelectObject(m_pbmpOldBk);
	} // if

    if (m_bmpBk.m_hObject != NULL)
	    m_bmpBk.DeleteObject();

    if (m_dcBk.m_hDC != NULL)
        m_dcBk.DeleteDC();

    //if (m_dcBk.m_hDC == NULL)
    //=============================================================================================
    {
		m_dcBk.CreateCompatibleDC(&clDC);
		m_bmpBk.CreateCompatibleBitmap(&clDC, rect.Width(), rect.Height());
		m_pbmpOldBk = m_dcBk.SelectObject(&m_bmpBk);
		m_dcBk.BitBlt(0, 0, rect.Width(), rect.Height(), &clDC, rect1.left, rect1.top, SRCCOPY);
	} // if

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcBk, 0, 0, SRCCOPY);
}

HBITMAP CButtonST::CreateBitmapMask(HBITMAP hSourceBitmap, DWORD dwWidth, DWORD dwHeight, COLORREF crTransColor)
{
	HBITMAP		hMask		= NULL;
	HDC			hdcSrc		= NULL;
	HDC			hdcDest		= NULL;
	HBITMAP		hbmSrcT		= NULL;
	HBITMAP		hbmDestT	= NULL;
	COLORREF	crSaveBk;
	COLORREF	crSaveDestText;

	hMask = ::CreateBitmap(dwWidth, dwHeight, 1, 1, NULL);
	if (hMask == NULL)	return NULL;

	hdcSrc	= ::CreateCompatibleDC(NULL);
	hdcDest	= ::CreateCompatibleDC(NULL);

	hbmSrcT = (HBITMAP)::SelectObject(hdcSrc, hSourceBitmap);
	hbmDestT = (HBITMAP)::SelectObject(hdcDest, hMask);

	crSaveBk = ::SetBkColor(hdcSrc, crTransColor);

	::BitBlt(hdcDest, 0, 0, dwWidth, dwHeight, hdcSrc, 0, 0, SRCCOPY);

	crSaveDestText = ::SetTextColor(hdcSrc, RGB(255, 255, 255));
	::SetBkColor(hdcSrc,RGB(0, 0, 0));

	::BitBlt(hdcSrc, 0, 0, dwWidth, dwHeight, hdcDest, 0, 0, SRCAND);

	SetTextColor(hdcDest, crSaveDestText);

	::SetBkColor(hdcSrc, crSaveBk);
	::SelectObject(hdcSrc, hbmSrcT);
	::SelectObject(hdcDest, hbmDestT);

	::DeleteDC(hdcSrc);
	::DeleteDC(hdcDest);

	return hMask;
}

//
// Parameters:
//		[IN]	bHasTitle
//				TRUE if the button has a text
//		[IN]	rpItem
//				A pointer to a RECT structure indicating the allowed paint area
//		[IN/OUT]rpTitle
//				A pointer to a CRect object indicating the paint area reserved for the
//				text. This structure will be modified if necessary.
//		[IN]	bIsPressed
//				TRUE if the button is currently pressed
//		[IN]	dwWidth
//				Width of the image (icon or bitmap)
//		[IN]	dwHeight
//				Height of the image (icon or bitmap)
//		[OUT]	rpImage
//				A pointer to a CRect object that will receive the area available to the image
//
void CButtonST::PrepareImageRect(BOOL bHasTitle, RECT* rpItem, CRect* rpTitle, BOOL bIsPressed, DWORD dwWidth, DWORD dwHeight, CRect* rpImage)
{
	CRect rBtn;

	rpImage->CopyRect(rpItem);

	switch (m_eAlignmentStyle)
	{
        case BTNST_ALIGN_HORZ:
			if (bHasTitle == FALSE)
			{
				// Center image horizontally
				rpImage->left += ((rpImage->Width() - (long)dwWidth)/2);
			}
			else
			{
				// Image must be placed just inside the focus rect
				rpImage->left += m_ptImageOrg.x + m_iIconOffsetX;  
				rpTitle->left += dwWidth + m_ptImageOrg.x - m_iTextOffsetX;
				rpTitle->top  += m_iTextOffsetY;
			}
			// Center image vertically
			rpImage->top += ((rpImage->Height() - (long)dwHeight)/2) + m_iIconOffsetY;
			break;

        case BTNST_ALIGN_HORZ_RIGHT:
			GetClientRect(&rBtn);
			if (bHasTitle == FALSE)
			{
				// Center image horizontally
				rpImage->left += ((rpImage->Width() - (long)dwWidth)/2);
			}
			else
			{
				// Image must be placed just inside the focus rect
				rpTitle->right = rpTitle->Width() - dwWidth - m_ptImageOrg.x;
				rpTitle->left = m_ptImageOrg.x + m_iTextOffsetX;
				rpTitle->top += m_iTextOffsetY;
				rpImage->left = rBtn.right - dwWidth - m_ptImageOrg.x - m_iIconOffsetX;
				// Center image vertically
				rpImage->top += ((rpImage->Height() - (long)dwHeight)/2) + m_iIconOffsetY;
			}
			break;
		
        case BTNST_ALIGN_VERT:
			// Center image horizontally
			rpImage->left += ((rpImage->Width() - (long)dwWidth)/2);
			if (bHasTitle == FALSE)
			{
				// Center image vertically
				rpImage->top += ((rpImage->Height() - (long)dwHeight)/2);           
			}
			else
			{
				rpImage->top = m_ptImageOrg.y + m_iIconOffsetY + m_iIconOffsetY;
				rpTitle->top += dwHeight + m_iTextOffsetY;
			}
			break;

        case BTNST_ALIGN_OVERLAP:
			break;
	}
    
	// If button is pressed then press image also
	if (bIsPressed && m_bIsCheckBox == FALSE)
		rpImage->OffsetRect(m_ptPressedOffset.x, m_ptPressedOffset.y);
}

void CButtonST::DrawTheIcon(CDC* pDC, BOOL bHasTitle, RECT* rpItem, CRect* rpCaption, BOOL bIsPressed, BOOL bIsDisabled)
{
	BYTE byIndex = 0;

	// Select the icon to use
	if ((m_bIsCheckBox && bIsPressed) || (!m_bIsCheckBox && m_bMouseOnButton && !bIsPressed)
		|| !m_bIsCheckBox && !m_csIcons[2].hIcon && bIsPressed)
	{
		// IconIn
		// Checkbox: Checked, Button: Hover, Pressed (if there is no separate pressed icon)
		byIndex = BYTE(0);
	}
	else if(!m_bIsCheckBox && m_bMouseOnButton && bIsPressed)
	{
		// IconPressed
		// Button: Pressed
		if(m_csIcons[2].hIcon)
			byIndex = BYTE(2);
		else if(m_csIcons[1].hIcon)
			byIndex = BYTE(1);
		else
			byIndex = BYTE(0);
	}
	else if(bIsDisabled)
	{
		// Disabled
		if(m_csIcons[3].hIcon)
            byIndex = BYTE(3);
		else
			byIndex = BYTE(0);
	}
	else
	{
		// IconIn (if there is a valid handle)
		// Checkbox: Not checked, Button: Normal
		byIndex = (m_csIcons[1].hIcon == NULL ? BYTE(0) : BYTE(1));
	}

	CRect	rImage;
	PrepareImageRect(bHasTitle, rpItem, rpCaption, bIsPressed, m_csIcons[byIndex].dwWidth, m_csIcons[byIndex].dwHeight, &rImage);

	// Ole'!
	pDC->DrawState(	rImage.TopLeft(),
					rImage.Size(), 
					m_csIcons[byIndex].hIcon,
					((bIsDisabled && !m_csIcons[3].hIcon) ? DSS_DISABLED : DSS_NORMAL), 
					(CBrush*)NULL);
}

void CButtonST::DrawTheBitmap(CDC* pDC, BOOL bHasTitle, RECT* rpItem, CRect* rpCaption, BOOL bIsPressed, BOOL bIsDisabled)
{
	HDC			hdcBmpMem	= NULL;
	HBITMAP		hbmOldBmp	= NULL;
	HDC			hdcMem		= NULL;
	HBITMAP		hbmT		= NULL;

	BYTE		byIndex		= 0;

	// Select the bitmap to use
	if ((m_bIsCheckBox && bIsPressed) || (!m_bIsCheckBox && (bIsPressed || m_bMouseOnButton)))
		byIndex = BYTE(0);
	else
		byIndex = (m_csBitmaps[1].hBitmap == NULL ? BYTE(0) : BYTE(1));

	CRect	rImage;
	PrepareImageRect(bHasTitle, rpItem, rpCaption, bIsPressed, m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight, &rImage);

	hdcBmpMem = ::CreateCompatibleDC(pDC->m_hDC);

	hbmOldBmp = (HBITMAP)::SelectObject(hdcBmpMem, m_csBitmaps[byIndex].hBitmap);

	hdcMem = ::CreateCompatibleDC(NULL);

	hbmT = (HBITMAP)::SelectObject(hdcMem, m_csBitmaps[byIndex].hMask);

	if (bIsDisabled && m_bShowDisabledBitmap)
	{
		HDC		hDC = NULL;
		HBITMAP	hBitmap = NULL;

		hDC = ::CreateCompatibleDC(pDC->m_hDC);
		hBitmap = ::CreateCompatibleBitmap(pDC->m_hDC, m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight);
		HBITMAP	hOldBmp2 = (HBITMAP)::SelectObject(hDC, hBitmap);

		RECT	rRect;
		rRect.left = 0;
		rRect.top = 0;
		rRect.right = rImage.right + 1;
		rRect.bottom = rImage.bottom + 1;
		::FillRect(hDC, &rRect, (HBRUSH)RGB(255, 255, 255));

		COLORREF crOldColor = ::SetBkColor(hDC, RGB(255,255,255));

		::BitBlt(hDC, 0, 0, m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight, hdcMem, 0, 0, SRCAND);
		::BitBlt(hDC, 0, 0, m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight, hdcBmpMem, 0, 0, SRCPAINT);

		::SetBkColor(hDC, crOldColor);
		::SelectObject(hDC, hOldBmp2);
		::DeleteDC(hDC);

		pDC->DrawState(	CPoint(rImage.left/*+1*/, rImage.top), 
						CSize(m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight), 
						hBitmap, DST_BITMAP | DSS_DISABLED);

		::DeleteObject(hBitmap);
	}
	else
	{
		::BitBlt(pDC->m_hDC, rImage.left, rImage.top, m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight, hdcMem, 0, 0, SRCAND);

		::BitBlt(pDC->m_hDC, rImage.left, rImage.top, m_csBitmaps[byIndex].dwWidth, m_csBitmaps[byIndex].dwHeight, hdcBmpMem, 0, 0, SRCPAINT);
	}

	::SelectObject(hdcMem, hbmT);
	::DeleteDC(hdcMem);

	::SelectObject(hdcBmpMem, hbmOldBmp);
	::DeleteDC(hdcBmpMem);
}

void CButtonST::DrawTheText(CDC* pDC, LPCTSTR lpszText, RECT* rpItem, CRect* rpCaption, BOOL bIsPressed, BOOL bIsDisabled)
{
	// select font into DC
	CFont* pOldFont = NULL;
	if(m_font.m_hObject)
	{
		pOldFont = pDC->SelectObject(&m_font);
	}
	else
	{
		pDC->SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	}

	// Draw the button's title
	// If button is pressed then "press" title also
	if (m_bIsPressed && m_bIsCheckBox == FALSE)
		rpCaption->OffsetRect(m_ptPressedOffset.x, m_ptPressedOffset.y);

	// ONLY FOR DEBUG 
	//CBrush brBtnShadow(RGB(255, 0, 0));
	//pDC->FrameRect(rCaption, &brBtnShadow);

    // Ask parent whether to show the button accelerator (e.g. '&Cancel')
    UINT nHidePrefixFlag = (GetParent()->SendMessage(WM_QUERYUISTATE) & UISF_HIDEACCEL) ? DT_HIDEPREFIX : 0;

	// Center text
	CRect centerRect = rpCaption;
	pDC->DrawText(lpszText, -1, rpCaption, DT_WORDBREAK | DT_CENTER | DT_CALCRECT | nHidePrefixFlag); // only rect is calculated, no text is drawn
	rpCaption->OffsetRect((centerRect.Width() - rpCaption->Width())/2, (centerRect.Height() - rpCaption->Height())/2);
	/* RFU
	rpCaption->OffsetRect(0, (centerRect.Height() - rpCaption->Height())/2);
	rpCaption->OffsetRect((centerRect.Width() - rpCaption->Width())-4, (centerRect.Height() - rpCaption->Height())/2);
	*/

	pDC->SetBkMode(TRANSPARENT);
	/*
	pDC->DrawState(rCaption->TopLeft(), rCaption->Size(), (LPCTSTR)sTitle, (bIsDisabled ? DSS_DISABLED : DSS_NORMAL), 
					TRUE, 0, (CBrush*)NULL);
	*/
	if (m_bIsDisabled)
	{
		rpCaption->OffsetRect(1, 1);
		pDC->SetTextColor(::GetSysColor(COLOR_3DHILIGHT));
		pDC->DrawText(lpszText, -1, rpCaption, DT_WORDBREAK | DT_CENTER | nHidePrefixFlag);
		rpCaption->OffsetRect(-1, -1);
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
		pDC->DrawText(lpszText, -1, rpCaption, DT_WORDBREAK | DT_CENTER | nHidePrefixFlag);
	}
	else
	{
		if (m_bMouseOnButton || m_bIsPressed)
		{
			pDC->SetTextColor(m_crColors[BTNST_COLOR_FG_IN]);
			pDC->SetBkColor(m_crColors[BTNST_COLOR_BK_IN]);
		}
		else 
		{
			if (m_bIsFocused)
			{
				pDC->SetTextColor(m_crColors[BTNST_COLOR_FG_FOCUS]);
				pDC->SetBkColor(m_crColors[BTNST_COLOR_BK_FOCUS]);
			}
			else
			{
				pDC->SetTextColor(m_crColors[BTNST_COLOR_FG_OUT]);
				pDC->SetBkColor(m_crColors[BTNST_COLOR_BK_OUT]);
			}
		}
		pDC->DrawText(lpszText, -1, rpCaption, DT_WORDBREAK | DT_CENTER | nHidePrefixFlag);
	}

	// select old font into DC
	if(pOldFont)
	{
		pDC->SelectObject(pOldFont);
	}

}

// This function creates a grayscale bitmap starting from a given bitmap.
// The resulting bitmap will have the same size of the original one.
//
// Parameters:
//		[IN]	hBitmap
//				Handle to the original bitmap.
//		[IN]	dwWidth
//				Specifies the bitmap width, in pixels.
//		[IN]	dwHeight
//				Specifies the bitmap height, in pixels.
//		[IN]	crTrans
//				Color to be used as transparent color. This color will be left unchanged.
//
// Return value:
//		If the function succeeds, the return value is the handle to the newly created
//		grayscale bitmap.
//		If the function fails, the return value is NULL.
//
HBITMAP CButtonST::CreateGrayscaleBitmap(HBITMAP hBitmap, DWORD dwWidth, DWORD dwHeight, COLORREF crTrans)
{
	HBITMAP		hGrayBitmap = NULL;
	HDC			hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
	HBITMAP		hOldBmp1 = NULL, hOldBmp2 = NULL;

	hMainDC = ::GetDC(NULL);
	if (hMainDC == NULL)	return NULL;
	hMemDC1 = ::CreateCompatibleDC(hMainDC);
	if (hMemDC1 == NULL)
	{
		::ReleaseDC(NULL, hMainDC);
		return NULL;
	} // if
	hMemDC2 = ::CreateCompatibleDC(hMainDC);
	if (hMemDC2 == NULL)
	{
		::DeleteDC(hMemDC1);
		::ReleaseDC(NULL, hMainDC);
		return NULL;
	} // if

	hGrayBitmap = ::CreateCompatibleBitmap(hMainDC, dwWidth, dwHeight);
	if (hGrayBitmap)
	{
		hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, hGrayBitmap);
		hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, hBitmap);

		//::BitBlt(hMemDC1, 0, 0, dwWidth, dwHeight, hMemDC2, 0, 0, SRCCOPY);

		DWORD		dwLoopY = 0, dwLoopX = 0;
		COLORREF	crPixel = 0;
		BYTE		byNewPixel = 0;

		for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
		{
			for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
			{
				crPixel = ::GetPixel(hMemDC2, dwLoopX, dwLoopY);
				byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) + (GetGValue(crPixel) * 0.587) + (GetBValue(crPixel) * 0.114));

				if (crPixel != crTrans)
					::SetPixel(hMemDC1, dwLoopX, dwLoopY, RGB(byNewPixel, byNewPixel, byNewPixel));
				else
					::SetPixel(hMemDC1, dwLoopX, dwLoopY, crPixel);
			} // for
		} // for

		::SelectObject(hMemDC1, hOldBmp1);
		::SelectObject(hMemDC2, hOldBmp2);
	} // if

	::DeleteDC(hMemDC1);
	::DeleteDC(hMemDC2);
	::ReleaseDC(NULL, hMainDC);

	return hGrayBitmap;
}

// This function creates a bitmap that is 25% darker than the original.
// The resulting bitmap will have the same size of the original one.
//
// Parameters:
//		[IN]	hBitmap
//				Handle to the original bitmap.
//		[IN]	dwWidth
//				Specifies the bitmap width, in pixels.
//		[IN]	dwHeight
//				Specifies the bitmap height, in pixels.
//		[IN]	crTrans
//				Color to be used as transparent color. This color will be left unchanged.
//
// Return value:
//		If the function succeeds, the return value is the handle to the newly created
//		darker bitmap.
//		If the function fails, the return value is NULL.
//
HBITMAP CButtonST::CreateDarkerBitmap(HBITMAP hBitmap, DWORD dwWidth, DWORD dwHeight, COLORREF crTrans)
{
	HBITMAP		hGrayBitmap = NULL;
	HDC			hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
	HBITMAP		hOldBmp1 = NULL, hOldBmp2 = NULL;

	hMainDC = ::GetDC(NULL);
	if (hMainDC == NULL)	return NULL;
	hMemDC1 = ::CreateCompatibleDC(hMainDC);
	if (hMemDC1 == NULL)
	{
		::ReleaseDC(NULL, hMainDC);
		return NULL;
	} // if
	hMemDC2 = ::CreateCompatibleDC(hMainDC);
	if (hMemDC2 == NULL)
	{
		::DeleteDC(hMemDC1);
		::ReleaseDC(NULL, hMainDC);
		return NULL;
	} // if

	hGrayBitmap = ::CreateCompatibleBitmap(hMainDC, dwWidth, dwHeight);
	if (hGrayBitmap)
	{
		hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, hGrayBitmap);
		hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, hBitmap);

		//::BitBlt(hMemDC1, 0, 0, dwWidth, dwHeight, hMemDC2, 0, 0, SRCCOPY);

		DWORD		dwLoopY = 0, dwLoopX = 0;
		COLORREF	crPixel = 0;

		for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
		{
			for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
			{
				crPixel = ::GetPixel(hMemDC2, dwLoopX, dwLoopY);

				if (crPixel != crTrans)
					::SetPixel(hMemDC1, dwLoopX, dwLoopY, DarkenColor(crPixel, 0.25));
				else
					::SetPixel(hMemDC1, dwLoopX, dwLoopY, crPixel);
			} // for
		} // for

		::SelectObject(hMemDC1, hOldBmp1);
		::SelectObject(hMemDC2, hOldBmp2);
	} // if

	::DeleteDC(hMemDC1);
	::DeleteDC(hMemDC2);
	::ReleaseDC(NULL, hMainDC);

	return hGrayBitmap;
}

// This function creates a gray scale icon starting from a given icon.
// The resulting icon will have the same size of the original one.
//
// Parameters:
//		[IN]	hIcon
//				Handle to the original icon.
//
// Return value:
//		If the function succeeds, the return value is the handle to the newly created
//		gray scale icon.
//		If the function fails, the return value is NULL.
//
// Updates:
//		26/Nov/2002	Restored 1 BitBlt operation
//		03/May/2002	Removed dependency from m_hWnd
//					Removed 1 BitBlt operation
//
HICON CButtonST::CreateGrayscaleIcon(HICON hIcon)
{
	HICON		hGrayIcon = NULL;
	HDC			hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
	BITMAP		bmp;
	HBITMAP		hOldBmp1 = NULL, hOldBmp2 = NULL;
	ICONINFO	csII, csGrayII;
	BOOL		bRetValue = FALSE;

	bRetValue = ::GetIconInfo(hIcon, &csII);
	if(bRetValue == FALSE)
		return NULL;

	hMainDC = ::GetDC(NULL);
	hMemDC1 = ::CreateCompatibleDC(hMainDC);
	hMemDC2 = ::CreateCompatibleDC(hMainDC);
	if (hMainDC == NULL || hMemDC1 == NULL || hMemDC2 == NULL)
		return NULL;

	if (::GetObject(csII.hbmColor, sizeof(BITMAP), &bmp))
	{
		DWORD	dwWidth = csII.xHotspot * 2;
		DWORD	dwHeight = csII.yHotspot * 2;

		csGrayII.hbmColor = ::CreateBitmap(dwWidth, dwHeight, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
		if(csGrayII.hbmColor)
		{
			hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, csII.hbmColor);
			hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, csGrayII.hbmColor);

			//::BitBlt(hMemDC2, 0, 0, dwWidth, dwHeight, hMemDC1, 0, 0, SRCCOPY);

			DWORD		dwLoopY = 0, dwLoopX = 0;
			COLORREF	crPixel = 0;
			BYTE		byNewPixel = 0;

			for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
			{
				for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
				{
					crPixel = ::GetPixel(hMemDC1, dwLoopX, dwLoopY);
					byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) + (GetGValue(crPixel) * 0.587) + (GetBValue(crPixel) * 0.114));

					if (crPixel)	
						::SetPixel(hMemDC2, dwLoopX, dwLoopY, RGB(byNewPixel, byNewPixel, byNewPixel));
					else
						::SetPixel(hMemDC2, dwLoopX, dwLoopY, crPixel);
				} // for
			} // for

			::SelectObject(hMemDC1, hOldBmp1);
			::SelectObject(hMemDC2, hOldBmp2);

			csGrayII.hbmMask = csII.hbmMask;

			csGrayII.fIcon = TRUE;
			hGrayIcon = ::CreateIconIndirect(&csGrayII);
		} // if

		::DeleteObject(csGrayII.hbmColor);
		//::DeleteObject(csGrayII.hbmMask);
	} // if

	::DeleteObject(csII.hbmColor);
	::DeleteObject(csII.hbmMask);
	::DeleteDC(hMemDC1);
	::DeleteDC(hMemDC2);
	::ReleaseDC(NULL, hMainDC);

	return hGrayIcon;
}

// This function creates a icon that is 25% darker than the original.
// The resulting icon will have the same size of the original one.
//
// Parameters:
//		[IN]	hIcon
//				Handle to the original icon.
//
// Return value:
//		If the function succeeds, the return value is the handle to the newly created
//		darker icon.
//		If the function fails, the return value is NULL.
//
HICON CButtonST::CreateDarkerIcon(HICON hIcon)
{
	HICON		hGrayIcon = NULL;
	HDC			hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
	BITMAP		bmp;
	HBITMAP		hOldBmp1 = NULL, hOldBmp2 = NULL;
	ICONINFO	csII, csGrayII;
	BOOL		bRetValue = FALSE;

	bRetValue = ::GetIconInfo(hIcon, &csII);
	if (bRetValue == FALSE)	return NULL;

	hMainDC = ::GetDC(NULL);
	hMemDC1 = ::CreateCompatibleDC(hMainDC);
	hMemDC2 = ::CreateCompatibleDC(hMainDC);
	if (hMainDC == NULL || hMemDC1 == NULL || hMemDC2 == NULL)	return NULL;
  
	if (::GetObject(csII.hbmColor, sizeof(BITMAP), &bmp))
	{
		DWORD	dwWidth = csII.xHotspot*2;
		DWORD	dwHeight = csII.yHotspot*2;

		csGrayII.hbmColor = ::CreateBitmap(dwWidth, dwHeight, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
		if (csGrayII.hbmColor)
		{
			hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, csII.hbmColor);
			hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, csGrayII.hbmColor);

			//::BitBlt(hMemDC2, 0, 0, dwWidth, dwHeight, hMemDC1, 0, 0, SRCCOPY);

			DWORD		dwLoopY = 0, dwLoopX = 0;
			COLORREF	crPixel = 0;

			for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
			{
				for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
				{
					crPixel = ::GetPixel(hMemDC1, dwLoopX, dwLoopY);

					if (crPixel)	
						::SetPixel(hMemDC2, dwLoopX, dwLoopY, DarkenColor(crPixel, 0.25));
					else
						::SetPixel(hMemDC2, dwLoopX, dwLoopY, crPixel);
				} // for
			} // for

			::SelectObject(hMemDC1, hOldBmp1);
			::SelectObject(hMemDC2, hOldBmp2);

			csGrayII.hbmMask = csII.hbmMask;

			csGrayII.fIcon = TRUE;
			hGrayIcon = ::CreateIconIndirect(&csGrayII);
		} // if

		::DeleteObject(csGrayII.hbmColor);
		//::DeleteObject(csGrayII.hbmMask);
	} // if

	::DeleteObject(csII.hbmColor);
	::DeleteObject(csII.hbmMask);
	::DeleteDC(hMemDC1);
	::DeleteDC(hMemDC2);
	::ReleaseDC(NULL, hMainDC);

	return hGrayIcon;
}

COLORREF CButtonST::DarkenColor(COLORREF crColor, double dFactor)
{
	if (dFactor > 0.0 && dFactor <= 1.0)
	{
		BYTE red,green,blue,lightred,lightgreen,lightblue;
		red = GetRValue(crColor);
		green = GetGValue(crColor);
		blue = GetBValue(crColor);
		lightred = (BYTE)(red-(dFactor * red));
		lightgreen = (BYTE)(green-(dFactor * green));
		lightblue = (BYTE)(blue-(dFactor * blue));
		crColor = RGB(lightred,lightgreen,lightblue);
	} // if

	return crColor;
}

// This function assigns icons to the button.
// Any previous icon or bitmap will be removed.
//
// Parameters:
//		[IN]	nIconIn
//				ID number of the icon resource to show when the mouse is over the button.
//				Pass NULL to remove any icon from the button.
//		[IN]	nCxDesiredIn
//				Specifies the width, in pixels, of the icon to load.
//		[IN]	nCyDesiredIn
//				Specifies the height, in pixels, of the icon to load.
//		[IN]	nIconOut
//				ID number of the icon resource to show when the mouse is outside the button.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY (cast to int) the second
//				icon will be automatically created starting from nIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER (cast to int) the second
//				icon will be automatically created 25% darker starting from nIconIn.
//		[IN]	nCxDesiredOut
//				Specifies the width, in pixels, of the icon to load.
//		[IN]	nCyDesiredOut
//				Specifies the height, in pixels, of the icon to load.
//		[IN]	nIconPressed
//				ID number of the icon resource to show when the user clicks the button.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY (cast to int) the third
//				icon will be automatically created starting from nIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER (cast to int) the third
//				icon will be automatically created 25% darker starting from nIconIn.
//		[IN]	nCxDesiredPressed
//				Specifies the width, in pixels, of the icon to load.
//		[IN]	nCyDesiredPressed
//				Specifies the height, in pixels, of the icon to load.
//		[IN]	nIconDisabled
//				ID number of the icon resource to show when the button is disabled.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY (cast to int) the fourth
//				icon will be automatically created starting from nIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER (cast to int) the fourth
//				icon will be automatically created 25% darker starting from nIconIn.
//		[IN]	nCxDesiredDisabled
//				Specifies the width, in pixels, of the icon to load.
//		[IN]	nCyDesiredDisabled
//				Specifies the height, in pixels, of the icon to load.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//
DWORD CButtonST::SetIcon(
	int nIconIn, int nCxDesiredIn, int nCyDesiredIn,
	int nIconOut /*=NULL*/, int nCxDesiredOut /*=0*/, int nCyDesiredOut /*=0*/,
	int nIconPressed /*=NULL*/, int nCxDesiredPressed /*=0*/, int nCyDesiredPressed /*=0*/,
	int nIconDisabled /*=NULL*/, int nCxDesiredDisabled /*=0*/, int nCyDesiredDisabled /*=0*/)
{
	HICON		hIconIn			= NULL;
	HICON		hIconOut		= NULL;
	HICON		hIconPressed	= NULL;
	HICON		hIconDisabled	= NULL;
	HINSTANCE	hInstResource	= NULL;

	// Find correct resource handle
	hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nIconIn), RT_GROUP_ICON);

	// Set icon when the mouse is IN the button
	hIconIn = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(nIconIn), IMAGE_ICON, nCxDesiredIn, nCyDesiredIn, 0);

  	// Set icon when the mouse is OUT the button
	switch (nIconOut)
	{
		case NULL:
			break;
		case (INT_PTR)BTNST_AUTO_GRAY:
			hIconOut = BTNST_AUTO_GRAY;
			break;
		case (INT_PTR)BTNST_AUTO_DARKER:
			hIconOut = BTNST_AUTO_DARKER;
			break;
		default:
			hIconOut = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(nIconOut), IMAGE_ICON, nCxDesiredOut, nCyDesiredOut, 0);
			break;
	} // switch

	// Set icon when the user CLICKS the button
	switch (nIconPressed)
	{
	case NULL:
		break;
	case (INT_PTR)BTNST_AUTO_GRAY:
		hIconPressed = BTNST_AUTO_GRAY;
		break;
	case (INT_PTR)BTNST_AUTO_DARKER:
		hIconPressed = BTNST_AUTO_DARKER;
		break;
	default:
		hIconPressed = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(nIconPressed), IMAGE_ICON, nCxDesiredPressed, nCyDesiredPressed, 0);
		break;
	} // switch

	// Set icon when the button is DISABLED
	switch (nIconDisabled)
	{
	case NULL:
		break;
	case (INT_PTR)BTNST_AUTO_GRAY:
		hIconDisabled = BTNST_AUTO_GRAY;
		break;
	case (INT_PTR)BTNST_AUTO_DARKER:
		hIconDisabled = BTNST_AUTO_DARKER;
		break;
	default:
		hIconDisabled = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(nIconDisabled), IMAGE_ICON, nCxDesiredDisabled, nCyDesiredDisabled, 0);
		break;
	} // switch

	return SetIcon(hIconIn, hIconOut, hIconPressed, hIconDisabled);
}

// This function assigns icons to the button.
// Any previous icon or bitmap will be removed.
//
// Parameters:
//		[IN]	nIconIn
//				ID number of the icon resource to show when the mouse is over the button.
//				Pass NULL to remove any icon from the button.
//		[IN]	nIconOut
//				ID number of the icon resource to show when the mouse is outside the button.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY (cast to int) the second
//				icon will be automatically created starting from nIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER (cast to int) the second
//				icon will be automatically created 25% darker starting from nIconIn.
//		[IN]	nIconPressed
//				ID number of the icon resource to show when the clicks the button.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY (cast to int) the third
//				icon will be automatically created starting from nIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER (cast to int) the third
//				icon will be automatically created 25% darker starting from nIconIn.
//		[IN]	nIconDisabled
//				ID number of the icon resource to show when the button is disabled.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY (cast to int) the fourth
//				icon will be automatically created starting from nIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER (cast to int) the fourth
//				icon will be automatically created 25% darker starting from nIconIn.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//
DWORD CButtonST::SetIcon(
	int nIconIn,
	int nIconOut /*=NULL*/,
	int nIconPressed /*=NULL*/,
	int nIconDisabled /*=NULL*/)
{
	return SetIcon(nIconIn, 0, 0, nIconOut, 0, 0, nIconPressed, 0, 0, nIconDisabled, 0, 0);
}

// This function assigns icons to the button.
// Any previous icon or bitmap will be removed.
//
// Parameters:
//		[IN]	hIconIn
//				Handle for the icon to show when the mouse is over the button.
//				Pass NULL to remove any icon from the button.
//		[IN]	hIconOut
//				Handle to the icon to show when the mouse is outside the button.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY the second
//				icon will be automatically created starting from hIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER the second
//				icon will be automatically created 25% darker starting from hIconIn.
//		[IN]	hIconPressed
//				Handle to the icon to show when the user clicks the button.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY the third
//				icon will be automatically created starting from hIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER the third
//				icon will be automatically created 25% darker starting from hIconIn.
//		[IN]	hIconDisabled
//				Handle to the icon to show when the button is disabled.
//				Can be NULL.
//				If this parameter is the special value BTNST_AUTO_GRAY the fourth
//				icon will be automatically created starting from hIconIn and converted to gray scale.
//				If this parameter is the special value BTNST_AUTO_DARKER the fourth
//				icon will be automatically created 25% darker starting from hIconIn.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//
DWORD CButtonST::SetIcon(HICON hIconIn, HICON hIconOut, HICON hIconPressed, HICON hIconDisabled)
{
	BOOL		bRetValue;
	ICONINFO	ii;

	// Free any loaded resource
	FreeResources();

	if (hIconIn)
	{
		// Icon when mouse over button?
		m_csIcons[0].hIcon = hIconIn;
		// Get icon dimension
		::ZeroMemory(&ii, sizeof(ICONINFO));
		bRetValue = ::GetIconInfo(hIconIn, &ii);
		if (bRetValue == FALSE)
		{
			FreeResources();
			return BTNST_INVALIDRESOURCE;
		}

		m_csIcons[0].dwWidth	= (DWORD)(ii.xHotspot * 2);
		m_csIcons[0].dwHeight	= (DWORD)(ii.yHotspot * 2);
		::DeleteObject(ii.hbmMask);
		::DeleteObject(ii.hbmColor);
	}

	if (hIconOut)
	{
		switch ((INT_PTR)hIconOut)
		{
			case (INT_PTR)BTNST_AUTO_GRAY:
				hIconOut = CreateGrayscaleIcon(hIconIn);
				break;
			case (INT_PTR)BTNST_AUTO_DARKER:
				hIconOut = CreateDarkerIcon(hIconIn);
				break;
		}

		m_csIcons[1].hIcon = hIconOut;
		// Get icon dimension
		::ZeroMemory(&ii, sizeof(ICONINFO));
		bRetValue = ::GetIconInfo(hIconOut, &ii);
		if (bRetValue == FALSE)
		{
			FreeResources();
			return BTNST_INVALIDRESOURCE;
		}

		m_csIcons[1].dwWidth	= (DWORD)(ii.xHotspot * 2);
		m_csIcons[1].dwHeight	= (DWORD)(ii.yHotspot * 2);
		::DeleteObject(ii.hbmMask);
		::DeleteObject(ii.hbmColor);
	}

	if (hIconPressed)
	{
		// no offset when pressed
		SetPressedStyle(BTNST_PRESSED_NO_OFFSET, FALSE);

		switch ((INT_PTR)hIconPressed)
		{
		    case (INT_PTR)BTNST_AUTO_GRAY:
			    hIconPressed = CreateGrayscaleIcon(hIconIn);
			    break;
		    case (INT_PTR)BTNST_AUTO_DARKER:
			    hIconPressed = CreateDarkerIcon(hIconIn);
			    break;
		}

		m_csIcons[2].hIcon = hIconPressed;
		// Get icon dimension
		::ZeroMemory(&ii, sizeof(ICONINFO));
		bRetValue = ::GetIconInfo(hIconPressed, &ii);
		if (bRetValue == FALSE)
		{
			FreeResources();
			return BTNST_INVALIDRESOURCE;
		}

		m_csIcons[2].dwWidth	= (DWORD)(ii.xHotspot * 2);
		m_csIcons[2].dwHeight	= (DWORD)(ii.yHotspot * 2);
		::DeleteObject(ii.hbmMask);
		::DeleteObject(ii.hbmColor);
	}

	if (hIconDisabled)
	{
		switch ((INT_PTR)hIconDisabled)
		{
		    case (INT_PTR)BTNST_AUTO_GRAY:
			    hIconDisabled = CreateGrayscaleIcon(hIconIn);
			    break;
		    case (INT_PTR)BTNST_AUTO_DARKER:
			    hIconDisabled = CreateDarkerIcon(hIconIn);
			    break;
		}

		m_csIcons[3].hIcon = hIconDisabled;
		// Get icon dimension
		::ZeroMemory(&ii, sizeof(ICONINFO));
		bRetValue = ::GetIconInfo(hIconDisabled, &ii);
		if (bRetValue == FALSE)
		{
			FreeResources();
			return BTNST_INVALIDRESOURCE;
		}

		m_csIcons[3].dwWidth	= (DWORD)(ii.xHotspot * 2);
		m_csIcons[3].dwHeight	= (DWORD)(ii.yHotspot * 2);
		::DeleteObject(ii.hbmMask);
		::DeleteObject(ii.hbmColor);
	}

	Invalidate();

	return BTNST_OK;
}

// This function assigns bitmaps to the button.
// Any previous icon or bitmap will be removed.
//
// Parameters:
//		[IN]	nBitmapIn
//				ID number of the bitmap resource to show when the mouse is over the button.
//				Pass NULL to remove any bitmap from the button.
//		[IN]	crTransColorIn
//				Color (inside nBitmapIn) to be used as transparent color.
//		[IN]	nBitmapOut
//				ID number of the bitmap resource to show when the mouse is outside the button.
//				Can be NULL.
//		[IN]	crTransColorOut
//				Color (inside nBitmapOut) to be used as transparent color.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//		BTNST_FAILEDMASK
//			Failed creating mask bitmap.
//
DWORD CButtonST::SetBitmaps(int nBitmapIn, COLORREF crTransColorIn, int nBitmapOut, COLORREF crTransColorOut)
{
	HBITMAP		hBitmapIn		= NULL;
	HBITMAP		hBitmapOut		= NULL;
	HINSTANCE	hInstResource	= NULL;
	
	// Find correct resource handle
	hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nBitmapIn), RT_BITMAP);

	// Load bitmap In
	hBitmapIn = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(nBitmapIn), IMAGE_BITMAP, 0, 0, 0);

	// Load bitmap Out
	switch (nBitmapOut)
	{
		case NULL:
			break;
		case (INT_PTR)BTNST_AUTO_GRAY:
			hBitmapOut = (HBITMAP)BTNST_AUTO_GRAY;
			break;
		case (INT_PTR)BTNST_AUTO_DARKER:
			hBitmapOut = (HBITMAP)BTNST_AUTO_DARKER;
			break;
		default:
			hBitmapOut = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(nBitmapOut), IMAGE_BITMAP, 0, 0, 0);
			break;
	} // if

	return SetBitmaps(hBitmapIn, crTransColorIn, hBitmapOut, crTransColorOut);
}

// This function assigns bitmaps to the button.
// Any previous icon or bitmap will be removed.
//
// Parameters:
//		[IN]	hBitmapIn
//				Handle fo the bitmap to show when the mouse is over the button.
//				Pass NULL to remove any bitmap from the button.
//		[IN]	crTransColorIn
//				Color (inside hBitmapIn) to be used as transparent color.
//		[IN]	hBitmapOut
//				Handle to the bitmap to show when the mouse is outside the button.
//				Can be NULL.
//		[IN]	crTransColorOut
//				Color (inside hBitmapOut) to be used as transparent color.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//		BTNST_FAILEDMASK
//			Failed creating mask bitmap.
//
DWORD CButtonST::SetBitmaps(HBITMAP hBitmapIn, COLORREF crTransColorIn, HBITMAP hBitmapOut, COLORREF crTransColorOut)
{
	int		nRetValue = 0;
	BITMAP	csBitmapSize;

	// Free any loaded resource
	FreeResources();

	if (hBitmapIn)
	{
		m_csBitmaps[0].hBitmap = hBitmapIn;
		m_csBitmaps[0].crTransparent = crTransColorIn;
		// Get bitmap size
		nRetValue = ::GetObject(hBitmapIn, sizeof(csBitmapSize), &csBitmapSize);
		if (nRetValue == 0)
		{
			FreeResources();
			return BTNST_INVALIDRESOURCE;
		} // if
		m_csBitmaps[0].dwWidth = (DWORD)csBitmapSize.bmWidth;
		m_csBitmaps[0].dwHeight = (DWORD)csBitmapSize.bmHeight;

		// Create grayscale/darker bitmap BEFORE mask (of hBitmapIn)
		switch ((INT_PTR)hBitmapOut)
		{
			case (INT_PTR)BTNST_AUTO_GRAY:
				hBitmapOut = CreateGrayscaleBitmap(hBitmapIn, m_csBitmaps[0].dwWidth, m_csBitmaps[0].dwHeight, crTransColorIn);
				m_csBitmaps[1].hBitmap = hBitmapOut;
				crTransColorOut = crTransColorIn;
				break;
			case (INT_PTR)BTNST_AUTO_DARKER:
				hBitmapOut = CreateDarkerBitmap(hBitmapIn, m_csBitmaps[0].dwWidth, m_csBitmaps[0].dwHeight, crTransColorIn);
				m_csBitmaps[1].hBitmap = hBitmapOut;
				crTransColorOut = crTransColorIn;
				break;
		} // switch

		// Create mask for bitmap In
		m_csBitmaps[0].hMask = CreateBitmapMask(hBitmapIn, m_csBitmaps[0].dwWidth, m_csBitmaps[0].dwHeight, crTransColorIn);
		if (m_csBitmaps[0].hMask == NULL)
		{
			FreeResources();
			return BTNST_FAILEDMASK;
		} // if

		if (hBitmapOut)
		{
			m_csBitmaps[1].hBitmap = hBitmapOut;
			m_csBitmaps[1].crTransparent = crTransColorOut;
			// Get bitmap size
			nRetValue = ::GetObject(hBitmapOut, sizeof(csBitmapSize), &csBitmapSize);
			if (nRetValue == 0)
			{
				FreeResources();
				return BTNST_INVALIDRESOURCE;
			} // if
			m_csBitmaps[1].dwWidth = (DWORD)csBitmapSize.bmWidth;
			m_csBitmaps[1].dwHeight = (DWORD)csBitmapSize.bmHeight;

			// Create mask for bitmap Out
			m_csBitmaps[1].hMask = CreateBitmapMask(hBitmapOut, m_csBitmaps[1].dwWidth, m_csBitmaps[1].dwHeight, crTransColorOut);
			if (m_csBitmaps[1].hMask == NULL)
			{
				FreeResources();
				return BTNST_FAILEDMASK;
			} // if
		} // if
	} // if

	Invalidate();

	return BTNST_OK;
}

// This functions sets the button to have a standard or flat style.
//
// Parameters:
//		[IN]	bFlat
//				If TRUE the button will have a flat style, else
//				will have a standard style.
//				By default, CButtonST buttons are flat.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::SetFlat(BOOL bFlat, BOOL bRepaint)
{
	m_bIsFlat = bFlat;
	if (bRepaint)	Invalidate();

	return BTNST_OK;
}

// This function sets the alignment style between image and text.
//
// Parameters:
//		[IN]	eAlignmentStyle
//				The alignment style can be one of the following values:
//				BTNST_ALIGN_HORZ            Image on the left, text on the right
//				BTNST_ALIGN_VERT            Image on the top, text on the bottom
//				BTNST_ALIGN_HORZ_RIGHT      Image on the right, text on the left
//				BTNST_ALIGN_OVERLAP         Image on the same space as text
//				By default, CButtonST buttons have BTNST_ALIGN_HORZ alignment.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDALIGN
//			Alignment type not supported.
//
DWORD CButtonST::SetAlignmentStyle(ButtonAlignmentStyle eAlignmentStyle, BOOL bRepaint)
{
	switch (eAlignmentStyle)
	{    
		case BTNST_ALIGN_HORZ:
		case BTNST_ALIGN_HORZ_RIGHT:
		case BTNST_ALIGN_VERT:
		case BTNST_ALIGN_OVERLAP:
			m_eAlignmentStyle = eAlignmentStyle;
			if (bRepaint)   Invalidate();
			return BTNST_OK;
	}

	return BTNST_INVALIDALIGN;
}

// This function sets the pressed style.
//
// Parameters:
//		[IN]	ePressedStyle
//				The pressed style can be one of the following values:
//				BTNST_PRESSED_LEFTRIGHT     Pressed from left to right (usual behavior)
//				BTNST_PRESSED_TOPBOTTOM     Pressed from top to bottom
//              BTNST_PRESSED_NO_OFFSET     Pressed with no offset
//				By default, CButtonST buttons have BTNST_PRESSED_LEFTRIGHT style.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDPRESSEDSTYLE
//			Pressed style not supported.
//
DWORD CButtonST::SetPressedStyle(ButtonPressedStyle ePressedStyle, BOOL bRepaint)
{
	switch (ePressedStyle)
	{
		case BTNST_PRESSED_LEFTRIGHT:
			m_ptPressedOffset.x = 1;
			m_ptPressedOffset.y = 1;
			break;
		case BTNST_PRESSED_TOPBOTTOM:
			m_ptPressedOffset.x = 0;
			m_ptPressedOffset.y = 2;
			break;
		case BTNST_PRESSED_NO_OFFSET:
			m_ptPressedOffset.x = 0;
			m_ptPressedOffset.y = 0;
			break;
		default:
			ASSERT(0);
			return BTNST_INVALIDPRESSEDSTYLE;
	}

	if (bRepaint)   Invalidate();

	return BTNST_OK;
}

// This function sets the state of the checkbox.
// If the button is not a checkbox, this function has no meaning.
//
// Parameters:
//		[IN]	nCheck
//				1 to check the checkbox.
//				0 to un-check the checkbox.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::SetCheck(int nCheck, BOOL bRepaint)
{
	if (m_bIsCheckBox)
	{
		if (nCheck == 0) m_nCheck = 0;
		else m_nCheck = 1;

		if (bRepaint) Invalidate();
	} // if

	return BTNST_OK;
}

// This function returns the current state of the checkbox.
// If the button is not a checkbox, this function has no meaning.
//
// Return value:
//		The current state of the checkbox.
//			1 if checked.
//			0 if not checked or the button is not a checkbox.
//
int CButtonST::GetCheck()
{
	return m_nCheck;
}

// This function sets all colors to a default value.
//
// Parameters:
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::SetDefaultColors(BOOL bRepaint)
{
	m_crColors[BTNST_COLOR_BK_IN]		= ::GetSysColor(COLOR_BTNFACE);
	m_crColors[BTNST_COLOR_FG_IN]		= ::GetSysColor(COLOR_BTNTEXT);
	m_crColors[BTNST_COLOR_BK_OUT]		= ::GetSysColor(COLOR_BTNFACE);
	m_crColors[BTNST_COLOR_FG_OUT]		= ::GetSysColor(COLOR_BTNTEXT);
	m_crColors[BTNST_COLOR_BK_FOCUS]	= ::GetSysColor(COLOR_BTNFACE);
	m_crColors[BTNST_COLOR_FG_FOCUS]	= ::GetSysColor(COLOR_BTNTEXT);

	if (bRepaint) Invalidate();

	return BTNST_OK;
}

// This function sets the color to use for a particular state.
//
// Parameters:
//		[IN]	btnColor
//				The button color to be set can be one of the following values:
//				BTNST_COLOR_BK_IN		Background color when mouse is over the button
//				BTNST_COLOR_FG_IN		Text color when mouse is over the button
//				BTNST_COLOR_BK_OUT		Background color when mouse is outside the button
//				BTNST_COLOR_FG_OUT		Text color when mouse is outside the button
//				BTNST_COLOR_BK_FOCUS	Background color when the button is focused
//				BTNST_COLOR_FG_FOCUS	Text color when the button is focused
//		[IN]	crColor
//				New color.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDINDEX
//			Invalid color index.
//
DWORD CButtonST::SetColor(ButtonColor btnColor, COLORREF crColor, BOOL bRepaint)
{
	if (btnColor >= BTNST_MAX_COLORS) return BTNST_INVALIDINDEX;

	// Set new color
	m_crColors[btnColor] = crColor;

	if (bRepaint) Invalidate();

	return BTNST_OK;
}


DWORD CButtonST::SetBackgroundColorForAllStates(COLORREF crColor, BOOL bRepaint /*=TRUE*/)
{
	SetColor(BTNST_COLOR_BK_IN, crColor, bRepaint);
	SetColor(BTNST_COLOR_BK_OUT, crColor, bRepaint);
	return SetColor(BTNST_COLOR_BK_FOCUS, crColor, bRepaint);
}

DWORD CButtonST::SetTextColorForAllStates(COLORREF crColor, BOOL bRepaint /*=TRUE*/)
{
	SetColor(BTNST_COLOR_FG_IN, crColor, bRepaint);
	SetColor(BTNST_COLOR_FG_OUT, crColor, bRepaint);
	return SetColor(BTNST_COLOR_FG_FOCUS, crColor, bRepaint);
}

// This functions returns the color used for a particular state.
//
// Parameters:
//		[IN]	btnColor
//				Button color to get.
//				See SetColor for the list of available colors.
//		[OUT]	crpColor
//				A pointer to a COLORREF that will receive the color.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDINDEX
//			Invalid color index.
//
DWORD CButtonST::GetColor(ButtonColor btnColor, COLORREF* crpColor)
{
	if (btnColor >= BTNST_MAX_COLORS) return BTNST_INVALIDINDEX;

	// Get color
	*crpColor = m_crColors[btnColor];

	return BTNST_OK;
}

// This function applies an offset to the RGB components of the specified color.
// This function can be seen as an easy way to make a color darker or lighter than
// its default value.
//
// Parameters:
//		[IN]	btnColor
//				Button color to be set.
//				See SetColor for the list of available colors.
//		[IN]	shOffsetColor
//				A short value indicating the offset to apply to the color.
//				This value must be between -255 and 255.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDINDEX
//			Invalid color index.
//		BTNST_BADPARAM
//			The specified offset is out of range.
//
DWORD CButtonST::OffsetColor(ButtonColor btnColor, short shOffset, BOOL bRepaint)
{
	BYTE	byRed = 0;
	BYTE	byGreen = 0;
	BYTE	byBlue = 0;
	short	shOffsetR = shOffset;
	short	shOffsetG = shOffset;
	short	shOffsetB = shOffset;

	if (btnColor >= BTNST_MAX_COLORS)       return BTNST_INVALIDINDEX;
	if (shOffset < -255 || shOffset > 255)  return BTNST_BADPARAM;

	// Get RGB components of specified color
	byRed = GetRValue(m_crColors[btnColor]);
	byGreen = GetGValue(m_crColors[btnColor]);
	byBlue = GetBValue(m_crColors[btnColor]);

	// Calculate max. allowed real offset
	if (shOffset > 0)
	{
		if (byRed + shOffset > 255)		shOffsetR = 255 - byRed;
		if (byGreen + shOffset > 255)	shOffsetG = 255 - byGreen;
		if (byBlue + shOffset > 255)	shOffsetB = 255 - byBlue;

		shOffset = min(min(shOffsetR, shOffsetG), shOffsetB);
	}
	else
	{
		if (byRed + shOffset < 0)		shOffsetR = -byRed;
		if (byGreen + shOffset < 0)		shOffsetG = -byGreen;
		if (byBlue + shOffset < 0)		shOffsetB = -byBlue;

		shOffset = max(max(shOffsetR, shOffsetG), shOffsetB);
	}

	// Set new color
	m_crColors[btnColor] = RGB(byRed + shOffset, byGreen + shOffset, byBlue + shOffset);

	if (bRepaint) Invalidate();

	return BTNST_OK;
}

// This function sets the highlight logic for the button.
// Applies only to flat buttons.
//
// Parameters:
//		[IN]	bAlwaysTrack
//				If TRUE the button will be highlighted even if the window that owns it, is
//				not the active window.
//				If FALSE the button will be highlighted only if the window that owns it,
//				is the active window.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::SetAlwaysTrack(BOOL bAlwaysTrack)
{
	m_bAlwaysTrack = bAlwaysTrack;
	return BTNST_OK;
}

// This function sets the cursor to be used when the mouse is over the button.
//
// Parameters:
//		[IN]	nCursorId
//				ID number of the cursor resource.
//				Pass NULL to remove a previously loaded cursor.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//
DWORD CButtonST::SetBtnCursor(int nCursorId, BOOL bRepaint)
{
	HINSTANCE	hInstResource = NULL;
	// Destroy any previous cursor
	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}

	// Load cursor
	if (nCursorId)
	{
		hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nCursorId), RT_GROUP_CURSOR);
		// Load cursor resource
		m_hCursor = (HCURSOR)::LoadImage(hInstResource, MAKEINTRESOURCE(nCursorId), IMAGE_CURSOR, 0, 0, 0);
		// Repaint the button
		if (bRepaint) Invalidate();
		// If something wrong
		if (m_hCursor == NULL) return BTNST_INVALIDRESOURCE;
	}

	return BTNST_OK;
}

DWORD CButtonST::SetBtnCursor(HCURSOR hCursor, BOOL bRepaint)
{
	// Destroy any previous cursor
	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}

    // Set new cursor
    m_hCursor = hCursor;

    // Repaint the button
    if (bRepaint) Invalidate();
    // If something wrong
    if (m_hCursor == NULL) return BTNST_INVALIDRESOURCE;

	return BTNST_OK;
}

// This function sets if the button border must be drawn.
// Applies only to flat buttons.
//
// Parameters:
//		[IN]	bDrawBorder
//				If TRUE the border will be drawn.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::DrawBorder(BOOL bDrawBorder, BOOL bRepaint)
{
	m_bDrawBorder = bDrawBorder;
	// Repaint the button
	if (bRepaint) Invalidate();

	return BTNST_OK;
}

// This function sets if the focus rectangle must be drawn for flat buttons.
//
// Parameters:
//		[IN]	bDrawFlatFocus
//				If TRUE the focus rectangle will be drawn also for flat buttons.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::DrawFlatFocus(BOOL bDrawFlatFocus, BOOL bRepaint)
{
	m_bDrawFlatFocus = bDrawFlatFocus;
	// Repaint the button
	if (bRepaint) Invalidate();

	return BTNST_OK;
}

void CButtonST::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		// Create ToolTip control
		m_ToolTip.Create(this, m_dwToolTipStyle);
		// Create inactive
		m_ToolTip.Activate(FALSE);
		// Enable multi line
		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
		//m_ToolTip.SendMessage(TTM_SETTITLE, TTI_INFO, (LPARAM)_T("Title"));
	} // if
}

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	nText
//				ID number of the string resource containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CButtonST::SetTooltipText(int nText, BOOL bActivate)
{
	CString sText;

	// Load string resource
	sText.LoadString(nText);
	// If string resource is not empty
	if (sText.IsEmpty() == FALSE) SetTooltipText((LPCTSTR)sText, bActivate);
}

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	lpszText
//				Pointer to a null-terminated string containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CButtonST::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
{
	// We cannot accept NULL pointer
	if (lpszText == NULL) return;

	// Initialize ToolTip
	InitToolTip();
	m_strTooltipText = lpszText;

	// If there is no tooltip defined then add it
	if (m_ToolTip.GetToolCount() == 0)
	{
		CRect rectBtn; 
		GetClientRect(rectBtn);
		m_ToolTip.AddTool(this, lpszText, rectBtn, 1);
	} // if

	// Set text for tooltip
	m_ToolTip.UpdateTipText(lpszText, this, 1);
	m_ToolTip.Activate(bActivate);
}

void CButtonST::SetTooltipColor(COLORREF crText, COLORREF crBackground)
{
	ASSERT(m_ToolTip.m_hWnd);
	if(m_ToolTip.m_hWnd)
	{
		SetTooltipTextColor(crText);
		SetTooltipBackgroundColor(crBackground);
	}
}

void CButtonST::SetTooltipTextColor(COLORREF cr)
{
	ASSERT(m_ToolTip.m_hWnd);
	if(m_ToolTip.m_hWnd)
	{
		m_ToolTip.SetTipTextColor(cr);
	}
}

void CButtonST::SetTooltipBackgroundColor(COLORREF cr)
{
	ASSERT(m_ToolTip.m_hWnd);
	if(m_ToolTip.m_hWnd)
	{
		m_ToolTip.SetTipBkColor(cr);
	}
}

// This function enables or disables the button tooltip.
//
// Parameters:
//		[IN]	bActivate
//				If TRUE the tooltip will be activated.
//
void CButtonST::ActivateTooltip(BOOL bActivate)
{
	// If there is no tooltip then do nothing
	if (m_ToolTip.GetToolCount() == 0) return;

	// Activate tooltip
	m_ToolTip.Activate(bActivate);
}

// This function enables the tooltip to be displayed using the balloon style.
// This function must be called before any call to SetTooltipText is made.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::EnableBalloonTooltip()
{
	m_dwToolTipStyle |= TTS_BALLOON;
	return BTNST_OK;
}

// This function returns if the button is the default button.
//
// Return value:
//		TRUE
//			The button is the default button.
//		FALSE
//			The button is not the default button.
//
BOOL CButtonST::GetDefault()
{
	return m_bIsDefault;
}

// This function enables the transparent mode.
// Note: this operation is not reversible.
// DrawTransparent should be called just after the button is created.
// Do not use transparent buttons until you really need it (you have a bitmap
// background) since each transparent button makes a copy in memory of its background.
// This may bring unnecessary memory use and execution overload.
//
// Parameters:
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
void CButtonST::DrawTransparent(BOOL bRepaint)
{
	m_bDrawTransparent = TRUE;

	// Restore old bitmap (if any)
	if (m_dcBk.m_hDC != NULL && m_pbmpOldBk != NULL)
	{
		m_dcBk.SelectObject(m_pbmpOldBk);
	} // if

	m_bmpBk.DeleteObject();
	m_dcBk.DeleteDC();

	// Repaint the button
	if (bRepaint) Invalidate();
}

DWORD CButtonST::SetBk(CDC* pDC)
{
	if (m_bDrawTransparent && pDC)
	{
		// Restore old bitmap (if any)
		if (m_dcBk.m_hDC != NULL && m_pbmpOldBk != NULL)
		{
			m_dcBk.SelectObject(m_pbmpOldBk);
		} // if

		m_bmpBk.DeleteObject();
		m_dcBk.DeleteDC();

		CRect rect;
		CRect rect1;

		GetClientRect(rect);

		GetWindowRect(rect1);
		GetParent()->ScreenToClient(rect1);

		m_dcBk.CreateCompatibleDC(pDC);
		m_bmpBk.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		m_pbmpOldBk = m_dcBk.SelectObject(&m_bmpBk);
		m_dcBk.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, rect1.left, rect1.top, SRCCOPY);

		return BTNST_OK;
	} // if

	return BTNST_BADPARAM;
}

// This function sets the URL that will be opened when the button is clicked.
//
// Parameters:
//		[IN]	lpszURL
//				Pointer to a null-terminated string that contains the URL.
//				Pass NULL to removed any previously specified URL.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::SetURL(LPCTSTR lpszURL)
{
	// Remove any existing URL
	memset(m_szURL, 0, sizeof(m_szURL));

	if (lpszURL)
	{
		// Store the URL
#if _MSC_VER >= 1400  // VS2005
		_tcsncpy_s(m_szURL, _countof(m_szURL), lpszURL, _TRUNCATE);
#else
		_tcsncpy(m_szURL, lpszURL, _MAX_PATH);
#endif
	} // if

	return BTNST_OK;
}

// This function associates a menu to the button.
// The menu will be displayed clicking the button.
//
// Parameters:
//		[IN]	nMenu
//				ID number of the menu resource.
//				Pass NULL to remove any menu from the button.
//		[IN]	hParentWnd
//				Handle to the window that owns the menu.
//				This window receives all messages from the menu.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//
#ifndef	BTNST_USE_BCMENU
DWORD CButtonST::SetMenu(UINT nMenu, HWND hParentWnd, BOOL bRepaint)
{
	HINSTANCE	hInstResource	= NULL;

	// Destroy any previous menu
	if (m_hMenu)
	{
		::DestroyMenu(m_hMenu);
		m_hMenu = NULL;
		m_hParentWndMenu = NULL;
		m_bMenuDisplayed = FALSE;
	} // if

	// Load menu
	if (nMenu)
	{
		// Find correct resource handle
		hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nMenu), RT_MENU);
		// Load menu resource
		m_hMenu = ::LoadMenu(hInstResource, MAKEINTRESOURCE(nMenu));
		m_hParentWndMenu = hParentWnd;
		// If something wrong
		if (m_hMenu == NULL) return BTNST_INVALIDRESOURCE;
	} // if

	// Repaint the button
	if (bRepaint) Invalidate();

	return BTNST_OK;
}
#endif

// This function associates a menu to the button.
// The menu will be displayed clicking the button.
// The menu will be handled by the BCMenu class.
//
// Parameters:
//		[IN]	nMenu
//				ID number of the menu resource.
//				Pass NULL to remove any menu from the button.
//		[IN]	hParentWnd
//				Handle to the window that owns the menu.
//				This window receives all messages from the menu.
//		[IN]	bWinXPStyle
//				If TRUE the menu will be displayed using the new Windows XP style.
//				If FALSE the menu will be displayed using the standard style.
//		[IN]	nToolbarID
//				Resource ID of the toolbar to be associated to the menu.
//		[IN]	sizeToolbarIcon
//				A CSize object indicating the size (in pixels) of each icon into the toolbar.
//				All icons into the toolbar must have the same size.
//		[IN]	crToolbarBk
//				A COLORREF value indicating the color to use as background for the icons into the toolbar.
//				This color will be used as the "transparent" color.
//		[IN]	bRepaint
//				If TRUE the control will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//		BTNST_INVALIDRESOURCE
//			Failed loading the specified resource.
//
#ifdef	BTNST_USE_BCMENU
DWORD CButtonST::SetMenu(UINT nMenu, HWND hParentWnd, BOOL bWinXPStyle, UINT nToolbarID, CSize sizeToolbarIcon, COLORREF crToolbarBk, BOOL bRepaint)
{
	BOOL	bRetValue = FALSE;

	// Destroy any previous menu
	if (m_menuPopup.m_hMenu)
	{
		m_menuPopup.DestroyMenu();
		m_hParentWndMenu = NULL;
		m_bMenuDisplayed = FALSE;
	} // if

	// Load menu
	if (nMenu)
	{
		m_menuPopup.SetMenuDrawMode(bWinXPStyle);
		// Load menu
		bRetValue = m_menuPopup.LoadMenu(nMenu);
		// If something wrong
		if (bRetValue == FALSE) return BTNST_INVALIDRESOURCE;

		// Load toolbar
		if (nToolbarID)
		{
			m_menuPopup.SetBitmapBackground(crToolbarBk);
			m_menuPopup.SetIconSize(sizeToolbarIcon.cx, sizeToolbarIcon.cy);

			bRetValue = m_menuPopup.LoadToolbar(nToolbarID);
			// If something wrong
			if (bRetValue == FALSE) 
			{
				m_menuPopup.DestroyMenu();
				return BTNST_INVALIDRESOURCE;
			} // if
		} // if

		m_hParentWndMenu = hParentWnd;
	} // if

	// Repaint the button
	if (bRepaint) Invalidate();

	return BTNST_OK;
}
#endif

// This function sets the callback message that will be sent to the
// specified window just before the menu associated to the button is displayed.
//
// Parameters:
//		[IN]	hWnd
//				Handle of the window that will receive the callback message.
//				Pass NULL to remove any previously specified callback message.
//		[IN]	nMessage
//				Callback message to send to window.
//		[IN]	lParam
//				A 32 bits user specified value that will be passed to the callback function.
//
// Remarks:
//		the callback function must be in the form:
//		LRESULT On_MenuCallback(WPARAM wParam, LPARAM lParam)
//		Where:
//				[IN]	wParam
//						If support for BCMenu is enabled: a pointer to BCMenu
//						else a HMENU handle to the menu that is being to be displayed.
//				[IN]	lParam
//						The 32 bits user specified value.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::SetMenuCallback(HWND hWnd, UINT nMessage, LPARAM lParam)
{
	m_csCallbacks.hWnd = hWnd;
	m_csCallbacks.nMessage = nMessage;
	m_csCallbacks.lParam = lParam;

	return BTNST_OK;
}

// This function resizes the button to the same size of the image.
// To get good results both the IN and OUT images should have the same size.
//
void CButtonST::SizeToContent()
{
	if (m_csIcons[0].hIcon)
	{
		m_ptImageOrg.x = 0;
		m_ptImageOrg.y = 0;
	    SetWindowPos(	NULL, -1, -1, m_csIcons[0].dwWidth, m_csIcons[0].dwHeight,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
	} // if
	else
	if (m_csBitmaps[0].hBitmap)
	{
		m_ptImageOrg.x = 0;
		m_ptImageOrg.y = 0;
	    SetWindowPos(	NULL, -1, -1, m_csBitmaps[0].dwWidth, m_csBitmaps[0].dwHeight,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
	} // if
}

// This function sets the sound that must be played on particular button states.
//
// Parameters:
//		[IN]	lpszSound
//				A string that specifies the sound to play.
//				If hMod is NULL this string is interpreted as a filename, else it
//				is interpreted as a resource identifier.
//				Pass NULL to remove any previously specified sound.
//		[IN]	hMod
//				Handle to the executable file that contains the resource to be loaded.
//				This parameter must be NULL unless lpszSound specifies a resource identifier.
//		[IN]	bPlayOnClick
//				TRUE if the sound must be played when the button is clicked.
//				FALSE if the sound must be played when the mouse is moved over the button.
//		[IN]	bPlayAsync
//				TRUE if the sound must be played asynchronously.
//				FALSE if the sound must be played synchronously. The application takes control
//				when the sound is completely played.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
#ifdef BTNST_USE_SOUND
DWORD CButtonST::SetSound(LPCTSTR lpszSound, HMODULE hMod, BOOL bPlayOnClick, BOOL bPlayAsync)
{
	BYTE byIndex = bPlayOnClick ? BYTE(1) : BYTE(0);

	// Store new sound
	if (lpszSound)
	{
		if (hMod)	// From resource identifier ?
		{
			m_csSounds[byIndex].lpszSound = lpszSound;
		} // if
		else
		{
			_tcscpy(m_csSounds[byIndex].szSound, lpszSound);
			m_csSounds[byIndex].lpszSound = m_csSounds[byIndex].szSound;
		} // else

		m_csSounds[byIndex].hMod = hMod;
		m_csSounds[byIndex].dwFlags = SND_NODEFAULT | SND_NOWAIT;
		m_csSounds[byIndex].dwFlags |= hMod ? SND_RESOURCE : SND_FILENAME;
		m_csSounds[byIndex].dwFlags |= bPlayAsync ? SND_ASYNC : SND_SYNC;
	} // if
	else
	{
		// Or remove any existing
		::ZeroMemory(&m_csSounds[byIndex], sizeof(STRUCT_SOUND));
	} // else

	return BTNST_OK;
}
#endif

// This function is called every time the button background needs to be painted.
// If the button is in transparent mode this function will NOT be called.
// This is a virtual function that can be rewritten in CButtonST-derived classes
// to produce a whole range of buttons not available by default.
//
// Parameters:
//		[IN]	pDC
//				Pointer to a CDC object that indicates the device context.
//		[IN]	pRect
//				Pointer to a CRect object that indicates the bounds of the
//				area to be painted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::OnDrawBackground(CDC* pDC, CRect* pRect)
{
	COLORREF crColor;

	if (m_bIsFlat == FALSE)
	{
		if (m_bIsFocused || m_bIsDefault)
		{
			CBrush br(RGB(0,0,0));  
			pDC->FrameRect(pRect, &br);
			pRect->DeflateRect(1, 1);
		}
	}

	if (m_bMouseOnButton || m_bIsPressed)
    {
		crColor = m_crColors[BTNST_COLOR_BK_IN];
    }
	else
	{
		if (m_bIsFocused)
			crColor = m_crColors[BTNST_COLOR_BK_FOCUS];
		else
			crColor = m_crColors[BTNST_COLOR_BK_OUT];
	}

	CBrush brBackground(crColor);

	pDC->FillRect(pRect, &brBackground);

	return BTNST_OK;
}

// This function is called every time the button border needs to be painted.
// This is a virtual function that can be rewritten in CButtonST-derived classes
// to produce a whole range of buttons not available by default.
//
// Parameters:
//		[IN]	pDC
//				Pointer to a CDC object that indicates the device context.
//		[IN]	pRect
//				Pointer to a CRect object that indicates the bounds of the
//				area to be painted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CButtonST::OnDrawBorder(CDC* pDC, CRect* pRect)
{
	// Draw pressed button
	if (m_bIsPressed)
	{
		if (m_bIsFlat)
		{
			if (m_bDrawBorder)
				pDC->Draw3dRect(pRect, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNHILIGHT));
		}
		else    
		{
			CBrush brBtnShadow(GetSysColor(COLOR_BTNSHADOW));
			pDC->FrameRect(pRect, &brBtnShadow);
		}
	}
	else // ...else draw non pressed button
	{
		CPen penBtnHiLight(PS_SOLID, 0, GetSysColor(COLOR_BTNHILIGHT)); // White
		CPen pen3DLight(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));       // Light gray
		CPen penBtnShadow(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));   // Dark gray
		CPen pen3DDKShadow(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW)); // Black

		if (m_bIsFlat)
		{
			if (m_bMouseOnButton && m_bDrawBorder)
				pDC->Draw3dRect(pRect, ::GetSysColor(COLOR_BTNHILIGHT), ::GetSysColor(COLOR_BTNSHADOW));
		}
		else
		{
			// Draw top-left borders
			// White line
			CPen* pOldPen = pDC->SelectObject(&penBtnHiLight);
			pDC->MoveTo(pRect->left, pRect->bottom-1);
			pDC->LineTo(pRect->left, pRect->top);
			pDC->LineTo(pRect->right, pRect->top);
			// Light gray line
			pDC->SelectObject(pen3DLight);
			pDC->MoveTo(pRect->left+1, pRect->bottom-1);
			pDC->LineTo(pRect->left+1, pRect->top+1);
			pDC->LineTo(pRect->right, pRect->top+1);
			// Draw bottom-right borders
			// Black line
			pDC->SelectObject(pen3DDKShadow);
			pDC->MoveTo(pRect->left, pRect->bottom-1);
			pDC->LineTo(pRect->right-1, pRect->bottom-1);
			pDC->LineTo(pRect->right-1, pRect->top-1);
			// Dark gray line
			pDC->SelectObject(penBtnShadow);
			pDC->MoveTo(pRect->left+1, pRect->bottom-2);
			pDC->LineTo(pRect->right-2, pRect->bottom-2);
			pDC->LineTo(pRect->right-2, pRect->top);
			//
			pDC->SelectObject(pOldPen);
		} // else
	} // else

	return BTNST_OK;
}

#undef BS_TYPEMASK

BOOL CButtonST::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CButton::OnEraseBkgnd(pDC);
	return FALSE;
}

void CButtonST::SetIconOffsetX(int iOffsetX)
{
	m_iIconOffsetX = iOffsetX;
}

void CButtonST::SetIconOffsetY(int iOffsetY)
{
	m_iIconOffsetY = iOffsetY;
}

void CButtonST::SetIconOffset(int iOffsetX, int iOffsetY)
{
	SetIconOffsetX(iOffsetX);
	SetIconOffsetY(iOffsetY);
}

void CButtonST::SetTextOffsetX(int iOffsetX)
{
	m_iTextOffsetX = iOffsetX;
}

void CButtonST::SetTextOffsetY(int iOffsetY)
{
	m_iTextOffsetY = iOffsetY;
}

void CButtonST::SetTextOffset(int iOffsetX, int iOffsetY)
{
	SetTextOffsetX(iOffsetX);
	SetTextOffsetY(iOffsetY);
}

BOOL CButtonST::SetFont(CString sFontName, long lSize, long lWeight, BYTE bItalic, BYTE bUnderline)
{
	if(m_pLF == NULL)
	{
		m_pLF = (LOGFONT*)calloc(1, sizeof(LOGFONT));
	}
	
	if(m_pLF)
	{
#if _MSC_VER >= 1400  // VS2005
		strncpy_s(m_pLF->lfFaceName, _countof(m_pLF->lfFaceName), sFontName, _TRUNCATE);
#else
		strncpy(m_pLF->lfFaceName, sFontName, 31);
#endif
		m_pLF->lfHeight = lSize;
		m_pLF->lfWeight = lWeight;
		m_pLF->lfItalic = bItalic;
		m_pLF->lfUnderline = bUnderline;
		m_font.DeleteObject();
		if(m_font.CreateFontIndirect(m_pLF))
			return TRUE;
	}
	return FALSE;
}

BOOL CButtonST::SetFont(LOGFONT* pFont)
{
	if (pFont)
	{
		if(m_pLF == NULL)
		{
			m_pLF = (LOGFONT*)calloc(1, sizeof(LOGFONT));
		}

		if(m_pLF)
		{
			memcpy(m_pLF, pFont, sizeof(LOGFONT));
			m_font.DeleteObject();
			if(m_font.CreateFontIndirect(m_pLF))
				return TRUE;
		}
	}
	return FALSE;
}

LOGFONT* CButtonST::GetFont()
{
	return m_pLF;
}


#pragma warning (pop)
