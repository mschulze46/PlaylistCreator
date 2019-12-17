/*----------------------------------------------------------------------------
| File:    CreatePlaylistDlg.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CCreatePlaylistDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 493 $
| $Id: CreatePlaylistDlg.cpp 493 2010-01-15 14:59:10Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "CreatePlaylistDlg.h"
#include <multimedia/MediaInfo/MediaInfoDLL.h>
#include <filesystem/Path.h>

using namespace MediaInfoDLL;
#define MEDIAINFO_VERSION                   _T("0.7.27.0")      // %version%

///// defines /////
#define TITLEBAR_GRADIENT_BEGIN             RGB(255,255,255)
#define TITLEBAR_GRADIENT_END               RGB(255,255,255)
#define PROGRESSBAR_MAX_VALUE               100


// CCreatePlaylistDlg dialog

IMPLEMENT_DYNAMIC(CCreatePlaylistDlg, CDialog)
CCreatePlaylistDlg::CCreatePlaylistDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CCreatePlaylistDlg::IDD, pParent)
{
    m_pTaskbarListProvider  = NULL;

    m_pIconCurrentStep      = NULL;

    m_pPlaylist             = NULL;
    m_pPlaylistFormat       = NULL;

    m_bWriteExtendedInfo    = FALSE;
    m_bReadTags             = FALSE;
    m_nSaveMode             = SAVE_MODE_RELATIVE;
    m_bUseForwardSlashes    = FALSE;
    m_strTitleMask          = _T("");

    m_bCancel               = FALSE;

    m_strErrorMsg           = _T("");

    m_bAutoCloseDialog      = FALSE;
}


CCreatePlaylistDlg::~CCreatePlaylistDlg()
{
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }

    if(m_fontBold.m_hObject)
    {
        m_fontBold.DeleteObject();
    }

    if(m_pPlaylistFormat)
    {
        delete m_pPlaylistFormat;
        m_pPlaylistFormat = NULL;
    }

    if(m_pPlaylist)
    {
        delete m_pPlaylist;
        m_pPlaylist = NULL;
    }
}


void CCreatePlaylistDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDS_CREATEDLG_DONATIONS_LINK, m_DonationsLink);
    DDX_Control(pDX, IDS_CREATEDLG_STEP_PREPARE, m_textStepPrepare);
    DDX_Control(pDX, IDI_CREATEDLG_STEP_PREPARE, m_iconStepPrepare);
    DDX_Control(pDX, IDS_CREATEDLG_STEP_CREATE, m_textStepCreate);
    DDX_Control(pDX, IDI_CREATEDLG_STEP_CREATE, m_iconStepCreate);
    DDX_Control(pDX, IDS_CREATEDLG_STEP_FINISH, m_textStepFinish);
    DDX_Control(pDX, IDI_CREATEDLG_STEP_FINISH, m_iconStepFinish);
    DDX_Control(pDX, IDC_CREATEDLG_PROGRESS, m_ProgressCtrl);
    DDX_Check(pDX, IDC_CREATEDLG_CHECK_AUTOCLOSE, m_bAutoCloseDialog);
}


BEGIN_MESSAGE_MAP(CCreatePlaylistDlg, CDialog)
    ON_WM_HELPINFO()
    ON_BN_CLICKED(IDOK, OnBtnOk)
    ON_BN_CLICKED(IDCANCEL, OnBtnCancel)
    ON_BN_CLICKED(IDC_CREATEDLG_CHECK_AUTOCLOSE, OnCheckAutoCloseDialog)
    ON_REGISTERED_MESSAGE(UWM_START_CREATION, DoCreatePlaylist)
    ON_REGISTERED_MESSAGE(UWM_PROGRESS_MARK, OnProgressMark)
    ON_REGISTERED_MESSAGE(UWM_PROGRESS_INFO, OnProgressInfo)
    ON_REGISTERED_MESSAGE(UWM_ERROR_NOTIFY, OnErrorNotify)
END_MESSAGE_MAP()


// CCreatePlaylistDlg message handlers
BOOL CCreatePlaylistDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    __init__();

    CenterWindow();
    ShowWindow(SW_SHOW);
    PostMessage(UWM_START_CREATION);

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CCreatePlaylistDlg::__init__()
{
    InitLanguage();
    InitTitleBar();
    InitDonationsLink();

    // init dialog font
    VERIFY(m_font.CreateFontIndirect(&CAppSettings::GetDialogFont()));
    SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);

    // init bold font
    VERIFY(m_fontBold.CreateFontIndirect(&CAppSettings::GetDialogBoldFont()));
    m_textStepPrepare.SetFont(&m_fontBold);

    // init progress bar
    if(m_pTaskbarListProvider)
    {
        m_pTaskbarListProvider->SetProgressState(TBPF_NOPROGRESS);
    }

    // init controls
    m_bAutoCloseDialog = (BOOL)CAppSettings::GetIntProperty(PROP_AUTOCLOSECREATEDLG, 0);
    GetDlgItem(IDOK)->EnableWindow(FALSE);

    UpdateData(TO_DIALOG);
}


void CCreatePlaylistDlg::InitLanguage()
{
    // caption
    SetWindowText(CAppSettings::GetWindowTitle());

    // content
    SetDlgItemText(IDS_CREATEDLG_CONTENT, CAppSettings::LoadString(_T("IDS_CREATEDLG_CONTENT")));
    SetDlgItemText(IDS_CREATEDLG_STATUS_TITLE, CAppSettings::LoadString(_T("IDS_CREATEDLG_STATUS_TITLE")));

    // steps
    SetDlgItemText(IDS_CREATEDLG_STEP_PREPARE, CAppSettings::LoadString(_T("IDS_CREATEDLG_STEP_PREPARE")));
    SetDlgItemText(IDS_CREATEDLG_STEP_CREATE, CAppSettings::LoadString(_T("IDS_CREATEDLG_STEP_CREATE")));
    SetDlgItemText(IDS_CREATEDLG_STEP_FINISH, CAppSettings::LoadString(_T("IDS_CREATEDLG_STEP_FINISH")));

    // progress text
    SetDlgItemText(IDS_CREATEDLG_PROGRESS_INFO, _T(""));

    // buttons
    SetDlgItemText(IDOK, CAppSettings::LoadString(_T("IDS_OK")));
    SetDlgItemText(IDCANCEL, CAppSettings::LoadString(_T("IDS_CANCEL")));
    SetDlgItemText(IDC_CREATEDLG_CHECK_AUTOCLOSE, CAppSettings::LoadString(_T("IDS_CREATEDLG_CHECK_AUTOCLOSE")));
}


void CCreatePlaylistDlg::InitTitleBar()
{
    // normal font
    CFont normalFont;
    normalFont.CreatePointFont(85, _T("Tahoma"));

    // bold font
    CFont boldFont;
    LOGFONT lf;
    normalFont.GetLogFont(&lf);
    lf.lfWeight = FW_BOLD;
    boldFont.CreateFontIndirect(&lf);

    // attach control
    m_SideBannerWnd.Attach(this, KCSB_ATTACH_TOP);

    // title
    CSize offsetTitle(15,8);
    m_SideBannerWnd.SetTitle(CAppSettings::LoadString(_T("IDS_CREATEDLG_TITLE")));
    m_SideBannerWnd.SetTitleFont(&boldFont);
    m_SideBannerWnd.SetEdgeOffset(offsetTitle);

    // subtitle
    CSize offsetSubtitle(25,5);
    m_SideBannerWnd.SetCaption(CAppSettings::LoadString(_T("IDS_CREATEDLG_DETAIL_TITLE")));
    m_SideBannerWnd.SetCaptionFont(&normalFont);
    m_SideBannerWnd.SetCaptionOffset(offsetSubtitle);

    // background and icon
    m_imgTitleBar.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_CREATEDLG_TITLEBAR), "PNG"), CXIMAGE_FORMAT_PNG);
    HBITMAP hBmp = m_imgTitleBar.MakeBitmap(m_SideBannerWnd.GetDC()->m_hDC);
    m_SideBannerWnd.SetFillFlag(KCSB_FILL_TEXTURE);
    m_SideBannerWnd.SetTexture(hBmp, true, KCSB_TEXTURE_RIGHT);
    m_SideBannerWnd.SetIcon(AfxGetApp()->LoadIcon(IDI_PLAYLIST_CREATOR), KCSB_ICON_RIGHT|KCSB_ICON_VCENTER, false);
    m_SideBannerWnd.SetColBkg(RGB(255,255,255)); // if the window is larger than the texture

    // clean up
    normalFont.DeleteObject();
    boldFont.DeleteObject();
}


void CCreatePlaylistDlg::InitDonationsLink()
{
    m_DonationsLink.SetLinkText(ReplaceTags(CAppSettings::LoadString(_T("IDS_CREATEDLG_DONATIONS_LINK")), _T("APPLICATION"), CAppSettings::GetAppName(NAMESTYLE_FULL)).GetBuffer());
    m_DonationsLink.SetLinkUrl(CAppSettings::GetPayPalDonationsUrl().GetBuffer());
    m_DonationsLink.ActivateTooltip(FALSE);
    m_DonationsLink.SetMultiline(TRUE);
    m_DonationsLink.UnderlineAlways(TRUE);

    int iconId;
    SIZE iconSize;
    if(CAppSettings::GetLanguageString().MakeUpper() == _T("DE")) // German
    {
        iconId = IDI_DONATIONS_DE;
        iconSize.cx = 85;
        iconSize.cy = 42;
    }
    else // English
    {
        iconId = IDI_DONATIONS_EN;
        iconSize.cx = 85;
        iconSize.cy = 42;
    }

    CButtonIcons icons(m_hWnd, iconId, iconSize.cx, iconSize.cy);
    m_DonationsLink.SetIcons(
        icons.GetIconOut(),
        icons.GetIconOut(),
        icons.GetIconOut(),
        0x5555);
}


LRESULT CCreatePlaylistDlg::DoCreatePlaylist(WPARAM wParam, LPARAM lParam)
{
    WaitAndCheckMessages(1000); // give the user a second to realize that
                                // he is maybe doing something wrong

    if(m_bCancel)
    {
        // really cancel?
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        int nResult = XMessageBox(m_hWnd,
                                  CAppSettings::LoadString(_T("IDS_MB_CANCEL_PLAYLIST_CREATION")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDYES)
        {
            DoStepCancelled();
            return S_OK;
        }
    }

    m_bCancel = FALSE;
    m_result  = RESULT_EXECUTE_FAILED;

    Start(); // start thread

    return S_OK;
}


LRESULT CCreatePlaylistDlg::OnProgressMark(WPARAM wParam, LPARAM lParam)
{
    int iStep = (int)wParam;
    switch(iStep)
    {
        case PROGRESS_MARK_PREPARE:
        {
            // step "prepare" -> start
            m_iconStepPrepare.SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_BULLET), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED));
            m_textStepPrepare.SetFont(&m_fontBold);

            // update current step
            m_pIconCurrentStep = &m_iconStepPrepare;
        }
        break;

        case PROGRESS_MARK_CREATE:
        {
            // step "prepare" -> done
            m_iconStepPrepare.SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DONE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED));
            m_textStepPrepare.SetFont(&m_font);

            // step "create" -> start
            m_iconStepCreate.SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_BULLET), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED));
            m_textStepCreate.SetFont(&m_fontBold);

            // update current step
            m_pIconCurrentStep = &m_iconStepCreate;
        }
        break;

        case PROGRESS_MARK_FINISH:
        {
            // step "create" -> done
            m_iconStepCreate.SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DONE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED));
            m_textStepCreate.SetFont(&m_font);

            // step "finish" -> start
            m_iconStepFinish.SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_BULLET), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED));
            m_textStepFinish.SetFont(&m_fontBold);

            // update current step
            m_pIconCurrentStep = &m_iconStepFinish;

            // sleep a moment, because otherwise bullet icon and bold font aren't visible at all
            Sleep(250);
        }
        break;

        case PROGRESS_MARK_DONE:
        {
            // step "finish" -> done
            m_iconStepFinish.SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DONE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED));
            m_textStepFinish.SetFont(&m_font);

            // play success sound
            if(CAppSettings::GetIntProperty(PROP_PLAYSOUND_SUCCESS, (int)TRUE))
            {
                PlaySoundFromResource(IDR_WAV_NOTIFY);
            }

            // update button states
            GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
            GetDlgItem(IDOK)->EnableWindow(TRUE);
            PostMessage(WM_NEXTDLGCTL, (WPARAM)GetDlgItem(IDOK)->m_hWnd, MAKELPARAM(TRUE, 0));

            m_result = RESULT_SUCCESS;
            AutoCloseDialog();
        }
        break;

        case PROGRESS_MARK_CANCELLED:
        {
            // set cancel icon (bold font stays)
            if(!m_pIconCurrentStep) // cancelled before first step
            {
                m_pIconCurrentStep = &m_iconStepPrepare;
            }
            m_pIconCurrentStep->SetIcon((HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_CANCEL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_SHARED));

            // display cancelled state in progress bars
            CProgressCtrl_SetState(m_ProgressCtrl, PBST_ERROR);
            if(m_pTaskbarListProvider)
            {
                m_pTaskbarListProvider->SetProgressState(TBPF_ERROR);
            }

            // update button states
            GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
            GetDlgItem(IDOK)->EnableWindow(TRUE);
            PostMessage(WM_NEXTDLGCTL, (WPARAM)GetDlgItem(IDOK)->m_hWnd, MAKELPARAM(TRUE, 0));
        }
        break;

        default:
            ASSERT(0);
            break;
    }
    return S_OK;
}


LRESULT CCreatePlaylistDlg::OnProgressInfo(WPARAM wParam, LPARAM lParam)
{
    // process new position
    if(wParam)
    {
        UINT nPos = (UINT)wParam;

        // update progress bar position
        if(m_pTaskbarListProvider)
        {
            m_pTaskbarListProvider->SetProgressValue(nPos, PROGRESSBAR_MAX_VALUE);
        }
        m_ProgressCtrl.SetPos(nPos);
    }

    // process new info text
    if(lParam)
    {
        CString* pStrInfo = (CString*)lParam;

        // update progress bar text
        if(pStrInfo)
        {
            SetDlgItemText(IDS_CREATEDLG_PROGRESS_INFO, *pStrInfo);
            delete pStrInfo;
        }
    }

    return S_OK;
}


LRESULT CCreatePlaylistDlg::OnErrorNotify(WPARAM wParam, LPARAM lParam)
{
    ASSERT(!m_strErrorMsg.IsEmpty());

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


void CCreatePlaylistDlg::OnBtnOk()
{
    if(m_bCancel)
    {
        EndDialog(-1);
        return;
    }
    EndDialog(0);
}


void CCreatePlaylistDlg::OnBtnCancel()
{
    // set cancel flag
    m_bCancel = TRUE;

    // is worker thread running?
    if(IsRunning())
    {
        // pause worker thread
        Pause();

        // display paused state in progress bars
        CProgressCtrl_SetState(m_ProgressCtrl, PBST_PAUSED);
        if(m_pTaskbarListProvider)
        {
            m_pTaskbarListProvider->SetProgressState(TBPF_PAUSED);
        }

        // really cancel?
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        int nResult = XMessageBox(m_hWnd,
                                  CAppSettings::LoadString(_T("IDS_MB_CANCEL_PLAYLIST_CREATION")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDYES)
        {
            Shutdown();
            return;
        }

        // reset state of progress bars
        CProgressCtrl_SetState(m_ProgressCtrl, PBST_NORMAL);
        if(m_pTaskbarListProvider)
        {
            m_pTaskbarListProvider->SetProgressState(TBPF_NORMAL);
        }

        // resume worker thread
        Resume();

        // reset cancel flag
        m_bCancel = FALSE;
    }
}


/////////////////////////////////////////////////////////////////////////////////
// worker thread functions
/////////////////////////////////////////////////////////////////////////////////

void CCreatePlaylistDlg::ThreadProc()
{
    RESULT result = RESULT_SUCCESS;

    result = DoStepPrepare();
    if(result == RESULT_SUCCESS)
    {
        result = DoStepCreate();
        if(result == RESULT_SUCCESS)
        {
            result = DoStepFinish();
            if(result == RESULT_SUCCESS)
            {
                result = DoStepDone();
            }
        }
    }

    if(result == RESULT_SHUTDOWN_THREAD)
    {
        result = DoStepCancelled();
    }
    else if(result != RESULT_SUCCESS)
    {
        PostMessage(UWM_ERROR_NOTIFY);
    }
}


void CCreatePlaylistDlg::UpdateProgress(UINT nPos)
{
    PostMessage(UWM_PROGRESS_INFO, (WPARAM)nPos, NULL);
}


void CCreatePlaylistDlg::UpdateProgress(const CString& strInfo)
{
    CString* pStrInfo = new CString(strInfo);
    PostMessage(UWM_PROGRESS_INFO, NULL, (LPARAM)pStrInfo);
}


void CCreatePlaylistDlg::UpdateProgress(UINT nPos, const CString& strInfo)
{
    CString* pStrInfo = new CString(strInfo);
    PostMessage(UWM_PROGRESS_INFO, (WPARAM)nPos, (LPARAM)pStrInfo);
}


RESULT CCreatePlaylistDlg::DoStepPrepare()
{
    RESULT result = RESULT_SUCCESS;

    UINT nProgressPosInitial = 0;
    UINT nStepRange = 5;
    
    PostMessage(UWM_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_PREPARE);
    UpdateProgress(nProgressPosInitial, CAppSettings::LoadString(_T("IDS_CREATEDLG_PROGRESS_INFO_PREPARE")));


    // verify that there's not already a playlist
    if(m_pPlaylist)
    {
        ASSERT(0);
        delete m_pPlaylist;
        m_pPlaylist = NULL;
    }

    // get playlist from parent
    CWnd* pParent = GetParent();                                            ASSERT(pParent);
    if(pParent)
    {
        m_pPlaylist = new CPlaylist();
        pParent->SendMessage(UWM_FILL_PLAYLIST, (WPARAM)m_pPlaylist, NULL);
    }

    if(!m_pPlaylist || m_pPlaylist->IsEmpty())
    {
        m_strErrorMsg = CAppSettings::LoadString(_T("IDS_MB_ERR_CREATE_PLAYLIST"));
        return RESULT_ERROR;
    }

    // get playlist type
    if(m_pPlaylist->GetType() == PT_PLS)
    {
        m_pPlaylistFormat = new CPlaylistFormatPls();
    }
    else if(m_pPlaylist->GetType() == PT_M3U)
    {
        m_pPlaylistFormat = new CPlaylistFormatM3u();
    }

    if(!m_pPlaylistFormat)
    {
        ASSERT(0);
        m_strErrorMsg = CAppSettings::LoadString(_T("IDS_MB_ERR_INVALID_PLAYLIST_FORMAT"));
        return RESULT_ERROR;
    }

    // write extended info? (or minimalistic playlist, e.g. plain file list)
    m_bWriteExtendedInfo = CAppSettings::GetIntProperty(PROP_WRITEEXTENDEDINFO, (int)TRUE);

    // read tags?
    m_bReadTags = CAppSettings::GetIntProperty(PROP_READTAGS, (int)TRUE);

    // get save mode
    m_nSaveMode = (SAVE_MODE)CAppSettings::GetIntProperty(PROP_SAVEMODE, SAVE_MODE_RELATIVE);

    // use forward slashes?
    m_bUseForwardSlashes = CAppSettings::GetIntProperty(PROP_SAVEMODE_USEFSLASHES, (int)FALSE);

    // get title mask
    m_strTitleMask = CAppSettings::GetStringProperty(PROP_TITLEMASK, CAppSettings::GetDefaultTitleMask());

    // create playlist folder if necessary
    CString strPlaylistFolder = PathAddTrailingBackSlash(m_pPlaylist->GetFolder());
    if(!IsFolder(strPlaylistFolder))
    {
        CreateFolder(strPlaylistFolder);
        if(!IsFolder(strPlaylistFolder))
        {
            m_strErrorMsg = CAppSettings::LoadString(_T("IDS_MB_ERR_CREATE_PLAYLIST_FOLDER"));
            return RESULT_ERROR;
        }
    }

    // open playlist file
    BOOL bOpen = m_PlaylistFile.Open(m_pPlaylist->GetFilePath(),
                                     CFile::modeCreate | CFile::modeWrite | CFile::typeText);
    if(!bOpen)
    {
        m_strErrorMsg = CAppSettings::LoadString(_T("IDS_MB_ERR_OPEN_PLAYLIST_FILE"));
        return RESULT_ERROR;
    }

    return result;
}


RESULT CCreatePlaylistDlg::DoStepCreate()
{
    RESULT result = RESULT_SUCCESS;

    UINT nProgressPosInitial = 5;
    UINT nStepRange = 90;
    double fStepSize = (double)nStepRange / (double)m_pPlaylist->GetCount();

    PostMessage(UWM_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_CREATE);
    CString strInitialProgressInfo = ReplaceTags(
        CAppSettings::LoadString(_T("IDS_CREATEDLG_PROGRESS_INFO_CREATE")),
        _T("NUMBER"), IntegerToString(0),
        _T("TOTAL"), IntegerToString(m_pPlaylist->GetCount()));
    UpdateProgress(nProgressPosInitial, strInitialProgressInfo);


    ////////////////////////////////////////////
    // create the playlist
    ////////////////////////////////////////////

    // configure MediaInfo object
    MediaInfo mediaInfo;
    mediaInfo.Option(_T("Internet"), _T("No")); // don't check for newer versions

    // verify that loaded MediaInfoDLL is compatible
    //   Method: MediaInfo::Options("Info_Version", "**VERSION**;**APP_NAME**;**APP_VERSION**")
    //
    //   **VERSION** is the version of MediaInfo you have tested. Must be like this : "A.B.C.D" (example : "0.7.0.0")
    //   **APP_NAME** is the unique name of your application. Examples : "MediaInfoGUI", "MediaInfoCmd"
    //   **APP_VERSION** is the version of your application. Example : "0.7.0.0", "1.2", "1.26beta1"
    //
    //   Returns:
    //     - no incompatibilities: string like "MediaInfoLib - vA.B.C.D xxx"
    //     - incompatibilities: empty string
    //
    // Note: It seems that the method doesn't work as expected. It couldn't be achieved that the method
    //       returned an empty string. Perhaps it will work in a future version?!
    ASSERT(
        !mediaInfo.Option(
            _T("Info_Version"),
            MediaInfoDLL::String(CString(MEDIAINFO_VERSION) + _T(";") + CAppSettings::GetAppName(NAMESTYLE_FULL) + _T(";") + CAppSettings::GetVersionString(PRODUCT_VERSION_BUILD))
            ).empty()
    );

    ///// buffers /////
    int nItems = m_pPlaylist->GetCount();
    bool bTagsFound = false;

    ///// header /////
    CString strHeader = (m_bWriteExtendedInfo) ? m_pPlaylistFormat->GetHeader() : m_pPlaylistFormat->GetHeaderPlain();
    m_PlaylistFile.WriteString(strHeader);

    ///// items /////
    for(int i = 0; i < nItems; i++)
    {
        // reset buffers
        bTagsFound = false;

        // get next element
        CPlaylistElement* pElement = m_pPlaylist->GetAt(i);
        if(!pElement)
        {
            ASSERT(0);
            continue;
        }

        CString strItem;
        CString strFileType = MakeLower(pElement->GetExtension());

        // tags
        CString strTagTrack     = IntegerToString(i+1);
        CString strTagFilePath  = pElement->GetFilePath();
        CString strTagFileName  = pElement->GetFileName();
        CString strTagTitle     = m_strTitleMask;
        CString strTagLength;

        // process save mode
        switch(m_nSaveMode)
        {
            case SAVE_MODE_ABSOLUTE:
            {
                // lean back and enjoy the show
            }
            break;

            case SAVE_MODE_ABSOLUTE_NODRIVE:
            {
                CString strFilePath = pElement->GetFilePath();
                CPath pathElement(strFilePath);
                strFilePath.Delete(0, pathElement.GetDrive().GetLength());
                strTagFilePath = strFilePath;
            }
            break;

            default: ASSERT(0); // wtf?!
            case SAVE_MODE_RELATIVE:
            {
                CPath pathElement(PathAddTrailingBackSlash(pElement->GetFolder()), TRUE);
                CPath pathPlaylist(PathAddTrailingBackSlash(m_pPlaylist->GetFolder()), TRUE);

                // build relative path only if item and playlist are on the same drive
                if(pathElement.GetDrive() == pathPlaylist.GetDrive())
                {
                    CString strRelativePath = pathElement.GetRelativePath(PathRemoveTrailingBackSlash(m_pPlaylist->GetFolder()));
                    strTagFilePath = PathAddTrailingBackSlash(strRelativePath) + pElement->GetFileNameDotExtension();
                }
            }
            break;
        }

        // use forward slashes?
        if(m_bUseForwardSlashes)
        {
            strTagFilePath.Replace('\\', '/');
        }

        // write extended info?
        if(m_bWriteExtendedInfo)
        {
            // get tag data from file?
            if(m_bReadTags)
            {
                // open the file
                VERIFY(mediaInfo.Open((LPCTSTR)pElement->GetFilePath()) == 1);

#pragma message(Reminder "MediaInfo: How can it be checked whether tags are available?")
                bTagsFound = true;

                // get duration
                mediaInfo.Option(_T("Inform"), _T("General;%Duration%")); // in milliseconds
                CString strDurationInMsec = mediaInfo.Inform().c_str();
                CString strDurationInSec  = IntegerToString(StringToInteger(strDurationInMsec) / 1000);

                // get artist
                mediaInfo.Option(_T("Inform"), _T("General;%Performer%"));
                CString strArtist = mediaInfo.Inform().c_str();

                // get title
                mediaInfo.Option(_T("Inform"), _T("General;%Title%"));
                CString strTitle = mediaInfo.Inform().c_str();

                // get album
                mediaInfo.Option(_T("Inform"), _T("General;%Album%"));
                CString strAlbum = mediaInfo.Inform().c_str();

                // get year
                mediaInfo.Option(_T("Inform"), _T("General;%Recorded_Date%"));
                CString strYear = mediaInfo.Inform().c_str();

                // get genre
                mediaInfo.Option(_T("Inform"), _T("General;%Genre%"));
                CString strGenre = mediaInfo.Inform().c_str();

                // get track number
                mediaInfo.Option(_T("Inform"), _T("General;%Track/Position%"));
                CString strTrack = mediaInfo.Inform().c_str();

                // close the file
                mediaInfo.Close();

                // process results
                strTagLength = strDurationInSec;
                strTagTitle.Replace(_T("%1"), strArtist);
                strTagTitle.Replace(_T("%2"), strTitle);
                strTagTitle.Replace(_T("%3"), strAlbum);
                strTagTitle.Replace(_T("%4"), strYear);
                strTagTitle.Replace(_T("%5"), strGenre);
                strTagTitle.Replace(_T("%6"), strTrack);

                // prepare the item
                strItem = m_pPlaylistFormat->GetItemWithTagData();
            }

            // tags available?
            if(!m_bReadTags || !bTagsFound)
            {
                strItem = m_pPlaylistFormat->GetItem();
            }
        }
        else // don't write extended info (i.e. minimalistic playlist -> plain file list)
        {
            strItem = m_pPlaylistFormat->GetItemPlain();
        }

        // replace all tags in the item
        strItem = ReplaceTags(strItem,
            _T("TRACK"), strTagTrack,
            _T("FILEPATH"), strTagFilePath,
            _T("FILENAME"), strTagFileName,
            _T("TITLE"), strTagTitle,
            _T("LENGTH"), strTagLength);

        // write to file
        m_PlaylistFile.WriteString(strItem);

        // advance progressbar
        double fNewPos = nProgressPosInitial + ((i+1) * fStepSize);
        CString strProcessedItems = ReplaceTags(
            CAppSettings::LoadString(_T("IDS_CREATEDLG_PROGRESS_INFO_CREATE")),
            _T("NUMBER"), IntegerToString(i+1),
            _T("TOTAL"), IntegerToString(nItems));
        UpdateProgress((UINT)fNewPos, strProcessedItems);

        #ifdef _SLOW_PLAYLIST_CREATION
        Sleep(50);
        #endif

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
    }

    ///// footer /////
    CString strFooter = (m_bWriteExtendedInfo) ? m_pPlaylistFormat->GetFooter() : m_pPlaylistFormat->GetFooterPlain();

    CString strTrackCount = IntegerToString(nItems);
    strFooter = ReplaceTags(strFooter, _T("TRACKCOUNT"), strTrackCount);

    m_PlaylistFile.WriteString(strFooter);


    return result;
}


RESULT CCreatePlaylistDlg::DoStepFinish()
{
    RESULT result = RESULT_SUCCESS;

    UINT nProgressPosInitial = 95;
    UINT nStepRange = 5;

    PostMessage(UWM_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_FINISH);
    UpdateProgress(nProgressPosInitial, CAppSettings::LoadString(_T("IDS_CREATEDLG_PROGRESS_INFO_FINISH")));


    // close playlist file stream
    m_PlaylistFile.Flush();
    m_PlaylistFile.Close();

    // update playlist count
    int nTotalPlaylists = CAppSettings::GetIntProperty(PROP_TOTALPLAYLISTS, 0);
    int nTotalSongs = CAppSettings::GetIntProperty(PROP_TOTALSONGS, 0);

    if(nTotalPlaylists < 0) nTotalPlaylists = 0;
    if(nTotalSongs < 0) nTotalSongs = 0;

    CAppSettings::SetIntProperty(PROP_TOTALPLAYLISTS, nTotalPlaylists + 1);
    CAppSettings::SetIntProperty(PROP_TOTALSONGS, nTotalSongs + m_pPlaylist->GetCount());

    return result;
}


RESULT CCreatePlaylistDlg::DoStepDone()
{
    RESULT result = RESULT_SUCCESS;

    UINT nProgressPosInitial = 100;

    PostMessage(UWM_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_DONE);
    UpdateProgress(nProgressPosInitial, CAppSettings::LoadString(_T("IDS_CREATEDLG_PROGRESS_INFO_COMPLETED")));


    return result;
}


RESULT CCreatePlaylistDlg::DoStepCancelled()
{
    RESULT result = RESULT_SUCCESS;

    // update progress bar text (progress bar position remains unchanged)
    UpdateProgress(CAppSettings::LoadString(_T("IDS_CREATEDLG_CREATION_CANCELLED")));

    // close the playlist file
    if(m_PlaylistFile.m_pStream)
    {
        m_PlaylistFile.Flush();
        m_PlaylistFile.Close();
    }

    PostMessage(UWM_PROGRESS_MARK, (WPARAM)PROGRESS_MARK_CANCELLED);

    return result;
}


BOOL CCreatePlaylistDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    CAppSettings::OnHelp();

    return TRUE;
    //return CDialog::OnHelpInfo(pHelpInfo);
}


// gets called when auto close checkbox is clicked
void CCreatePlaylistDlg::OnCheckAutoCloseDialog()
{
    UpdateData(FROM_DIALOG);

    CAppSettings::SetIntProperty(PROP_AUTOCLOSECREATEDLG, (int)m_bAutoCloseDialog);
}


// checks auto close state and closes the dialog if set
void CCreatePlaylistDlg::AutoCloseDialog()
{
    UpdateData(FROM_DIALOG);

    if(m_result != RESULT_SUCCESS) // no autoclose if creation wasn't sucessful
    {
        ASSERT(0);
        return;
    }

    if(m_bAutoCloseDialog)
    {
        WaitAndCheckMessages(500); // wait a moment before closing
        EndDialog(0);
    }
}
