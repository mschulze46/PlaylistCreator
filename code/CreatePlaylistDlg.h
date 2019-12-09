/*----------------------------------------------------------------------------
| File:    CreatePlaylistDlg.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CCreatePlaylistDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 492 $
| $Id: CreatePlaylistDlg.h 492 2010-01-15 14:57:42Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once
#include <controls/KCSideBannerWnd.h>
#include <controls/HyperLinkWithIcons.h>
#include "Playlist.h"
#include "PlaylistFormats.h"
#include "ITaskbarListProvider.h"


///// user window messages /////

// Sent to parent window for requesting the playlist
// wParam: CPlaylist* pPlaylist
// lParam: NULL
#define UWM_FILL_PLAYLIST_MSG       _T("UWM_FILL_PLAYLIST-{12F0C9A3-27C3-4f42-BAD5-389DABE6F3A5}")
                                    DECLARE_USER_MESSAGE(UWM_FILL_PLAYLIST)

// gui thread callbacks
#define UWM_START_CREATION_MSG      _T("UWM_START_CREATION-{471BE0F5-B1D1-49c4-A540-1A71B4CD5CDF}")
                                    DECLARE_USER_MESSAGE(UWM_START_CREATION)

#define UWM_PROGRESS_MARK_MSG       _T("UWM_PROGRESS_MARK-{E95F2F3A-E3E7-4709-A55E-E0AF93164C4B}")
                                    DECLARE_USER_MESSAGE(UWM_PROGRESS_MARK)

#define UWM_PROGRESS_INFO_MSG       _T("UWM_PROGRESS_INFO-{4EE43259-34B0-40e9-894B-26A86600CD90}")
                                    DECLARE_USER_MESSAGE(UWM_PROGRESS_INFO)

#define UWM_ERROR_NOTIFY_MSG        _T("UWM_ERROR_NOTIFY-{09D83AEA-4442-4f06-8521-C5E46A2E4B0B}")
                                    DECLARE_USER_MESSAGE(UWM_ERROR_NOTIFY)


// CCreatePlaylistDlg dialog

class CCreatePlaylistDlg : public CDialog, public CThread<CCreatePlaylistDlg>
{
    ///// types /////
    enum
    {
        PROGRESS_MARK_PREPARE = 0,
        PROGRESS_MARK_CREATE,
        PROGRESS_MARK_FINISH,
        PROGRESS_MARK_DONE,
        PROGRESS_MARK_CANCELLED,

        PROGRESS_MARK_COUNT     // must remain the last item!
    };


    ///// members /////
protected:
    CFont            m_font;
    CFont            m_fontBold;

    CKCSideBannerWnd m_SideBannerWnd;

    CxImage          m_imgTitleBar;

    CProgressCtrl    m_ProgressCtrl;
    ITaskbarListProvider* m_pTaskbarListProvider;


    // processing steps
    CStatic          m_textStepPrepare;     // step: prepare
    CStatic          m_iconStepPrepare;
    CStatic          m_textStepCreate;      // step: create
    CStatic          m_iconStepCreate;
    CStatic          m_textStepFinish;      // step: finish
    CStatic          m_iconStepFinish;
    CStatic*         m_pIconCurrentStep;    // current step


    CPlaylist*       m_pPlaylist;
    IPlaylistFormat* m_pPlaylistFormat;

    BOOL             m_bWriteExtendedInfo;
    BOOL             m_bReadTags;
    SAVE_MODE        m_nSaveMode;
    BOOL             m_bUseForwardSlashes;
    CString          m_strTitleMask;

    CStdioFile       m_PlaylistFile;

    BOOL             m_bCancel;
    RESULT           m_result;

    CString          m_strErrorMsg;

    BOOL             m_bAutoCloseDialog;

    // donations
    CHyperLinkWithIcons m_DonationsLink;

    ///// methods /////
public:
    void __init__();
    void InitLanguage();
    void InitTitleBar();
    void InitDonationsLink();
    void SetTaskbarListProvider(ITaskbarListProvider* pProvider) { m_pTaskbarListProvider = pProvider; }

    virtual void ThreadProc();

protected:
    RESULT DoStepPrepare();
    RESULT DoStepCreate();
    RESULT DoStepFinish();
    RESULT DoStepDone();
    RESULT DoStepCancelled();

    void UpdateProgress(UINT nPos);
    void UpdateProgress(const CString& strInfo);
    void UpdateProgress(UINT nPos, const CString& strInfo);

    void AutoCloseDialog();


    DECLARE_DYNAMIC(CCreatePlaylistDlg)

// Construction
public:
    CCreatePlaylistDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CCreatePlaylistDlg();

// Dialog Data
    enum { IDD = IDD_CREATE_PLAYLIST_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg LRESULT DoCreatePlaylist(WPARAM, LPARAM);
    afx_msg LRESULT OnProgressMark(WPARAM, LPARAM);
    afx_msg LRESULT OnProgressInfo(WPARAM, LPARAM);
    afx_msg LRESULT OnErrorNotify(WPARAM, LPARAM);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnBtnOk();
    afx_msg void OnBtnCancel();
    afx_msg void OnCheckAutoCloseDialog();

    DECLARE_MESSAGE_MAP()
};
