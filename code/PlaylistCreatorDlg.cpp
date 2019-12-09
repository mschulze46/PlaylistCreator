/*----------------------------------------------------------------------------
| File:    PlaylistCreatorDlg.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPlaylistCreatorDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 485 $
| $Id: PlaylistCreatorDlg.cpp 485 2010-01-10 13:16:55Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PlaylistCreatorDlg.h"
#include <controls/BCMenu.h>
#include <dialogs/FECFileDialog.h>
#include <dialogs/FolderDlg.h>
#include <filesystem/Path.h>
#include <system/MultiMon/MultiMonitor.h>
#include <system/MultiMon/Monitors.h>
#include "AboutDlg.h"
#include "PropSheetSettings.h"
#include "CreatePlaylistDlg.h"
#include "PlaylistFormats.h"
#include "AddFilesWaitDlg.h"
#include "PlaylistFileManager.h"
#include "CheckForUpdatesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///// context menu /////
#define CMID_MOVE_UP                        1000
#define CMID_MOVE_DOWN                      1001
#define CMID_MOVE_TO_TOP                    1002
#define CMID_MOVE_TO_BOTTOM                 1003
#define CMID_DELETE_SELECTED                1004
#define CMID_DELETE_ALL                     1005
#define CMID_REFRESH_ITEM_STATE             1006
#define CMID_SET_PLAYLIST_SAVE_LOCATION     1007
#define CMID_SHUFFLE_PLAYLIST               1008

///// timers /////
#define TIMER_FLASH_EDIT_ON                 1
#define TIMER_FLASH_EDIT_OFF                2
#define TIMER_PLAYLIST_ITEM_STATE_CHANGED   3

///// status bar /////
#define SB_PANE_COUNT                       2       // count of status bar panes
static UINT BASED_CODE indicators[] =
{
    ID_SB_INDICATOR,
    ID_SB_INDICATOR
};

////// misc defines /////
#define WINDOW_SNAP_RANGE                   10      // size in pixels of window snapping range
#define FLASH_DURATION_MSEC                 200     // duration in ms when flashing a control
#define DEFAULT_SYSTEM_SOUND                SOUND_QUESTION  // sound that is played when something is wrong

// ITaskbarList3
const UINT CPlaylistCreatorDlg::m_uTaskbarBtnCreatedMsg = RegisterWindowMessage(_T("TaskbarButtonCreated"));


/////////////////////////////////////////////////////////////////////////////
// list sorting functionality
//

typedef struct
{
    CListCtrl*  pList;
    int         nColumn;
    BOOL        bAscending;

} ListCtrlCompareInfo;


int CALLBACK ListCtrlAlphabeticalCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    ListCtrlCompareInfo* plcci = reinterpret_cast<ListCtrlCompareInfo*>(lParamSort);
    if(plcci == NULL)
    {
        ASSERT(0); // error: compare info not available
        return 0;
    }

    int index1 = (int)lParam1;
    int index2 = (int)lParam2;

    CListCtrl* pList = plcci->pList;
    CString str1 = pList->GetItemText(index1, plcci->nColumn);
    CString str2 = pList->GetItemText(index2, plcci->nColumn);

    int iResult = 0;
    if(plcci->bAscending)
        iResult = str1.Compare(str2);
    else
        iResult = str2.Compare(str1);

    return iResult;
}


int CALLBACK ListCtrlNumericalCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    ListCtrlCompareInfo* plcci = reinterpret_cast<ListCtrlCompareInfo*>(lParamSort);
    if(plcci == NULL)
    {
        ASSERT(0); // error: compare info not available
        return 0;
    }

    int index1 = (int)lParam1;
    int index2 = (int)lParam2;

    CListCtrl* pList = plcci->pList;
    int number1 = atoi(pList->GetItemText(index1, plcci->nColumn));
    int number2 = atoi(pList->GetItemText(index2, plcci->nColumn));

    int iResult = 0;
    if(plcci->bAscending)
        iResult = (number1 > number2) ? TRUE : FALSE;
    else
        iResult = (number2 > number1) ? TRUE : FALSE;

    return iResult;
}


/////////////////////////////////////////////////////////////////////////////
// CPlaylistCreatorDlg dialog
//

CPlaylistCreatorDlg::CPlaylistCreatorDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CPlaylistCreatorDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDI_PLAYLIST_CREATOR);

    m_minDlgSizeX           = 533;      // minimum dialog width
    m_minDlgSizeY           = 603;      // minimum dialog height

    m_pStatusBar            = NULL;

    m_pReBar                = NULL;
    m_pToolBar              = NULL;
    m_pilToolBarCold        = NULL;
    m_pilToolBarHot         = NULL;

    m_strToolTipText        = _T("");

    m_nSortColumn           = -1;
    m_bSortAscending        = TRUE;

    m_pEditToFlash          = NULL;

    m_bTransparency         = FALSE;
    m_iAlpha                = 0;
    HMODULE hUser32         = GetModuleHandle(_T("USER32.DLL"));
    m_pSetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    m_pChangeWindowMessageFilterEx = (lpfnChangeWindowMessageFilterEx)GetProcAddress(hUser32, "ChangeWindowMessageFilterEx");

    m_strPlaylistFolder     = _T("");

    m_dlgSnap.SetSnapWidth(WINDOW_SNAP_RANGE);
    m_bSnapToScreenBorder   = FALSE;

    m_DefaultHighlightColor = ::GetSysColor(COLOR_HIGHLIGHT);

    m_bHighlightMissingItems = TRUE;
    m_crMissingItems        = DEFAULT_MISSING_ITEMS_COLOR;
}


CPlaylistCreatorDlg::~CPlaylistCreatorDlg()
{
    // dialog font
    if(m_font.m_hObject)
    {
        m_font.DeleteObject();
    }

    // toolbar
    if(m_pReBar)
    {
        delete m_pReBar;
        m_pReBar = NULL;
    }

    if(m_pToolBar)
    {
        delete m_pToolBar;
        m_pToolBar = NULL;
    }

    if(m_pilToolBarCold)
    {
        delete m_pilToolBarCold;
        m_pilToolBarCold = NULL;
    }

    if(m_pilToolBarHot)
    {
        delete m_pilToolBarHot;
        m_pilToolBarHot = NULL;
    }

    // status bar
    if(m_pStatusBar)
    {
        delete m_pStatusBar;
        m_pStatusBar = NULL;
    }

    // brushes
    if(m_brushFlash.m_hObject)
    {
        m_brushFlash.DeleteObject();
    }
}


void CPlaylistCreatorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    
    DDX_Control(pDX, IDC_LIST_PLAYLIST_CONTENT, m_listPlaylistContent);
    DDX_Control(pDX, IDC_BTN_ADD_FOLDER, m_btnAddFolder);
    DDX_Control(pDX, IDC_BTN_ADD_FILES, m_btnAddFiles);
    DDX_Control(pDX, IDC_BTN_ADD_PLAYLISTS, m_btnAddPlaylists);
    DDX_Control(pDX, IDC_BTN_MOVE_SEL_ITEMS_TOP, m_btnMoveSelectedItemsTop);
    DDX_Control(pDX, IDC_BTN_MOVE_SEL_ITEMS_UP, m_btnMoveSelectedItemsUp);
    DDX_Control(pDX, IDC_BTN_MOVE_SEL_ITEMS_DOWN, m_btnMoveSelectedItemsDown);
    DDX_Control(pDX, IDC_BTN_MOVE_SEL_ITEMS_BOTTOM, m_btnMoveSelectedItemsBottom);
    DDX_Control(pDX, IDC_BTN_DELETE_SEL_ITEMS, m_btnDeleteSelectedItems);
    DDX_Control(pDX, IDC_BTN_DELETE_ALL_ITEMS, m_btnDeleteAllItems);
    DDX_Control(pDX, IDC_BTN_SELECT_PLAYLIST_SAVE_LOCATION, m_btnSelectPlaylistSaveLocation);
    DDX_Control(pDX, IDC_EDIT_PLAYLIST_NAME, m_editPlaylistName);
    DDX_Control(pDX, IDC_COMBO_PLAYLIST_TYPES, m_cbPlaylistTypes);
    DDX_Control(pDX, IDC_BTN_CREATE_PLAYLIST, m_btnCreatePlaylist);
    DDX_Text(pDX, IDC_EDIT_PLAYLIST_FOLDER, m_strPlaylistFolder);
}


BEGIN_MESSAGE_MAP(CPlaylistCreatorDlg, CDialog)
    //}}AFX_MSG_MAP
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_WM_ACTIVATEAPP()
    ON_WM_NCDESTROY()
    ON_WM_HELPINFO()
    ON_COMMAND(ID_TB_BTN_NEW_PLAYLIST, OnToolBarNewPlaylist)
    ON_COMMAND(ID_TB_BTN_OPEN_PLAYLIST, OnToolBarOpenPlaylist)
    ON_COMMAND(ID_TB_BTN_SETTINGS, OnToolBarSettings)
    ON_COMMAND(ID_TB_BTN_HELP, OnToolBarHelp)
    ON_COMMAND(ID_TB_BTN_ABOUT, OnToolBarAbout)
    ON_BN_CLICKED(IDC_BTN_ADD_FOLDER, OnAddFolder)
    ON_BN_CLICKED(IDC_BTN_ADD_FILES, OnAddFiles)
    ON_BN_CLICKED(IDC_BTN_ADD_PLAYLISTS, OnAddPlaylists)
    ON_BN_CLICKED(IDC_BTN_SELECT_PLAYLIST_SAVE_LOCATION, OnSelectPlaylistSaveLocation)
    ON_BN_CLICKED(IDC_BTN_MOVE_SEL_ITEMS_TOP, OnMoveSelectedItemsToTop)
    ON_BN_CLICKED(IDC_BTN_MOVE_SEL_ITEMS_UP, OnMoveSelectedItemsUp)
    ON_BN_CLICKED(IDC_BTN_MOVE_SEL_ITEMS_DOWN, OnMoveSelectedItemsDown)
    ON_BN_CLICKED(IDC_BTN_MOVE_SEL_ITEMS_BOTTOM, OnMoveSelectedItemsToBottom)
    ON_BN_CLICKED(IDC_BTN_DELETE_SEL_ITEMS, OnDeleteSelectedItems)
    ON_BN_CLICKED(IDC_BTN_DELETE_ALL_ITEMS, OnDeleteAllItems)
    ON_BN_CLICKED(IDC_BTN_CREATE_PLAYLIST, OnCreatePlaylist)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PLAYLIST_CONTENT, OnPlaylistItemChanged)
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
    ON_CBN_SELCHANGE(IDC_COMBO_PLAYLIST_TYPES, OnSelChangedPlaylistType)
    ON_WM_SIZE()
    ON_WM_SIZING()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_ERASEBKGND()
    ON_WM_CONTEXTMENU()
    ON_REGISTERED_MESSAGE(UWM_SINGLE_INSTANCE, OnSingleInstance)
    ON_REGISTERED_MESSAGE(UWM_AUTO_UPDATE_CHECK, OnAutoUpdateCheck)
    ON_REGISTERED_MESSAGE(UWM_FILL_PLAYLIST, OnFillPlaylist)
    ON_REGISTERED_MESSAGE(UWM_SORT_COLUMN, OnSortPlaylistColumn)
    ON_REGISTERED_MESSAGE(UWM_LTTN_NEEDTEXT, OnPlaylistItemToolTipNeedText)
    ON_REGISTERED_MESSAGE(m_uTaskbarBtnCreatedMsg, OnTaskbarBtnCreated)
END_MESSAGE_MAP()


BEGIN_EASYSIZE_MAP(CPlaylistCreatorDlg)
    EASYSIZE(IDC_REBAR_EDGE,                        ES_BORDER,    ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDS_GB_PLAYLIST_CONTENT,               ES_BORDER,    ES_BORDER,   ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDS_PLAYLIST_CONTENT_INFO,             ES_BORDER,    ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_LIST_PLAYLIST_CONTENT,             ES_BORDER,    ES_BORDER,   ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDC_BTN_ADD_FOLDER,                    ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_ADD_FILES,                     ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_ADD_PLAYLISTS,                 ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_MOVE_SEL_ITEMS_TOP,            ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_MOVE_SEL_ITEMS_UP,             ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_MOVE_SEL_ITEMS_DOWN,           ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_MOVE_SEL_ITEMS_BOTTOM,         ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_DELETE_SEL_ITEMS,              ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDC_BTN_DELETE_ALL_ITEMS,              ES_KEEPSIZE,  ES_BORDER,   ES_BORDER,   ES_KEEPSIZE,  0)
    EASYSIZE(IDS_GB_PLAYLIST_CREATION,              ES_BORDER,    ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDS_PLAYLIST_SAVE_LOCATION,            ES_BORDER,    ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDC_EDIT_PLAYLIST_FOLDER,              ES_BORDER,    ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDC_BTN_SELECT_PLAYLIST_SAVE_LOCATION, ES_KEEPSIZE,  ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDS_PLAYLIST_NAME_AND_TYPE,            ES_BORDER,    ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDC_EDIT_PLAYLIST_NAME,                ES_BORDER,    ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDC_COMBO_PLAYLIST_TYPES,              ES_KEEPSIZE,  ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
    EASYSIZE(IDC_BTN_CREATE_PLAYLIST,               ES_KEEPSIZE,  ES_KEEPSIZE, ES_BORDER,   ES_BORDER,    0)
END_EASYSIZE_MAP


// CPlaylistCreatorDlg message handlers

BOOL CPlaylistCreatorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // do all the dialog initialization
    // note: don't get any localized strings before this call!
    __init__();


    // Add "About..." menu item to system menu

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu = ReplaceTags(
            CAppSettings::LoadString(_T("IDS_SYSMENU_ABOUT")),
            _T("APPLICATION"), CAppSettings::GetAppName());

        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    //SetIcon(m_hIcon, FALSE);        // Set small icon

    // trigger start of auto update check
    PostMessage(UWM_AUTO_UPDATE_CHECK);

    return FALSE; // return TRUE  unless you set the focus to a control
}


void CPlaylistCreatorDlg::__init__()
{
    InitWindowMessageFilter();
    FirstTimeInit();
    InitLanguage();
    UpdateFromRegistry();
    InitControls();
    InitResizableDialog();
    InitWindowPlacement();
    InitPlaylistColumnSizes();
    InitColors();
    UpdateItemStateDependentControls();
}


void CPlaylistCreatorDlg::InitWindowMessageFilter()
{
    // TaskbarButtonCreated message [>= Windows 7]
    //  let the TaskbarButtonCreated message through the UIPI filter. if we don't
    //  do this, Explorer would be unable to send that message to our window if we
    //  were running elevated. call can be made all the time, since if we're not
    //  elevated, this is a no-op.
    if(m_pChangeWindowMessageFilterEx)
    {
        CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };
        m_pChangeWindowMessageFilterEx(m_hWnd, m_uTaskbarBtnCreatedMsg, MSGFLT_ALLOW, &cfs);
    }
}


BOOL CPlaylistCreatorDlg::FirstTimeInit()
{
    BOOL bIsFirstTimeInitComplete = (BOOL)CAppSettings::GetIntProperty(PROP_FIRSTTIMEINITCOMPLETE, (int)FALSE);
    if(!bIsFirstTimeInitComplete)
    {
        CAppSettings::InitRegistryEntries();
        CAppSettings::SetIntProperty(PROP_FIRSTTIMEINITCOMPLETE, (int)TRUE);
        return TRUE;
    }
    return FALSE;
}


void CPlaylistCreatorDlg::InitLanguage()
{
    // caption
    SetWindowText(CAppSettings::GetWindowTitle());

    // group: playlist content
    SetDlgItemText(IDS_GB_PLAYLIST_CONTENT, CAppSettings::LoadString(_T("IDS_GB_PLAYLIST_CONTENT")));
    SetDlgItemText(IDS_PLAYLIST_CONTENT_INFO, CAppSettings::LoadString(_T("IDS_PLAYLIST_CONTENT_INFO")));
    m_btnAddFolder.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_ADD_FOLDER")));
    m_btnAddFiles.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_ADD_FILES")));
    m_btnAddPlaylists.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_ADD_PLAYLISTS")));
    m_btnMoveSelectedItemsTop.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_MOVE_TO_TOP")));
    m_btnMoveSelectedItemsUp.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_MOVE_UP")));
    m_btnMoveSelectedItemsDown.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_MOVE_DOWN")));
    m_btnMoveSelectedItemsBottom.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_MOVE_TO_BOTTOM")));
    m_btnDeleteSelectedItems.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_DELETE_SEL_ITEMS")));
    m_btnDeleteAllItems.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_DELETE_ALL_ITEMS")));

    // group: create playlist
    SetDlgItemText(IDS_GB_PLAYLIST_CREATION, CAppSettings::LoadString(_T("IDS_GB_PLAYLIST_CREATION")));
    SetDlgItemText(IDS_PLAYLIST_SAVE_LOCATION, CAppSettings::LoadString(_T("IDS_PLAYLIST_SAVE_LOCATION")));
    SetDlgItemText(IDS_PLAYLIST_NAME_AND_TYPE, CAppSettings::LoadString(_T("IDS_PLAYLIST_NAME_AND_TYPE")));
    m_btnSelectPlaylistSaveLocation.SetTooltipText(CAppSettings::LoadString(_T("IDS_BTN_SELECT_PLAYLIST_SAVE_LOCATION")));
}


void CPlaylistCreatorDlg::InitControls()
{
    InitToolBar();
    InitStatusBar();

    InitButtons();
    InitPlaylistListCtrl();
    InitThemeDependentControls();

    // init dialog font
    if(m_font.CreateFontIndirect(&CAppSettings::GetDialogFont()))
    {
        SetFontToDialog(m_hWnd, (HFONT)m_font.m_hObject);
    }

    // init playlist name edit box
    m_editPlaylistName.SetPromptText(CAppSettings::LoadString(_T("IDS_EDIT_PROMPT_PLAYLIST_NAME")),
                                     true /*regain*/, pmGray);

    // init combo playlist types
    m_cbPlaylistTypes.InsertString(0, PLAYLIST_TYPE_PLS);
    m_cbPlaylistTypes.InsertString(1, PLAYLIST_TYPE_M3U);
    CString strType = CAppSettings::GetStringProperty(PROP_PLAYLISTTYPE, PLAYLIST_TYPE_PLS);
    m_cbPlaylistTypes.SelectString(-1, strType);

    // set the initial input focus to the create button
    m_btnCreatePlaylist.SetFocus();
}


void CPlaylistCreatorDlg::InitButtons()
{
    // group: playlist content

    CButtonIcons gfxBtnAddFolder(m_hWnd, IDI_ADD_FOLDER, 24, 24);
    CButtonIcons gfxBtnAddFiles(m_hWnd, IDI_ADD_FILE, 24, 24);
    CButtonIcons gfxBtnAddPlaylists(m_hWnd, IDI_ADD_PLAYLIST, 24, 24);
    CButtonIcons gfxBtnMoveSelectedItemsTop(m_hWnd, IDI_GO_TOP, 24, 24);
    CButtonIcons gfxBtnMoveSelectedItemsUp(m_hWnd, IDI_GO_UP, 24, 24);
    CButtonIcons gfxBtnMoveSelectedItemsDown(m_hWnd, IDI_GO_DOWN, 24, 24);
    CButtonIcons gfxBtnMoveSelectedItemsBottom(m_hWnd, IDI_GO_BOTTOM, 24, 24);
    CButtonIcons gfxBtnDeleteSelectedItems(m_hWnd, IDI_DELETE, 24, 24);
    CButtonIcons gfxBtnDeleteAllItems(m_hWnd, IDI_RECYCLE_BIN, 24, 24);

    m_btnAddFolder.SetIcon(
        gfxBtnAddFolder.GetIconIn(),
        gfxBtnAddFolder.GetIconOut(),
        NULL,
        gfxBtnAddFolder.GetIconDisabled());
    m_btnAddFolder.DrawBorder(FALSE);

    m_btnAddFiles.SetIcon(
        gfxBtnAddFiles.GetIconIn(),
        gfxBtnAddFiles.GetIconOut(),
        NULL,
        gfxBtnAddFiles.GetIconDisabled());
    m_btnAddFiles.DrawBorder(FALSE);

    m_btnAddPlaylists.SetIcon(
        gfxBtnAddPlaylists.GetIconIn(),
        gfxBtnAddPlaylists.GetIconOut(),
        NULL,
        gfxBtnAddPlaylists.GetIconDisabled());
    m_btnAddPlaylists.DrawBorder(FALSE);

    m_btnMoveSelectedItemsTop.SetIcon(
        gfxBtnMoveSelectedItemsTop.GetIconIn(),
        gfxBtnMoveSelectedItemsTop.GetIconOut(),
        NULL,
        gfxBtnMoveSelectedItemsTop.GetIconDisabled());
    m_btnMoveSelectedItemsTop.DrawBorder(FALSE);

    m_btnMoveSelectedItemsUp.SetIcon(
        gfxBtnMoveSelectedItemsUp.GetIconIn(),
        gfxBtnMoveSelectedItemsUp.GetIconOut(),
        NULL,
        gfxBtnMoveSelectedItemsUp.GetIconDisabled());
    m_btnMoveSelectedItemsUp.DrawBorder(FALSE);

    m_btnMoveSelectedItemsDown.SetIcon(
        gfxBtnMoveSelectedItemsDown.GetIconIn(),
        gfxBtnMoveSelectedItemsDown.GetIconOut(),
        NULL,
        gfxBtnMoveSelectedItemsDown.GetIconDisabled());
    m_btnMoveSelectedItemsDown.DrawBorder(FALSE);

    m_btnMoveSelectedItemsBottom.SetIcon(
        gfxBtnMoveSelectedItemsBottom.GetIconIn(),
        gfxBtnMoveSelectedItemsBottom.GetIconOut(),
        NULL,
        gfxBtnMoveSelectedItemsBottom.GetIconDisabled());
    m_btnMoveSelectedItemsBottom.DrawBorder(FALSE);

    m_btnDeleteSelectedItems.SetIcon(
        gfxBtnDeleteSelectedItems.GetIconIn(),
        gfxBtnDeleteSelectedItems.GetIconOut(),
        NULL,
        gfxBtnDeleteSelectedItems.GetIconDisabled());
    m_btnDeleteSelectedItems.DrawBorder(FALSE);

    m_btnDeleteAllItems.SetIcon(
        gfxBtnDeleteAllItems.GetIconIn(),
        gfxBtnDeleteAllItems.GetIconOut(),
        NULL,
        gfxBtnDeleteAllItems.GetIconDisabled());
    m_btnDeleteAllItems.DrawBorder(FALSE);


    // group: create playlist

    CButtonIcons gfxBtnSelectPlaylistSaveLocation(m_hWnd, IDI_FOLDER, 24, 24);
    CButtonIcons gfxBtnCreatePlaylist(m_hWnd, IDI_PLAYLIST_CREATOR, 24, 24);

    m_btnSelectPlaylistSaveLocation.SetIcon(
        gfxBtnSelectPlaylistSaveLocation.GetIconIn(),
        gfxBtnSelectPlaylistSaveLocation.GetIconOut(),
        NULL,
        gfxBtnSelectPlaylistSaveLocation.GetIconDisabled());
    m_btnSelectPlaylistSaveLocation.DrawBorder(FALSE);

    m_btnCreatePlaylist.SetWindowText(CAppSettings::LoadString(_T("IDS_BTN_CREATE_PLAYLIST")));
    m_btnCreatePlaylist.SetThemeHelper(&m_ThemeHelper);
    m_btnCreatePlaylist.SetFont(&CAppSettings::GetDialogLargeBoldFont());
    m_btnCreatePlaylist.SetIcon(
        gfxBtnCreatePlaylist.GetIconIn(),
        gfxBtnCreatePlaylist.GetIconOut(),
        NULL,
        gfxBtnCreatePlaylist.GetIconDisabled());
    m_btnCreatePlaylist.SetIconOffsetX(10);
}


void CPlaylistCreatorDlg::InitPlaylistListCtrl()
{
    // add columns
    m_listPlaylistContent.InsertColumn(         // column: filename
        0,
        CAppSettings::LoadString(_T("IDS_PLAYLIST_COLUMN_FILENAME")),
        LVCFMT_LEFT,
        100);

    m_listPlaylistContent.InsertColumn(         // column: type
        1,
        CAppSettings::LoadString(_T("IDS_PLAYLIST_COLUMN_TYPE")),
        LVCFMT_LEFT,
        100);

    m_listPlaylistContent.InsertColumn(         // column: folder
        2,
        CAppSettings::LoadString(_T("IDS_PLAYLIST_COLUMN_FOLDER")),
        LVCFMT_LEFT,
        100);

    // init drag and drop
    m_listPlaylistContent.Register(&m_listPlaylistContent);
    CFileDropListCtrl::DROPLISTMODE dropMode;
    dropMode.nMask = CFileDropListCtrl::DL_ACCEPT_FILES |
                     CFileDropListCtrl::DL_ACCEPT_FOLDERS |
                     CFileDropListCtrl::DL_USE_CALLBACK;
    dropMode.pfnCallback = CPlaylistCreatorDlg::OnPlaylistItemsDropped;
    m_listPlaylistContent.SetDropMode(dropMode);

    // misc features
    m_listPlaylistContent.EnableSorting(TRUE);
    m_listPlaylistContent.EnableFullRowSelect();
    m_listPlaylistContent.EnableToolTips(TRUE, LTTS_PER_ITEM_TIP);
    m_listPlaylistContent.SetEmptyMessage(CAppSettings::LoadString(_T("IDS_INFO_PLAYLIST_IS_EMPTY")));
}


void CPlaylistCreatorDlg::InitWindowPlacement()
{
    // get app rect
    CRect rect;
    rect.left   = max(CAppSettings::GetIntProperty(PROP_MAINDLG_WNDPOSX, 50), 0);
    rect.top    = max(CAppSettings::GetIntProperty(PROP_MAINDLG_WNDPOSY, 50), 0);
    rect.right  = max(rect.left + CAppSettings::GetIntProperty(PROP_MAINDLG_WNDSIZEX, m_minDlgSizeX), rect.left + m_minDlgSizeX);
    rect.bottom = max(rect.top  + CAppSettings::GetIntProperty(PROP_MAINDLG_WNDSIZEY, m_minDlgSizeY), rect.top  + m_minDlgSizeY);

    // verify that app rect is visible
    if(!CMonitors::IsOnScreen(&rect))
    {
        TRACE(">>>>> Warning: app rect isn't on screen! restoring initial position...\n");

        int screenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

        if(screenWidth < rect.left)
        {
            rect.right = (screenWidth / 3) + rect.Width();
            rect.left = screenWidth / 3;
        }

        if(screenHeight < rect.top)
        {
            rect.bottom = (screenHeight / 3) + rect.Height();
            rect.top = screenHeight / 3;
        }
    }

    // restore window placement
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    wndpl.flags = 0;
    wndpl.ptMinPosition = CPoint(0, 0);
    wndpl.ptMaxPosition = CPoint(-1, -1);
    wndpl.rcNormalPosition = rect;
    wndpl.showCmd = (UINT)CAppSettings::GetIntProperty(PROP_MAINDLG_WNDSHOWSTATE, (int)SW_SHOWNORMAL);

    if(wndpl.showCmd != SW_SHOWNORMAL
        && wndpl.showCmd != SW_SHOWMINIMIZED
        && wndpl.showCmd != SW_SHOWMAXIMIZED)
    {
        ASSERT(0);
        wndpl.showCmd = SW_SHOWNORMAL;
    }

    SetWindowPlacement(&wndpl);
}


void CPlaylistCreatorDlg::InitResizableDialog()
{
    UpdateToolBar();
    UpdateStatusBar();
    INIT_EASYSIZE;
}


void CPlaylistCreatorDlg::InitPlaylistColumnSizes()
{
    CRect rect;
    m_listPlaylistContent.GetWindowRect(&rect);
    int nWidth = rect.Width();

    CHeaderCtrl* pHeader = m_listPlaylistContent.GetHeaderCtrl();                       ASSERT(pHeader);
    if(pHeader)
    {
        HDITEM hdItem;
        hdItem.mask = HDI_WIDTH;

        // column: filename
        hdItem.cxy = max(AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("PlaylistColumn0"), (nWidth*50)/100), 30);
        pHeader->SetItem(0, &hdItem);

        // column: type
        hdItem.cxy = max(AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("PlaylistColumn1"), (nWidth*10)/100), 30);
        pHeader->SetItem(1, &hdItem);

        // column: folder
        hdItem.cxy = max(AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("PlaylistColumn2"), (nWidth*35)/100), 30);
        pHeader->SetItem(2, &hdItem);
    }
}


void CPlaylistCreatorDlg::InitColors()
{
    // default highlight color
    m_DefaultHighlightColor = LightenColor(::GetSysColor(COLOR_HIGHLIGHT), 0.50);

    // playlist control
    m_listPlaylistContent.SetBkColor(::GetSysColor(COLOR_WINDOW));
    m_listPlaylistContent.SetTextBkColor(::GetSysColor(COLOR_WINDOW));
}


void CPlaylistCreatorDlg::InitThemeDependentControls()
{
    BOOL bIsAppThemed = m_ThemeHelper.IsAppThemed();

    // create button
    m_btnCreatePlaylist.SetFlat(bIsAppThemed);
}


void CPlaylistCreatorDlg::UpdateFromRegistry()
{
    // playlist save location
    SetPlaylistSaveLocation(CAppSettings::GetStringProperty(PROP_LASTPLAYLISTFOLDER));

    // get active file types
    CString strRegKey = CAppSettings::GetRegistryKey(NAMESTYLE_FULL) + _T("\\FileTypes");
    m_listActiveFileTypes.RemoveAll();
    CStringArray listFileTypes;
    RegistryEnumerateValues(HKEY_CURRENT_USER, strRegKey, listFileTypes);

    for(int i = 0; i < listFileTypes.GetCount(); i++)
    {
        CString strRegValue = listFileTypes.GetAt(i);
        CString strRegData  = RegistryGetStringValue(HKEY_CURRENT_USER, strRegKey, strRegValue);

        if(strRegData == _T("1"))
        {
            m_listActiveFileTypes.Add(strRegValue);
            continue;
        }
    }

    // filter the playlist file types (e.g. pls, m3u)
    CPlaylistFileManager playlistManager;
    for(int i = 0; i < m_listActiveFileTypes.GetCount(); i++)
    {
        if(playlistManager.IsPlaylistFileExtension(m_listActiveFileTypes.GetAt(i)))
        {
            m_listActiveFileTypes.RemoveAt(i);
            i--;
        }
    }


    // transparency
    if(m_pSetLayeredWindowAttributes)
    {
        m_bTransparency = CAppSettings::GetIntProperty(PROP_TRANSPARENCY, (int)FALSE);
        if(m_bTransparency)
        {
            m_iAlpha = 255;
            AddWindowExStyle(m_hWnd, WS_EX_LAYERED);
            m_pSetLayeredWindowAttributes(m_hWnd, 0, (BYTE)m_iAlpha, LWA_ALPHA);
        }
        else
        {
            RemoveWindowExStyle(m_hWnd, WS_EX_LAYERED);
        }
    }

    // gridlines
    m_listPlaylistContent.EnableCustomGridlines((BOOL)CAppSettings::GetIntProperty(PROP_SHOWGRIDLINES, (int)TRUE));

    // alternate row color
    if((BOOL)CAppSettings::GetIntProperty(PROP_SHOWALTERNATEROWCOLOR, (int)TRUE))
    {
        COLORREF color = CAppSettings::GetIntProperty(PROP_ALTERNATEROWCOLOR, (int)DEFAULT_ALTERNATE_ROW_COLOR);
        m_listPlaylistContent.SetAlternatingRowColor(color);
    }
    else
    {
        m_listPlaylistContent.EnableAlternatingRowColor(FALSE);
    }

    // highlight missing items
    m_bHighlightMissingItems = (BOOL)CAppSettings::GetIntProperty(PROP_HIGHLIGHTMISSINGITEMS, (int)TRUE);
    m_crMissingItems = CAppSettings::GetIntProperty(PROP_MISSINGITEMSCOLOR, (int)DEFAULT_MISSING_ITEMS_COLOR);
    RefreshItemState();

    // snap to screen border
    m_bSnapToScreenBorder = (BOOL)CAppSettings::GetIntProperty(PROP_SNAPTOSCREENBORDER, (int)TRUE);
}


void CPlaylistCreatorDlg::UpdateToRegistry()
{
    SavePlaylistColumnSizes();
    SaveWindowPlacement();

    // playlist type
    CAppSettings::SetStringProperty(PROP_PLAYLISTTYPE, GetPlaylistType());
}


// saves the column sizes of the playlist
void CPlaylistCreatorDlg::SavePlaylistColumnSizes()
{
    CHeaderCtrl* pHeader = m_listPlaylistContent.GetHeaderCtrl();                   ASSERT(pHeader);
    if(pHeader)
    {
        int nCount = pHeader->GetItemCount();
        for(int nIndex = 0; nIndex < nCount; nIndex++)
        {
            CRect rect;
            if(pHeader->GetItemRect(nIndex, &rect))
            {
                CString strRegValue = _T("PlaylistColumn") + IntegerToString(nIndex);
                AfxGetApp()->WriteProfileInt(_T("Dialogs\\MainDlg"), strRegValue, rect.Width());
            }
        }
    }
}


// saves dialog position, size and show state to registry
void CPlaylistCreatorDlg::SaveWindowPlacement()
{
    // get window placement
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    if(!GetWindowPlacement(&wndpl))
    {
        ASSERT(0);
        return; // error: couldn't retrieve window placement
    }

    // window position
    CAppSettings::SetIntProperty(PROP_MAINDLG_WNDPOSX, (int)wndpl.rcNormalPosition.left);
    CAppSettings::SetIntProperty(PROP_MAINDLG_WNDPOSY, (int)wndpl.rcNormalPosition.top);

    // window size
    CAppSettings::SetIntProperty(PROP_MAINDLG_WNDSIZEX, (int)wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left);
    CAppSettings::SetIntProperty(PROP_MAINDLG_WNDSIZEY, (int)wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top);

    // show state (e.g. minimized, maximized)
    CAppSettings::SetIntProperty(PROP_MAINDLG_WNDSHOWSTATE, (int)wndpl.showCmd);
}


// resets the sort column and removes the current sort arrow from the playlist
void CPlaylistCreatorDlg::ResetPlaylistSorting()
{
    if(m_nSortColumn == -1)
        return; // nothing to do

    // reset the sort column
    m_nSortColumn = -1;

    // remove sort arrow
    ListView_RemoveHeaderSortArrow(m_listPlaylistContent.GetSafeHwnd());
}


void CPlaylistCreatorDlg::SetPlaylistSaveLocation(const CString& strLocation)
{
    // update necessary?
    if(PathRemoveTrailingBackSlash(strLocation) == m_strPlaylistFolder)
    {
        return; // path didn't change
    }

    // update folder
    m_strPlaylistFolder = (strLocation.GetLength() > 3) ? PathRemoveTrailingBackSlash(strLocation) : strLocation;
    UpdateData(TO_DIALOG);

    // write folder to registry
    CAppSettings::SetStringProperty(PROP_LASTPLAYLISTFOLDER, m_strPlaylistFolder);
}


// sets the playlist save location to the folder of the passed item
void CPlaylistCreatorDlg::SetPlaylistSaveLocationFromItem(int nItem)
{
    if(!m_listPlaylistContent.IsValidItemIndex(nItem))
    {
        ASSERT(0);
        return; // error: invalid index
    }

    // get item's folder
    CString strItemFolder = _T("");
    CPlaylistElement* pElement = reinterpret_cast<CPlaylistElement*>(m_listPlaylistContent.GetItemData(nItem));
    if(pElement)
    {
        strItemFolder = pElement->GetFolder();
        if(strItemFolder.GetLength() > 3) // not the root folder
        {
            strItemFolder = PathRemoveTrailingBackSlash(strItemFolder);
        }
    }

    // update the playlist
    if(!strItemFolder.IsEmpty())
    {
        SetPlaylistSaveLocation(strItemFolder);

        CEdit* pEdit = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_PLAYLIST_FOLDER));
        if(pEdit)
        {
            FlashEdit(pEdit, m_DefaultHighlightColor);
        }
    }
}


// sets the playlist name
void CPlaylistCreatorDlg::SetPlaylistName(const CString& strName)
{
    m_editPlaylistName.SetWindowText(StripInvalidFileNameCharacters(strName));
}


// clears the playlist name
void CPlaylistCreatorDlg::ClearPlaylistName()
{
    m_editPlaylistName.SetWindowText(_T(""));
}


// gets the current playlist type
CString CPlaylistCreatorDlg::GetPlaylistType()
{
    int nCurSel = m_cbPlaylistTypes.GetCurSel();

    CString strType;
    m_cbPlaylistTypes.GetLBText(nCurSel, strType);

    return strType;
}


// sets the playlist type
bool CPlaylistCreatorDlg::SetPlaylistType(const CString& strType)
{
    if(strType.IsEmpty())
    {
        ASSERT(0); // error: no type passed
        return false;
    }

    CString strTypeBuffer = strType;
    if(strTypeBuffer.Left(1) != _T("."))
        strTypeBuffer.Insert(0, _T("."));

    for(int i = 0; i < m_cbPlaylistTypes.GetCount(); i++)
    {
        CString strComboItem;
        m_cbPlaylistTypes.GetLBText(i, strComboItem);

        if(strComboItem.CompareNoCase(strTypeBuffer) == 0)
        {
            m_cbPlaylistTypes.SetCurSel(i);
            return true;
        }
    }

    ASSERT(0); // error: unknown type
    return false;
}


// updates the controls that are dependent on the state of the playlist items
void CPlaylistCreatorDlg::UpdateItemStateDependentControls()
{
    // store currently focused window
    CWnd* pFocusedWnd = GetFocus();

    // get info about current playlist
    int nItems                  = m_listPlaylistContent.GetItemCount();
    int nSelectedItems          = m_listPlaylistContent.GetSelectedCount();

    bool bIsPlaylistEmpty       = (nItems == 0)         ? true : false;
    bool bIsItemSelected        = (nSelectedItems > 0)  ? true : false;
    bool bIsFirstItemSelected   = m_listPlaylistContent.IsItemSelected(0);
    bool bIsLastItemSelected    = m_listPlaylistContent.IsItemSelected(m_listPlaylistContent.GetTailIndex());

    // status bar: number of selected items
    CString strSelectedItems = _T("");

    if(nSelectedItems > 0)
    {
        strSelectedItems = ReplaceTags(CAppSettings::LoadString(_T("IDS_SELECTED_ITEM_COUNT")),
                                        _T("NUMBER"), IntegerToString(nSelectedItems));
    }

    // status bar: total number of items
    CString strItems = ReplaceTags(CAppSettings::LoadString(_T("IDS_ITEM_COUNT")),
                                    _T("NUMBER"), IntegerToString(nItems));

    // status bar: update panes
    SetStatusBarText(0, strSelectedItems);
    SetStatusBarText(1, strItems);

    // buttons
    m_btnMoveSelectedItemsTop.EnableWindow(     (bIsItemSelected && !bIsFirstItemSelected));
    m_btnMoveSelectedItemsUp.EnableWindow(      (bIsItemSelected && !bIsFirstItemSelected));
    m_btnMoveSelectedItemsDown.EnableWindow(    (bIsItemSelected && !bIsLastItemSelected));
    m_btnMoveSelectedItemsBottom.EnableWindow(  (bIsItemSelected && !bIsLastItemSelected));
    m_btnDeleteSelectedItems.EnableWindow(      (!bIsPlaylistEmpty && bIsItemSelected));
    m_btnDeleteAllItems.EnableWindow(           (!bIsPlaylistEmpty));


    // restore focus
    if(pFocusedWnd && pFocusedWnd != GetFocus())
    {
        pFocusedWnd->SetFocus();
    }
}


// initializes the toolbar
void CPlaylistCreatorDlg::InitToolBar()
{
    // button indices
    const int nButtonNewPlaylist    = 0;
    const int nButtonOpenPlaylist   = 1;
    const int nButtonSettings       = 2;
    const int nButtonHelp           = 3;
    const int nButtonAbout          = 4;

    // icon size
    int nIconSize = CAppSettings::GetToolBarIconSize();

    // create rebar
    if(!m_pReBar)
    {
        DWORD dwReBarStyles = RBS_FIXEDORDER;

        m_pReBar = new CReBar();

        if(!m_pReBar->Create(this, dwReBarStyles))
        {
            ASSERT(0);
            return; // error: couldn't create rebar
        }

        // set rebar info
        REBARINFO rbi;
        SecureZeroMemory(&rbi, sizeof(rbi));
        rbi.cbSize = sizeof(rbi);
        m_pReBar->GetReBarCtrl().SetBarInfo(&rbi);
    }

    // create toolbar and attach images
    if(!m_pToolBar)
    {
        DWORD dwToolBarStyles = TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS;

        m_pToolBar = new CToolBar();

        if(!m_pToolBar->CreateEx(this, dwToolBarStyles, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
           !m_pToolBar->LoadToolBar(IDR_TOOLBAR_MAIN_DIALOG))
        {
            ASSERT(0);
            return; // error: couldn't create toolbar
        }

        m_pToolBar->SetFont(&m_font);
    }
    AttachToolBarImages();

    // button and bitmap sizes
    CSize sizeBitmap;
    sizeBitmap.cx = nIconSize; // bitmap width
    sizeBitmap.cy = nIconSize; // bitmap height
    CSize sizeButton;
    sizeButton.cx = sizeBitmap.cx + 7; // button width (minimum size, see MSDN)
    sizeButton.cy = sizeBitmap.cy + 8; // button height (minimum size + 2, see MSDN)
    m_pToolBar->SetSizes(sizeButton, sizeBitmap);

    // button auto sizing
    int nButtonCount = m_pToolBar->GetToolBarCtrl().GetButtonCount();
    for(int i = 0; i < nButtonCount; i++)
    {
        m_pToolBar->SetButtonStyle(i, TBBS_BUTTON | TBBS_AUTOSIZE);
    }

    // button texts
    m_pToolBar->SetButtonText(nButtonNewPlaylist,  CAppSettings::LoadString(_T("IDS_TOOLBAR_NEW_PLAYLIST")));
    m_pToolBar->SetButtonText(nButtonOpenPlaylist, CAppSettings::LoadString(_T("IDS_TOOLBAR_OPEN_PLAYLIST")));
    m_pToolBar->SetButtonText(nButtonSettings,     CAppSettings::LoadString(_T("IDS_TOOLBAR_SETTINGS")));
    m_pToolBar->SetButtonText(nButtonHelp,         CAppSettings::LoadString(_T("IDS_TOOLBAR_HELP")));
    m_pToolBar->SetButtonText(nButtonAbout,        CAppSettings::LoadString(_T("IDS_TOOLBAR_ABOUT")));

    // toolbar indentation
    int iIndent = 2;
    m_pToolBar->GetToolBarCtrl().SetIndent(iIndent);

    // toolbar background bitmap
    if(m_imgReBarBkgnd.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_MAINDLG_REBAR), "PNG"), CXIMAGE_FORMAT_PNG))
    {
        // expand bitmap if width is smaller than parent width
        CRect rect;
        GetClientRect(&rect);
        if(rect.Width() > (int)m_imgReBarBkgnd.GetWidth()+2)
        {
            int cx = rect.Width() - (int)m_imgReBarBkgnd.GetWidth() - 2;
            TRACE(">>>>> Note: expanding toolbar background bitmap by %d pixels...\n", cx);
            RGBQUAD rgba = {255, 255, 255, 0};
            m_imgReBarBkgnd.Expand(cx, 0, 0, 0, rgba);
        }
    }
    HBITMAP hBmpBkgnd = m_imgReBarBkgnd.MakeBitmap();

    // insert toolbar band into rebar
    REBARBANDINFO rbbi;
    SecureZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize     = sizeof(rbbi);
    rbbi.fMask      = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_BACKGROUND;
    rbbi.fStyle     = RBBS_VARIABLEHEIGHT;
    rbbi.hwndChild  = m_pToolBar->m_hWnd;
    rbbi.cxMinChild = 0;
    rbbi.cyMinChild = sizeButton.cy + (iIndent * 2);
    rbbi.cyMaxChild = sizeButton.cy + (iIndent * 2);
    rbbi.cx         = 200;
    rbbi.hbmBack    = hBmpBkgnd;
    m_pReBar->GetReBarCtrl().InsertBand(-1, &rbbi);

    // position rebar
    CRect clientRect;
    GetClientRect(&clientRect);
    m_pReBar->MoveWindow(clientRect.left, clientRect.top, clientRect.Width(), sizeButton.cy + (iIndent * 2), TRUE);

    // position rebar edge
    WINDOWPLACEMENT wndpl;
    ::GetWindowPlacement(m_pReBar->m_hWnd, &wndpl);
    CRect rectReBar = wndpl.rcNormalPosition;
    GetDlgItem(IDC_REBAR_EDGE)->MoveWindow(-1, rectReBar.bottom, clientRect.Width() + 10, 1, TRUE);
}


// updates size and position of the toolbar
void CPlaylistCreatorDlg::UpdateToolBar()
{
    if(!m_pReBar)
    {
        return; // rebar isn't created yet
    }

    // get toolbar rect
    CRect rect;
    GetClientRect(&rect);
    rect.bottom = m_pReBar->GetReBarCtrl().GetBarHeight();

    if(rect.Width() <= 0)
        return;

    // verify that background image is loaded
    if(m_imgReBarBkgnd.GetWidth() <= 0)
    {
        VERIFY(m_imgReBarBkgnd.LoadResource(FindResource(NULL, MAKEINTRESOURCE(IDR_PNG_MAINDLG_REBAR), "PNG"), CXIMAGE_FORMAT_PNG));
    }

    // prepare new background image
    int nOffset = rect.Width() - ((int)m_imgReBarBkgnd.GetWidth() + 2);

    if(nOffset > 0) // expand image
    {
        RGBQUAD rgba = {255, 255, 255, 0};
        m_imgReBarBkgnd.Expand(nOffset, 0, 0, 0, rgba);
    }
    else if(nOffset < 0) // crop image
    {
        RECT rectCrop = {abs(nOffset), 0, m_imgReBarBkgnd.GetWidth(), m_imgReBarBkgnd.GetHeight()};
        m_imgReBarBkgnd.Crop(rectCrop);
    }

    // set new background image
    HBITMAP hBmpBkgnd = m_imgReBarBkgnd.MakeBitmap();

    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize  = sizeof(rbbi);
    rbbi.fMask   = RBBIM_BACKGROUND;
    rbbi.hbmBack = hBmpBkgnd;

    m_pReBar->GetReBarCtrl().SetBandInfo(0, &rbbi);

    // position rebar
    m_pReBar->MoveWindow(rect);
}


void CPlaylistCreatorDlg::AttachToolBarImages()
{
    ASSERT(m_pToolBar->m_hWnd);

    // get icon size
    int nIconSize = CAppSettings::GetToolBarIconSize();

    // prepare images
    CButtonIcons gfxBtnNewPlaylist(  m_hWnd, IDI_NEW_PLAYLIST,   nIconSize, nIconSize);
    CButtonIcons gfxBtnOpenPlaylist( m_hWnd, IDI_OPEN_PLAYLIST,  nIconSize, nIconSize);
    CButtonIcons gfxBtnSettings(     m_hWnd, IDI_CONTROL_PANEL,  nIconSize, nIconSize);
    CButtonIcons gfxBtnHelp(         m_hWnd, IDI_HELP,           nIconSize, nIconSize);
    CButtonIcons gfxBtnAbout(        m_hWnd, IDI_INFO,           nIconSize, nIconSize);

    // create cold toolbar image list
    if(!m_pilToolBarCold)
    {
        m_pilToolBarCold = new CImageList();
        m_pilToolBarCold->Create(nIconSize, nIconSize, ILC_COLOR32 | ILC_MASK, 5, 1);
        m_pilToolBarCold->Add(gfxBtnNewPlaylist.GetIconOut());
        m_pilToolBarCold->Add(gfxBtnOpenPlaylist.GetIconOut());
        m_pilToolBarCold->Add(gfxBtnSettings.GetIconOut());
        m_pilToolBarCold->Add(gfxBtnHelp.GetIconOut());
        m_pilToolBarCold->Add(gfxBtnAbout.GetIconOut());
    }

    // create hot toolbar image list
    if(!m_pilToolBarHot)
    {
        m_pilToolBarHot = new CImageList();
        m_pilToolBarHot->Create(nIconSize, nIconSize, ILC_COLOR32 | ILC_MASK, 5, 1);
        m_pilToolBarHot->Add(gfxBtnNewPlaylist.GetIconIn());
        m_pilToolBarHot->Add(gfxBtnOpenPlaylist.GetIconIn());
        m_pilToolBarHot->Add(gfxBtnSettings.GetIconIn());
        m_pilToolBarHot->Add(gfxBtnHelp.GetIconIn());
        m_pilToolBarHot->Add(gfxBtnAbout.GetIconIn());
    }

    // set toolbar image lists
    m_pToolBar->GetToolBarCtrl().SetImageList(m_pilToolBarCold);
    m_pToolBar->GetToolBarCtrl().SetHotImageList(m_pilToolBarHot);
}


// initializes the statusbar
void CPlaylistCreatorDlg::InitStatusBar()
{
    if(!m_pStatusBar)
    {
        // create status bar
        m_pStatusBar = new CStatusBar();
        m_pStatusBar->CreateEx(
            this,
            0,
            WS_CHILD | WS_VISIBLE | CBRS_BORDER_BOTTOM | SBARS_SIZEGRIP,
            AFX_IDW_STATUS_BAR);

        ASSERT(m_pStatusBar);

        // set indicators
        m_pStatusBar->SetIndicators(indicators, SB_PANE_COUNT);

        // set panes
        m_pStatusBar->SetPaneInfo(0, ID_SB_INDICATOR, SBPS_NORMAL | SBPS_STRETCH, 0);
        m_pStatusBar->SetPaneInfo(1, ID_SB_INDICATOR, SBPS_NORMAL, 100);

        // init font
        m_pStatusBar->SetFont(&m_font);

        // init texts
        SetStatusBarText(0, _T(""));
        SetStatusBarText(1, _T(""));
    }
}


// updates size and position of the statusbar
void CPlaylistCreatorDlg::UpdateStatusBar()
{
    if(!m_pStatusBar || !IsWindow(m_pStatusBar->m_hWnd))
    {
        return; // status bar isn't created yet
    }

    // get client size
    RECT rect;
    GetClientRect(&rect);
    rect.top = rect.bottom - (::GetSystemMetrics(SM_CXHSCROLL) + 4);

    // position status bar
    m_pStatusBar->MoveWindow(&rect);

    // don't show gripper if dialog is maximized
    if(IsZoomed())
    {
        m_pStatusBar->ModifyStyle(SBARS_SIZEGRIP, 0);
    }
    else
    {
        m_pStatusBar->ModifyStyle(0, SBARS_SIZEGRIP);
    }
}


// sets the text of the passed status bar pane. if nDuration is greater
// than zero the passed text is only visible for nDuration msecs.
void CPlaylistCreatorDlg::SetStatusBarText(UINT nPane, const CString& strText, UINT nDuration /*=0*/)
{
    if(!m_pStatusBar || (nPane >= SB_PANE_COUNT))
        return;

    if(nDuration > 0)
    {
        // %TODO%: implement temporary status bar text display
        ASSERT(0);
    }

    m_pStatusBar->SetPaneText(nPane, strText);
}


CString CPlaylistCreatorDlg::GetBaseFolder(const CStringArray& arrayFolders)
{
    #pragma message(Reminder "GetBaseFolder(): Performance killer when number of folders is large!")

    if(arrayFolders.IsEmpty())
        return _T("");

    CPath path(arrayFolders.GetAt(0));
    CString strBaseFolder = path.GetLocation();
    int nBaseFolderDirCount = path.GetDirCount();

    if(nBaseFolderDirCount > 0 && arrayFolders.GetCount() > 1)
    {
        for(int i=0; i < arrayFolders.GetCount(); i++)
        {
            CPath pathNextFolder(arrayFolders.GetAt(i));
            CString strNextFolder = pathNextFolder.GetLocation();
            int nNextFolderDirCount = pathNextFolder.GetDirCount();

            CString strRelativePath = pathNextFolder.GetRelativePath(strBaseFolder);

            if(strRelativePath.Left(3) != _T("..\\"))
            {
                // same folder or deeper
                continue;
            }

            strRelativePath.Replace(_T("..\\"), _T(""));        // "..\..\Air\Moon Safari\Remember.mp3"

            strNextFolder += pathNextFolder.GetFileName();
            strNextFolder.Replace(strRelativePath, _T(""));     // "Air\Moon Safari\Remember.mp3"

            CPath pathNewBaseFolder(strNextFolder);
            strBaseFolder = pathNewBaseFolder.GetLocation();
            nBaseFolderDirCount = pathNewBaseFolder.GetDirCount();

            if(nBaseFolderDirCount == 0)
            {
                break;
            }
        }
    }
    return strBaseFolder;
}


BOOL CPlaylistCreatorDlg::AutoPlaylistFolder(const CString& strFolder)
{
    if(strFolder.IsEmpty())
        return FALSE;

    if(CAppSettings::GetIntProperty(PROP_AUTOPLAYLISTFOLDER, (int)FALSE))
    {
        SetPlaylistSaveLocation(strFolder);

        CEdit* pEdit = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_PLAYLIST_FOLDER));
        if(pEdit)
        {
            FlashEdit(pEdit, m_DefaultHighlightColor);
        }
        return TRUE;
    }
    return FALSE;
}


BOOL CPlaylistCreatorDlg::AutoPlaylistFolder(const CStringArray& arrayFolders)
{
    if(arrayFolders.IsEmpty())
        return FALSE;

    if(CAppSettings::GetIntProperty(PROP_AUTOPLAYLISTFOLDER, (int)FALSE))
    {
        return AutoPlaylistFolder(GetBaseFolder(arrayFolders));
    }
    return FALSE;
}


BOOL CPlaylistCreatorDlg::AutoPlaylistName(const CString& strFolder)
{
    if(strFolder.IsEmpty())
        return FALSE;

    if(CAppSettings::GetIntProperty(PROP_AUTOPLAYLISTNAME, (int)FALSE))
    {
        CString strFolderBuffer = PathRemoveTrailingBackSlash(strFolder);
        strFolderBuffer.MakeReverse();
        strFolderBuffer = strFolderBuffer.SpanExcluding(_T("\\"));
        strFolderBuffer.MakeReverse();

        SetPlaylistName(strFolderBuffer);
        FlashEdit(&m_editPlaylistName, m_DefaultHighlightColor);

        return TRUE;
    }
    return FALSE;
}


BOOL CPlaylistCreatorDlg::AutoPlaylistName(const CStringArray& arrayFolders)
{
    if(arrayFolders.IsEmpty())
        return FALSE;

    if(CAppSettings::GetIntProperty(PROP_AUTOPLAYLISTNAME, (int)FALSE))
    {
        return AutoPlaylistName(GetBaseFolder(arrayFolders));
    }
    return FALSE;
}


BOOL CPlaylistCreatorDlg::AutoClearPlaylist()
{
    if(CAppSettings::GetIntProperty(PROP_AUTOCLEAR, (int)FALSE))
    {
        ClearPlaylist();
        return TRUE;
    }
    return FALSE;
}


void CPlaylistCreatorDlg::ClearPlaylist()
{
    // free item data
    int nItems = m_listPlaylistContent.GetItemCount();
    for(int item = 0; item < nItems; item++)
    {
        CPlaylistElement* pElement = reinterpret_cast<CPlaylistElement*>(m_listPlaylistContent.GetItemData(item));
        if(pElement)
        {
            delete pElement;
            pElement = NULL;
        }
    }

    // update gui
    m_listPlaylistContent.DeleteAllItems();
    UpdateItemStateDependentControls();
    ResetPlaylistSorting();
}


void CPlaylistCreatorDlg::OpenPlaylist(const CString& strPlaylistFilePath)
{
    if(strPlaylistFilePath.IsEmpty())
    {
        ASSERT(0);
        return;
    }

    // clear current playlist and name
    ClearPlaylist();
    ClearPlaylistName();

    // set save location, name and type
    SetPlaylistSaveLocation(GetFolderComponent(strPlaylistFilePath));
    SetPlaylistName(GetFileNameComponent(strPlaylistFilePath));
    SetPlaylistType(GetFileExtensionComponent(strPlaylistFilePath));

    // open and import playlist
    AddToPlaylist(strPlaylistFilePath, -1, false);
}


BOOL CPlaylistCreatorDlg::AddToPlaylist(const CString& strFileOrFolder,
                                        int            customIndex /*-1*/,
                                        bool           bRecursive /*false*/)
{
    CStringArray arrayFilesAndFolders;
    arrayFilesAndFolders.Add(strFileOrFolder);

    return AddToPlaylist(arrayFilesAndFolders, customIndex, bRecursive);
}


BOOL CPlaylistCreatorDlg::AddToPlaylist(CStringArray& arrayFilesAndFolders,
                                        int           customIndex /*-1*/,
                                        bool          bRecursive /*false*/)
{
    // init and launch file adding dialog
    CAddFilesWaitDlg dlg;
    dlg.SetTaskbarListProvider(this);
    dlg.SetFilesToAdd(this, &arrayFilesAndFolders, customIndex, bRecursive);
    dlg.DoModal();

    // reset taskbar
    SetProgressState(TBPF_NOPROGRESS);

    // get results
    int nInsertedFiles     = dlg.GetInsertedFilesCount();
    int nLastInsertedIndex = dlg.GetLastInsertedIndex();

    // update selection
    if(nInsertedFiles > 0)
    {
        // clear existing item selection
        m_listPlaylistContent.DeselectAllItems();

        // select last inserted item and scroll into view
        m_listPlaylistContent.SetItemState(nLastInsertedIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        m_listPlaylistContent.SetSelectionMark(nLastInsertedIndex); // needed when multi-selection is on!
        m_listPlaylistContent.EnsureVisible(nLastInsertedIndex, FALSE);
        ListCtrl_EnsureVisibleOffset(m_listPlaylistContent, nLastInsertedIndex, FALSE, 4);

        // force redraw
        m_listPlaylistContent.Invalidate();
        m_listPlaylistContent.UpdateWindow();

        ResetPlaylistSorting();
        m_listPlaylistContent.SetFocus();

        return TRUE;
    }
    else // no files have been inserted
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
    }

    return FALSE;
}


// select all items in the list
void CPlaylistCreatorDlg::SelectAllItems()
{
    if(m_listPlaylistContent.IsEmpty())
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
        return;
    }

    m_listPlaylistContent.SelectAllItems();
    m_listPlaylistContent.SetFocus();
}


// deselects all items in the list
void CPlaylistCreatorDlg::DeselectAllItems()
{
    if(m_listPlaylistContent.IsEmpty())
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
        return;
    }

    m_listPlaylistContent.DeselectAllItems();
    m_listPlaylistContent.SetFocus();
}


void CPlaylistCreatorDlg::RefreshItemState()
{
    if(m_listPlaylistContent.IsEmpty())
        return;

    int nItems = m_listPlaylistContent.GetItemCount();
    for(int item = 0; item < nItems; item++)
    {
        CPlaylistElement* pElement = reinterpret_cast<CPlaylistElement*>(m_listPlaylistContent.GetItemData(item));
        if(!pElement)
        {
            ASSERT(0);
            continue;
        }

        bool bExists = IsFile(pElement->GetFilePath());
        COLORREF bkColor = (bExists || !m_bHighlightMissingItems) ? COLOR_INVALID : m_crMissingItems;

        m_listPlaylistContent.SetItemBkColor(item, -1, bkColor, FALSE);
    }

    // perform single redraw after state of all items was updated
    m_listPlaylistContent.RedrawWindow();
}


void CPlaylistCreatorDlg::ShufflePlaylist()
{
    if(m_listPlaylistContent.GetItemCount() <= 1)
        return; // nothing to shuffle

    // really shuffle playlist items?
    if(!CAppSettings::GetDontShowDialog(_T("Ask_ShufflePlaylist")))
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        int nResult = XMessageBox(m_hWnd,
                                  CAppSettings::LoadString(_T("IDS_ASK_SHUFFLE_PLAYLIST")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_YESNO | MB_ICONQUESTION | MB_DONOTASKAGAIN,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDNO)
            return;

        if(nResult & MB_DONOTASKAGAIN)
            CAppSettings::SetDontShowDialog(_T("Ask_ShufflePlaylist"));
    }

    // init random number generator
    srand(::GetTickCount());

    // shuffle playlist
    int newPos = 0;
    int nItems = m_listPlaylistContent.GetItemCount();
    for(int index = 0; index < nItems; index++)
    {
        newPos = rand() % nItems;
        m_listPlaylistContent.SwapItems(index, newPos);
    }

    ResetPlaylistSorting();
}


HRESULT CPlaylistCreatorDlg::OnPlaylistItemsDropped(CDroppedItems* pDroppedItems)
{
    if(!pDroppedItems)
    {
        ASSERT(0);
        return S_OK;
    }

    // buffers
    CListCtrl* pListCtrl             = pDroppedItems->pList;
    CPlaylistCreatorDlg* pParentWnd  = (pListCtrl) ? static_cast<CPlaylistCreatorDlg*>(pListCtrl->GetParent()) : NULL;
    CStringArray* pArrayDroppedItems = &pDroppedItems->arrayDroppedItems;

    // check buffers
    if(!pListCtrl || !pParentWnd || !pArrayDroppedItems)
    {
        ASSERT(0);
        return S_OK;
    }

    if(pArrayDroppedItems->IsEmpty()) // no dropped items
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
        return S_OK;
    }

    // update members
    pParentWnd->UpdateData(FROM_DIALOG);

    // if a single playlist file was dropped we do some special handling
    CPlaylistFileManager playlistManager;
    if(pArrayDroppedItems->GetCount() == 1                                      // single item was dropped
        && playlistManager.IsPlaylistFilePath(pArrayDroppedItems->GetAt(0)))    // it's a playlist
    {
        // ask whether to open or insert the playlist
        // custom buttons: (1) open playlist, (2) insert playlist, (3) cancel
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        xmb.dwOptions |= XMSGBOXPARAMS::Narrow; // limit max width
        _tcsncpy_s(xmb.szCustomButtons, _countof(xmb.szCustomButtons), CAppSettings::LoadString(_T("IDS_ASK_OPEN_OR_INSERT_PLAYLIST_CUSTOM_BUTTONS")), _TRUNCATE);
        int nResult = XMessageBox(pParentWnd->GetSafeHwnd(),
                                  CAppSettings::LoadString(_T("IDS_ASK_OPEN_OR_INSERT_PLAYLIST")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_ICONQUESTION,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDCUSTOM3) // cancel
            return S_OK;

        // open or insert?
        if(rc == IDCUSTOM1) // open playlist
        {
            pParentWnd->OpenPlaylist(pArrayDroppedItems->GetAt(0));
            return S_OK;
        }
        //if(rc == IDCUSTOM2) // insert playlist (is handled below)
    }

    // note: we can be at this point in various situations
    //       (1) more than one item was dropped
    //       (2) a single item was dropped but it's not a playlist
    //       (3) a single item was dropped, it's a playlist and the
    //           user wants to insert (not open) the playlist

    // add dropped files to playlist
    pParentWnd->AddToPlaylist(*pArrayDroppedItems, pDroppedItems->iDropIndex, pDroppedItems->bRecursive);


    #pragma message(Reminder "OnPlaylistItemsDropped(): Check if it makes sense to make a copy of the dropped items array for the automatisms")
    // pragma: base folder calculation will be much faster (folders aren't populated yet)
    // pragma: when dropping a playlist the wrong folder can be set (playlist file is filtered while adding the files)

    // automatisms
    pParentWnd->AutoPlaylistFolder(*pArrayDroppedItems);
    pParentWnd->AutoPlaylistName(*pArrayDroppedItems);

    return S_OK;
}


void CPlaylistCreatorDlg::FlashEdit(CEdit* pEdit, COLORREF color)
{
    if(!pEdit)
    {
        ASSERT(0);
        return;
    }

    // check if there is flashing in process
    DWORD dwTimeStart = timeGetTime();
    while(m_pEditToFlash)
    {
        WaitAndCheckMessages(FLASH_DURATION_MSEC / 10);
        DWORD dwTimeNow = timeGetTime();
        if(dwTimeNow > dwTimeStart + FLASH_DURATION_MSEC + 100)
        {
            break;
        }
    }

    // set new edit control to flash
    m_pEditToFlash = pEdit;

    // (re)create brush
    if(m_brushFlash.m_hObject)
    {
        m_brushFlash.DeleteObject();
    }
    m_brushFlash.CreateSolidBrush(color);

    // launch timer for flashing
    SetTimer(TIMER_FLASH_EDIT_ON, 0, NULL);
}


void CPlaylistCreatorDlg::OnOK() 
{
}


void CPlaylistCreatorDlg::OnCancel() 
{
}


BOOL CPlaylistCreatorDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN)
    {
        switch(pMsg->wParam)
        {
            case VK_HOME:
                {
                    // [Ctrl + Home]: move selected items to top
                    if(IsKeyPressed(VK_CONTROL))
                    {
                        OnMoveSelectedItemsToTop();
                        return TRUE;
                    }
                }
                break;

            case VK_UP:
                {
                    // [Ctrl + Up]: move selected items up
                    if(IsKeyPressed(VK_CONTROL))
                    {
                        OnMoveSelectedItemsUp();
                        return TRUE;
                    }
                }
                break;

            case VK_DOWN:
                {
                    // [Ctrl + Down]: move selected items down
                    if(IsKeyPressed(VK_CONTROL))
                    {
                        OnMoveSelectedItemsDown();
                        return TRUE;
                    }
                }
                break;

            case VK_END:
                {
                    // [Ctrl + End]: move selected items to bottom
                    if(IsKeyPressed(VK_CONTROL))
                    {
                        OnMoveSelectedItemsToBottom();
                        return TRUE;
                    }
                }
                break;

            case VK_DELETE:
                {
                    // [Ctrl + Shift + Delete]: delete all items
                    if(IsKeyPressed(VK_CONTROL) && (IsKeyPressed(VK_LSHIFT) || IsKeyPressed(VK_RSHIFT)))
                    {
                        OnDeleteAllItems();
                        return TRUE;
                    }

                    // [Ctrl + Delete]: delete selected items
                    if(IsKeyPressed(VK_CONTROL))
                    {
                        OnDeleteSelectedItems();
                        return TRUE;
                    }
                }
                break;

            case VK_F5:
                {
                    // [F5]: refresh item state
                    RefreshItemState();
                }
                break;

            default:
                {
                    if(IsKeyPressed(VK_CONTROL))
                    {
                        // [Ctrl + A]: select all items
                        if(IsKeyPressed('A') || IsKeyPressed('a'))
                        {
                            SelectAllItems();
                            return TRUE;
                        }

                        // [Ctrl + D]: deselect all items
                        if(IsKeyPressed('D') || IsKeyPressed('d'))
                        {
                            DeselectAllItems();
                            return TRUE;
                        }

                        // [Ctrl + O]: add folder
                        if(IsKeyPressed('O') || IsKeyPressed('o'))
                        {
                            OnAddFolder();
                            return TRUE;
                        }

                        // [Ctrl + F]: add files
                        if(IsKeyPressed('F') || IsKeyPressed('f'))
                        {
                            OnAddFiles();
                            return TRUE;
                        }

                        // [Ctrl + P]: add playlists
                        if(IsKeyPressed('P') || IsKeyPressed('p'))
                        {
                            OnAddPlaylists();
                            return TRUE;
                        }
                    }
                }
                break;
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}


void CPlaylistCreatorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlaylistCreatorDlg::OnPaint() 
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}


void CPlaylistCreatorDlg::OnToolBarNewPlaylist()
{
    // ask whether to create a new playlist
    XMSGBOXPARAMS xmb;
    CAppSettings::GetDefaultXMessageBoxParams(xmb);
    int nResult = XMessageBox(m_hWnd,
                              CAppSettings::LoadString(_T("IDS_ASK_CREATE_NEW_PLAYLIST")),
                              CAppSettings::GetWindowTitle(),
                              MB_YESNO | MB_ICONQUESTION,
                              &xmb);

    int rc = nResult & 0xFFFF;
    if(rc == IDNO) // hell no!
        return;

    // clear playlist and name
    ClearPlaylist();
    ClearPlaylistName();
}


void CPlaylistCreatorDlg::OnToolBarOpenPlaylist()
{
    // tell user that current playlist will be discarded
    if(!CAppSettings::GetDontShowDialog(_T("Ask_OpenPlaylist")))
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        int nResult = XMessageBox(m_hWnd,
                                  CAppSettings::LoadString(_T("IDS_ASK_OPEN_PLAYLIST")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_YESNO | MB_ICONQUESTION | MB_DONOTASKAGAIN,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDNO)
            return;

        if(nResult & MB_DONOTASKAGAIN)
            CAppSettings::SetDontShowDialog(_T("Ask_OpenPlaylist"));
    }

    // open file dialog: select playlist

    // assemble strings
    CString strTitle      = CAppSettings::GetWindowTitle() + _T(" - ") + CAppSettings::LoadString(_T("IDS_SELECT_PLAYLIST"));
    CString strPrevFolder = CAppSettings::GetStringProperty(PROP_OPENPLAYLISTFOLDER);
    CString strPrevFilter = CAppSettings::GetStringProperty(PROP_OPENPLAYLISTFILTER, _T("*.*"));

    // get supported playlist file types
    CStringArray aPlaylistFileTypes;
    CPlaylistFileManager playlistManager;
    playlistManager.GetSupportedFileTypes(aPlaylistFileTypes);

    // assemble file filter
    int nFilterIndex = 1;
    CString strFileTypesFilter;
    for(int i = 0; i < aPlaylistFileTypes.GetCount(); i++)
    {
        CString strType = aPlaylistFileTypes.GetAt(i);

        if(strType == strPrevFilter)
        {
            nFilterIndex = i + 1;
        }

        CString strFilter;
        strFilter.Format(_T("%s %s (*.%s)|*.%s|"),
                         oddgravity::MakeUpper(strType),
                         CAppSettings::LoadString(_T("IDS_PLAYLIST_FILES")),
                         oddgravity::MakeLower(strType),
                         oddgravity::MakeLower(strType));

        strFileTypesFilter += strFilter;
    }
    strFileTypesFilter += CAppSettings::LoadString(_T("IDS_ALL_FILES")) + _T(" (*.*)|*.*|");
    strFileTypesFilter += _T("|"); // add end delimiter

    if(strPrevFilter == _T("*.*"))
    {
        nFilterIndex = int(aPlaylistFileTypes.GetCount()) + 1;
    }

    // show PJ Arends buffer-safe-multi-select open file dialog
    DWORD dwFlags = OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                    OFN_HIDEREADONLY | OFN_EXPLORER;

    CFECFileDialog dlg(TRUE, NULL, NULL, dwFlags, strFileTypesFilter, this, 0 /*dwSize*/, FALSE /*bVistaStyle*/);

    dlg.m_ofn.lpstrTitle      = strTitle;
    dlg.m_ofn.lpstrInitialDir = strPrevFolder;
    dlg.m_ofn.nFilterIndex    = nFilterIndex;

    if(dlg.DoModal() == IDOK)
    {
        // get all selected files
        CStringArray aFileNames;
        POSITION pos = dlg.GetStartPosition();
        while(pos != NULL)
        {
            CString strFileName = dlg.GetNextPathName(pos);
            aFileNames.Add(strFileName);
        }

        // get selected folder
        CString strSelectedFolder = PathRemoveTrailingBackSlash(GetFolderComponent(aFileNames.GetAt(0)));

        // get playlist (currently only single-selection!)
        CString strPlaylistFilePath = aFileNames.GetAt(0);
        if(!playlistManager.IsPlaylistFilePath(strPlaylistFilePath))
        {
            PlaySystemSound(DEFAULT_SYSTEM_SOUND);
            return;
        }

        // open and import playlist
        OpenPlaylist(strPlaylistFilePath);

        // write selected folder to registry
        CAppSettings::SetStringProperty(PROP_OPENPLAYLISTFOLDER, strSelectedFolder);

        // write selected filter to registry
        CString strSelectedFilter;
        int nSelectedFilterIndex = dlg.m_ofn.nFilterIndex;
        if(nSelectedFilterIndex > aPlaylistFileTypes.GetCount())
        {
            strSelectedFilter = _T("*.*");
        }
        else
        {
            strSelectedFilter = aPlaylistFileTypes.GetAt(nSelectedFilterIndex - 1);
        }
        CAppSettings::SetStringProperty(PROP_OPENPLAYLISTFILTER, strSelectedFilter);
    }
}


void CPlaylistCreatorDlg::OnToolBarSettings()
{
    CString strCaption = CAppSettings::GetWindowTitle() + _T(" - ") + CAppSettings::LoadString(_T("IDS_PROPSHEET_SETTINGS_TITLE"));
    CPropSheetSettings dlg(strCaption);
    dlg.SetIsResizable(false);
    dlg.SetTaskbarListProvider(this);
    dlg.DoModal();

    // reset taskbar
    SetProgressState(TBPF_NOPROGRESS);

    UpdateFromRegistry();

    if(dlg.IsRestartNeeded())
    {
        CString strMessage = ReplaceTags(
            CAppSettings::LoadString(_T("IDS_SETTINGS_RESTART_NEEDED")),
            _T("APPLICATION"),
            CAppSettings::GetAppName() + _T(" ") + CAppSettings::GetVersionString(PRODUCT_VERSION_MAJOR));

        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd, strMessage, CAppSettings::GetWindowTitle(), MB_OK | MB_ICONINFORMATION, &xmb);
    }
}


void CPlaylistCreatorDlg::OnToolBarHelp()
{
    OnHelpInfo(NULL);
}


void CPlaylistCreatorDlg::OnToolBarAbout()
{
    CAboutDlg dlg;
    dlg.DoModal();
}


void CPlaylistCreatorDlg::OnClose()
{
    ClearPlaylist();
    UpdateToRegistry();

    EndDialog(IDOK);

    // don't call default handler, because OnCancel would be
    // called which is empty and the dialog would stay open
    //CDialog::OnClose();
}


void CPlaylistCreatorDlg::OnAddFolder()
{
    // do we have active file types?
    if(m_listActiveFileTypes.IsEmpty())
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_NO_ACTIVE_FILE_TYPES")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
        return;
    }

    // assemble strings
    CString strCaption  = CAppSettings::GetWindowTitle() + _T(" - ") + CAppSettings::LoadString(_T("IDS_SELECT_FOLDER_TITLE"));
    CString strTitle    = CAppSettings::LoadString(_T("IDS_SELECT_SONG_FOLDER_INFO"));
    CString strInitialFolder = CAppSettings::GetStringProperty(PROP_LASTSONGFOLDER, _T(""));

    // select folder dialog
    CFolderDialogEx dlg(this);
    dlg.RemoveContextHelpButton();

    // init recursive checkbox
    BOOL bRecursive = (BOOL)CAppSettings::GetIntProperty(PROP_FOLDERDLG_ADDRECURSIVE, (int)FALSE);
    dlg.ShowCheckbox(CAppSettings::LoadString(_T("IDS_FOLDERDLG_ADD_RECURSIVE")), bRecursive);

    LPCITEMIDLIST pidl = dlg.BrowseForFolder(strCaption,
                                             strTitle,
                                             strInitialFolder,
                                             BIF_USENEWUI | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS,
                                             NULL,
                                             TRUE);

    // folder selected?
    if(pidl != NULL)
    {
        // get selected folder
        CString strSelectedFolder = CFolderDialog::GetPathName(pidl);

        // get state of recursive checkbox
        bRecursive = dlg.IsCheckboxChecked();

        // add folder to playlist
        AddToPlaylist(strSelectedFolder, -1, (bRecursive == TRUE));

        // store selected folder and recursive checkbox state
        CAppSettings::SetStringProperty(PROP_LASTSONGFOLDER, strSelectedFolder);
        CAppSettings::SetIntProperty(PROP_FOLDERDLG_ADDRECURSIVE, (int)bRecursive);

        // automatisms
        AutoPlaylistFolder(strSelectedFolder);
        AutoPlaylistName(strSelectedFolder);

        // free PIDL when done (important)
        dlg.FreePIDL(pidl);
    }
}


void CPlaylistCreatorDlg::OnAddFiles()
{
    // do we have active file types?
    if(m_listActiveFileTypes.IsEmpty())
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        XMessageBox(m_hWnd,
                    CAppSettings::LoadString(_T("IDS_MB_ERR_NO_ACTIVE_FILE_TYPES")),
                    CAppSettings::GetWindowTitle(),
                    MB_OK | MB_ICONINFORMATION,
                    &xmb);
        return;
    }

    // assemble strings
    CString strTitle      = CAppSettings::GetWindowTitle() + _T(" - ") + CAppSettings::LoadString(_T("IDS_SELECT_FILES_TITLE"));
    CString strPrevFolder = CAppSettings::GetStringProperty(PROP_ADDFILESFOLDER);
    CString strPrevFilter = CAppSettings::GetStringProperty(PROP_ADDFILESFILTER, _T("*.*"));

    ////////////////////////////////////////////////////////////
    // assemble file filter

    // get active file types
    CStringArray aFileTypes;
    aFileTypes.Append(m_listActiveFileTypes);

    // build the filter string
    int nFilterIndex = 1;
    CString strFileTypesFilter;
    for(int i = 0; i < aFileTypes.GetCount(); i++)
    {
        CString strType = aFileTypes.GetAt(i);

        if(strType == strPrevFilter)
        {
            nFilterIndex = i + 1;
        }

        CString strFilter;
        strFilter.Format(_T("%s %s (*.%s)|*.%s|"),
                         oddgravity::MakeUpper(strType),
                         CAppSettings::LoadString(_T("IDS_FILES")),
                         oddgravity::MakeLower(strType),
                         oddgravity::MakeLower(strType));

        strFileTypesFilter += strFilter;
    }

    // finalize filter string (all files, end delimiter)
    strFileTypesFilter += CAppSettings::LoadString(_T("IDS_ALL_FILES")) + _T(" (*.*)|*.*|");
    strFileTypesFilter += _T("|");

    if(strPrevFilter == _T("*.*"))
    {
        nFilterIndex = int(aFileTypes.GetCount()) + 1;
    }


    ////////////////////////////////////////////////////////////
    // show PJ Arends buffer-safe-multi-select open file dialog
    DWORD dwFlags = OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_PATHMUSTEXIST |
                    OFN_FILEMUSTEXIST   | OFN_HIDEREADONLY     | OFN_EXPLORER;

    CFECFileDialog dlg(TRUE, NULL, NULL, dwFlags, strFileTypesFilter, this, 0 /*dwSize*/, FALSE /*bVistaStyle*/);

    dlg.m_ofn.lpstrTitle      = strTitle;
    dlg.m_ofn.lpstrInitialDir = strPrevFolder;
    dlg.m_ofn.nFilterIndex    = nFilterIndex;

    if(dlg.DoModal() == IDOK)
    {
        // get all selected files
        CStringArray aFileNames;
        POSITION pos = dlg.GetStartPosition();
        while(pos != NULL)
        {
            CString strFileName = dlg.GetNextPathName(pos);
            aFileNames.Add(strFileName);
        }

        // get selected folder
        CString strSelectedFolder = PathRemoveTrailingBackSlash(GetFolderComponent(aFileNames.GetAt(0)));

        // add files to playlist
        AddToPlaylist(aFileNames, -1, false);

        // write selected folder to registry
        CAppSettings::SetStringProperty(PROP_ADDFILESFOLDER, strSelectedFolder);

        // write selected filter to registry
        CString strSelectedFilter;
        int nSelectedFilterIndex = dlg.m_ofn.nFilterIndex;
        if(nSelectedFilterIndex > aFileTypes.GetCount())
        {
            strSelectedFilter = _T("*.*");
        }
        else
        {
            strSelectedFilter = aFileTypes.GetAt(nSelectedFilterIndex - 1);
        }
        CAppSettings::SetStringProperty(PROP_ADDFILESFILTER, strSelectedFilter);
    }
}


void CPlaylistCreatorDlg::OnAddPlaylists()
{
    // assemble strings
    CString strTitle      = CAppSettings::GetWindowTitle() + _T(" - ") + CAppSettings::LoadString(_T("IDS_SELECT_PLAYLISTS_TITLE"));
    CString strPrevFolder = CAppSettings::GetStringProperty(PROP_ADDPLAYLISTFOLDER);
    CString strPrevFilter = CAppSettings::GetStringProperty(PROP_ADDPLAYLISTFILTER, _T("*.*"));

    ////////////////////////////////////////////////////////////
    // assemble file filter

    // buffers
    CPlaylistFileManager playlistManager;

    // get playlist file types
    CStringArray aFileTypes;
    playlistManager.GetSupportedFileTypes(aFileTypes);

    // build the filter string
    int nFilterIndex = 1;
    CString strFileTypesFilter;
    for(int i = 0; i < aFileTypes.GetCount(); i++)
    {
        CString strType = aFileTypes.GetAt(i);

        if(strType == strPrevFilter)
        {
            nFilterIndex = i + 1;
        }

        CString strFilter;
        strFilter.Format(_T("%s %s (*.%s)|*.%s|"),
                         oddgravity::MakeUpper(strType),
                         CAppSettings::LoadString(_T("IDS_PLAYLIST_FILES")),
                         oddgravity::MakeLower(strType),
                         oddgravity::MakeLower(strType));

        strFileTypesFilter += strFilter;
    }

    // finalize filter string (all files, end delimiter)
    strFileTypesFilter += CAppSettings::LoadString(_T("IDS_ALL_FILES")) + _T(" (*.*)|*.*|");
    strFileTypesFilter += _T("|");

    if(strPrevFilter == _T("*.*"))
    {
        nFilterIndex = int(aFileTypes.GetCount()) + 1;
    }

    ////////////////////////////////////////////////////////////
    // show PJ Arends buffer-safe-multi-select open file dialog

    DWORD dwFlags = OFN_OVERWRITEPROMPT | OFN_ENABLESIZING | OFN_PATHMUSTEXIST |
                    OFN_FILEMUSTEXIST   | OFN_HIDEREADONLY | OFN_EXPLORER;

    CFECFileDialog dlg(TRUE, NULL, NULL, dwFlags, strFileTypesFilter, this, 0 /*dwSize*/, FALSE /*bVistaStyle*/);

    dlg.m_ofn.lpstrTitle      = strTitle;
    dlg.m_ofn.lpstrInitialDir = strPrevFolder;
    dlg.m_ofn.nFilterIndex    = nFilterIndex;

    if(dlg.DoModal() == IDOK)
    {
        // get all selected files
        CStringArray aFileNames;
        POSITION pos = dlg.GetStartPosition();
        while(pos != NULL)
        {
            CString strFileName = dlg.GetNextPathName(pos);
            aFileNames.Add(strFileName);
        }

        // get selected folder
        CString strSelectedFolder = PathRemoveTrailingBackSlash(GetFolderComponent(aFileNames.GetAt(0)));

        // verify that a playlist was selected (currently only single-selection!)
        CString strPlaylistFilePath = aFileNames.GetAt(0);
        if(!playlistManager.IsPlaylistFilePath(strPlaylistFilePath))
        {
            PlaySystemSound(DEFAULT_SYSTEM_SOUND);
            return;
        }

        // import and insert playlist
        AddToPlaylist(strPlaylistFilePath, -1, false);

        // write selected folder to registry
        CAppSettings::SetStringProperty(PROP_ADDPLAYLISTFOLDER, strSelectedFolder);

        // write selected filter to registry
        CString strSelectedFilter;
        int nSelectedFilterIndex = dlg.m_ofn.nFilterIndex;
        if(nSelectedFilterIndex > aFileTypes.GetCount())
        {
            strSelectedFilter = _T("*.*");
        }
        else
        {
            strSelectedFilter = aFileTypes.GetAt(nSelectedFilterIndex - 1);
        }
        CAppSettings::SetStringProperty(PROP_ADDPLAYLISTFILTER, strSelectedFilter);
    }
}


void CPlaylistCreatorDlg::OnSelectPlaylistSaveLocation()
{
    UpdateData(FROM_DIALOG);

    // dialog properties
    CString strCaption  = CAppSettings::GetWindowTitle() + _T(" - ") + CAppSettings::LoadString(_T("IDS_SELECT_FOLDER_TITLE"));
    CString strTitle    = CAppSettings::LoadString(_T("IDS_SELECT_PLAYLIST_FOLDER_INFO"));
    CString strInitialFolder = CAppSettings::GetStringProperty(PROP_LASTPLAYLISTFOLDER, _T(""));

    // select folder dialog
    CFolderDialogEx dlg(this);
    dlg.RemoveContextHelpButton();

    LPCITEMIDLIST pidl = dlg.BrowseForFolder(strCaption,
                                             strTitle,
                                             strInitialFolder,
                                             BIF_USENEWUI | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS,
                                             NULL,
                                             TRUE);

    // folder selected?
    if(pidl != NULL)
    {
        SetPlaylistSaveLocation(CFolderDialog::GetPathName(pidl));

        // free PIDL when done (important)
        dlg.FreePIDL(pidl);
    }
}


// moves selected items to top
void CPlaylistCreatorDlg::OnMoveSelectedItemsToTop()
{
    // try to move the selected items to top
    if(m_listPlaylistContent.MoveSelectedItemsToTop())
    {
        ListCtrl_EnsureVisibleOffset(m_listPlaylistContent, 0, FALSE, 0);

        ResetPlaylistSorting();
    }
    else
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
    }
}


// moves selected items up
void CPlaylistCreatorDlg::OnMoveSelectedItemsUp()
{
    // try to move the selected items up
    if(m_listPlaylistContent.MoveSelectedItemsUp())
    {
        m_listPlaylistContent.EnsureVisible(m_listPlaylistContent.GetFirstSelectedIndex(), FALSE);
        ListCtrl_EnsureVisibleOffset(m_listPlaylistContent, m_listPlaylistContent.GetFirstSelectedIndex(), FALSE, -4);

        ResetPlaylistSorting();
    }
    else
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
    }
}


// moves selected items down
void CPlaylistCreatorDlg::OnMoveSelectedItemsDown()
{
    // try to move the selected items down
    if(m_listPlaylistContent.MoveSelectedItemsDown())
    {
        m_listPlaylistContent.EnsureVisible(m_listPlaylistContent.GetLastSelectedIndex(), FALSE);
        ListCtrl_EnsureVisibleOffset(m_listPlaylistContent, m_listPlaylistContent.GetLastSelectedIndex(), FALSE, 4);

        ResetPlaylistSorting();
    }
    else
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
    }
}


// moves selected items to bottom
void CPlaylistCreatorDlg::OnMoveSelectedItemsToBottom()
{
    // try to move the selected items to bottom
    if(m_listPlaylistContent.MoveSelectedItemsToBottom())
    {
        ListCtrl_EnsureVisibleOffset(m_listPlaylistContent, m_listPlaylistContent.GetTailIndex(), FALSE, 0);

        ResetPlaylistSorting();
    }
    else
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
    }
}


// deletes the selected items
void CPlaylistCreatorDlg::OnDeleteSelectedItems()
{
    // get selected items
    CItemsArray arraySelectedItems;
    m_listPlaylistContent.GetSelectedItems(&arraySelectedItems);

    // valid selection?
    if(m_listPlaylistContent.IsEmpty() || arraySelectedItems.empty())
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
        return;
    }

    // disable redraw while deleting the selected items (there can be really many of them!)
    m_listPlaylistContent.SetRedraw(FALSE);

    // remove selected items (in reverse order!)
    CItemsArray::reverse_iterator rvIter;
    for(rvIter = arraySelectedItems.rbegin(); rvIter != arraySelectedItems.rend(); rvIter++)
    {
        int nItem = *rvIter;

        CPlaylistElement* pElement = reinterpret_cast<CPlaylistElement*>(m_listPlaylistContent.GetItemData(nItem));
        if(pElement)
        {
            delete pElement;
            pElement = NULL;
        }
        m_listPlaylistContent.DeleteItem(nItem);
    }

    // enable redraw again and force a repaint
    m_listPlaylistContent.SetRedraw(TRUE);
    m_listPlaylistContent.Invalidate();
    m_listPlaylistContent.UpdateWindow();

    // update selection and ensure comfortable visibility
    int nLastSelectedItem = arraySelectedItems.back() - (int)arraySelectedItems.size() + 1;
    int nNewSelectedItem = ListCtrl_RefreshSelectionAfterItemRemoval(m_listPlaylistContent, nLastSelectedItem);
    ListCtrl_EnsureVisibleOffset(m_listPlaylistContent, nNewSelectedItem, FALSE, 4);

    // reset sorting if list is now empty
    if(m_listPlaylistContent.IsEmpty())
        ResetPlaylistSorting();
}


// deletes all items
void CPlaylistCreatorDlg::OnDeleteAllItems()
{
    // check if there are items
    if(m_listPlaylistContent.IsEmpty())
    {
        PlaySystemSound(DEFAULT_SYSTEM_SOUND);
        return;
    }

    // really delete all items?
    if(!CAppSettings::GetDontShowDialog(_T("Ask_DeleteAllItems")))
    {
        XMSGBOXPARAMS xmb;
        CAppSettings::GetDefaultXMessageBoxParams(xmb);
        int nResult = XMessageBox(m_hWnd,
                                  CAppSettings::LoadString(_T("IDS_MB_DELETE_ALL_ITEMS")),
                                  CAppSettings::GetWindowTitle(),
                                  MB_YESNO | MB_ICONEXCLAMATION | MB_DONOTASKAGAIN,
                                  &xmb);

        int rc = nResult & 0xFFFF;
        if(rc == IDNO)
            return;

        if(nResult & MB_DONOTASKAGAIN)
            CAppSettings::SetDontShowDialog(_T("Ask_DeleteAllItems"));
    }

    // delete them all, spare no one!
    ClearPlaylist();
}


void CPlaylistCreatorDlg::OnCreatePlaylist()
{
    UpdateData(FROM_DIALOG);

    ///////////////////////////////////////////////////////
    // check if we are ready for creating the playlist

    CString strMessage = _T("");
    UINT nStyle = MB_OK|MB_ICONINFORMATION;
    UINT nCancelStyle = IDNO;
    CString strDontShowDialog = _T("");

    // buffers
    CString strPlaylistName;
    m_editPlaylistName.GetWindowText(strPlaylistName);

    CString strPlaylistFilePath = PathAddTrailingBackSlash(m_strPlaylistFolder)
                                   + strPlaylistName
                                   + GetPlaylistType();
    CPath pathPlaylist(PathAddTrailingBackSlash(m_strPlaylistFolder), TRUE);
    CFileStatus status;

    if(m_listPlaylistContent.GetItemCount() == 0)
    {
        strMessage = CAppSettings::LoadString(_T("IDS_MB_EMPTY_PLAYLIST"));
    }
    else if(m_strPlaylistFolder.IsEmpty())
    {
        strMessage = CAppSettings::LoadString(_T("IDS_MB_EMPTY_PLAYLIST_FOLDER"));
    }
    else if(strPlaylistName.IsEmpty())
    {
        strMessage = CAppSettings::LoadString(_T("IDS_MB_EMPTY_PLAYLIST_NAME"));
    }
    else if(!IsValidFileName(strPlaylistName))
    {
        strMessage = CAppSettings::LoadString(_T("IDS_MB_INVALID_PLAYLIST_NAME"));
    }
    else if(!pathPlaylist.ExistLocation())
    {
        strMessage = ReplaceTags(
            CAppSettings::LoadString(_T("IDS_MB_PLAYLIST_FOLDER_DOES_NOT_EXIST")),
            _T("FOLDER"), m_strPlaylistFolder);
        nStyle = MB_YESNO | MB_ICONQUESTION | MB_DONOTASKAGAIN;
        nCancelStyle = IDNO;
        strDontShowDialog = _T("Ask_CreatePlaylistFolder");
    }
    else if(CFile::GetStatus(strPlaylistFilePath, status))
    {
        strMessage = ReplaceTags(
            CAppSettings::LoadString(_T("IDS_MB_PLAYLIST_ALREADY_EXISTS")),
            _T("FILEPATH"), strPlaylistFilePath);
        nStyle = MB_YESNO | MB_ICONEXCLAMATION | MB_DONOTASKAGAIN;
        nCancelStyle = IDNO;
        strDontShowDialog = _T("Ask_OverwritePlaylist");
    }

    // something wrong?
    if(strDontShowDialog.IsEmpty()
        || !CAppSettings::GetDontShowDialog(strDontShowDialog))
    {
        if(!strMessage.IsEmpty())
        {
            XMSGBOXPARAMS xmb;
            CAppSettings::GetDefaultXMessageBoxParams(xmb);
            int nResult = XMessageBox(m_hWnd,
                                      strMessage,
                                      CAppSettings::GetWindowTitle(),
                                      nStyle,
                                      &xmb);

            if((nStyle & MB_YESNO) || (nStyle & MB_OKCANCEL))
            {
                int rc = nResult & 0xFFFF;
                if(rc == nCancelStyle)
                {
                    return;
                }
            }
            else
            {
                return;
            }

            if(!strDontShowDialog.IsEmpty()
                && (nResult & MB_DONOTASKAGAIN))
            {
                CAppSettings::SetDontShowDialog(strDontShowDialog);
            }
        }
    }


    ///////////////////////////////////////////////////////
    // create the playlist

    CCreatePlaylistDlg dlg;
    dlg.SetTaskbarListProvider(this);
    INT_PTR iResult = dlg.DoModal();

    // reset taskbar
    SetProgressState(TBPF_NOPROGRESS);

    // playlist successfully created?
    if(iResult == 0)
    {
        AutoClearPlaylist();
    }
}


void CPlaylistCreatorDlg::OnPlaylistItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if(pNMLV->uChanged & LVIF_STATE) // item state changed
    {
        // update item state dependent controls
        KillTimer(TIMER_PLAYLIST_ITEM_STATE_CHANGED);
        SetTimer(TIMER_PLAYLIST_ITEM_STATE_CHANGED, 50, NULL);
    }

    *pResult = 0;
}


BOOL CPlaylistCreatorDlg::OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;
    UINT_PTR nID = pNMHDR->idFrom;

    switch(nID)
    {
        case ID_TB_BTN_NEW_PLAYLIST:
            m_strToolTipText = CAppSettings::LoadString(_T("IDS_TB_BTN_NEW_PLAYLIST"));
            pTTT->lpszText = m_strToolTipText.GetBuffer();
            m_strToolTipText.ReleaseBuffer();
            return TRUE;

        case ID_TB_BTN_OPEN_PLAYLIST:
            m_strToolTipText = CAppSettings::LoadString(_T("IDS_TB_BTN_OPEN_PLAYLIST"));
            pTTT->lpszText = m_strToolTipText.GetBuffer();
            m_strToolTipText.ReleaseBuffer();
            return TRUE;

        default:
            break;
    }

    return FALSE;
}


void CPlaylistCreatorDlg::OnSelChangedPlaylistType()
{
    // update registry
    CAppSettings::SetStringProperty(PROP_PLAYLISTTYPE, GetPlaylistType());
}


void CPlaylistCreatorDlg::OnTimer(UINT nIDEvent)
{
    switch(nIDEvent)
    {
        case TIMER_FLASH_EDIT_ON:
            {
                if(m_pEditToFlash)
                {
                    m_pEditToFlash->Invalidate();
                    KillTimer(TIMER_FLASH_EDIT_ON);
                    SetTimer(TIMER_FLASH_EDIT_OFF, FLASH_DURATION_MSEC, NULL);
                }
            }
            break;

        case TIMER_FLASH_EDIT_OFF:
            {
                if(m_pEditToFlash)
                {
                    CEdit* pEdit = m_pEditToFlash;
                    m_pEditToFlash = NULL;
                    pEdit->Invalidate();
                    pEdit->SendMessage(WM_NCPAINT, 1);
                    if(m_brushFlash.m_hObject)
                    {
                        m_brushFlash.DeleteObject();
                    }
                    KillTimer(TIMER_FLASH_EDIT_OFF);
                }
            }
            break;

        case TIMER_PLAYLIST_ITEM_STATE_CHANGED:
            {
                // kill timer
                KillTimer(TIMER_PLAYLIST_ITEM_STATE_CHANGED);

                // update controls that are dependent on playlist item state
                UpdateItemStateDependentControls();
            }
            break;

        default:
            break;
    }

    CDialog::OnTimer(nIDEvent);
}


void CPlaylistCreatorDlg::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
    CDialog::OnActivateApp(bActive, dwThreadID);

    if(m_bTransparency && m_pSetLayeredWindowAttributes)
    {
        if(bActive)
        {
            m_iAlpha = 255;
            m_pSetLayeredWindowAttributes(m_hWnd, 0, (BYTE)m_iAlpha, LWA_ALPHA);
        }
        else
        {
            while(m_iAlpha > 200)
            {
                m_iAlpha -= 5;
                m_pSetLayeredWindowAttributes(m_hWnd, 0, (BYTE)m_iAlpha, LWA_ALPHA);
                WaitAndCheckMessages(10);
            }
            m_iAlpha = 200;
        }
    }
}


void CPlaylistCreatorDlg::OnNcDestroy()
{
    AfxGetApp()->m_pMainWnd = NULL;

    CDialog::OnNcDestroy();
}


void CPlaylistCreatorDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // playlist
    if(pWnd == &m_listPlaylistContent)
    {
        // check if user clicked on header ctrl
        CHeaderCtrl* pHeaderCtrl = m_listPlaylistContent.GetHeaderCtrl();
        if(pHeaderCtrl)
        {
            CRect rectHeaderCtrl;
            pHeaderCtrl->GetWindowRect(&rectHeaderCtrl);
            if(rectHeaderCtrl.PtInRect(point)) // user clicked on header
            {
                // display context menu for the header ctrl
                // currently not used
                return;
            }
        }

        // get point in client coordinates
        CPoint ptClient(point);
        m_listPlaylistContent.ScreenToClient(&ptClient);

        // get item from point
        int nClickedItem = m_listPlaylistContent.HitTest(ptClient);

        // calculate popup position
        CPoint ptPopup(point);

        // check if point is inside of playlist rect
        CRect rectPlaylistWindow;
        m_listPlaylistContent.GetWindowRect(&rectPlaylistWindow);

        if(!rectPlaylistWindow.PtInRect(ptPopup)) // point is not in playlist rect
        {
            POSITION pos = m_listPlaylistContent.GetFirstSelectedItemPosition();
            int index    = m_listPlaylistContent.GetNextSelectedItem(pos);

            if(index != -1) // there is a selected item
            {
                m_listPlaylistContent.EnsureVisible(index, FALSE);

                CRect rectItem;
                m_listPlaylistContent.GetItemRect(index, &rectItem, LVIR_BOUNDS);
                m_listPlaylistContent.ClientToScreen(&rectItem);

                ptPopup.SetPoint(rectItem.left + ((m_minDlgSizeX * 2) / 10),
                                 rectItem.top  + (rectItem.Height() / 2));
            }
            else // list is empty or no item selected
            {
                ptPopup.SetPoint(rectPlaylistWindow.left + 25, rectPlaylistWindow.top + 25);
            }
        }

        ptPopup.Offset(0, 1); // so context menu can be closed by a simple left click

        // buffer for the selected command
        int nCommand = 0;

        // create popup menu
        BCMenu popupMenu;
        if(popupMenu.CreatePopupMenu())
        {
            // get icon size
            int nIconSize = CAppSettings::GetContextMenuIconSize();

            // init context menu
            popupMenu.SetMenuDrawMode(BCMenu::BCMDM_XP);
            popupMenu.SetXPBitmap3D(FALSE);         // not raised while hovered
            popupMenu.SetXPBitmapDisabled3D(TRUE);  // high quality grayscale
            popupMenu.SetIconSize(nIconSize, nIconSize);

            // create image list for context menu
            int cmIcons[] =
            {
                IDI_GO_TOP,
                IDI_GO_UP,
                IDI_GO_DOWN,
                IDI_GO_BOTTOM,
                IDI_DELETE,
                IDI_RECYCLE_BIN,
                IDI_SHUFFLE,
                IDI_REFRESH,
                IDI_SAVE,
            };

            CImageList ilPlaylistContextMenu;
            ilPlaylistContextMenu.Create(nIconSize, nIconSize, ILC_COLOR32 | ILC_MASK, _countof(cmIcons), 1);
            for(int index = 0; index < _countof(cmIcons); index++)
            {
                HICON hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(cmIcons[index]), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR | LR_SHARED);
                ilPlaylistContextMenu.Add(hIcon);
            }

            // get info about current playlist
            bool bIsPlaylistEmpty       = m_listPlaylistContent.IsEmpty();
            bool bIsItemSelected        = (m_listPlaylistContent.GetFirstSelectedIndex() >= 0) ? true : false;
            bool bIsFirstItemSelected   = m_listPlaylistContent.IsItemSelected(0);
            bool bIsLastItemSelected    = m_listPlaylistContent.IsItemSelected(m_listPlaylistContent.GetTailIndex());

            // assemble context menu

            UINT nFlags = MF_STRING;

            // command: move to top
            nFlags = MF_STRING | ((bIsItemSelected && !bIsFirstItemSelected) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_MOVE_TO_TOP, CAppSettings::LoadString(_T("IDS_POPUP_MOVE_TO_TOP")),
                                 &ilPlaylistContextMenu, 0);

            // command: move up
            nFlags = MF_STRING | ((bIsItemSelected && !bIsFirstItemSelected) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_MOVE_UP, CAppSettings::LoadString(_T("IDS_POPUP_MOVE_UP")),
                                 &ilPlaylistContextMenu, 1);

            // command: move down
            nFlags = MF_STRING | ((bIsItemSelected && !bIsLastItemSelected) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_MOVE_DOWN, CAppSettings::LoadString(_T("IDS_POPUP_MOVE_DOWN")),
                                 &ilPlaylistContextMenu, 2);

            // command: move to bottom
            nFlags = MF_STRING | ((bIsItemSelected && !bIsLastItemSelected) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_MOVE_TO_BOTTOM, CAppSettings::LoadString(_T("IDS_POPUP_MOVE_TO_BOTTOM")),
                                 &ilPlaylistContextMenu, 3);

            // separator
            popupMenu.AppendMenu(MF_SEPARATOR);

            // command: delete selected
            nFlags = MF_STRING | ((!bIsPlaylistEmpty && bIsItemSelected) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_DELETE_SELECTED, CAppSettings::LoadString(_T("IDS_POPUP_DELETE_SELECTED")),
                                 &ilPlaylistContextMenu, 4);

            // command: delete all
            nFlags = MF_STRING | ((!bIsPlaylistEmpty) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_DELETE_ALL, CAppSettings::LoadString(_T("IDS_POPUP_DELETE_ALL")),
                                 &ilPlaylistContextMenu, 5);

            // separator
            popupMenu.AppendMenu(MF_SEPARATOR);

            // command: shuffle playlist
            nFlags = MF_STRING | ((!bIsPlaylistEmpty) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_SHUFFLE_PLAYLIST, CAppSettings::LoadString(_T("IDS_POPUP_SHUFFLE_PLAYLIST")),
                                 &ilPlaylistContextMenu, 6);

            // separator
            popupMenu.AppendMenu(MF_SEPARATOR);

            // command: refresh item state
            nFlags = MF_STRING | ((!bIsPlaylistEmpty && m_bHighlightMissingItems) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_REFRESH_ITEM_STATE, CAppSettings::LoadString(_T("IDS_POPUP_REFRESH_ITEM_STATE")),
                                 &ilPlaylistContextMenu, 7);

            // separator
            popupMenu.AppendMenu(MF_SEPARATOR);

            // command: use folder as playlist save location
            nFlags = MF_STRING | ((bIsItemSelected) ? MF_ENABLED : MF_GRAYED);
            popupMenu.AppendMenu(nFlags, CMID_SET_PLAYLIST_SAVE_LOCATION, CAppSettings::LoadString(_T("IDS_POPUP_SET_PLAYLIST_SAVE_LOCATION")),
                                 &ilPlaylistContextMenu, 8);

            // display popup menu
            nCommand = popupMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RETURNCMD,
                                                  ptPopup.x, ptPopup.y,
                                                  this, NULL);

            // clean up
            popupMenu.DestroyMenu();
        }
        else
        {
            ASSERT(0); // error: couldn't create popup menu
        }

        // process command
        switch(nCommand)
        {
            case CMID_MOVE_TO_TOP:
                OnMoveSelectedItemsToTop();
                break;

            case CMID_MOVE_UP:
                OnMoveSelectedItemsUp();
                break;

            case CMID_MOVE_DOWN:
                OnMoveSelectedItemsDown();
                break;

            case CMID_MOVE_TO_BOTTOM:
                OnMoveSelectedItemsToBottom();
                break;

            case CMID_DELETE_SELECTED:
                OnDeleteSelectedItems();
                break;

            case CMID_DELETE_ALL:
                OnDeleteAllItems();
                break;

            case CMID_REFRESH_ITEM_STATE:
                RefreshItemState();
                break;

            case CMID_SET_PLAYLIST_SAVE_LOCATION:
                SetPlaylistSaveLocationFromItem(
                    (nClickedItem == -1) ? m_listPlaylistContent.GetFirstSelectedIndex() : nClickedItem);
                break;

            case CMID_SHUFFLE_PLAYLIST:
                ShufflePlaylist();
                break;

            case 0:
            default:
                // e.g. user canceled
                break;
        }

        return;
    }

    // default processing
    CDialog::OnContextMenu(pWnd, point);
}


void CPlaylistCreatorDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    UPDATE_EASYSIZE;

    UpdateToolBar();
    UpdateStatusBar();
}


void CPlaylistCreatorDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
    CDialog::OnSizing(fwSide, pRect);
    EASYSIZE_MINSIZE(m_minDlgSizeX, m_minDlgSizeY, fwSide, pRect);
}


void CPlaylistCreatorDlg::OnSysColorChange()
{
    CDialog::OnSysColorChange();

    InitColors();
    InitThemeDependentControls();

    Invalidate();
}


HBRUSH CPlaylistCreatorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = NULL;

    if(m_pEditToFlash && m_pEditToFlash->GetDlgCtrlID() == pWnd->GetDlgCtrlID()) // edit to flash found
    {
        if(m_brushFlash.m_hObject)
        {
            pDC->SetBkMode(OPAQUE);
            pDC->SetBkColor(m_DefaultHighlightColor);
            hbr = m_brushFlash;
        }
    }

    if(hbr == NULL) // do default processing
    {
        hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    }

    return hbr;
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlaylistCreatorDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


BOOL CPlaylistCreatorDlg::OnEraseBkgnd(CDC* pDC)
{
    // toolbar
    if(m_pReBar)
    {
        m_pReBar->SetRedraw(FALSE);
        GetDlgItem(IDC_REBAR_EDGE)->SetRedraw(FALSE);

        RECT rect;
        m_pReBar->GetWindowRect(&rect);
        ScreenToClient(&rect);
        pDC->ExcludeClipRect(&rect);
        EASYSIZE_EXCLUDE_ERASEBKGND(IDC_REBAR_EDGE);

        m_pReBar->SetRedraw(TRUE);
        GetDlgItem(IDC_REBAR_EDGE)->SetRedraw(TRUE);
    }

    // group: playlist content
    //EASYSIZE_EXCLUDE_ERASEBKGND(IDS_GB_PLAYLIST_CONTENT);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDS_PLAYLIST_CONTENT_INFO);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_LIST_PLAYLIST_CONTENT);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_ADD_FOLDER);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_ADD_FILES);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_ADD_PLAYLISTS);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_MOVE_SEL_ITEMS_TOP);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_MOVE_SEL_ITEMS_UP);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_MOVE_SEL_ITEMS_DOWN);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_MOVE_SEL_ITEMS_BOTTOM);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_DELETE_SEL_ITEMS);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_DELETE_ALL_ITEMS);

    // group: create playlist
    //EASYSIZE_EXCLUDE_ERASEBKGND(IDS_GB_PLAYLIST_CREATION);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDS_PLAYLIST_SAVE_LOCATION);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_EDIT_PLAYLIST_FOLDER);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_SELECT_PLAYLIST_SAVE_LOCATION);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDS_PLAYLIST_NAME_AND_TYPE);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_EDIT_PLAYLIST_NAME);
    EASYSIZE_EXCLUDE_ERASEBKGND(IDC_COMBO_PLAYLIST_TYPES);
    // %TODO%: black border around button when excluding from clip rect
    //EASYSIZE_EXCLUDE_ERASEBKGND(IDC_BTN_CREATE_PLAYLIST);

    // status bar
    // %TODO%: no border redraw when excluding from clip rect
    //CRect rect;
    //m_pStatusBar->GetWindowRect(&rect);
    //rect.bottom -= 1;
    //ScreenToClient(&rect);
    //pDC->ExcludeClipRect(&rect);

    return CDialog::OnEraseBkgnd(pDC);
}


BOOL CPlaylistCreatorDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    CAppSettings::OnHelp();

    return TRUE;
    //return CDialog::OnHelpInfo(pHelpInfo);
}


LRESULT CPlaylistCreatorDlg::OnSingleInstance(WPARAM wp, LPARAM lp)
{
    return UWM_SINGLE_INSTANCE;
}


LRESULT CPlaylistCreatorDlg::OnAutoUpdateCheck(WPARAM wp, LPARAM lp)
{
    // only continue if auto update check is enabled
    if(!CAppSettings::GetIntProperty(PROP_AUTOUPDATECHECK, (int)TRUE))
        return S_OK;

    // perform update check only if the last check is more than a week ago
    time_t timeLastCheck = CAppSettings::GetLastUpdateCheckTimestamp();
    if(timeLastCheck == 0) // timestamp not available (must be the first start)
    {
        // initialize timestamp of last update check
        CAppSettings::UpdateLastUpdateCheckTimestamp();
    }
    else // timestamp is available
    {
        // get current time
        time_t timeNow;
        time(&timeNow);

        // calculate time difference from current time to last update check
        double secondsElapsed = difftime(timeNow /*end*/, timeLastCheck /*start*/);
        if(secondsElapsed > 0.0)
        {
            double daysElapsed = secondsElapsed / (60 * 60 * 24);
            if(daysElapsed > 7.0) // last update check is more than a week ago
            {
                // perform update check
                CCheckForUpdatesDlg dlg;
                dlg.SetTaskbarListProvider(this);
                dlg.DoModal();

                // reset taskbar
                SetProgressState(TBPF_NOPROGRESS);
            }
        }
    }

    return S_OK;
}


LRESULT CPlaylistCreatorDlg::OnFillPlaylist(WPARAM wp, LPARAM lp)
{
    CPlaylist* pPlaylist = reinterpret_cast<CPlaylist*>(wp);
    if(!pPlaylist)
    {
        ASSERT(0);
        return S_FALSE;
    }

    // buffers
    CString strPlaylistName;
    m_editPlaylistName.GetWindowText(strPlaylistName);

    // set playlist details
    pPlaylist->SetFolder(m_strPlaylistFolder);
    pPlaylist->SetName(strPlaylistName);
    pPlaylist->SetTypeAsString(GetPlaylistType());

    // assemble playlist
    int nItems = m_listPlaylistContent.GetItemCount();                  ASSERT(nItems > 0);
    for(int item = 0; item < nItems; item++)
    {
        CPlaylistElement* pElement = reinterpret_cast<CPlaylistElement*>(m_listPlaylistContent.GetItemData(item));
        if(!pElement)
        {
            ASSERT(0);
            continue;
        }
        pPlaylist->Add(pElement);
    }

    return S_OK;
}


LRESULT CPlaylistCreatorDlg::OnSortPlaylistColumn(WPARAM wp, LPARAM lp)
{
    // no sorting when playlist is empty
    if(m_listPlaylistContent.IsEmpty())
        return S_FALSE;

    // get new sort column
    int nColumn = static_cast<int>(wp);

    // get list header
    CHeaderCtrl* pHeader = m_listPlaylistContent.GetHeaderCtrl();
    if(!pHeader)
    {
        ASSERT(0); // error: no header control
        return S_FALSE;
    }

    // verify sort params
    if(nColumn < 0 || nColumn >= pHeader->GetItemCount())
    {
        ASSERT(0); // error: invalid column
        return S_FALSE; 
    }

    // update sort params
    if(nColumn == m_nSortColumn) // sort by same column
    {
        // reverse sort order
        m_bSortAscending = !m_bSortAscending;
    }
    else // sort by another column
    {
        // init sort order
        m_bSortAscending = TRUE;

        // update sort column
        m_nSortColumn = nColumn;
    }

    // perform list sort
    ListCtrlCompareInfo lcci;
    lcci.pList      = &m_listPlaylistContent;
    lcci.nColumn    = m_nSortColumn;
    lcci.bAscending = m_bSortAscending;
    ListView_SortItemsEx(m_listPlaylistContent.GetSafeHwnd(), ListCtrlAlphabeticalCompare, (LPARAM)&lcci);

    // update GUI
    ListView_SetHeaderSortArrow(m_listPlaylistContent.GetSafeHwnd(), m_nSortColumn, m_bSortAscending, IDB_SORT_UP_LEGACY, IDB_SORT_DOWN_LEGACY);
    UpdateItemStateDependentControls();

    return S_OK;
}


LRESULT CPlaylistCreatorDlg::OnPlaylistItemToolTipNeedText(WPARAM wp, LPARAM lp)
{
    // no tooltips when list is empty
    if(m_listPlaylistContent.IsEmpty())
        return S_FALSE;

    // get item tooltip info
    CReportListCtrlItemToolTip* pItemToolTip = reinterpret_cast<CReportListCtrlItemToolTip*>(wp);
    if(!pItemToolTip)
    {
        ASSERT(0);
        return S_FALSE;
    }

    // get the playlist item
    CPlaylistElement* pElement = reinterpret_cast<CPlaylistElement*>(m_listPlaylistContent.GetItemData(pItemToolTip->nItem));
    if(!pElement)
    {
        ASSERT(0);
        return S_FALSE;
    }

    // display tooltip if item is nonexistant on the volume
    bool bExists = IsFile(pElement->GetFilePath());

    if(!bExists && m_bHighlightMissingItems)
        pItemToolTip->strTooltipText = CAppSettings::LoadString(_T("IDS_TT_ITEM_DOESNT_EXIST"));

    return S_OK;
}


LRESULT CPlaylistCreatorDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    // perform snap to screen border, if activated
    if(m_bSnapToScreenBorder)
    {
        m_dlgSnap.OnMessage(this, message, wParam, lParam);
    }

    return CDialog::WindowProc(message, wParam, lParam);
}


// checks if the passed file type is an active file type
BOOL CPlaylistCreatorDlg::IsActiveFileType(const CString& strFileType)
{
    #pragma message(Reminder "IsActiveFileType(): Increase speed by using a map")

    INT_PTR nActiveTypesCount = m_listActiveFileTypes.GetCount();
    for(int i = 0; i < nActiveTypesCount; i++)
    {
        if(m_listActiveFileTypes.GetAt(i).CompareNoCase(strFileType) == 0)
            return TRUE;
    }
    return FALSE;
}


// called by Windows when taskbar button is created/recreated (>= Windows 7)
LRESULT CPlaylistCreatorDlg::OnTaskbarBtnCreated(WPARAM wParam, LPARAM lParam)
{
    // on pre-Win 7, anyone can register a message called "TaskbarButtonCreated"
    // and broadcast it, so make sure the OS is Win 7 or later before acting on
    // the message.
    if(WinVersion.IsSevenOrLater())
    {
        m_pTaskbarList.Release();
        m_pTaskbarList.CoCreateInstance(CLSID_TaskbarList);
    }

    return 0;
}


// ITaskbarListProvider
HRESULT CPlaylistCreatorDlg::SetProgressState(TBPFLAG tbpFlags)
{
    if(m_pTaskbarList)
    {
        return m_pTaskbarList->SetProgressState(m_hWnd, tbpFlags);
    }

    return S_OK;
}


// ITaskbarListProvider
HRESULT CPlaylistCreatorDlg::SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal)
{
    if(m_pTaskbarList)
    {
        return m_pTaskbarList->SetProgressValue(m_hWnd, ullCompleted, ullTotal);
    }

    return S_OK;
}
