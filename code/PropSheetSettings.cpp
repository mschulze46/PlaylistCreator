/*----------------------------------------------------------------------------
| File:    PropSheetSettings.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPropSheetSettings class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 468 $
| $Id: PropSheetSettings.cpp 468 2009-11-12 16:48:36Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PropSheetSettings.h"


// CPropSheetSettings

IMPLEMENT_DYNAMIC(CPropSheetSettings, CTreePropSheetOffice2003)
CPropSheetSettings::CPropSheetSettings(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :inherited(nIDCaption, pParentWnd, iSelectPage)
{
    __init__();
}


CPropSheetSettings::CPropSheetSettings(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :inherited(pszCaption, pParentWnd, iSelectPage)
{
    __init__();
}


CPropSheetSettings::~CPropSheetSettings()
{
    if(m_font.m_hObject)
        m_font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CPropSheetSettings, inherited)
    ON_WM_HELPINFO()
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CPropSheetSettings message handlers

BOOL CPropSheetSettings::OnInitDialog()
{
    inherited::OnInitDialog();


    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPropSheetSettings::__init__()
{
    // init members
    m_bRestartNeeded = false;
    m_pTaskbarListProvider = NULL;

    // set view mode
    SetTreeViewMode(TRUE, TRUE, TRUE);
    SetTreeWidth(175);
    SetCaptionStyle(TRUE, FALSE);

    // set default image list (for items without icon)
    SetTreeDefaultImages(IDB_SETTINGS_EMPTY_IMAGE_LIST, 16, RGB(255,255,255));

    // create treectrl image list and attach it
    CImageList images;
    images.Create(24, 24, ILC_COLOR32 | ILC_MASK, 6, 1);
    images.Add(m_PropPageGeneralSettings.GetIcon());
    images.Add(m_PropPageUserInterface.GetIcon());
    images.Add(m_PropPagePlaylist.GetIcon());
    images.Add(m_PropPageExtendedInfo.GetIcon());
    images.Add(m_PropPageFileTypes.GetIcon());
    images.Add(m_PropPageDonations.GetIcon());
    SetTreeCtrlImages(images, 5);

    // init font
    SetTreeCtrlFont(CAppSettings::GetDialogFont());

    // add property pages
    AddPage(&m_PropPageGeneralSettings);
    AddPage(&m_PropPageUserInterface);
    AddPage(&m_PropPagePlaylist);
    AddPage(&m_PropPageExtendedInfo);
    AddPage(&m_PropPageFileTypes);
    AddPage(&m_PropPageDonations);

    // tell the property pages who the boss is
    m_PropPageGeneralSettings.SetParent(this);
    m_PropPageUserInterface.SetParent(this);
    m_PropPagePlaylist.SetParent(this);
    m_PropPageExtendedInfo.SetParent(this);
    m_PropPageFileTypes.SetParent(this);
    m_PropPageDonations.SetParent(this);

    // restore last active page
    SetActivePage(CAppSettings::GetIntProperty(PROP_SETTINGS_LASTACTIVEPAGE, 0));
}


BOOL CPropSheetSettings::OnHelpInfo(HELPINFO* pHelpInfo)
{
    CAppSettings::OnHelp();

    return TRUE;
    //return CDialog::OnHelpInfo(pHelpInfo);
}


void CPropSheetSettings::OnDestroy()
{
    // store active page
    CAppSettings::SetIntProperty(PROP_SETTINGS_LASTACTIVEPAGE, GetActiveIndex());

    inherited::OnDestroy();
}


