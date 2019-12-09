/*----------------------------------------------------------------------------
| File:    PropPageFileTypes.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropPageFileTypes class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 461 $
| $Id: PropPageFileTypes.h 461 2009-10-21 15:24:45Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include <controls/GroupLine.h>
#include <controls/TreePropSheetEx/TreePropSheetUtil.hpp> // CWhiteBackgroundProvider
#include <controls/XPStyleButtonST.h>
#include <controls/ThemeHelperST.h>

using namespace TreePropSheet;

///// forwards /////
class CPropSheetSettings;

// CPropPageFileTypes dialog

class CPropPageFileTypes : public CPropertyPage, public CWhiteBackgroundProvider
{
    ///// members /////
private:
    CPropSheetSettings* m_pParent;

    CFont       m_font;

    CString     m_strTitle;

    CGroupLine  m_groupFileTypes;

    CXPStyleButtonST m_btnAddFileType;
    CXPStyleButtonST m_btnRemoveFileType;
    CThemeHelperST m_ThemeHelper;

    CStringArray m_listFileTypes;
    CListCtrl   m_listActiveFileTypes;
    CListCtrl   m_listInactiveFileTypes;

    CString     m_strNewFileType;

    BOOL        m_bApplyFilterOnOpenPlaylists;


    ///// methods /////
private:
    void __init__();
    void InitPropPage();
    void InitLanguage();
    void InitButtons();
    void InitFileTypesLists();
    void InitThemeDependentControls();

    void UpdateFromRegistry();
    void UpdateToRegistry();

    BOOL MoveToActiveFileTypes();
    BOOL MoveToInactiveFileTypes();
    BOOL DeleteInactiveFileType();
    BOOL IsNewFileType(CString strNewFileType);

    DECLARE_DYNAMIC(CPropPageFileTypes)

public:
    CPropPageFileTypes();
    virtual ~CPropPageFileTypes();

    void SetParent(CPropSheetSettings* pParent) { m_pParent = pParent; }

    CString GetTitle();
    HICON GetIcon();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_FILE_TYPES };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnSysColorChange();
    afx_msg void OnClickAddFileTypes();
    afx_msg void OnClickRemoveFileTypes();
    afx_msg void OnDoubleClickActiveFileTypes(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDoubleClickInactiveFileTypes(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClickResetFileTypes();
    afx_msg void OnClickNewFileType();
    afx_msg void OnClickDeleteFileTypes();
    afx_msg void OnClickApplyFilterOnOpenPlaylists();
};
