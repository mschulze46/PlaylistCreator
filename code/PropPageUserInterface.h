/*----------------------------------------------------------------------------
| File:    PropPageUserInterface.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropPageUserInterface class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 392 $
| $Id: PropPageUserInterface.h 392 2009-06-12 16:38:16Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include "controls/GroupLine.h"
#include "controls/TreePropSheetEx/TreePropSheetUtil.hpp" // CWhiteBackgroundProvider
#include "controls/ColourPickerXP.h"

using namespace TreePropSheet;

///// forwards /////
class CPropSheetSettings;

// CPropPageUserInterface dialog

class CPropPageUserInterface : public CPropertyPage, public CWhiteBackgroundProvider
{
    ///// members /////
private:
    CPropSheetSettings* m_pParent;

    CFont       m_font;

    CString     m_strTitle;

    CGroupLine  m_groupProgram;
    CGroupLine  m_groupPlaylist;

    BOOL        m_bSingleInstance;
    BOOL        m_bTransparency;
    BOOL        m_bSnapToScreenBorder;
    BOOL        m_bPlaySound_Success;

    BOOL        m_bShowGridlines;
    BOOL        m_bShowAlternateRowColor;
    BOOL        m_bHighlightMissingItems;

    CColourPickerXP m_btnAlternateRowColor;
    COLORREF    m_clrAlternateRowColor;

    CColourPickerXP m_btnMissingItemsColor;
    COLORREF    m_clrMissingItems;

    ///// methods /////
private:
    void __init__();
    void InitPropPage();
    void InitLanguage();
    void InitControls();

    void UpdateFromRegistry();
    void UpdateToRegistry();


    DECLARE_DYNAMIC(CPropPageUserInterface)

public:
    CPropPageUserInterface();
    virtual ~CPropPageUserInterface();

    void SetParent(CPropSheetSettings* pParent) { m_pParent = pParent; }

    CString GetTitle();
    HICON GetIcon();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_USER_INTERFACE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnClickSingleInstance();
    afx_msg void OnClickTransparency();
    afx_msg void OnClickSnapToScreenBorder();
    afx_msg void OnClickPlaySound_Success();
    afx_msg void OnClickShowGridlines();
    afx_msg void OnClickShowAlternateRowColor();
    afx_msg void OnClickHighlightMissingItems();
    afx_msg LONG OnColorSelChange(UINT lParam, LONG wParam);
};
