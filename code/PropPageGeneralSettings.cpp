/*----------------------------------------------------------------------------
| File:    PropPageGeneralSettings.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropPageGeneralSettings class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 468 $
| $Id: PropPageGeneralSettings.cpp 468 2009-11-12 16:48:36Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropPageGeneralSettings.h"
#include "PropSheetSettings.h"
#include "CheckForUpdatesDlg.h"


// CPropPageGeneralSettings dialog

IMPLEMENT_DYNAMIC(CPropPageGeneralSettings, CPropertyPage)
CPropPageGeneralSettings::CPropPageGeneralSettings()
    : CPropertyPage(CPropPageGeneralSettings::IDD)
    , m_bAutoUpdateCheck(FALSE)
{
    __init__();
}


CPropPageGeneralSettings::~CPropPageGeneralSettings()
{
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }
}


void CPropPageGeneralSettings::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDS_SETTINGS_GENERAL_GROUP_LANGUAGE, m_groupLanguage);
    DDX_Control(pDX, IDS_SETTINGS_GENERAL_GROUP_HIDDEN_MESSAGES, m_groupShowHiddenDialogs);
    DDX_Control(pDX, IDS_SETTINGS_GENERAL_GROUP_RESET_SETTINGS, m_groupResetSettings);
    DDX_Control(pDX, IDS_SETTINGS_GENERAL_GROUP_UPDATE, m_groupUpdate);
    DDX_Control(pDX, IDC_COMBO_SETTINGS_GENERAL_LANGUAGES, m_cbLanguages);
    DDX_Check(pDX, IDC_SETTINGS_GENERAL_AUTO_UPDATE_CHECK, m_bAutoUpdateCheck);
}


BEGIN_MESSAGE_MAP(CPropPageGeneralSettings, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_CBN_SELCHANGE(IDC_COMBO_SETTINGS_GENERAL_LANGUAGES, OnSelChangeLanguage)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_GENERAL_HIDDEN_MESSAGES, OnClickShowHiddenMessages)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_GENERAL_RESET_SETTINGS, OnClickResetSettings)
    ON_BN_CLICKED(IDC_SETTINGS_GENERAL_AUTO_UPDATE_CHECK, OnAutoUpdateCheck)
    ON_BN_CLICKED(IDC_BTN_SETTINGS_GENERAL_CHECK_NOW, OnClickCheckNow)
END_MESSAGE_MAP()


// CPropPageGeneralSettings message handlers

BOOL CPropPageGeneralSettings::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    InitPropPage();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropPageGeneralSettings::__init__()
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


void CPropPageGeneralSettings::InitPropPage()
{
    InitLanguage();
    UpdateFromRegistry();
    InitControls();

    UpdateData(TO_DIALOG);
}


void CPropPageGeneralSettings::InitLanguage()
{
    // language
    SetDlgItemText(IDS_SETTINGS_GENERAL_GROUP_LANGUAGE, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_GROUP_LANGUAGE")));
    SetDlgItemText(IDS_SETTINGS_GENERAL_LANGUAGE_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_LANGUAGE_INFO")));

    // show hidden dialogs
    SetDlgItemText(IDS_SETTINGS_GENERAL_GROUP_HIDDEN_MESSAGES, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_GROUP_HIDDEN_MESSAGES")));
    SetDlgItemText(IDS_SETTINGS_GENERAL_HIDDEN_MESSAGES_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_HIDDEN_MESSAGES_INFO")));
    SetDlgItemText(IDC_BTN_SETTINGS_GENERAL_HIDDEN_MESSAGES, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_BTN_HIDDEN_MESSAGES")));

    // reset setttings
    SetDlgItemText(IDS_SETTINGS_GENERAL_GROUP_RESET_SETTINGS, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_GROUP_RESET_SETTINGS")));
    SetDlgItemText(IDS_SETTINGS_GENERAL_RESET_SETTINGS_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_RESET_SETTINGS_INFO")));
    SetDlgItemText(IDC_BTN_SETTINGS_GENERAL_RESET_SETTINGS, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_BTN_RESET_SETTINGS")));

    // update
    SetDlgItemText(IDS_SETTINGS_GENERAL_GROUP_UPDATE, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_GROUP_UPDATE")));
    SetDlgItemText(IDC_SETTINGS_GENERAL_AUTO_UPDATE_CHECK, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_AUTO_UPDATE_CHECK")));
    SetDlgItemText(IDC_BTN_SETTINGS_GENERAL_CHECK_NOW, CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_BTN_CHECK_NOW")));
}


void CPropPageGeneralSettings::InitControls()
{
    // dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    // language
    CStringArray arrayLanguages;
    CAppSettings::GetSupportedLanguages(arrayLanguages);
    for(int i = 0; i < arrayLanguages.GetCount(); i++)
    {
        m_cbLanguages.InsertString(i, arrayLanguages.GetAt(i));
    }
    m_cbLanguages.SelectString(-1, arrayLanguages.GetAt(CAppSettings::GetCurrentLanguageInt()));
}


CString CPropPageGeneralSettings::GetTitle()
{
    if(m_strTitle.IsEmpty())
        m_strTitle = CAppSettings::LoadString(_T("IDS_PROPPAGE_GENERAL_SETTINGS_TITLE"));

    return m_strTitle;
}


HBRUSH CPropPageGeneralSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


HICON CPropPageGeneralSettings::GetIcon()
{
    return AfxGetApp()->LoadIcon(IDI_SETTINGS_GENERAL);
}


void CPropPageGeneralSettings::UpdateFromRegistry()
{
    // update
    m_bAutoUpdateCheck = CAppSettings::GetIntProperty(PROP_AUTOUPDATECHECK, (int)TRUE);
}


void CPropPageGeneralSettings::UpdateToRegistry()
{
    // language
    int nSelectedLanguage = m_cbLanguages.GetCurSel();
    if(nSelectedLanguage != CAppSettings::GetCurrentLanguageInt()) // not the currently set language
    {
        // set the new language (will be applied on the next restart)
        CAppSettings::SetLanguageInt(nSelectedLanguage);

        // changing the language makes a restart necessary
        if(m_pParent)
        {
            m_pParent->SetRestartNeeded(true);
        }
    }

    // update
    CAppSettings::SetIntProperty(PROP_AUTOUPDATECHECK, (int)m_bAutoUpdateCheck);
}


BOOL CPropPageGeneralSettings::OnApply() 
{
    UpdateData(FROM_DIALOG);
    UpdateToRegistry();

    return CPropertyPage::OnApply();
}


void CPropPageGeneralSettings::OnOK() 
{
    CPropertyPage::OnOK();
}


void CPropPageGeneralSettings::OnCancel() 
{
    CPropertyPage::OnCancel();
}


void CPropPageGeneralSettings::OnSelChangeLanguage()
{
    SetModified(TRUE);
}


void CPropPageGeneralSettings::OnClickShowHiddenMessages()
{
    XMSGBOXPARAMS xmb;
    CAppSettings::GetDefaultXMessageBoxParams(xmb);
    int nResult = XMessageBox(m_hWnd,
                              CAppSettings::LoadString(_T("IDS_SETTINGS_GENERAL_ASK_SHOW_HIDDEN_MESSAGES")),
                              CAppSettings::GetWindowTitle(),
                              MB_YESNO | MB_ICONEXCLAMATION,
                              &xmb);

    int rc = nResult & 0xFFFF;
    if(rc == IDNO)
        return;

    // show hidden messages again
    CAppSettings::ShowHiddenMessages();
}


void CPropPageGeneralSettings::OnClickResetSettings()
{
    XMSGBOXPARAMS xmb;
    CAppSettings::GetDefaultXMessageBoxParams(xmb);
    int nResult = XMessageBox(m_hWnd,
                              CAppSettings::LoadString(_T("IDS_MB_RESET_SETTINGS")),
                              CAppSettings::GetWindowTitle(),
                              MB_YESNO | MB_ICONEXCLAMATION,
                              &xmb);

    int rc = nResult & 0xFFFF;
    if(rc == IDNO)
        return;

    // reset registry entries and close settings dialog
    CAppSettings::ResetRegistryEntries();
    EndDialog(-1);
}


void CPropPageGeneralSettings::OnAutoUpdateCheck()
{
    SetModified(TRUE);
}


void CPropPageGeneralSettings::OnClickCheckNow()
{
    ITaskbarListProvider* pTaskbarListProvider = (m_pParent) ? m_pParent->GetTaskbarListProvider() : NULL;

    CCheckForUpdatesDlg dlg;
    dlg.SetTaskbarListProvider(pTaskbarListProvider);
    dlg.DoModal();

    // reset taskbar
    if(pTaskbarListProvider)
    {
        pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
    }
}

