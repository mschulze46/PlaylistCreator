/*----------------------------------------------------------------------------
| File:    PropPageDonations.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropPageDonations class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 431 $
| $Id: PropPageDonations.h 431 2009-10-14 17:27:49Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include <controls/GroupLine.h>
#include <controls/TreePropSheetEx/TreePropSheetUtil.hpp> // CWhiteBackgroundProvider
#include <controls/XPStyleButtonST.h>
#include <controls/ThemeHelperST.h>
#include <vector>

using namespace std;
using namespace TreePropSheet;

///// forwards /////
class CPropSheetSettings;

// CPropPageDonations dialog

class CPropPageDonations : public CPropertyPage, public CWhiteBackgroundProvider
{
    ///// types /////
public:
    typedef vector<CxImage> CxImageList;
    typedef vector<CxImage>::iterator CxImageListIterator;

    ///// members /////
private:
    CPropSheetSettings* m_pParent;

    CFont       m_font;

    CString     m_strTitle;

    CGroupLine  m_groupDonationsWhy;
    CGroupLine  m_groupDonationsHow;
    CGroupLine  m_groupUsageFrequency;

    CXPStyleButtonST m_btnWebsite;
    CXPStyleButtonST m_btnPayPal;
    CThemeHelperST m_ThemeHelper;

    int         m_nTotalPlaylists;
    int         m_nTotalSongs;

    CxImage     m_imageDonations;
    CxImage     m_imagePlaylist;

    int         m_nRotatedImages;
    CxImage     m_imagePlaylistRot1;
    CxImage     m_imagePlaylistRot2;
    CxImage     m_imagePlaylistRot3;
    CxImage     m_imagePlaylistRot4;
    CxImage     m_imagePlaylistRot5;
    CxImageList m_listPlaylistRot;


    ///// methods /////
private:
    void __init__();
    void InitPropPage();
    void InitLanguage();
    void InitButtons();
    void InitLayout();
    void InitUsageFrequency();
    void InitThemeDependentControls();
    void LoadGraphics();

    void UpdateFromRegistry();
    void UpdateToRegistry();


    DECLARE_DYNAMIC(CPropPageDonations)

public:
    CPropPageDonations();
    virtual ~CPropPageDonations();

    void SetParent(CPropSheetSettings* pParent) { m_pParent = pParent; }

    CString GetTitle();
    HICON GetIcon();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_DONATIONS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnPaint();
    afx_msg void OnSysColorChange();
    afx_msg void OnBtnWebsite();
    afx_msg void OnBtnPaypal();
};
