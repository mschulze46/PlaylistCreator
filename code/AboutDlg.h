/*----------------------------------------------------------------------------
| File:    AboutDlg.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CAboutDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 430 $
| $Id: AboutDlg.h 430 2009-10-13 16:55:56Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include <controls/HyperEdit.h>


// CAboutDlg dialog

class CAboutDlg : public CDialog
{
    ///// members /////
private:
    CFont       m_font;

    CxImage     m_imgBackground;

    CHyperEdit  m_editCredits;

    CPen        m_horzEdgePen;
    COLORREF    m_grayColor;
    CBrush      m_grayBrush;

    ///// methods /////
private:
    void __init__();
    void InitLanguage();
    void InitColors();
    void InitDialogBackground(BOOL bResizeDialog = TRUE);
    void InitPositions();

public:
    CAboutDlg();
    ~CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    // Implementation
protected:
    afx_msg void OnPaint();
    afx_msg void OnSysColorChange();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBtnVisitWebsite();

    DECLARE_MESSAGE_MAP()
};
