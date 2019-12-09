/*----------------------------------------------------------------------------
| File:    PropPageExtendedInfo.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropPageExtendedInfo class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 393 $
| $Id: PropPageExtendedInfo.h 393 2009-06-17 15:44:24Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include "controls/GroupLine.h"
#include "controls/TreePropSheetEx/TreePropSheetUtil.hpp" // CWhiteBackgroundProvider

using namespace TreePropSheet;

///// forwards /////
class CPropSheetSettings;

// CPropPageExtendedInfo dialog

class CPropPageExtendedInfo : public CPropertyPage, public CWhiteBackgroundProvider
{
    ///// members /////
private:
    CPropSheetSettings* m_pParent;

    CFont       m_font;

    CString     m_strTitle;

    CGroupLine  m_groupProcessing;
    CGroupLine  m_groupTitleFormat;

    CBrush      m_brReadOnlyEditCtrl;

    BOOL        m_bWriteExtInfo;
    BOOL        m_bReadTags;
    CString     m_strTitleMask;
    CString     m_strTitleMaskPreview;


    ///// methods /////
private:
    void __init__();
    void InitPropPage();
    void InitLanguage();

    void UpdateFromRegistry();
    void UpdateToRegistry();

    void UpdateDependentControls();
    void ParseTitleMaskInput();


    DECLARE_DYNAMIC(CPropPageExtendedInfo)

public:
    CPropPageExtendedInfo();
    virtual ~CPropPageExtendedInfo();

    void SetParent(CPropSheetSettings* pParent) { m_pParent = pParent; }

    CString GetTitle();
    HICON GetIcon();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_EXT_INFO };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnClickWriteExtInfo();
    afx_msg void OnClickReadTags();
    afx_msg void OnChangeTitleMask();
};
