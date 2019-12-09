/*----------------------------------------------------------------------------
| File:    PropPageUserInterface.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropPageUserInterface class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 392 $
| $Id: PropPageUserInterface.cpp 392 2009-06-12 16:38:16Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropPageUserInterface.h"
#include "PropSheetSettings.h"


// CPropPageUserInterface dialog

IMPLEMENT_DYNAMIC(CPropPageUserInterface, CPropertyPage)
CPropPageUserInterface::CPropPageUserInterface()
    : CPropertyPage(CPropPageUserInterface::IDD)
    , m_bSingleInstance(FALSE)
    , m_bTransparency(FALSE)
    , m_bSnapToScreenBorder(FALSE)
    , m_bPlaySound_Success(FALSE)
    , m_bShowGridlines(TRUE)
    , m_bShowAlternateRowColor(TRUE)
    , m_bHighlightMissingItems(TRUE)
    , m_clrAlternateRowColor(DEFAULT_ALTERNATE_ROW_COLOR)
    , m_clrMissingItems(DEFAULT_MISSING_ITEMS_COLOR)
{
    __init__();
}


CPropPageUserInterface::~CPropPageUserInterface()
{
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }
}


void CPropPageUserInterface::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SETTINGS_UI_GROUP_GENERAL, m_groupProgram);
    DDX_Control(pDX, IDC_SETTINGS_UI_GROUP_PLAYLIST, m_groupPlaylist);
    DDX_Check(pDX, IDC_SETTINGS_UI_SINGLE_INSTANCE, m_bSingleInstance);
    DDX_Check(pDX, IDC_SETTINGS_UI_TRANSPARENCY, m_bTransparency);
    DDX_Check(pDX, IDC_SETTINGS_UI_SNAP_TO_SCREEN_BORDER, m_bSnapToScreenBorder);
    DDX_Check(pDX, IDC_SETTINGS_UI_PLAY_SOUND_SUCCESS, m_bPlaySound_Success);
    DDX_Check(pDX, IDC_SETTINGS_UI_SHOW_GRIDLINES, m_bShowGridlines);
    DDX_Check(pDX, IDC_SETTINGS_UI_SHOW_ALTERNATE_ROW_COLOR, m_bShowAlternateRowColor);
    DDX_Control(pDX, IDC_BTN_SETTINGS_UI_ALTERNATE_ROW_COLOR, m_btnAlternateRowColor);
    DDX_Check(pDX, IDC_SETTINGS_UI_HIGHLIGHT_MISSING_ITEMS, m_bHighlightMissingItems);
    DDX_Control(pDX, IDC_BTN_SETTINGS_UI_HIGHLIGHT_MISSING_ITEMS, m_btnMissingItemsColor);
}


BEGIN_MESSAGE_MAP(CPropPageUserInterface, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_SETTINGS_UI_SINGLE_INSTANCE, OnClickSingleInstance)
    ON_BN_CLICKED(IDC_SETTINGS_UI_TRANSPARENCY, OnClickTransparency)
    ON_BN_CLICKED(IDC_SETTINGS_UI_SNAP_TO_SCREEN_BORDER, OnClickSnapToScreenBorder)
    ON_BN_CLICKED(IDC_SETTINGS_UI_PLAY_SOUND_SUCCESS, OnClickPlaySound_Success)
    ON_BN_CLICKED(IDC_SETTINGS_UI_SHOW_GRIDLINES, OnClickShowGridlines)
    ON_BN_CLICKED(IDC_SETTINGS_UI_SHOW_ALTERNATE_ROW_COLOR, OnClickShowAlternateRowColor)
    ON_BN_CLICKED(IDC_SETTINGS_UI_HIGHLIGHT_MISSING_ITEMS, OnClickHighlightMissingItems)
    ON_MESSAGE(CPN_SELENDOK, OnColorSelChange)
END_MESSAGE_MAP()


// CPropPageUserInterface message handlers

BOOL CPropPageUserInterface::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    InitPropPage();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropPageUserInterface::__init__()
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


void CPropPageUserInterface::InitPropPage()
{
    InitLanguage();
    UpdateFromRegistry();
    InitControls();

    UpdateData(TO_DIALOG);
}


void CPropPageUserInterface::InitLanguage()
{
    // program
    SetDlgItemText(IDC_SETTINGS_UI_GROUP_GENERAL, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_GROUP_GENERAL")));
    SetDlgItemText(IDC_SETTINGS_UI_SINGLE_INSTANCE, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_SINGLE_INSTANCE")));
    SetDlgItemText(IDC_SETTINGS_UI_TRANSPARENCY, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_TRANSPARENCY")));
    SetDlgItemText(IDC_SETTINGS_UI_SNAP_TO_SCREEN_BORDER, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_SNAP_TO_SCREEN_BORDER")));
    SetDlgItemText(IDC_SETTINGS_UI_PLAY_SOUND_SUCCESS, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_PLAY_SOUND_SUCCESS")));

    // playlist control
    SetDlgItemText(IDC_SETTINGS_UI_GROUP_PLAYLIST, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_GROUP_PLAYLIST")));
    SetDlgItemText(IDC_SETTINGS_UI_SHOW_GRIDLINES, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_SHOW_GRIDLINES")));
    SetDlgItemText(IDC_SETTINGS_UI_SHOW_ALTERNATE_ROW_COLOR, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_SHOW_ALTERNATE_ROW_COLOR")));
    SetDlgItemText(IDC_SETTINGS_UI_HIGHLIGHT_MISSING_ITEMS, CAppSettings::LoadString(_T("IDS_SETTINGS_UI_HIGHLIGHT_MISSING_ITEMS")));
}


void CPropPageUserInterface::InitControls()
{
    // dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    // program

    // playlist control
    m_btnAlternateRowColor.SetDefaultText(CAppSettings::LoadString(_T("IDS_COLOR_PICKER_POPUP_DEFAULT")));
    m_btnAlternateRowColor.SetCustomText(CAppSettings::LoadString(_T("IDS_COLOR_PICKER_POPUP_CUSTOM")));
    m_btnAlternateRowColor.SetDefaultColor(DEFAULT_ALTERNATE_ROW_COLOR);
    m_btnAlternateRowColor.SetColor(m_clrAlternateRowColor);
    m_btnAlternateRowColor.EnableWindow(m_bShowAlternateRowColor);

    m_btnMissingItemsColor.SetDefaultText(CAppSettings::LoadString(_T("IDS_COLOR_PICKER_POPUP_DEFAULT")));
    m_btnMissingItemsColor.SetCustomText(CAppSettings::LoadString(_T("IDS_COLOR_PICKER_POPUP_CUSTOM")));
    m_btnMissingItemsColor.SetDefaultColor(DEFAULT_MISSING_ITEMS_COLOR);
    m_btnMissingItemsColor.SetColor(m_clrMissingItems);
    m_btnMissingItemsColor.EnableWindow(m_bHighlightMissingItems);
}


CString CPropPageUserInterface::GetTitle()
{
    if(m_strTitle.IsEmpty())
        m_strTitle = CAppSettings::LoadString(_T("IDS_PROPPAGE_USER_INTERFACE_TITLE"));

    return m_strTitle;
}


HBRUSH CPropPageUserInterface::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


HICON CPropPageUserInterface::GetIcon()
{
    return AfxGetApp()->LoadIcon(IDI_SETTINGS_USER_INTERFACE);
}


void CPropPageUserInterface::UpdateFromRegistry()
{
    // program
    m_bSingleInstance       = CAppSettings::GetIntProperty(PROP_SINGLEINSTANCE, (int)TRUE);
    m_bTransparency         = CAppSettings::GetIntProperty(PROP_TRANSPARENCY, (int)FALSE);
    m_bSnapToScreenBorder   = CAppSettings::GetIntProperty(PROP_SNAPTOSCREENBORDER, (int)TRUE);
    m_bPlaySound_Success    = CAppSettings::GetIntProperty(PROP_PLAYSOUND_SUCCESS, (int)TRUE);

    // playlist control
    m_bShowGridlines         = CAppSettings::GetIntProperty(PROP_SHOWGRIDLINES, (int)TRUE);
    m_bShowAlternateRowColor = CAppSettings::GetIntProperty(PROP_SHOWALTERNATEROWCOLOR, (int)TRUE);
    m_clrAlternateRowColor   = CAppSettings::GetIntProperty(PROP_ALTERNATEROWCOLOR, (int)DEFAULT_ALTERNATE_ROW_COLOR);
    m_bHighlightMissingItems = CAppSettings::GetIntProperty(PROP_HIGHLIGHTMISSINGITEMS, (int)TRUE);
    m_clrMissingItems        = CAppSettings::GetIntProperty(PROP_MISSINGITEMSCOLOR, (int)DEFAULT_MISSING_ITEMS_COLOR);
}


void CPropPageUserInterface::UpdateToRegistry()
{
    // program
    CAppSettings::SetIntProperty(PROP_SINGLEINSTANCE, (int)m_bSingleInstance);
    CAppSettings::SetIntProperty(PROP_TRANSPARENCY, (int)m_bTransparency);
    CAppSettings::SetIntProperty(PROP_SNAPTOSCREENBORDER, (int)m_bSnapToScreenBorder);
    CAppSettings::SetIntProperty(PROP_PLAYSOUND_SUCCESS, (int)m_bPlaySound_Success);

    // playlist control
    CAppSettings::SetIntProperty(PROP_SHOWGRIDLINES, (int)m_bShowGridlines);
    CAppSettings::SetIntProperty(PROP_SHOWALTERNATEROWCOLOR, (int)m_bShowAlternateRowColor);
    CAppSettings::SetIntProperty(PROP_ALTERNATEROWCOLOR, (int)m_btnAlternateRowColor.GetColor(TRUE));
    CAppSettings::SetIntProperty(PROP_HIGHLIGHTMISSINGITEMS, (int)m_bHighlightMissingItems);
    CAppSettings::SetIntProperty(PROP_MISSINGITEMSCOLOR, (int)m_btnMissingItemsColor.GetColor(TRUE));
}


BOOL CPropPageUserInterface::OnApply() 
{
    UpdateData(FROM_DIALOG);
    UpdateToRegistry();

    return CPropertyPage::OnApply();
}


void CPropPageUserInterface::OnOK() 
{
    CPropertyPage::OnOK();
}


void CPropPageUserInterface::OnCancel() 
{
    CPropertyPage::OnCancel();
}


void CPropPageUserInterface::OnClickSingleInstance()
{
    SetModified(TRUE);
}


void CPropPageUserInterface::OnClickTransparency()
{
    SetModified(TRUE);
}


void CPropPageUserInterface::OnClickSnapToScreenBorder()
{
    SetModified(TRUE);
}


void CPropPageUserInterface::OnClickPlaySound_Success()
{
    SetModified(TRUE);
}


void CPropPageUserInterface::OnClickShowGridlines()
{
    SetModified(TRUE);
}


void CPropPageUserInterface::OnClickShowAlternateRowColor()
{
    UpdateData(FROM_DIALOG);
    m_btnAlternateRowColor.EnableWindow(m_bShowAlternateRowColor);

    SetModified(TRUE);
}


void CPropPageUserInterface::OnClickHighlightMissingItems()
{
    UpdateData(FROM_DIALOG);
    m_btnMissingItemsColor.EnableWindow(m_bHighlightMissingItems);

    SetModified(TRUE);
}


LONG CPropPageUserInterface::OnColorSelChange(UINT lParam, LONG wParam)
{
    // lParam: COLORREF
    // wParam: control ID

    SetModified(TRUE);
    return TRUE;
}


