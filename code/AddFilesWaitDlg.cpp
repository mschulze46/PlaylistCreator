/*----------------------------------------------------------------------------
| File:    AddFilesWaitDlg.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CAddFilesWaitDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: AddFilesWaitDlg.cpp 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "AddFilesWaitDlg.h"
#include <shared/Color.h>
#include <system/DiskObject.h>
#include "PlaylistCreatorDlg.h"
#include "PlaylistFileManager.h"
#include <algorithm>


/////////////////////////////////////////////////////////////////////////////
// defines
//
#define TIME_INVISIBLE_ON_STARTUP        750    // msecs until dialog is shown


/////////////////////////////////////////////////////////////////////////////
// timers
//
#define TIMER_SHOW_WINDOW                  1
#define TIMER_UPDATE_PROGRESS              2


/////////////////////////////////////////////////////////////////////////////
// CAddFilesWaitDlg dialog
//
IMPLEMENT_DYNAMIC(CAddFilesWaitDlg, CDialog)
CAddFilesWaitDlg::CAddFilesWaitDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CAddFilesWaitDlg::IDD, pParent)
{
    __init__();
}


CAddFilesWaitDlg::~CAddFilesWaitDlg()
{
    if(m_bkgndBrush)
        ::DeleteObject(m_bkgndBrush);

    if(m_font.m_hObject)
        m_font.DeleteObject();
}


void CAddFilesWaitDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROCESS_WORKING_ANIMATION, m_ProcessWorkingAnimation);
}


BEGIN_MESSAGE_MAP(CAddFilesWaitDlg, CDialog)
    ON_BN_CLICKED(IDCANCEL, OnBtnCancel)
    ON_WM_TIMER()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_HELPINFO()
    ON_WM_PAINT()
    ON_WM_CTLCOLOR()
    ON_REGISTERED_MESSAGE(UWM_ADD_FILES_START, OnAddFilesStart)
    ON_REGISTERED_MESSAGE(UWM_ADD_FILES_PROGRESS_MARK, OnAddFilesProgressMark)
    ON_REGISTERED_MESSAGE(UWM_ADD_FILES_PROGRESS_INFO, OnAddFilesProgressInfo)
    ON_REGISTERED_MESSAGE(UWM_ADD_FILES_ERROR, OnAddFilesError)
END_MESSAGE_MAP()


// CAddFilesWaitDlg message handlers

BOOL CAddFilesWaitDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // dialog initialization
    InitLanguage();
    InitControls();
    CenterWindow();

    // trigger start of file adding
    PostMessage(UWM_ADD_FILES_START);

    // dialog is invisible until timer fires
    SetTimer(TIMER_SHOW_WINDOW, TIME_INVISIBLE_ON_STARTUP, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
}


void CAddFilesWaitDlg::__init__()
{
    m_bIsVisible         = false;

    m_bkgndColor         = RGB(255,255,255);
    m_bkgndBrush         = ::CreateSolidBrush(m_bkgndColor);
    m_HorzEdgeId         = IDC_HORZ_EDGE;

    m_pTaskbarListProvider = NULL;

    m_bCancel            = FALSE;
    m_result             = RESULT_ERROR;
    m_strErrorMsg        = _T("");

    m_dwLastProgressUpdateTime = 0;
    m_nLastProgressUpdateCount = 0;

    m_pParent            = NULL;
    m_pPlaylistListCtrl  = NULL;
    m_pArrayFileNames    = NULL;
    m_nCustomIndex       = -1;
    m_bRecursive         = false;

    m_nInsertedFiles     = 0;
    m_nLastInsertedIndex = -1;
}


void CAddFilesWaitDlg::InitLanguage()
{
    // caption
    SetWindowText(CAppSettings::GetWindowTitle());

    // statics
    SetDlgItemText(IDS_WAITDLG_INFO_HEADING, CAppSettings::LoadString(_T("IDS_WAITDLG_INFO_HEADING")));
    SetDlgItemText(IDS_WAITDLG_INFO_DETAILS, CAppSettings::LoadString(_T("IDS_WAITDLG_INFO_DETAILS")));
    SetDlgItemText(IDS_WAITDLG_INFO_PROGRESS, _T(""));

    // buttons
    SetDlgItemText(IDCANCEL, CAppSettings::LoadString(_T("IDS_CANCEL")));
}


void CAddFilesWaitDlg::InitControls()
{
    // init dialog font
    VERIFY(m_font.CreateFontIndirect(&CAppSettings::GetDialogFont()));

    // set dialog font
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    CDC *pDC = GetDC();
    {
        // subclass info heading text
        if(m_InfoHeading.SubclassDlgItem(IDS_WAITDLG_INFO_HEADING, this))
        {
            m_InfoHeading.SetFontScale(1.6);
            m_InfoHeading.SetFontColor(RGB(0,50,150));

            CRect old_coord;

            // if text too large for default rectangle, enlarge it & move controls below
            int yAdjust = m_InfoHeading.AdjustClientRect(pDC, &old_coord);
            
            if(yAdjust > 0)
            {
                // adjust dialog size
                CRect rw; GetWindowRect(&rw);
                SetWindowPos(NULL, 0, 0, rw.Width(), rw.Height() + yAdjust, SWP_NOZORDER | SWP_NOMOVE);
                
                // adjust controls
                MoveInfoType moveinfo = {old_coord.bottom, yAdjust};
                ::EnumChildWindows(*this, &EnumChildProc_AdjustClientRect, (LPARAM)&moveinfo);
            }
        }
    }
    ReleaseDC(pDC);

    // hide horizontal edge (we paint the edge later by ourselves)
    CStatic* pHorzEdge = (CStatic*)GetDlgItem(m_HorzEdgeId);
    if(pHorzEdge)
    {
        pHorzEdge->ShowWindow(SW_HIDE);
    }

    // process working animation
    m_imgProcessWorking.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_PROCESS_WORKING), "PNG"), CXIMAGE_FORMAT_PNG);
    if(m_imgProcessWorking.IsValid())
    {
        m_ProcessWorkingAnimation.SetAnimationImage(&m_imgProcessWorking, 32, 32);
        m_ProcessWorkingAnimation.SetExcludedTile(0); // first tile is empty!
        m_ProcessWorkingAnimation.DrawBorder(false);
        m_ProcessWorkingAnimation.UseParentBkgndColor();
        m_ProcessWorkingAnimation.SetSpeed(25);
    }
    else
    {
        ASSERT(0); // error: graphic couldn't be loaded
    }

    // init progress bar
    if(m_pTaskbarListProvider)
    {
        m_pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
    }

    // disable cancel button (will be enabled later)
    EnableControl(IDCANCEL, false);
}


void CAddFilesWaitDlg::SetFilesToAdd(CPlaylistCreatorDlg* pParent,
                                     CStringArray* pArrayFileNames,
                                     int customIndex /*-1*/,
                                     bool bRecursive /*false*/)
{
    m_pParent           = pParent;                                                ASSERT(m_pParent);
    m_pPlaylistListCtrl = (m_pParent) ? m_pParent->GetPlaylistListCtrl() : NULL;  ASSERT(m_pPlaylistListCtrl);
    m_pArrayFileNames   = pArrayFileNames;                                        ASSERT(m_pArrayFileNames);
    m_nCustomIndex      = customIndex;
    m_bRecursive        = bRecursive;
}


void CAddFilesWaitDlg::OnTimer(UINT nIDEvent)
{
    switch(nIDEvent)
    {
        case TIMER_SHOW_WINDOW:
            {
                // remove timer
                KillTimer(TIMER_SHOW_WINDOW);

                // show the window
                m_bIsVisible = true;
                ShowWindow(SW_SHOW);
            }
            break;

        case TIMER_UPDATE_PROGRESS:
            {
                // remove timer
                KillTimer(TIMER_UPDATE_PROGRESS);

                // update progress
                OnAddFilesProgressInfo(m_nLastProgressUpdateCount, 0);
            }
            break;

        default:
            break;
    }

    __super::OnTimer(nIDEvent);
}


void CAddFilesWaitDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    // little hack that hides a modal dialog on startup
    // (http://www.codeproject.com/dialog/dlgboxtricks.asp)
    if(!m_bIsVisible)
    {
        lpwndpos->flags &= ~SWP_SHOWWINDOW;
    }

    __super::OnWindowPosChanging(lpwndpos);
}


LRESULT CAddFilesWaitDlg::OnAddFilesStart(WPARAM wParam, LPARAM lParam)
{
    if(!m_pParent || !m_pPlaylistListCtrl || !m_pArrayFileNames)
    {
        ASSERT(0); // error: not fully initialized
        DoStepCloseDialog();
        return S_OK;
    }

    // start process working animation
    m_ProcessWorkingAnimation.Start();

    // set progress bar to indeterminate state
    if(m_pTaskbarListProvider)
    {
        m_pTaskbarListProvider->SetProgressState(TBPF_INDETERMINATE);
    }

    // reset thread control flags
    m_bCancel = FALSE;
    m_result  = RESULT_EXECUTE_FAILED;

    // start worker thread
    Start();

    return S_OK;
}


LRESULT CAddFilesWaitDlg::OnAddFilesProgressMark(WPARAM wParam, LPARAM lParam)
{
    PROGRESS_MARK progressMark = static_cast<PROGRESS_MARK>(wParam);

    switch(progressMark)
    {
        case PM_EXPAND_FOLDERS:
            {
                // disable cancel button
                EnableControl(IDCANCEL, false);

                // update progress text
                SetDlgItemText(IDS_WAITDLG_INFO_PROGRESS,
                               CAppSettings::LoadString(_T("IDS_WAITDLG_PI_EXPAND_FOLDERS")));
            }
            break;

        case PM_OPEN_PLAYLIST:
            {
                // disable cancel button
                EnableControl(IDCANCEL, false);

                // update progress text
                SetDlgItemText(IDS_WAITDLG_INFO_PROGRESS,
                               CAppSettings::LoadString(_T("IDS_WAITDLG_PI_OPEN_PLAYLIST")));
            }
            break;

        case PM_ADD_FILES:
            {
                // enable cancel button
                EnableControl(IDCANCEL, true);

                // update progress text
                CString strMessage = ReplaceTags(CAppSettings::LoadString(_T("IDS_WAITDLG_PI_FILES_ADDED")),
                                                _T("NUMBER"), _T("0"));
                SetDlgItemText(IDS_WAITDLG_INFO_PROGRESS, strMessage);
            }
            break;

        case PM_DONE:
            {
                // end thread
                if(IsRunning())
                {
                    Stop();
                }

                // close dialog
                EndDialog(0);
            }
            break;

        default:
            ASSERT(0);
            break;
    }

    return S_OK;
}


LRESULT CAddFilesWaitDlg::OnAddFilesProgressInfo(WPARAM wParam, LPARAM lParam)
{
    // get count of added files
    int nFilesAdded = static_cast<int>(wParam);

    // get current time
    DWORD dwCurrentTime = ::GetTickCount();

    // only update progress if some time went by
    if(m_dwLastProgressUpdateTime == 0                          // first update
        || (dwCurrentTime - m_dwLastProgressUpdateTime) > 100)  // 100ms since last update
    {
        // stop gui thread updater
        KillTimer(TIMER_UPDATE_PROGRESS);

        // update last update time
        m_dwLastProgressUpdateTime = dwCurrentTime;

        // update progress text
        CString strMessage = ReplaceTags(CAppSettings::LoadString(_T("IDS_WAITDLG_PI_FILES_ADDED")),
                                        _T("NUMBER"), IntegerToString(nFilesAdded));
        SetDlgItemText(IDS_WAITDLG_INFO_PROGRESS, strMessage);
    }
    else // don't update
    {
        // don't swallow this message completely, since it can
        // be the last update from the worker thread

        m_nLastProgressUpdateCount = nFilesAdded;
        SetTimer(TIMER_UPDATE_PROGRESS, 200, NULL);
    }

    return S_OK;
}


LRESULT CAddFilesWaitDlg::OnAddFilesError(WPARAM wParam, LPARAM lParam)
{
    ASSERT(!m_strErrorMsg.IsEmpty());

    // stop animation
    m_ProcessWorkingAnimation.Stop();

    // stop taskbar progress bar
    if(m_pTaskbarListProvider)
    {
        m_pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
    }

    // display error message
    XMSGBOXPARAMS xmb;
    CAppSettings::GetDefaultXMessageBoxParams(xmb);
    XMessageBox(m_hWnd, m_strErrorMsg, CAppSettings::GetWindowTitle(), MB_OK | MB_ICONERROR, &xmb);

    // reset error message
    m_strErrorMsg = _T("");

    // end thread
    if(IsRunning())
    {
        Stop();
    }

    // close dialog
    EndDialog(-1);

    return S_OK;
}


void CAddFilesWaitDlg::OnBtnCancel()
{
    // set cancel flag
    m_bCancel = TRUE;

    // is worker thread running?
    if(IsRunning())
    {
        // pause worker thread
        Pause();

        // pause animation
        m_ProcessWorkingAnimation.Stop();

        // pause taskbar progress bar
        if(m_pTaskbarListProvider)
        {
            m_pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
        }

        // really cancel?
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        int nResult = XMessageBox(m_hWnd,
                                  CAppSettings::LoadString(_T("IDS_WAITDLG_CANCEL")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDYES)
        {
            Shutdown();
            return;
        }

        // resume animation
        m_ProcessWorkingAnimation.Start();

        // resume taskbar progress bar
        if(m_pTaskbarListProvider)
        {
            m_pTaskbarListProvider->SetProgressState(TBPF_INDETERMINATE);
        }

        // resume worker thread
        Resume();

        // reset cancel flag
        m_bCancel = FALSE;
    }
}


BOOL CAddFilesWaitDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    CAppSettings::OnHelp();

    return TRUE;
    //return CDialog::OnHelpInfo(pHelpInfo);
}


void CAddFilesWaitDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // Do not call __super::OnPaint() for painting messages

    // draw dialog background
    CStatic* pHorzEdge = (CStatic*)GetDlgItem(m_HorzEdgeId);
    if(pHorzEdge)
    {
        // convert horizontal edge to dialog client coords
        CRect horzEdgeRect;
        pHorzEdge->GetWindowRect(&horzEdgeRect);
        ScreenToClient(&horzEdgeRect);

        // draw background
        CRect clientRect;
        GetClientRect(&clientRect);
        clientRect.bottom = horzEdgeRect.bottom;
        dc.FillSolidRect(clientRect, m_bkgndColor);

        // draw edge
        dc.FillSolidRect(CRect(clientRect.left, clientRect.bottom, clientRect.right, clientRect.bottom+1), CColor::gainsboro);
    }
}


HBRUSH CAddFilesWaitDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);

    if((nCtlColor != CTLCOLOR_DLG) && pWnd)
    {
      CStatic* pHorzEdge = (CStatic*)GetDlgItem(m_HorzEdgeId);
      if(pHorzEdge)
      {
        CRect windowRect, horzEdgeRect;
        pWnd->GetWindowRect(&windowRect);
        pHorzEdge->GetWindowRect(&horzEdgeRect);

        if(windowRect.top < horzEdgeRect.top) // control is above separator
        {
          // set text background mode so bkgndBrush color is visible
          pDC->SetBkMode(OPAQUE);
          pDC->SetBkColor(m_bkgndColor);
          hbr = m_bkgndBrush;
        }
      }
    }
    return hbr;
}


/////////////////////////////////////////////////////////////////////////////////
// worker thread functions
/////////////////////////////////////////////////////////////////////////////////

void CAddFilesWaitDlg::ThreadProc()
{
    RESULT result = RESULT_SUCCESS;

    result = DoStepAddFiles();
    if(result == RESULT_SUCCESS)
    {
        result = DoStepCloseDialog();
    }

    if(result == RESULT_SHUTDOWN_THREAD)
    {
        result = DoStepCloseDialog();
    }
    else if(result != RESULT_SUCCESS)
    {
        PostMessage(UWM_ADD_FILES_ERROR);
    }
}


void CAddFilesWaitDlg::UpdateProgress(int nFilesAdded)
{
    PostMessage(UWM_ADD_FILES_PROGRESS_INFO, (WPARAM)nFilesAdded);
}


void CAddFilesWaitDlg::EnableControl(int nID, bool bEnable /*true*/)
{
    CWnd* pWnd = GetDlgItem(nID);
    if(pWnd)
    {
        pWnd->EnableWindow(bEnable);
    }
}


RESULT CAddFilesWaitDlg::DoStepAddFiles()
{
    RESULT result = RESULT_SUCCESS;


    // buffers
    CPlaylistFileManager playlistFileManager;
    m_nInsertedFiles    = 0;                // reset number of inserted files
    int nInsertionIndex = m_nCustomIndex;   // custom insertion point
    int nSelectedIndex  = m_pPlaylistListCtrl->GetFirstSelectedIndex(); // first selected item

    BOOL bHighlightMissingItems = (BOOL)CAppSettings::GetIntProperty(PROP_HIGHLIGHTMISSINGITEMS, (int)TRUE);
    COLORREF crMissingItems = CAppSettings::GetIntProperty(PROP_MISSINGITEMSCOLOR, (int)DEFAULT_MISSING_ITEMS_COLOR);

    // sort new items (optional)
    BOOL bSortNewItems = (BOOL)CAppSettings::GetIntProperty(PROP_SORTNEWITEMS, (int)FALSE);
    if(bSortNewItems)
    {
        std::sort(m_pArrayFileNames->GetData(), m_pArrayFileNames->GetData() + m_pArrayFileNames->GetSize(), oddgravity::SortAscendingString);
    }

    // get insertion type
    int nInsertionType = CAppSettings::GetIntProperty(PROP_ADDNEWENTRIES, NEW_ENTRIES_BOTTOM);
    if(m_nCustomIndex != -1)
    {
        nInsertionType = NEW_ENTRIES_CUSTOM;
    }

    // correct insertion type if necessary
    if((nInsertionType == NEW_ENTRIES_BEFORESELECTION || nInsertionType == NEW_ENTRIES_AFTERSELECTION)
        && nSelectedIndex == -1)
    {
        nInsertionType = NEW_ENTRIES_BOTTOM; // default: insert at bottom
    }

    // set progress mark: expand folders
    PostMessage(UWM_ADD_FILES_PROGRESS_MARK, (WPARAM)PM_EXPAND_FOLDERS);

    // expand and remove folders from file array
    int nFiles = (int)m_pArrayFileNames->GetCount();
    for(int item = 0; item < nFiles; item++)
    {
        // get next item
        CString strItem = m_pArrayFileNames->GetAt(item);

        // check if it's a folder
        if(IsFolder(strItem))
        {
            // remove folder from array
            m_pArrayFileNames->RemoveAt(item);

            // buffers
            CStringArray arrayAdditionalFiles;
            CDiskObject diskObject;

            // expand the folder
            if(m_bRecursive)
            {
                diskObject.EnumAllFiles(strItem, arrayAdditionalFiles, EF_FULLY_QUALIFIED);
            }
            else // non-recursive
            {
                diskObject.EnumFilesInDirectory(strItem, arrayAdditionalFiles, EF_FULLY_QUALIFIED);
            }

            // add files
            if(!arrayAdditionalFiles.IsEmpty())
            {
                // insert expanded folder into array
                m_pArrayFileNames->InsertAt(item, &arrayAdditionalFiles);
            }

            // update file count and iter position
            nFiles = (int)m_pArrayFileNames->GetCount();
            item--; // cause we removed a folder from the list
        }
    }

    // if a single file is added, check if it's a playlist
    // note: currently only adding single playlists is supported
    BOOL bApplyFileTypesFilter = TRUE;
    if(m_pArrayFileNames->GetCount() == 1)
    {
        if(playlistFileManager.IsPlaylistFilePath(m_pArrayFileNames->GetAt(0))) // it's a playlist
        {
            // set progress mark: open playlist
            PostMessage(UWM_ADD_FILES_PROGRESS_MARK, (WPARAM)PM_OPEN_PLAYLIST);

            // import playlist
            CStringArray aImportedFiles;
            playlistFileManager.ImportPlaylist(m_pArrayFileNames->GetAt(0), aImportedFiles);

            // remove playlist from the passed list, cause we processed it
            m_pArrayFileNames->RemoveAt(0);

            // add imported playlist files to global file list
            if(!aImportedFiles.IsEmpty())
            {
                m_pArrayFileNames->InsertAt(0, &aImportedFiles);
            }

            // check if file types filter should be applied
            bApplyFileTypesFilter = (BOOL)CAppSettings::GetIntProperty(PROP_APPLYFILTERONOPENPLAYLISTS, (int)FALSE);
        }
    }

    // set progress mark: add files
    PostMessage(UWM_ADD_FILES_PROGRESS_MARK, (WPARAM)PM_ADD_FILES);

    // iterate over file list
    int nFileCount = (int)m_pArrayFileNames->GetCount();
    for(int item = 0; item < nFileCount; item++)
    {
        // get file path
        CString strItem = m_pArrayFileNames->GetAt(item);

        // check if file type is accepted
        if(playlistFileManager.IsPlaylistFilePath(strItem)                       // playlist file type
            || (!m_pParent->IsActiveFileType(GetFileExtensionComponent(strItem)) // not an active file type
                && bApplyFileTypesFilter))                                       // filter is on
        {
            #pragma message(Reminder "DoStepAddFiles(): Think about removing the item from the list (speeds up following steps)")
            continue;
        }

        // create new playlist element
        CPlaylistElement* pNewElement = new CPlaylistElement();
        pNewElement->SetFilePath(strItem);

        // calculate insertion index
        switch(nInsertionType)
        {
            case NEW_ENTRIES_TOP:
                nInsertionIndex = m_nInsertedFiles;
                break;

            default: ASSERT(0);
            case NEW_ENTRIES_BOTTOM:
                nInsertionIndex = m_pPlaylistListCtrl->GetItemCount();
                break;

            case NEW_ENTRIES_BEFORESELECTION:
                nInsertionIndex = nSelectedIndex + m_nInsertedFiles;
                break;

            case NEW_ENTRIES_AFTERSELECTION:
                nInsertionIndex = nSelectedIndex + 1 + m_nInsertedFiles;
                break;

            case NEW_ENTRIES_CUSTOM:
                if(m_nInsertedFiles > 0)
                {
                    nInsertionIndex++;
                }
                break;
        }

        // get new element info
        CString strFileName = pNewElement->GetFileName();
        CString strFileType = MakeLower(pNewElement->GetExtension());
        CString strFolder   = PathRemoveTrailingBackSlash(pNewElement->GetFolder());
        bool bExists        = IsFile(pNewElement->GetFilePath());

        // insert item
        LVITEM lvi;
        lvi.mask     = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem    = nInsertionIndex;
        lvi.iSubItem = 0;
        lvi.pszText  = (LPTSTR)(LPCTSTR)strFileName; // column: filename
        lvi.lParam   = (LPARAM)pNewElement;
        m_pPlaylistListCtrl->InsertItem(&lvi);

        // set subitems
        lvi.mask     = LVIF_TEXT;

        // column: type
        lvi.iSubItem = 1;
        lvi.pszText  = (LPTSTR)(LPCTSTR)strFileType;
        m_pPlaylistListCtrl->SetItem(&lvi);

        // column: folder
        lvi.iSubItem = 2;
        lvi.pszText  = (LPTSTR)(LPCTSTR)strFolder;
        m_pPlaylistListCtrl->SetItem(&lvi);

        // highlight item if it doesn't exist
        COLORREF bkColor = (bExists || !bHighlightMissingItems) ? COLOR_INVALID : crMissingItems;
        m_pPlaylistListCtrl->SetItemBkColor(nInsertionIndex, -1, bkColor, FALSE);

        // update members
        m_nInsertedFiles++;
        m_nLastInsertedIndex = nInsertionIndex;

        // update progress
        UpdateProgress(m_nInsertedFiles);

        // check if thread is paused or cancelled
        BOOL bPaused = TRUE;
        while(bPaused)
        {
            DWORD result = Wait();
            switch(result)
            {
                case ThreadTimeout:
                    continue;

                case ThreadShutdown:
                    return RESULT_SHUTDOWN_THREAD;

                case ThreadRunning:
                    bPaused = FALSE;
                    break;
            }
        }

        #ifdef _SLOW_FILE_IMPORT
        Sleep(25);
        #endif
    }

    return result;
}


RESULT CAddFilesWaitDlg::DoStepCloseDialog()
{
    RESULT result = RESULT_SUCCESS;

    PostMessage(UWM_ADD_FILES_PROGRESS_MARK, (WPARAM)PM_DONE);

    return result;
}

