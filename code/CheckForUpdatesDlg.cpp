/*----------------------------------------------------------------------------
| File:    CheckForUpdatesDlg.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CCheckForUpdatesDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: CheckForUpdatesDlg.cpp 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "CheckForUpdatesDlg.h"
#include <shared/Color.h>
#include <xml/XMLite.h>
#include <Wininet.h>


///// defines /////


///// timers /////


// CCheckForUpdatesDlg dialog

IMPLEMENT_DYNAMIC(CCheckForUpdatesDlg, CDialog)
CCheckForUpdatesDlg::CCheckForUpdatesDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CCheckForUpdatesDlg::IDD, pParent)
{
    __init__();
}


CCheckForUpdatesDlg::~CCheckForUpdatesDlg()
{
    if(m_bkgndBrush)
        ::DeleteObject(m_bkgndBrush);

    if(m_font.m_hObject)
        m_font.DeleteObject();
}


void CCheckForUpdatesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROCESS_WORKING_ANIMATION, m_ProcessWorkingAnimation);
    DDX_Control(pDX, IDS_UPDATEDLG_UPDATE_URL, m_linkUpdateUrl);
}


BEGIN_MESSAGE_MAP(CCheckForUpdatesDlg, CDialog)
    ON_BN_CLICKED(IDCANCEL, OnBtnCancel)
    ON_WM_HELPINFO()
    ON_WM_PAINT()
    ON_WM_CTLCOLOR()
    ON_REGISTERED_MESSAGE(UWM_UPDATE_CHECK_START, OnUpdateCheckStart)
    ON_REGISTERED_MESSAGE(UWM_UPDATE_CHECK_PROGRESS_MARK, OnUpdateCheckProgressMark)
    ON_REGISTERED_MESSAGE(UWM_UPDATE_CHECK_ERROR, OnUpdateCheckError)
END_MESSAGE_MAP()


// CCheckForUpdatesDlg message handlers

BOOL CCheckForUpdatesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // dialog initialization
    InitLanguage();
    InitControls();
    CenterWindow();

    // update timestamp of last update check
    CAppSettings::UpdateLastUpdateCheckTimestamp();

    // trigger start of update check
    PostMessage(UWM_UPDATE_CHECK_START);

    return TRUE;  // return TRUE unless you set the focus to a control
}


void CCheckForUpdatesDlg::__init__()
{
    m_bkgndColor    = RGB(255,255,255);
    m_bkgndBrush    = ::CreateSolidBrush(m_bkgndColor);
    m_HorzEdgeId    = IDC_HORZ_EDGE;

    m_pTaskbarListProvider = NULL;

    m_bCancel       = FALSE;
    m_result        = RESULT_ERROR;
    m_strErrorMsg   = _T("");

    m_strUpdateInfoFile = _T("");
}


void CCheckForUpdatesDlg::InitLanguage()
{
    // caption
    SetWindowText(CAppSettings::GetWindowTitle());

    // statics
    SetDlgItemText(IDS_UPDATEDLG_INFO_HEADING, CAppSettings::LoadString(_T("IDS_UPDATEDLG_INFO_HEADING")));
    SetDlgItemText(IDS_UPDATEDLG_INFO_DETAILS, ReplaceTags(CAppSettings::LoadString(_T("IDS_UPDATEDLG_INFO_DETAILS")), _T("APPLICATION"), CAppSettings::GetAppName()));
    SetDlgItemText(IDS_UPDATEDLG_YOUR_VERSION_INFO, CAppSettings::LoadString(_T("IDS_UPDATEDLG_YOUR_VERSION_INFO")));
    SetDlgItemText(IDS_UPDATEDLG_CURRENT_VERSION_INFO, CAppSettings::LoadString(_T("IDS_UPDATEDLG_CURRENT_VERSION_INFO")));
    SetDlgItemText(IDS_UPDATEDLG_YOUR_VERSION, CAppSettings::GetVersionString(PRODUCT_VERSION_BUILD));
    SetDlgItemText(IDS_UPDATEDLG_CURRENT_VERSION, _T(""));
    SetDlgItemText(IDS_UPDATEDLG_RESULT, _T(""));

    // buttons
    SetDlgItemText(IDOK, CAppSettings::LoadString(_T("IDS_OK")));
    SetDlgItemText(IDCANCEL, CAppSettings::LoadString(_T("IDS_CANCEL")));
}


void CCheckForUpdatesDlg::InitControls()
{
    // init dialog font
    VERIFY(m_font.CreateFontIndirect(&CAppSettings::GetDialogFont()));

    // set dialog font
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    CDC* pDC = GetDC();
    {
        // subclass info heading text
        if(m_InfoHeading.SubclassDlgItem(IDS_UPDATEDLG_INFO_HEADING, this))
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

    // hide update URL link (will be activated when update is available)
    m_linkUpdateUrl.ShowWindow(SW_HIDE);

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

    // update state of buttons
    EnableControl(IDOK, false);
    EnableControl(IDCANCEL, true);
}


LRESULT CCheckForUpdatesDlg::OnUpdateCheckStart(WPARAM wParam, LPARAM lParam)
{
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


LRESULT CCheckForUpdatesDlg::OnUpdateCheckProgressMark(WPARAM wParam, LPARAM lParam)
{
    PROGRESS_MARK progressMark = static_cast<PROGRESS_MARK>(wParam);

    switch(progressMark)
    {
        case PROGRESS_MARK_DONE:
            {
                ProcessResult();
            }
            break;

        case PROGRESS_MARK_CANCELLED:
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


LRESULT CCheckForUpdatesDlg::OnUpdateCheckError(WPARAM wParam, LPARAM lParam)
{
    ASSERT(!m_strErrorMsg.IsEmpty());

    // stop animation
    m_ProcessWorkingAnimation.Stop();

    // stop taskbar progress bar
    if(m_pTaskbarListProvider)
    {
        m_pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
    }

    // load result icon (painted in OnPaint)
    m_imgResult.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_DIALOG_ERROR), "PNG"), CXIMAGE_FORMAT_PNG);
    Invalidate();

    // display error message
    SetDlgItemText(IDS_UPDATEDLG_RESULT, m_strErrorMsg);

    // reset error message
    m_strErrorMsg = _T("");

    // end thread
    if(IsRunning())
    {
        Stop();
    }

    // update state of buttons
    EnableControl(IDOK, true);
    EnableControl(IDCANCEL, false);
    PostMessage(WM_NEXTDLGCTL, (WPARAM)GetDlgItem(IDOK)->m_hWnd, MAKELPARAM(TRUE, 0));

    return S_OK;
}


void CCheckForUpdatesDlg::OnBtnCancel()
{
    // set cancel flag
    m_bCancel = TRUE;

    // end thread
    if(IsRunning())
    {
        Stop();
    }

    // close dialog
    EndDialog(0);
}


BOOL CCheckForUpdatesDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    CAppSettings::OnHelp();

    return TRUE;
    //return CDialog::OnHelpInfo(pHelpInfo);
}


void CCheckForUpdatesDlg::OnPaint()
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

    // if a result icon is set paint it
    if(m_imgResult.IsValid())
    {
        // get position of animation
        CRect rectAnimation;
        m_ProcessWorkingAnimation.GetWindowRect(&rectAnimation);
        ScreenToClient(rectAnimation);

        // hide animation
        if(m_ProcessWorkingAnimation.IsWindowVisible())
            m_ProcessWorkingAnimation.ShowWindow(SW_HIDE);

        // display icon
        CDC* pDC = GetDC();
        m_imgResult.Draw(pDC->GetSafeHdc(), rectAnimation.left, rectAnimation.top);
        ReleaseDC(pDC);
    }
}


HBRUSH CCheckForUpdatesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CCheckForUpdatesDlg::ThreadProc()
{
    RESULT result = RESULT_SUCCESS;

    // give the user a moment to realize what he has done,
    // so he can cancel the action if necessary
    Sleep(1000);

    result = DoStepDownloadInfoFile();
    if(result == RESULT_SUCCESS)
    {
        result = DoStepDone();
    }

    if(result == RESULT_SHUTDOWN_THREAD)
    {
        result = DoStepCloseDialog();
    }
    else if(result != RESULT_SUCCESS)
    {
        PostMessage(UWM_UPDATE_CHECK_ERROR);
    }
}


void CCheckForUpdatesDlg::EnableControl(int nID, bool bEnable /*true*/)
{
    CWnd* pWnd = GetDlgItem(nID);
    if(pWnd)
    {
        pWnd->EnableWindow(bEnable);
    }
}


CString CCheckForUpdatesDlg::ReadFileToString(CString strFile)
{
    // verify that file exists
    if(strFile.IsEmpty() || !IsFile(strFile))
    {
        ASSERT(0);
        return _T("");
    }

    // open file
    CStdioFile file;
    if(!file.Open(strFile, CFile::modeRead | CFile::typeText))
    {
        ASSERT(0);
        return _T("");
    }

    // read file
    CString strFileContent  = _T("");
    CString strBuffer       = _T("");
    while(file.ReadString(strBuffer))
    {
        strFileContent += strBuffer;
        strFileContent += _T("\n");
    }
    strFileContent.Trim();

    // close file
    file.Close();

    return strFileContent;
}


void CCheckForUpdatesDlg::ProcessResult()
{
    VERIFY(IsFile(m_strUpdateInfoFile));

    // buffers
    int versionMajor    = -1;
    int versionMinor    = -1;
    int versionFix      = -1;
    int versionBuild    = -1;

    CString strVersion      = _T("");
    CString strUpdateUrl    = _T("");
    CString strReleaseNotes = _T("");

    // extract info from xml
    CString strUpdateInfoXml = ReadFileToString(m_strUpdateInfoFile);
    XNode xml;
    if(xml.Load(strUpdateInfoXml))
    {
        // get version
        LPXNode child = xml.GetChild(_T("ProductVersion"));
        if(child)
        {
            strVersion = child->GetText().c_str();
            sscanf_s(strVersion, _T("%d.%d.%d.%d"), &versionMajor, &versionMinor, &versionFix, &versionBuild);
        }

        // get update URL
        child = xml.GetChild(_T("UpdateUrl"));
        if(child)
        {
            strUpdateUrl = child->GetText().c_str();
        }

        // get release notes
        child = xml.GetChild(_T("ReleaseNotes"));
        if(child)
        {
            strReleaseNotes = child->GetText().c_str();
        }
    }

    if(strUpdateUrl.IsEmpty())
    {
        strUpdateUrl = CAppSettings::LoadString(_T("IDS_COMPANY_WEBSITE"));
    }

    // display current version
    SetDlgItemText(IDS_UPDATEDLG_CURRENT_VERSION, strVersion);

    // update available?
    bool bUpdateAvailable = false;
    if(versionMajor > CAppSettings::GetVersionInt(PRODUCT_VERSION_MAJOR))
    {
        bUpdateAvailable = true;
    }
    else if (versionMinor > CAppSettings::GetVersionInt(PRODUCT_VERSION_MINOR)
                && versionMajor == CAppSettings::GetVersionInt(PRODUCT_VERSION_MAJOR))
    {
        bUpdateAvailable = true;
    }
    else if (versionFix > CAppSettings::GetVersionInt(PRODUCT_VERSION_FIX)
                && versionMinor == CAppSettings::GetVersionInt(PRODUCT_VERSION_MINOR)
                && versionMajor == CAppSettings::GetVersionInt(PRODUCT_VERSION_MAJOR))
    {
        bUpdateAvailable = true;
    }
    else if (versionBuild > CAppSettings::GetVersionInt(PRODUCT_VERSION_BUILD)
                && versionFix   == CAppSettings::GetVersionInt(PRODUCT_VERSION_FIX)
                && versionMinor == CAppSettings::GetVersionInt(PRODUCT_VERSION_MINOR)
                && versionMajor == CAppSettings::GetVersionInt(PRODUCT_VERSION_MAJOR))
    {
        bUpdateAvailable = true;
    }

    // process result
    if(bUpdateAvailable)
    {
        SetDlgItemText(IDS_UPDATEDLG_RESULT,
                       ReplaceTags(CAppSettings::LoadString(_T("IDS_UPDATECHECK_NEWERVERSIONAVAILABLE")),
                                   _T("APPLICATION"), CAppSettings::GetAppName()));

        // display update link
        m_linkUpdateUrl.SetWindowText(CAppSettings::LoadString(_T("IDS_UPDATECHECK_GOTO_WEBSITE_LINK")));
        m_linkUpdateUrl.SetURL(strUpdateUrl);
        m_linkUpdateUrl.SetUnderline(CHyperLink::ulAlways);
        m_linkUpdateUrl.ActivateToolTip(TRUE);
        m_linkUpdateUrl.ShowWindow(SW_SHOW);

        // load result icon and trigger redraw
        m_imgResult.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_SOFTWARE_UPDATE_AVAILABLE), "PNG"), CXIMAGE_FORMAT_PNG);
        Invalidate();
    }
    else // no update available
    {
        SetDlgItemText(IDS_UPDATEDLG_RESULT, CAppSettings::LoadString(_T("IDS_UPDATECHECK_YOURUPTODATE")));

        // load result icon and trigger redraw
        m_imgResult.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_OK), "PNG"), CXIMAGE_FORMAT_PNG);
        Invalidate();
    }

    // remove temp file
    VERIFY(DeleteFile(m_strUpdateInfoFile));
    m_strUpdateInfoFile = _T("");

    // update state of buttons
    EnableControl(IDOK, true);
    EnableControl(IDCANCEL, false);
    PostMessage(WM_NEXTDLGCTL, (WPARAM)GetDlgItem(IDOK)->m_hWnd, MAKELPARAM(TRUE, 0));

    // stop animation
    m_ProcessWorkingAnimation.Stop();

    // stop taskbar progress bar
    if(m_pTaskbarListProvider)
    {
        m_pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
    }
}


RESULT CCheckForUpdatesDlg::DoStepDownloadInfoFile()
{
    RESULT result = RESULT_SUCCESS;

    // TODO: (IMPROVEMENT) check if internet connection is available

    // create temp file path
    m_strUpdateInfoFile = MakeWindowsTempFilePath(GetSystemTempDirectory());

    // verify that target URL isn't in IE cache
    DeleteUrlCacheEntry(CAppSettings::GetUpdateCheckUrl());

    // download version info file
    HRESULT res = URLDownloadToFile(NULL, CAppSettings::GetUpdateCheckUrl(), m_strUpdateInfoFile, 0, NULL);
    if(res != S_OK || !IsFile(m_strUpdateInfoFile))
    {
        m_strErrorMsg = CAppSettings::LoadString(_T("IDS_UPDATECHECK_NETERROR"));
        result = RESULT_ERROR;
    }

    return result;
}


RESULT CCheckForUpdatesDlg::DoStepDone()
{
    RESULT result = RESULT_SUCCESS;

    #ifdef _SLOW_CHECK_FOR_UPDATES
    Sleep(2000);
    #endif

    PostMessage(UWM_UPDATE_CHECK_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_DONE);

    return result;
}


RESULT CCheckForUpdatesDlg::DoStepCloseDialog()
{
    RESULT result = RESULT_SUCCESS;

    PostMessage(UWM_UPDATE_CHECK_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_CANCELLED);

    return result;
}

