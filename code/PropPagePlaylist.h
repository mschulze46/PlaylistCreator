/*----------------------------------------------------------------------------
| File:    PropPagePlaylist.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropPagePlaylist class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 483 $
| $Id: PropPagePlaylist.h 483 2010-01-08 16:38:54Z Michael $
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

// CPropPagePlaylist dialog

class CPropPagePlaylist : public CPropertyPage, public CWhiteBackgroundProvider
{
    ///// members /////
private:
    CPropSheetSettings* m_pParent;

    CFont       m_font;

    CString     m_strTitle;

    CGroupLine  m_groupNewEntries;
    CGroupLine  m_groupSaveMode;
    CGroupLine  m_groupMiscellaneous;

    CComboBox   m_cbNewEntries;
    int         m_nNewEntries;
    BOOL        m_bSortNewItems;

    CComboBox   m_cbSaveMode;
    int         m_nSaveMode;
    BOOL        m_bUseForwardSlashes;

    BOOL        m_bAutoPlaylistName;
    BOOL        m_bAutoPlaylistFolder;
    BOOL        m_bAutoClear;


    ///// methods /////
private:
    void __init__();
    void InitPropPage();
    void InitLanguage();
    void InitControls();
    void InitSaveModes();

    void UpdateFromRegistry();
    void UpdateToRegistry();

    DECLARE_DYNAMIC(CPropPagePlaylist)

public:
    CPropPagePlaylist();
    virtual ~CPropPagePlaylist();

    void SetParent(CPropSheetSettings* pParent) { m_pParent = pParent; }

    CString GetTitle();
    HICON GetIcon();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_PLAYLIST };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnSelChangeNewEntries();
    afx_msg void OnClickSortNewItems();
    afx_msg void OnSelChangeSaveMode();
    afx_msg void OnClickSlashStyle();
    afx_msg void OnClickAutoName();
    afx_msg void OnClickAutoFolder();
    afx_msg void OnClickAutoClear();
};
