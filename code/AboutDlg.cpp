/*----------------------------------------------------------------------------
| File:    AboutDlg.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CAboutDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: AboutDlg.cpp 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "AboutDlg.h"
#include "version.h"
#include <shared/Color.h>


// CAboutDlg dialog

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    m_horzEdgePen.CreatePen(PS_SOLID, 1, CColor::gainsboro);
}


CAboutDlg::~CAboutDlg()
{
    if(m_horzEdgePen.m_hObject)
        m_horzEdgePen.DeleteObject();

    if(m_font.m_hObject)
        m_font.DeleteObject();
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_CREDITS, m_editCredits);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_HELPINFO()
    ON_WM_CTLCOLOR()
    ON_WM_SYSCOLORCHANGE()
    ON_BN_CLICKED(IDC_BTN_WEBSITE, OnBtnVisitWebsite)
END_MESSAGE_MAP()


// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    __init__();

    return TRUE;  // return TRUE unless you set the focus to a control
}


void CAboutDlg::__init__()
{
    InitLanguage();
    InitColors();
    InitDialogBackground();
    InitPositions();

    // init dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    // init credits box
    m_editCredits.SetLinkColors(RGB(0,0,255), RGB(255,0,0));
    m_editCredits.SetWindowText(CAppSettings::GetCredits());
    m_editCredits.EnableFontSmoothingFix(m_grayColor);
    m_editCredits.InitUrlOverlayFont();
}


void CAboutDlg::InitLanguage()
{
    // caption
    CString strCaption = ReplaceTags(
        CAppSettings::LoadString(_T("IDS_ABOUTBOX_CAPTION")),
        _T("APPLICATION"),
        CAppSettings::GetAppName() + _T(" ") + CAppSettings::GetVersionString(PRODUCT_VERSION_FIX));
    SetWindowText(strCaption);

    // assemble version number (e.g. "Version 3.2.1 (Revision 123)")
    CString strVersion = CAppSettings::GetVersionString(PRODUCT_VERSION_FIX)
        + _T(" (") + CAppSettings::LoadString(_T("IDS_REVISION")) + _T(" ") + CString(pRevision) + _T(")");

    // statics
    SetDlgItemText(IDS_ABOUT_VERSION, CAppSettings::LoadString(_T("IDS_VERSION")) + _T(" ") + strVersion);
    SetDlgItemText(IDS_ABOUT_COPYRIGHT, CAppSettings::LoadString(_T("IDS_COPYRIGHT")));
    SetDlgItemText(IDS_ABOUT_ALL_RIGHTS_RESERVED, CAppSettings::LoadString(_T("IDS_ALL_RIGHTS_RESERVED")));

    // buttons
    SetDlgItemText(IDOK, CAppSettings::LoadString(_T("IDS_OK")));
    SetDlgItemText(IDC_BTN_WEBSITE, CAppSettings::LoadString(_T("IDS_ABOUTBOX_BTN_WEBSITE")));
}


void CAboutDlg::InitColors()
{
    // credits control
    m_grayColor = ::GetSysColor(COLOR_BTNFACE);
    if(m_grayBrush.GetSafeHandle()) m_grayBrush.DeleteObject();
    m_grayBrush.CreateSolidBrush(m_grayColor);
}


void CAboutDlg::InitDialogBackground(BOOL bResizeDialog /*TRUE*/)
{
    // hide horizontal edge (we paint the edge later by ourselves)
    CStatic* pHorzEdge = (CStatic*)GetDlgItem(IDC_HORZ_EDGE);
    if(pHorzEdge)
    {
        pHorzEdge->ShowWindow(SW_HIDE);
    }

    // load background image
    m_imgBackground.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_ABOUT_BKGND), "PNG"), CXIMAGE_FORMAT_PNG);

    // resize window to match background image and add some
    // space for the lower part with the buttons
    if(bResizeDialog && m_imgBackground.IsValid())
    {
        // get height of OK button
        int nBtnHeight = 25;
        CButton* pOkBtn = (CButton*)GetDlgItem(IDOK);
        if(pOkBtn)
        {
            RECT buttonRect;
            pOkBtn->GetWindowRect(&buttonRect);
            nBtnHeight = buttonRect.bottom - buttonRect.top;
        }

        // calculate window rect and resize dialog
        CRect rect;
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = m_imgBackground.GetWidth();
        rect.bottom = m_imgBackground.GetHeight() + ((nBtnHeight * 3) / 2);
        AdjustWindowRect(&rect, this->GetStyle(), FALSE);
        SetWindowPos(&wndTop, 0, 0, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_FRAMECHANGED);
    }
}


void CAboutDlg::InitPositions()
{
    CRect rectDlg;
    GetClientRect(&rectDlg);

    CWnd* pWnd = NULL;
    CRect ctrlRect(0,0,0,0);

    // website button
    pWnd = GetDlgItem(IDC_BTN_WEBSITE);                                             ASSERT(pWnd);
    if(pWnd)
    {
        // calculate size of localized button text
        CString strBtn;
        pWnd->GetWindowText(strBtn);
        int textSize = GetTextExtent(pWnd, strBtn);

        // get current button size
        pWnd->GetClientRect(&ctrlRect);

        // resize button
        pWnd->SetWindowPos(&wndTop,
                           rectDlg.left + 5,
                           rectDlg.bottom - ctrlRect.Height() - 5,
                           (textSize > 0) ? textSize + 10       : 0,
                           (textSize > 0) ? ctrlRect.Height()   : 0,
                           (textSize > 0) ? SWP_FRAMECHANGED    : SWP_NOSIZE | SWP_FRAMECHANGED
                           );
    }

    // OK button
    pWnd = GetDlgItem(IDOK);                                                        ASSERT(pWnd);
    if(pWnd)
    {
        pWnd->GetClientRect(&ctrlRect);
        pWnd->SetWindowPos(&wndTop,
                           rectDlg.right - ctrlRect.Width() - 5,
                           rectDlg.bottom - ctrlRect.Height() - 5,
                           0, 0,
                           SWP_NOSIZE | SWP_FRAMECHANGED);
    }

    // credits box
    m_editCredits.MoveWindow(rectDlg.left + 30,
                             m_imgBackground.GetHeight() - 80,
                             rectDlg.Width() - 60,
                             60);

    // version and copyright statics
    int staticLeft = 148;
    int staticTop  = 105;

    pWnd = GetDlgItem(IDS_ABOUT_VERSION);                                           ASSERT(pWnd);
    if(pWnd)
    {
        pWnd->SetWindowPos(&wndTop,
                           staticLeft, staticTop,
                           0, 0,
                           SWP_NOSIZE | SWP_FRAMECHANGED);
    }

    pWnd = GetDlgItem(IDS_ABOUT_COPYRIGHT);                                         ASSERT(pWnd);
    if(pWnd)
    {
        pWnd->SetWindowPos(&wndTop,
                           staticLeft, staticTop + 20,
                           0, 0,
                           SWP_NOSIZE | SWP_FRAMECHANGED);
    }

    pWnd = GetDlgItem(IDS_ABOUT_ALL_RIGHTS_RESERVED);                               ASSERT(pWnd);
    if(pWnd)
    {
        pWnd->SetWindowPos(&wndTop,
                           staticLeft, staticTop + 35,
                           0, 0,
                           SWP_NOSIZE | SWP_FRAMECHANGED);
    }
}


void CAboutDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // Do not call CDialog::OnPaint() for painting messages

    // draw background image
    if(m_imgBackground.IsValid())
    {
        m_imgBackground.Draw(dc.m_hDC, 0, 0, m_imgBackground.GetWidth(), m_imgBackground.GetHeight());
    }

    // draw horizontal edge
    CPen* pOldPen = dc.SelectObject(&m_horzEdgePen);
    {
        dc.MoveTo(0, m_imgBackground.GetHeight());
        dc.LineTo(m_imgBackground.GetWidth(), m_imgBackground.GetHeight());
    }
    dc.SelectObject(pOldPen);
}


void CAboutDlg::OnSysColorChange()
{
    CDialog::OnSysColorChange();

    InitColors();
}


BOOL CAboutDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    CAppSettings::OnHelp();

    return TRUE;
    //return CDialog::OnHelpInfo(pHelpInfo);
}


HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = NULL;

    switch(nCtlColor)
    {
        // note: READONLY and DISABLED controls are processed using CTLCOLOR_STATIC!
        //       if your controls are set to DISABLED, then you CAN NOT alter the text color.
        //       if you want this ability, change all your DISABLED controls to READONLY.
        //       (http://www.codeguru.com/cpp/controls/editctrl/backgroundcolor/article.php/c3929/)

        case CTLCOLOR_EDIT:
        case CTLCOLOR_STATIC:
            {
                if(pWnd->GetDlgCtrlID() == IDC_EDIT_CREDITS)
                {
                    pDC->SetBkMode(OPAQUE);
                    pDC->SetBkColor(m_grayColor);
                    hbr = (HBRUSH)m_grayBrush.GetSafeHandle();
                }
                else
                {
                    pDC->SetBkMode(TRANSPARENT);
                    hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
                }
            }
            break;

        default:
            hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
            break;
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}


void CAboutDlg::OnBtnVisitWebsite()
{
    CString strUrl = CAppSettings::GetWebsiteUrl();                                ASSERT(!strUrl.IsEmpty());
    if(!strUrl.IsEmpty())
    {
        GotoURL(strUrl);
    }
}

