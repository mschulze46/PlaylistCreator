/*----------------------------------------------------------------------------
| File:    PropPageFileTypes.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropPageFileTypes class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 464 $
| $Id: PropPageFileTypes.cpp 464 2009-10-22 20:07:16Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropPageFileTypes.h"
#include "PropSheetSettings.h"
#include "PlaylistFileManager.h"


// CPropPageFileTypes dialog

IMPLEMENT_DYNAMIC(CPropPageFileTypes, CPropertyPage)
CPropPageFileTypes::CPropPageFileTypes()
    : CPropertyPage(CPropPageFileTypes::IDD)
    , m_strNewFileType(_T(""))
    , m_bApplyFilterOnOpenPlaylists(FALSE)
{
    __init__();
}


CPropPageFileTypes::~CPropPageFileTypes()
{
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }
}


void CPropPageFileTypes::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDS_SETTINGS_FILE_TYPES_GROUP, m_groupFileTypes);
    DDX_Control(pDX, IDC_LIST_ACTIVE_FILE_TYPES, m_listActiveFileTypes);
    DDX_Control(pDX, IDC_LIST_INACTIVE_FILE_TYPES, m_listInactiveFileTypes);
    DDX_Control(pDX, IDC_BTN_SETTINGS_FILE_TYPES_ADD, m_btnAddFileType);
    DDX_Control(pDX, IDC_BTN_SETTINGS_FILE_TYPES_REMOVE, m_btnRemoveFileType);
    DDX_Text(pDX, IDC_EDIT_SETTINGS_FILE_TYPES_NEW, m_strNewFileType);
    DDX_Check(pDX, IDC_SETTINGS_FILE_TYPES_OPEN_PLAYLISTS, m_bApplyFilterOnOpenPlaylists);
}


BEGIN_MESSAGE_MAP(CPropPageFileTypes, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_WM_SYSCOLORCHANGE()
    ON_BN_CLICKED(IDC_BTN_SETTINGS_FILE_TYPES_ADD, OnClickAddFileTypes)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_FILE_TYPES_REMOVE, OnClickRemoveFileTypes)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_ACTIVE_FILE_TYPES, OnDoubleClickActiveFileTypes)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_INACTIVE_FILE_TYPES, OnDoubleClickInactiveFileTypes)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_FILE_TYPES_RESET, OnClickResetFileTypes)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_FILE_TYPES_NEW, OnClickNewFileType)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_FILE_TYPES_DELETE, OnClickDeleteFileTypes)
    ON_BN_CLICKED(IDC_SETTINGS_FILE_TYPES_OPEN_PLAYLISTS, OnClickApplyFilterOnOpenPlaylists)
END_MESSAGE_MAP()


// CPropPageFileTypes message handlers

BOOL CPropPageFileTypes::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    InitPropPage();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropPageFileTypes::__init__()
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


void CPropPageFileTypes::InitPropPage()
{
    InitLanguage();
    UpdateFromRegistry();
    InitButtons();
    InitFileTypesLists();
    InitThemeDependentControls();

    // init dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    UpdateData(TO_DIALOG);
}


void CPropPageFileTypes::InitLanguage()
{
    // file types lists
    SetDlgItemText(IDS_SETTINGS_FILE_TYPES_GROUP, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_GROUP")));
    SetDlgItemText(IDS_SETTINGS_FILE_TYPES_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_INFO")));

    m_btnAddFileType.SetWindowText(_T(""));
    m_btnAddFileType.SetTooltipText(CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_ADD")));

    m_btnRemoveFileType.SetWindowText(_T(""));
    m_btnRemoveFileType.SetTooltipText(CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_REMOVE")));

    // new file type
    SetDlgItemText(IDS_SETTINGS_FILE_TYPES_NEW_TITLE, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_NEW_TITLE")));
    SetDlgItemText(IDC_BTN_SETTINGS_FILE_TYPES_NEW, CAppSettings::LoadString(_T("IDS_NEW")));

    // delete file types
    SetDlgItemText(IDS_SETTINGS_FILE_TYPES_DELETE_TITLE, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_DELETE_TITLE")));
    SetDlgItemText(IDC_BTN_SETTINGS_FILE_TYPES_DELETE, CAppSettings::LoadString(_T("IDS_DELETE")));

    // reset file types
    SetDlgItemText(IDS_SETTINGS_FILE_TYPES_RESET_TITLE, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_RESET_TITLE")));
    SetDlgItemText(IDC_BTN_SETTINGS_FILE_TYPES_RESET, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_RESET")));

    // apply filter on open playlists
    SetDlgItemText(IDC_SETTINGS_FILE_TYPES_OPEN_PLAYLISTS, CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_OPEN_PLAYLISTS")));
}


void CPropPageFileTypes::InitButtons()
{
    int nIconSize   = 16;

    // add file type button
    CButtonIcons gfxBtnAdd(m_hWnd, IDI_GO_PREVIOUS, nIconSize, nIconSize);
    m_btnAddFileType.SetThemeHelper(&m_ThemeHelper);
    m_btnAddFileType.SetIcon(
        gfxBtnAdd.GetIconIn(),
        gfxBtnAdd.GetIconOut(),
        NULL,
        gfxBtnAdd.GetIconDisabled());

    // remove file type button
    CButtonIcons gfxBtnRemove(m_hWnd, IDI_GO_NEXT, nIconSize, nIconSize);
    m_btnRemoveFileType.SetThemeHelper(&m_ThemeHelper);
    m_btnRemoveFileType.SetIcon(
        gfxBtnRemove.GetIconIn(),
        gfxBtnRemove.GetIconOut(),
        NULL,
        gfxBtnRemove.GetIconDisabled());
}


void CPropPageFileTypes::InitFileTypesLists()
{
    // create columns (if necessary)
    CHeaderCtrl* pHeader = m_listActiveFileTypes.GetHeaderCtrl();
    if(pHeader && pHeader->GetItemCount() == 0) // columns do not exist yet
    {
        CRect rect;
        int vScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL) + 2;

        m_listActiveFileTypes.GetClientRect(&rect);
        m_listActiveFileTypes.InsertColumn(
            0,
            CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_ACTIVE")),
            LVCFMT_LEFT,
            rect.Width() - vScrollWidth);

        m_listInactiveFileTypes.GetClientRect(&rect);
        m_listInactiveFileTypes.InsertColumn(
            0,
            CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_INACTIVE")),
            LVCFMT_LEFT,
            rect.Width() - vScrollWidth);

        // enable fullrow selection
        m_listActiveFileTypes.SetExtendedStyle(m_listActiveFileTypes.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
        m_listInactiveFileTypes.SetExtendedStyle(m_listInactiveFileTypes.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    }

    // clear lists
    m_listActiveFileTypes.DeleteAllItems();
    m_listInactiveFileTypes.DeleteAllItems();

    // assemble registry key
    CString strRegistryKey = CAppSettings::GetRegistryKey(NAMESTYLE_FULL) + _T("\\FileTypes");

    // fill active and inactive file types boxes
    for(int i = 0; i < m_listFileTypes.GetCount(); i++)
    {
        CString strRegValue     = m_listFileTypes[i];
        CString strRegValueData = RegistryGetStringValue(HKEY_CURRENT_USER, strRegistryKey, strRegValue);

        if(strRegValueData == _T("1")) // active
        {
            m_listActiveFileTypes.InsertItem(-1, strRegValue);
            continue;
        }
        else if(strRegValueData == _T("0")) // inactive
        {
            m_listInactiveFileTypes.InsertItem(-1, strRegValue);
            continue;
        }
    }
}


void CPropPageFileTypes::InitThemeDependentControls()
{
    BOOL bIsAppThemed = m_ThemeHelper.IsAppThemed();

    // buttons
    m_btnAddFileType.SetFlat(bIsAppThemed);
    m_btnRemoveFileType.SetFlat(bIsAppThemed);
}


CString CPropPageFileTypes::GetTitle()
{
    if(m_strTitle.GetLength() == 0)
    {
        m_strTitle = CAppSettings::LoadString(_T("IDS_PROPPAGE_FILE_TYPES_TITLE"));
    }
    return m_strTitle;
}


HBRUSH CPropPageFileTypes::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);


    if(HasWhiteBackground())
    {
        pDC->SetBkMode(TRANSPARENT);
        return ::GetSysColorBrush(COLOR_WINDOW);
    }

    return hbr;
}


void CPropPageFileTypes::OnSysColorChange()
{
    CPropertyPage::OnSysColorChange();

    InitThemeDependentControls();
    Invalidate();
}


HICON CPropPageFileTypes::GetIcon()
{
    return AfxGetApp()->LoadIcon(IDI_SETTINGS_FILE_TYPES);
}


void CPropPageFileTypes::UpdateFromRegistry()
{
    // assemble registry key
    CString strRegistryKey = CAppSettings::GetRegistryKey(NAMESTYLE_FULL) + _T("\\FileTypes");

    // file types
    RegistryEnumerateValues(HKEY_CURRENT_USER, strRegistryKey, m_listFileTypes);

    // filter the playlist file types (e.g. pls, m3u)
    CPlaylistFileManager playlistManager;
    for(int i = 0; i < m_listFileTypes.GetCount(); i++)
    {
        if(playlistManager.IsPlaylistFileExtension(m_listFileTypes.GetAt(i)))
        {
            m_listFileTypes.RemoveAt(i);
            i--;
        }
    }

    // apply filter on open playlists
    m_bApplyFilterOnOpenPlaylists = CAppSettings::GetIntProperty(PROP_APPLYFILTERONOPENPLAYLISTS, (int)FALSE);
}


void CPropPageFileTypes::UpdateToRegistry()
{
    // get registry keys
    CString strRegistryRootKey      = CAppSettings::GetRegistryKey(NAMESTYLE_FULL);
    CString strRegistryFileTypesKey = strRegistryRootKey + _T("\\FileTypes");

    // delete FileTypes key
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryRootKey, _T("FileTypes"));

    // write active file types to registry
    for(int i = 0; i < m_listActiveFileTypes.GetItemCount(); i++)
    {
        CString strBuffer = m_listActiveFileTypes.GetItemText(i, 0);
        RegistryCreateStringValue(HKEY_CURRENT_USER, strRegistryFileTypesKey, strBuffer, _T("1"));
    }

    // write inactive file types to registry
    for(int i = 0; i < m_listInactiveFileTypes.GetItemCount(); i++)
    {
        CString strBuffer = m_listInactiveFileTypes.GetItemText(i, 0);
        RegistryCreateStringValue(HKEY_CURRENT_USER, strRegistryFileTypesKey, strBuffer, _T("0"));
    }

    // apply filter on open playlists
    CAppSettings::SetIntProperty(PROP_APPLYFILTERONOPENPLAYLISTS, (int)m_bApplyFilterOnOpenPlaylists);
}


BOOL CPropPageFileTypes::OnApply() 
{
    UpdateData(FROM_DIALOG);
    UpdateToRegistry();

    return CPropertyPage::OnApply();
}


void CPropPageFileTypes::OnOK() 
{
    CPropertyPage::OnOK();
}


void CPropPageFileTypes::OnCancel() 
{
    CPropertyPage::OnCancel();
}


void CPropPageFileTypes::OnClickAddFileTypes()
{
    if(MoveToActiveFileTypes())
    {
        SetModified(TRUE);
    }
    else
    {
        PlaySystemSound(SOUND_QUESTION);
    }
}


void CPropPageFileTypes::OnClickRemoveFileTypes()
{
    if(MoveToInactiveFileTypes())
    {
        SetModified(TRUE);

        if(m_listActiveFileTypes.GetItemCount() == 0)
        {
            XMSGBOXPARAMS xmb;
            CAppSettings::GetDefaultXMessageBoxParams(xmb);
            XMessageBox(m_hWnd,
                        CAppSettings::LoadString(_T("IDS_MB_LAST_ACTIVE_FILE_TYPE_REMOVED")),
                        CAppSettings::GetWindowTitle(),
                        MB_OK | MB_ICONINFORMATION,
                        &xmb);
        }
    }
    else
    {
        PlaySystemSound(SOUND_QUESTION);
    }
}


void CPropPageFileTypes::OnDoubleClickActiveFileTypes(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    if(MoveToInactiveFileTypes())
    {
        SetModified(TRUE);

        if(m_listActiveFileTypes.GetItemCount() == 0)
        {
            XMSGBOXPARAMS xmb;
            CAppSettings::GetDefaultXMessageBoxParams(xmb);
            XMessageBox(m_hWnd,
                        CAppSettings::LoadString(_T("IDS_MB_LAST_ACTIVE_FILE_TYPE_REMOVED")),
                        CAppSettings::GetWindowTitle(),
                        MB_OK | MB_ICONINFORMATION,
                        &xmb);
        }
    }

    *pResult = 0;
}


void CPropPageFileTypes::OnDoubleClickInactiveFileTypes(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    if(MoveToActiveFileTypes())
    {
        SetModified(TRUE);
    }

    *pResult = 0;
}


BOOL CPropPageFileTypes::MoveToActiveFileTypes()
{
    // do we have inactive file types?
    if(m_listInactiveFileTypes.GetItemCount() == 0)
        return FALSE;

    // get index of selected inactive file type
    POSITION pos = m_listInactiveFileTypes.GetFirstSelectedItemPosition();
    if(!pos)
        return FALSE;

    int nSel = m_listInactiveFileTypes.GetNextSelectedItem(pos);

    // get selected file type
    CString strNewActiveFileType = m_listInactiveFileTypes.GetItemText(nSel, 0);
    if(strNewActiveFileType.IsEmpty())
        return FALSE;

    // add selected file type to active types
    m_listActiveFileTypes.InsertItem(-1, strNewActiveFileType);

    // remove selected type from inactive file types
    m_listInactiveFileTypes.DeleteItem(nSel);

    // refresh selection
    ListCtrl_RefreshSelectionAfterItemRemoval(m_listInactiveFileTypes, nSel);

    return TRUE;
}


BOOL CPropPageFileTypes::MoveToInactiveFileTypes()
{
    // do we have active file types?
    if(m_listActiveFileTypes.GetItemCount() == 0)
        return FALSE;

    // get index of selected active file type
    POSITION pos = m_listActiveFileTypes.GetFirstSelectedItemPosition();
    if(!pos)
        return FALSE;

    int nSel = m_listActiveFileTypes.GetNextSelectedItem(pos);

    // get selected file type
    CString strNewInactiveFileType = m_listActiveFileTypes.GetItemText(nSel, 0);
    if(strNewInactiveFileType.IsEmpty())
        return FALSE;

    // add selected file type to inactive types
    m_listInactiveFileTypes.InsertItem(-1, strNewInactiveFileType);

    // remove selected type from active file types
    m_listActiveFileTypes.DeleteItem(nSel);

    // refresh selection
    ListCtrl_RefreshSelectionAfterItemRemoval(m_listActiveFileTypes, nSel);

    return TRUE;
}


void CPropPageFileTypes::OnClickResetFileTypes()
{
    XMSGBOXPARAMS xmb;
    CAppSettings::GetDefaultXMessageBoxParams(xmb);
    int nResult = XMessageBox(m_hWnd,
                              CAppSettings::LoadString(_T("IDS_SETTINGS_FILE_TYPES_ASK_RESET")),
                              CAppSettings::GetWindowTitle(),
                              MB_YESNO | MB_ICONEXCLAMATION,
                              &xmb);

    int rc = nResult & 0xFFFF;
    if(rc == IDNO)
        return;

    // reset file types
    CAppSettings::ResetFileTypes();

    // update lists
    UpdateFromRegistry();
    InitFileTypesLists();

    SetModified(TRUE);
}


void CPropPageFileTypes::OnClickNewFileType()
{
    UpdateData(FROM_DIALOG);

    // file type entered?
    if(m_strNewFileType.IsEmpty())
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_EMPTY_NEW_FILE_TYPE")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
        return;
    }

    // valid file extension?
    if(!IsValidFileExtension(m_strNewFileType))
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_INVALID_FILE_TYPE")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
        return;
    }

    // file type already exists?
    if(!IsNewFileType(m_strNewFileType))
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_FILE_TYPE_EXISTS")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
        return;
    }

    // playlist file type?
    CPlaylistFileManager manager;
    if(manager.IsPlaylistFileExtension(m_strNewFileType))
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_IS_PLAYLIST_FILE_TYPE")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
        return;
    }

    // add new file type and select it
    int nNewType = m_listInactiveFileTypes.InsertItem(-1, m_strNewFileType);
    if(nNewType != -1) // success
    {
        m_listInactiveFileTypes.SetItemState(nNewType, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
        m_listInactiveFileTypes.EnsureVisible(nNewType, FALSE);
    }

    // clear new file type input field
    m_strNewFileType = _T("");
    UpdateData(TO_DIALOG);

    SetModified(TRUE);
}


void CPropPageFileTypes::OnClickDeleteFileTypes()
{
    if(DeleteInactiveFileType())
    {
        SetModified(TRUE);
    }
    else
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_NO_FILE_TYPE_SELECTED")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
    }
}


BOOL CPropPageFileTypes::DeleteInactiveFileType()
{
    // do we have inactive file types?
    if(m_listInactiveFileTypes.GetItemCount() == 0)
        return FALSE;

    // get index of selected inactive file type
    POSITION pos = m_listInactiveFileTypes.GetFirstSelectedItemPosition();
    if(!pos)
        return FALSE;

    int nSel = m_listInactiveFileTypes.GetNextSelectedItem(pos);

    // remove selected type
    m_listInactiveFileTypes.DeleteItem(nSel);

    // refresh selection
    ListCtrl_RefreshSelectionAfterItemRemoval(m_listInactiveFileTypes, nSel);

    return TRUE;
}


BOOL CPropPageFileTypes::IsNewFileType(CString strNewFileType)
{
    // compare with active file types
    for(int i = 0; i < m_listActiveFileTypes.GetItemCount(); i++)
    {
        CString strBuffer = m_listActiveFileTypes.GetItemText(i, 0);
        if(strBuffer == strNewFileType)
            return FALSE;
    }

    // compare with inactive file types
    for(int i = 0; i < m_listInactiveFileTypes.GetItemCount(); i++)
    {
        CString strBuffer = m_listInactiveFileTypes.GetItemText(i, 0);
        if(strBuffer == strNewFileType)
            return FALSE;
    }

    return TRUE;
}


void CPropPageFileTypes::OnClickApplyFilterOnOpenPlaylists()
{
    SetModified(TRUE);
}

