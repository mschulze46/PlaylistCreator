/*----------------------------------------------------------------------------
| File:    PropPageDonations.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropPageDonations class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: PropPageDonations.cpp 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropPageDonations.h"
#include "PropSheetSettings.h"

#define IMAGE_MARGIN 10 // image margin in pixels

// CPropPageDonations dialog

IMPLEMENT_DYNAMIC(CPropPageDonations, CPropertyPage)
CPropPageDonations::CPropPageDonations()
    : CPropertyPage(CPropPageDonations::IDD)
{
    __init__();
}


CPropPageDonations::~CPropPageDonations()
{
    if(m_font.m_hObject)
        m_font.DeleteObject();

    m_listPlaylistRot.clear();
}


void CPropPageDonations::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDS_DONATIONS_GROUP_WHY, m_groupDonationsWhy);
    DDX_Control(pDX, IDS_DONATIONS_GROUP_HOW, m_groupDonationsHow);
    DDX_Control(pDX, IDS_DONATIONS_GROUP_USAGE, m_groupUsageFrequency);
    DDX_Control(pDX, IDC_BTN_DONATIONS_WEBSITE, m_btnWebsite);
    DDX_Control(pDX, IDC_BTN_DONATIONS_PAYPAL, m_btnPayPal);
}


BEGIN_MESSAGE_MAP(CPropPageDonations, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_WM_PAINT()
    ON_WM_SYSCOLORCHANGE()
    ON_BN_CLICKED(IDC_BTN_DONATIONS_WEBSITE, OnBtnWebsite)
    ON_BN_CLICKED(IDC_BTN_DONATIONS_PAYPAL, OnBtnPaypal)
END_MESSAGE_MAP()


// CPropPageDonations message handlers

BOOL CPropPageDonations::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    InitPropPage();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropPageDonations::__init__()
{
    // init parent
    m_pParent = NULL;

    // init title
    m_psp.dwFlags |= PSP_USETITLE;
    m_psp.pszTitle = GetTitle();

    // no help
    m_psp.dwFlags &= ~PSP_HASHELP;

    // enable white background
    SetHasWhiteBackground(true);

    // init page icon
    m_psp.dwFlags |= PSP_USEHICON;
    HICON hIconPage = GetIcon();
    m_psp.hIcon = hIconPage;
}


void CPropPageDonations::InitPropPage()
{
    InitLanguage();
    UpdateFromRegistry();
    LoadGraphics();
    InitLayout();
    InitButtons();
    InitUsageFrequency();
    InitThemeDependentControls();

    // init dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    UpdateData(TO_DIALOG);
}


void CPropPageDonations::InitLanguage()
{
    // why donations
    SetDlgItemText(IDS_DONATIONS_GROUP_WHY, CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_GROUP_WHY")));
    SetDlgItemText(IDS_DONATIONS_WHY_INFO, ReplaceTags(CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_WHY_INFO")), _T("APPLICATION"), CAppSettings::GetAppName()));

    // how to donate
    SetDlgItemText(IDS_DONATIONS_GROUP_HOW, CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_GROUP_HOW")));
    SetDlgItemText(IDS_DONATIONS_HOW_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_HOW_INFO")));
    SetDlgItemText(IDC_BTN_DONATIONS_WEBSITE, CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_BTN_WEBSITE")));
    SetDlgItemText(IDC_BTN_DONATIONS_PAYPAL, CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_BTN_PAYPAL")));

    // usage frequency
    SetDlgItemText(IDS_DONATIONS_GROUP_USAGE, CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_GROUP_USAGE")));
}


void CPropPageDonations::LoadGraphics()
{
    // donations
    m_imageDonations.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_DONATIONS), "PNG"), CXIMAGE_FORMAT_PNG);

    // playlist
    m_imagePlaylist.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_PLAYLIST_CREATOR), "PNG"), CXIMAGE_FORMAT_PNG);

    // rotated graphics
    m_nRotatedImages = min(m_nTotalPlaylists / 20, 5); // limit to 5 rotated images
    float fAngle = -20.0;

    m_imagePlaylist.Rotate2(fAngle,   &m_imagePlaylistRot1, CxImage::IM_CATROM, CxImage::OM_TRANSPARENT);
    m_imagePlaylist.Rotate2(fAngle*2, &m_imagePlaylistRot2, CxImage::IM_CATROM, CxImage::OM_TRANSPARENT);
    m_imagePlaylist.Rotate2(fAngle*3, &m_imagePlaylistRot3, CxImage::IM_CATROM, CxImage::OM_TRANSPARENT);
    m_imagePlaylist.Rotate2(fAngle*4, &m_imagePlaylistRot4, CxImage::IM_CATROM, CxImage::OM_TRANSPARENT);
    m_imagePlaylist.Rotate2(fAngle*5, &m_imagePlaylistRot5, CxImage::IM_CATROM, CxImage::OM_TRANSPARENT);

    m_listPlaylistRot.push_back(m_imagePlaylistRot1);
    m_listPlaylistRot.push_back(m_imagePlaylistRot2);
    m_listPlaylistRot.push_back(m_imagePlaylistRot3);
    m_listPlaylistRot.push_back(m_imagePlaylistRot4);
    m_listPlaylistRot.push_back(m_imagePlaylistRot5);
}


void CPropPageDonations::InitButtons()
{
    int nIconSize   = 16;
    int nIconOffset = 10;

    // website button
    CButtonIcons gfxBtnHome(m_hWnd, IDI_HOME, nIconSize, nIconSize);
    m_btnWebsite.SetThemeHelper(&m_ThemeHelper);
    m_btnWebsite.SetIcon(
        gfxBtnHome.GetIconIn(),
        gfxBtnHome.GetIconOut(),
        NULL,
        gfxBtnHome.GetIconDisabled());
    m_btnWebsite.SetIconOffsetX(nIconOffset);

    // PayPal button
    CButtonIcons gfxBtnPayPal(m_hWnd, IDI_PAYPAL, nIconSize, nIconSize);
    m_btnPayPal.SetThemeHelper(&m_ThemeHelper);
    m_btnPayPal.SetIcon(
        gfxBtnPayPal.GetIconIn(),
        gfxBtnPayPal.GetIconOut(),
        NULL,
        gfxBtnPayPal.GetIconDisabled());
    m_btnPayPal.SetIconOffsetX(nIconOffset);
}


void CPropPageDonations::InitLayout()
{
    // get left and right border
    CRect rectTitle;
    GetDlgItem(IDS_DONATIONS_GROUP_WHY)->GetWindowRect(&rectTitle);
    ScreenToClient(rectTitle);

    int leftBorder  = rectTitle.left;
    int rightBorder = rectTitle.right;

    // resize 'why donations' text in dependency of right-aligned graphic
    CWnd* pStaticWhyDonations = GetDlgItem(IDS_DONATIONS_WHY_INFO);
    if(pStaticWhyDonations)
    {
        CRect rectStaticWhyDonations;
        pStaticWhyDonations->GetWindowRect(&rectStaticWhyDonations);
        ScreenToClient(&rectStaticWhyDonations);

        pStaticWhyDonations->MoveWindow(
            rectStaticWhyDonations.left,
            rectStaticWhyDonations.top,
            rightBorder - rectStaticWhyDonations.left - m_imageDonations.GetWidth() - (2 * IMAGE_MARGIN),
            rectStaticWhyDonations.Height(),
            TRUE);
    }
}


void CPropPageDonations::InitUsageFrequency()
{
    CString strInfo1 = ReplaceTags(
        CAppSettings::LoadString(_T("IDS_SETTINGS_DONATIONS_USAGE_INFO1")),
        _T("TOTAL_PLAYLISTS"), IntegerToString(m_nTotalPlaylists),
        _T("TOTAL_SONGS"), IntegerToString(m_nTotalSongs));
    SetDlgItemText(IDS_DONATIONS_USAGE_INFO1, strInfo1);

    CString strIdInfo2;
    if(CAppSettings::GetIsSupporter())
    {
        strIdInfo2 = _T("IDS_SETTINGS_DONATIONS_USAGE_INFO2_IS_SUPPORTER");
    }
    else
    {
        if(m_nTotalPlaylists > m_nTotalSongs)
        {
            strIdInfo2 = _T("IDS_SETTINGS_DONATIONS_USAGE_INFO2_STRANGE_VALUES");
        }
        else if(m_nTotalPlaylists < 25)
        {
            strIdInfo2 = _T("IDS_SETTINGS_DONATIONS_USAGE_INFO2_PHASE1");
        }
        else if(m_nTotalPlaylists >= 25 && m_nTotalPlaylists < 50)
        {
            strIdInfo2 = _T("IDS_SETTINGS_DONATIONS_USAGE_INFO2_PHASE2");
        }
        else if(m_nTotalPlaylists >= 50)
        {
            strIdInfo2 = _T("IDS_SETTINGS_DONATIONS_USAGE_INFO2_PHASE3");
        }
    }
    SetDlgItemText(IDS_DONATIONS_USAGE_INFO2, CAppSettings::LoadString(strIdInfo2));
}


void CPropPageDonations::InitThemeDependentControls()
{
    BOOL bIsAppThemed = m_ThemeHelper.IsAppThemed();

    // buttons
    m_btnWebsite.SetFlat(bIsAppThemed);
    m_btnPayPal.SetFlat(bIsAppThemed);
}


CString CPropPageDonations::GetTitle()
{
    if(m_strTitle.GetLength() == 0)
    {
        m_strTitle = CAppSettings::LoadString(_T("IDS_PROPPAGE_DONATIONS_TITLE"));
    }
    return m_strTitle;
}


HBRUSH CPropPageDonations::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

    if(HasWhiteBackground())
    {
        pDC->SetBkMode(TRANSPARENT);
        return ::GetSysColorBrush(COLOR_WINDOW);
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}


HICON CPropPageDonations::GetIcon()
{
    return AfxGetApp()->LoadIcon(IDI_SETTINGS_DONATIONS);
}


void CPropPageDonations::UpdateFromRegistry()
{
    m_nTotalPlaylists = CAppSettings::GetIntProperty(PROP_TOTALPLAYLISTS, 0);
    if(m_nTotalPlaylists < 0)
    {
        CAppSettings::SetIntProperty(PROP_TOTALPLAYLISTS, 0);
        m_nTotalPlaylists = 0;
    }

    m_nTotalSongs = CAppSettings::GetIntProperty(PROP_TOTALSONGS, 0);
    if(m_nTotalSongs < 0)
    {
        CAppSettings::SetIntProperty(PROP_TOTALSONGS, 0);
        m_nTotalSongs = 0;
    }
}


void CPropPageDonations::UpdateToRegistry()
{
}


BOOL CPropPageDonations::OnApply() 
{
    UpdateData(FROM_DIALOG);
    UpdateToRegistry();

    return CPropertyPage::OnApply();
}


void CPropPageDonations::OnOK() 
{
    CPropertyPage::OnOK();
}


void CPropPageDonations::OnCancel() 
{
    CPropertyPage::OnCancel();
}


void CPropPageDonations::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // Do not call __super::OnPaint() for painting messages

    HDC hDC = dc.m_hDC;                                                     ASSERT(hDC);

    // donations graphic
    RECT rectTitleDonationsWhy;
    GetDlgItem(IDS_DONATIONS_GROUP_WHY)->GetWindowRect(&rectTitleDonationsWhy);
    ScreenToClient(&rectTitleDonationsWhy);

    RECT rectStaticDonationsWhy;
    GetDlgItem(IDS_DONATIONS_WHY_INFO)->GetWindowRect(&rectStaticDonationsWhy);
    ScreenToClient(&rectStaticDonationsWhy);

    RECT rectDonations;
    rectDonations.left   = rectStaticDonationsWhy.right + IMAGE_MARGIN;
    rectDonations.top    = rectStaticDonationsWhy.top - (IMAGE_MARGIN / 2);
    rectDonations.right  = rectDonations.left + m_imageDonations.GetWidth();
    rectDonations.bottom = rectDonations.top  + m_imageDonations.GetHeight();

    m_imageDonations.Draw(hDC, rectDonations);

    // playlist pile
    RECT rectUsageInfo1;
    GetDlgItem(IDS_DONATIONS_USAGE_INFO1)->GetWindowRect(&rectUsageInfo1);
    ScreenToClient(&rectUsageInfo1);

    RECT rectPlaylist;
    rectPlaylist.left   = 30;
    rectPlaylist.top    = rectUsageInfo1.top;
    rectPlaylist.right  = rectPlaylist.left + m_imagePlaylist.GetWidth();
    rectPlaylist.bottom = rectPlaylist.top  + m_imagePlaylist.GetHeight();

    for(int i = m_nRotatedImages; i > 0; i--)
    {
        CxImage image = m_listPlaylistRot[i-1];

        int iOffsetX = (image.GetWidth() - m_imagePlaylist.GetWidth()) / 2;
        int iOffsetY = (image.GetHeight() - m_imagePlaylist.GetHeight()) / 2;

        RECT rectImageRot = rectPlaylist;
        rectImageRot.left   -= iOffsetX;
        rectImageRot.top    -= iOffsetY;
        rectImageRot.right  += iOffsetX;
        rectImageRot.bottom += iOffsetY;
        image.Draw(hDC, rectImageRot);
    }

    m_imagePlaylist.Draw(hDC, rectPlaylist);
}


void CPropPageDonations::OnSysColorChange()
{
    CPropertyPage::OnSysColorChange();

    InitThemeDependentControls();
    Invalidate();
}


void CPropPageDonations::OnBtnWebsite()
{
    GotoURL(CAppSettings::GetCompanyDonationsUrl());
}


void CPropPageDonations::OnBtnPaypal()
{
    GotoURL(CAppSettings::GetPayPalDonationsUrl());
}
