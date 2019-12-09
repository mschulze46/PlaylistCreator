/*----------------------------------------------------------------------------
| File:    PropPageExtendedInfo.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropPageExtendedInfo class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 393 $
| $Id: PropPageExtendedInfo.cpp 393 2009-06-17 15:44:24Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropPageExtendedInfo.h"
#include "PropSheetSettings.h"


// CPropPageExtendedInfo dialog

IMPLEMENT_DYNAMIC(CPropPageExtendedInfo, CPropertyPage)
CPropPageExtendedInfo::CPropPageExtendedInfo()
    : CPropertyPage(CPropPageExtendedInfo::IDD)
    , m_bWriteExtInfo(FALSE)
    , m_bReadTags(FALSE)
    , m_strTitleMask(_T(""))
    , m_strTitleMaskPreview(_T(""))
{
    __init__();

    // create brush
    m_brReadOnlyEditCtrl.CreateSolidBrush(RGB(245,245,245));
}


CPropPageExtendedInfo::~CPropPageExtendedInfo()
{
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }

    if(m_brReadOnlyEditCtrl.m_hObject)
    {
        m_brReadOnlyEditCtrl.DeleteObject();
    }
}


void CPropPageExtendedInfo::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDS_SETTINGS_EXT_INFO_GROUP_PROCESSING, m_groupProcessing);
    DDX_Control(pDX, IDS_SETTINGS_EXT_INFO_GROUP_TITLE_FORMAT, m_groupTitleFormat);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_EXT_INFO_WRITE_EXT_INFO, m_bWriteExtInfo);
    DDX_Check(pDX, IDC_CHECK_SETTINGS_EXT_INFO_READ_TAGS, m_bReadTags);
    DDX_Text(pDX, IDC_EDIT_SETTINGS_EXT_INFO_TITLE_MASK, m_strTitleMask);
    DDX_Text(pDX, IDC_EDIT_SETTINGS_EXT_INFO_TITLE_MASK_PREVIEW, m_strTitleMaskPreview);
}


BEGIN_MESSAGE_MAP(CPropPageExtendedInfo, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_EXT_INFO_WRITE_EXT_INFO, OnClickWriteExtInfo)
    ON_BN_CLICKED(IDC_CHECK_SETTINGS_EXT_INFO_READ_TAGS, OnClickReadTags)
    ON_EN_CHANGE(IDC_EDIT_SETTINGS_EXT_INFO_TITLE_MASK, OnChangeTitleMask)
END_MESSAGE_MAP()


// CPropPageExtendedInfo message handlers

BOOL CPropPageExtendedInfo::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    InitPropPage();

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropPageExtendedInfo::__init__()
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


void CPropPageExtendedInfo::InitPropPage()
{
    InitLanguage();
    UpdateFromRegistry();
    UpdateDependentControls();
    ParseTitleMaskInput();

    // init dialog font
    m_font.CreateFontIndirect(&CAppSettings::GetDialogFont());
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    UpdateData(TO_DIALOG);
}


void CPropPageExtendedInfo::InitLanguage()
{
    // processing of extended information
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_GROUP_PROCESSING, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_GROUP_PROCESSING")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_PROCESSING_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_PROCESSING_INFO")));
    SetDlgItemText(IDC_CHECK_SETTINGS_EXT_INFO_WRITE_EXT_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_WRITE_EXT_INFO")));
    SetDlgItemText(IDC_CHECK_SETTINGS_EXT_INFO_READ_TAGS, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_READ_TAGS")));

    // title format
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_GROUP_TITLE_FORMAT, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_GROUP_TITLE_FORMAT")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_TITLE_MASK_INFO, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_TITLE_MASK_INFO")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_TITLE_MASK, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_TITLE_MASK")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_TITLE_MASK_PREVIEW, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_TITLE_MASK_PREVIEW")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_TITLE_VARIABLES, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_TITLE_VARIABLES")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_ARTIST, _T("%1 - ") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_ARTIST")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_TITLE,  _T("%2 - ") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_TITLE")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_ALBUM,  _T("%3 - ") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_ALBUM")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_YEAR,   _T("%4 - ") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_YEAR")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_GENRE,  _T("%5 - ") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_GENRE")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_TRACK,  _T("%6 - ") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_TRACK")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_SAMPLE_TITLE, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_SAMPLE_TITLE")));
    SetDlgItemText(IDS_SETTINGS_EXT_INFO_VARIABLE_SAMPLE, CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_SAMPLE")));
}


CString CPropPageExtendedInfo::GetTitle()
{
    if(m_strTitle.IsEmpty())
        m_strTitle = CAppSettings::LoadString(_T("IDS_PROPPAGE_EXT_INFO_TITLE"));

    return m_strTitle;
}


HBRUSH CPropPageExtendedInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);


    if (pWnd->GetDlgCtrlID() == IDC_EDIT_SETTINGS_EXT_INFO_TITLE_MASK_PREVIEW)
    {
        return m_brReadOnlyEditCtrl;
    }

    if(HasWhiteBackground())
    {
        pDC->SetBkMode(TRANSPARENT);
        return ::GetSysColorBrush(COLOR_WINDOW);
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}


HICON CPropPageExtendedInfo::GetIcon()
{
    return AfxGetApp()->LoadIcon(IDI_SETTINGS_EXT_INFO);
}


void CPropPageExtendedInfo::UpdateFromRegistry()
{
    // processing of extended information
    m_bWriteExtInfo = CAppSettings::GetIntProperty(PROP_WRITEEXTENDEDINFO, (int)TRUE);
    m_bReadTags = CAppSettings::GetIntProperty(PROP_READTAGS, (int)TRUE);

    // title mask
    m_strTitleMask = CAppSettings::GetStringProperty(PROP_TITLEMASK, CAppSettings::GetDefaultTitleMask());
}


void CPropPageExtendedInfo::UpdateToRegistry()
{
    // processing of extended information
    CAppSettings::SetIntProperty(PROP_WRITEEXTENDEDINFO, (int)m_bWriteExtInfo);
    CAppSettings::SetIntProperty(PROP_READTAGS, (int)m_bReadTags);

    // title mask
    CAppSettings::SetStringProperty(PROP_TITLEMASK, m_strTitleMask);
}


BOOL CPropPageExtendedInfo::OnApply() 
{
    UpdateData(FROM_DIALOG);
    UpdateToRegistry();

    return CPropertyPage::OnApply();
}


void CPropPageExtendedInfo::OnOK() 
{
    CPropertyPage::OnOK();
}


void CPropPageExtendedInfo::OnCancel() 
{
    CPropertyPage::OnCancel();
}


void CPropPageExtendedInfo::OnClickWriteExtInfo()
{
    UpdateData(FROM_DIALOG);
    UpdateDependentControls();

    SetModified(TRUE);
}


void CPropPageExtendedInfo::OnClickReadTags()
{
    SetModified(TRUE);
}


void CPropPageExtendedInfo::OnChangeTitleMask()
{
    SetModified(TRUE);

    UpdateData(FROM_DIALOG);
    ParseTitleMaskInput();
    UpdateData(TO_DIALOG);
}


void CPropPageExtendedInfo::UpdateDependentControls()
{
    // update state of checkbox 'read tags'
    GetDlgItem(IDC_CHECK_SETTINGS_EXT_INFO_READ_TAGS)->EnableWindow(m_bWriteExtInfo);
}


void CPropPageExtendedInfo::ParseTitleMaskInput()
{
    CString strBuffer = m_strTitleMask;
    strBuffer.Replace(_T("%1"), _T("<") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_ARTIST")) + _T(">"));
    strBuffer.Replace(_T("%2"), _T("<") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_TITLE"))  + _T(">"));
    strBuffer.Replace(_T("%3"), _T("<") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_ALBUM"))  + _T(">"));
    strBuffer.Replace(_T("%4"), _T("<") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_YEAR"))   + _T(">"));
    strBuffer.Replace(_T("%5"), _T("<") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_GENRE"))  + _T(">"));
    strBuffer.Replace(_T("%6"), _T("<") + CAppSettings::LoadString(_T("IDS_SETTINGS_EXT_INFO_VARIABLE_TRACK"))  + _T(">"));
    m_strTitleMaskPreview = strBuffer;
}


