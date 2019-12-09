/*----------------------------------------------------------------------------
| File:    PropPagePlaylist.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropPagePlaylist class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 483 $
| $Id: PropPagePlaylist.cpp 483 2010-01-08 16:38:54Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropPagePlaylist.h"
#include "PropSheetSettings.h"


// CPropPagePlaylist dialog

IMPLEMENT_DYNAMIC(CPropPagePlaylist, CPropertyPage)
CPropPagePlaylist::CPropPagePlaylist()
    : CPropertyPage(CPropPagePlaylist::IDD)
    , m_nNewEntries(0)
    , m_bSortNewItems(FALSE)
    , m_nSaveMode(0)
    , m_bUseForwardSlashes(FALSE)
    , m_bAutoPlaylistName(FALSE)
    , m_bAutoPlaylistFolder(FALSE)
    , m_bAutoClear(FALSE)
{
    __init__();
}


CPropPagePlaylist::~CPropPagePlaylist()
{
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }
}


void CPropPagePlaylist::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDS_SETTINGS_PLAYLIST_GROUP_NEW_ENTRIES, m_groupNewEntries);
    DDX_Control(pDX, IDS_SETTINGS_PLAYLIST_GROUP_SAVE_MODE, m_groupSaveMode);
    DDX_Control(pDX, IDS_SETTINGS_PLAYLIST_GROUP_MISC, m_groupMiscellaneous);
    DDX_Control(pDX, IDC_COMBO_SETTINGS_NEW_ENTRIES, m_cbNewEntries);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_SORT_NEW_ITEMS, m_bSortNewItems);
    DDX_Control(pDX, IDC_COMBO_SETTINGS_SAVE_MODE, m_cbSaveMode);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_SLASH_STYLE, m_bUseForwardSlashes);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_AUTO_NAME, m_bAutoPlaylistName);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_AUTO_FOLDER, m_bAutoPlaylistFolder);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_AUTO_CLEAR, m_bAutoClear);
}


BEGIN_MESSAGE_MAP(CPropPagePlaylist, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_CBN_SELCHANGE(IDC_COMBO_SETTINGS_NEW_ENTRIES, OnSelChangeNewEntries)
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_SORT_NEW_ITEMS, OnClickSortNewItems)
    ON_CBN_SELCHANGE(IDC_COMBO_SETTINGS_SAVE_MODE, OnSelChangeSaveMode)
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_SLASH_STYLE, OnClickSlashStyle)
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_AUTO_NAME, OnClickAutoName)
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_AUTO_FOLDER, OnClickAutoFolder)
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_AUTO_CLEAR, OnClickAutoClear)
END_MESSAGE_MAP()


// CPropPagePlaylist message handlers

BOOL CPropPagePlaylist::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    InitPropPage();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropPagePlaylist::__init__()
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


void CPropPagePlaylist::InitPropPage()
{
    InitLanguage();
    UpdateFromRegistry();
    InitControls();

    UpdateData(TO_DIALOG);
}


void CPropPagePlaylist::InitLanguage()
{
    // adding new playlist entries
    SetDlgItemText(IDS_SETTINGS_PLAYLIST_GROUP_NEW_ENTRIES, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_GROUP_NEW_ENTRIES")));
    SetDlgItemText(IDS_SETTINGS_PLAYLIST_NEW_ENTRIES_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_NEW_ENTRIES_INFO")));
    SetDlgItemText(IDC_CHECK_SETTINGS_SORT_NEW_ITEMS, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_SORT_NEW_ITEMS")));
    
    // saving playlist entries
    SetDlgItemText(IDS_SETTINGS_PLAYLIST_GROUP_SAVE_MODE, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_GROUP_SAVE_MODE")));
    SetDlgItemText(IDS_SETTINGS_PLAYLIST_SAVE_MODE_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_SAVE_MODE_INFO")));
    SetDlgItemText(IDC_CHECK_SETTINGS_SLASH_STYLE, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_SAVE_MODE_SLASH_STYLE")));

    // misc
    SetDlgItemText(IDS_SETTINGS_PLAYLIST_GROUP_MISC, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_GROUP_MISC")));
    SetDlgItemText(IDC_CHECK_SETTINGS_AUTO_NAME, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_MISC_AUTO_NAME")));
    SetDlgItemText(IDC_CHECK_SETTINGS_AUTO_FOLDER, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_MISC_AUTO_FOLDER")));
    SetDlgItemText(IDC_CHECK_SETTINGS_AUTO_CLEAR, CAppSettings::LoadString(_T("IDS_SETTINGS_PLAYLIST_MISC_AUTO_CLEAR")));
}


void CPropPagePlaylist::InitControls()
{
    // adding new playlist entries
    CStringArray arrayNewEntries;
    CAppSettings::GetAddNewEntries(arrayNewEntries);
    for(int i = 0; i < arrayNewEntries.GetCount(); i++)
    {
        m_cbNewEntries.InsertString(i, arrayNewEntries.GetAt(i));
    }
    m_cbNewEntries.SetCurSel(m_nNewEntries);

    // saving playlist entries
    InitSaveModes();

    // init dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);
}


void CPropPagePlaylist::InitSaveModes()
{
    // remember current selection
    int curSel = m_cbSaveMode.GetCurSel();

    // clear current list
    m_cbSaveMode.ResetContent();

    // populate list of save modes
    CStringArray arraySaveModes;
    CAppSettings::GetSaveModes(arraySaveModes);
    for(int i = 0; i < arraySaveModes.GetCount(); i++)
    {
        CString strSaveMode = arraySaveModes.GetAt(i);
        if(m_bUseForwardSlashes) strSaveMode.Replace('\\', '/');
        m_cbSaveMode.InsertString(i, strSaveMode);
    }

    // restore or init selection
    m_cbSaveMode.SetCurSel((curSel != CB_ERR) ? curSel : m_nSaveMode);
}


CString CPropPagePlaylist::GetTitle()
{
    if(m_strTitle.GetLength() == 0)
    {
        m_strTitle = CAppSettings::LoadString(_T("IDS_PROPPAGE_PLAYLIST_TITLE"));
    }
    return m_strTitle;
}


HBRUSH CPropPagePlaylist::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


HICON CPropPagePlaylist::GetIcon()
{
    return AfxGetApp()->LoadIcon(IDI_SETTINGS_PLAYLIST);
}


void CPropPagePlaylist::UpdateFromRegistry()
{
    // adding new playlist entries
    m_nNewEntries = CAppSettings::GetIntProperty(PROP_ADDNEWENTRIES, NEW_ENTRIES_BOTTOM);
    m_bSortNewItems = (BOOL)CAppSettings::GetIntProperty(PROP_SORTNEWITEMS, (int)FALSE);

    // saving playlist entries
    m_nSaveMode = CAppSettings::GetIntProperty(PROP_SAVEMODE, SAVE_MODE_RELATIVE);
    m_bUseForwardSlashes = (BOOL)CAppSettings::GetIntProperty(PROP_SAVEMODE_USEFSLASHES, (int)FALSE);

    // misc
    m_bAutoPlaylistName = (BOOL)CAppSettings::GetIntProperty(PROP_AUTOPLAYLISTNAME, (int)FALSE);
    m_bAutoPlaylistFolder = (BOOL)CAppSettings::GetIntProperty(PROP_AUTOPLAYLISTFOLDER, (int)FALSE);
    m_bAutoClear = (BOOL)CAppSettings::GetIntProperty(PROP_AUTOCLEAR, (int)FALSE);
}


void CPropPagePlaylist::UpdateToRegistry()
{
    // adding new playlist entries
    m_nNewEntries = m_cbNewEntries.GetCurSel();
    CAppSettings::SetIntProperty(PROP_ADDNEWENTRIES, m_nNewEntries);
    CAppSettings::SetIntProperty(PROP_SORTNEWITEMS, (int)m_bSortNewItems);

    // saving playlist entries
    m_nSaveMode = m_cbSaveMode.GetCurSel();
    CAppSettings::SetIntProperty(PROP_SAVEMODE, m_nSaveMode);
    CAppSettings::SetIntProperty(PROP_SAVEMODE_USEFSLASHES, (int)m_bUseForwardSlashes);

    // misc
    CAppSettings::SetIntProperty(PROP_AUTOPLAYLISTNAME, (int)m_bAutoPlaylistName);
    CAppSettings::SetIntProperty(PROP_AUTOPLAYLISTFOLDER, (int)m_bAutoPlaylistFolder);
    CAppSettings::SetIntProperty(PROP_AUTOCLEAR, (int)m_bAutoClear);
}


BOOL CPropPagePlaylist::OnApply() 
{
    UpdateData(FROM_DIALOG);
    UpdateToRegistry();

    return CPropertyPage::OnApply();
}


void CPropPagePlaylist::OnOK() 
{
    CPropertyPage::OnOK();
}


void CPropPagePlaylist::OnCancel() 
{
    CPropertyPage::OnCancel();
}


void CPropPagePlaylist::OnSelChangeNewEntries()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);
}


void CPropPagePlaylist::OnClickSortNewItems()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);
}


void CPropPagePlaylist::OnSelChangeSaveMode()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);
}


void CPropPagePlaylist::OnClickSlashStyle()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);

    InitSaveModes();
}


void CPropPagePlaylist::OnClickAutoName()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);
}


void CPropPagePlaylist::OnClickAutoFolder()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);
}


void CPropPagePlaylist::OnClickAutoClear()
{
    UpdateData(FROM_DIALOG);
    SetModified(TRUE);
}


