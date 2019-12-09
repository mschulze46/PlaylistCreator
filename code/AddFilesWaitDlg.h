/*----------------------------------------------------------------------------
| File:    AddFilesWaitDlg.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CAddFilesWaitDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 468 $
| $Id: AddFilesWaitDlg.h 468 2009-11-12 16:48:36Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once
#include <controls/TileAnimationCtrl.h>
#include <controls/TaskDialogStatic.h>
#include <controls/FileDropListCtrl.h>
#include "Playlist.h"
#include "ITaskbarListProvider.h"


///// forwards /////
class CPlaylistCreatorDlg;


///// user window messages /////
#define UWM_ADD_FILES_START_MSG         _T("UWM_ADD_FILES_START-{514B0068-7185-4b4f-A29E-EA3C853A5D0E}")
                                        DECLARE_USER_MESSAGE(UWM_ADD_FILES_START)

#define UWM_ADD_FILES_PROGRESS_MARK_MSG _T("UWM_ADD_FILES_PROGRESS_MARK-{669DF50C-B5CD-4b42-98DB-4A76B7BC398A}")
                                        DECLARE_USER_MESSAGE(UWM_ADD_FILES_PROGRESS_MARK)

#define UWM_ADD_FILES_PROGRESS_INFO_MSG _T("UWM_ADD_FILES_PROGRESS_INFO-{D8C44621-A978-42a7-B016-AA93752D34C5}")
                                        DECLARE_USER_MESSAGE(UWM_ADD_FILES_PROGRESS_INFO)

#define UWM_ADD_FILES_ERROR_MSG         _T("UWM_ADD_FILES_ERROR-{7AB03EA9-FD98-4aa6-B8B1-D658892FE045}")
                                        DECLARE_USER_MESSAGE(UWM_ADD_FILES_ERROR)


// CAddFilesWaitDlg dialog

class CAddFilesWaitDlg : public CDialog, public CThread<CAddFilesWaitDlg>
{
    DECLARE_DYNAMIC(CAddFilesWaitDlg)

    ///// types /////
    typedef enum
    {
        PM_EXPAND_FOLDERS = 0,
        PM_OPEN_PLAYLIST,
        PM_ADD_FILES,
        PM_DONE,

        PM_COUNT        // must remain the last item!
    } PROGRESS_MARK;


    ///// attributes /////
private:
    bool                    m_bIsVisible;

    COLORREF                m_bkgndColor;
    HBRUSH                  m_bkgndBrush;
    int                     m_HorzEdgeId;

    CFont                   m_font;

    CTaskDialogStatic       m_InfoHeading;

    CTileAnimationCtrl      m_ProcessWorkingAnimation;
    CxImage                 m_imgProcessWorking;
    ITaskbarListProvider*   m_pTaskbarListProvider;

    BOOL                    m_bCancel;
    RESULT                  m_result;
    CString                 m_strErrorMsg;

    DWORD                   m_dwLastProgressUpdateTime;
    int                     m_nLastProgressUpdateCount;

    CPlaylistCreatorDlg*    m_pParent;
    CFileDropListCtrl*      m_pPlaylistListCtrl;
    CStringArray*           m_pArrayFileNames;
    int                     m_nCustomIndex;
    bool                    m_bRecursive;

    int                     m_nInsertedFiles;
    int                     m_nLastInsertedIndex;

    ///// methods /////
protected:
    void __init__();
    void InitLanguage();
    void InitControls();

    RESULT DoStepAddFiles();
    RESULT DoStepCloseDialog();

    void UpdateProgress(int nFilesAdded);

    void EnableControl(int nID, bool bEnable = true);


public:
    virtual void ThreadProc();

    void SetTaskbarListProvider(ITaskbarListProvider* pProvider) { m_pTaskbarListProvider = pProvider; }
    void SetFilesToAdd(CPlaylistCreatorDlg* pParent, CStringArray* pArrayFileNames, int customIndex = -1, bool bRecursive = false);

    int GetInsertedFilesCount()     { return m_nInsertedFiles; }
    int GetLastInsertedIndex()      { return m_nLastInsertedIndex; }


    ///// construction / destruction /////
public:
    CAddFilesWaitDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CAddFilesWaitDlg();

    // Dialog Data
    enum { IDD = IDD_ADD_FILES_WAIT_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    afx_msg void OnBtnCancel();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnPaint();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg LRESULT OnAddFilesStart(WPARAM, LPARAM);
    afx_msg LRESULT OnAddFilesProgressMark(WPARAM, LPARAM);
    afx_msg LRESULT OnAddFilesProgressInfo(WPARAM, LPARAM);
    afx_msg LRESULT OnAddFilesError(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};
