/*----------------------------------------------------------------------------
| File:    PropPageGeneralSettings.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropPageGeneralSettings class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 364 $
| $Id: PropPageGeneralSettings.h 364 2008-07-07 19:34:40Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include "controls/GroupLine.h"
#include "controls/TreePropSheetEx/TreePropSheetUtil.hpp" // CWhiteBackgroundProvider
#include "afxwin.h"

using namespace TreePropSheet;

///// forwards /////
class CPropSheetSettings;

// CPropPageGeneralSettings dialog

class CPropPageGeneralSettings : public CPropertyPage, public CWhiteBackgroundProvider
{
    ///// members /////
private:
    CPropSheetSettings* m_pParent;

    CFont       m_font;

    CString     m_strTitle;

    CGroupLine  m_groupLanguage;
    CGroupLine  m_groupShowHiddenDialogs;
    CGroupLine  m_groupResetSettings;
    CGroupLine  m_groupUpdate;

    CComboBox   m_cbLanguages;

    BOOL        m_bAutoUpdateCheck;


    ///// methods /////
private:
    void __init__();
    void InitPropPage();
    void InitLanguage();
    void InitControls();

    void UpdateFromRegistry();
    void UpdateToRegistry();


    DECLARE_DYNAMIC(CPropPageGeneralSettings)

public:
    CPropPageGeneralSettings();
    virtual ~CPropPageGeneralSettings();

    void SetParent(CPropSheetSettings* pParent) { m_pParent = pParent; }

    CString GetTitle();
    HICON GetIcon();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_GENERAL_SETTINGS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnSelChangeLanguage();
    afx_msg void OnClickShowHiddenMessages();
    afx_msg void OnClickResetSettings();
    afx_msg void OnAutoUpdateCheck();
    afx_msg void OnClickCheckNow();
};
