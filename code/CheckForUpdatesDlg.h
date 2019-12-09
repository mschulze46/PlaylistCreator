/*----------------------------------------------------------------------------
| File:    CheckForUpdatesDlg.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CCheckForUpdatesDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 468 $
| $Id: CheckForUpdatesDlg.h 468 2009-11-12 16:48:36Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once
#include <controls/TileAnimationCtrl.h>
#include <controls/TaskDialogStatic.h>
#include <controls/HyperLink.h>
#include "ITaskbarListProvider.h"

///// forwards /////


///// user window messages /////
#define UWM_UPDATE_CHECK_START_MSG          _T("UWM_UPDATE_CHECK_START-{181D88E7-4F6F-4014-851F-DAE9705A0164}")
                                            DECLARE_USER_MESSAGE(UWM_UPDATE_CHECK_START)

#define UWM_UPDATE_CHECK_PROGRESS_MARK_MSG  _T("UWM_UPDATE_CHECK_PROGRESS_MARK-{F3F50A54-0DC4-4189-9166-0EF7A8DB0CD6}")
                                            DECLARE_USER_MESSAGE(UWM_UPDATE_CHECK_PROGRESS_MARK)

#define UWM_UPDATE_CHECK_ERROR_MSG          _T("UWM_UPDATE_CHECK_ERROR-{3AC0CF5E-27D2-4b9c-B56A-56CF88295AFA}")
                                            DECLARE_USER_MESSAGE(UWM_UPDATE_CHECK_ERROR)


// CCheckForUpdatesDlg dialog

class CCheckForUpdatesDlg : public CDialog, public CThread<CCheckForUpdatesDlg>
{
    DECLARE_DYNAMIC(CCheckForUpdatesDlg)

    ///// types /////
    enum PROGRESS_MARK
    {
        PROGRESS_MARK_DONE = 0,
        PROGRESS_MARK_CANCELLED,

        PROGRESS_MARK_COUNT     // must remain the last item!
    };


    ///// attributes /////
private:
    COLORREF                m_bkgndColor;
    HBRUSH                  m_bkgndBrush;
    int                     m_HorzEdgeId;

    CFont                   m_font;

    CTaskDialogStatic       m_InfoHeading;

    CTileAnimationCtrl      m_ProcessWorkingAnimation;
    CxImage                 m_imgProcessWorking;
    CxImage                 m_imgResult;
    ITaskbarListProvider*   m_pTaskbarListProvider;

    BOOL                    m_bCancel;
    RESULT                  m_result;
    CString                 m_strErrorMsg;

    CString                 m_strUpdateInfoFile;

    CHyperLink              m_linkUpdateUrl;


    ///// methods /////
protected:
    void __init__();
    void InitLanguage();
    void InitControls();

    RESULT DoStepDownloadInfoFile();
    RESULT DoStepDone();
    RESULT DoStepCloseDialog();

    void EnableControl(int nID, bool bEnable = true);

    CString ReadFileToString(CString strFile);

    void ProcessResult();

public:
    virtual void ThreadProc();

    void SetTaskbarListProvider(ITaskbarListProvider* pProvider) { m_pTaskbarListProvider = pProvider; }


    ///// construction / destruction /////
public:
    CCheckForUpdatesDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CCheckForUpdatesDlg();

    // Dialog Data
    enum { IDD = IDD_CHECKFORUPDATES };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    afx_msg void OnBtnCancel();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnPaint();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg LRESULT OnUpdateCheckStart(WPARAM, LPARAM);
    afx_msg LRESULT OnUpdateCheckProgressMark(WPARAM, LPARAM);
    afx_msg LRESULT OnUpdateCheckError(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};
