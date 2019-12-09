/*----------------------------------------------------------------------------
| File:    PlaylistCreatorDlg.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPlaylistCreatorDlg class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 480 $
| $Id: PlaylistCreatorDlg.h 480 2009-11-23 18:04:53Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once
#include <dialogs/EasySize.h>
#include <dialogs/SnapperDialog.h>
#include <controls/FileDropListCtrl.h>
#include <controls/BtnST.h>
#include <controls/XPStyleButtonST.h>
#include <controls/ThemeHelperST.h>
#include <controls/PromptEdit.h>
#include "Playlist.h"
#include "ITaskbarListProvider.h"


///// user defined messages /////
#define UWM_SINGLE_INSTANCE_MSG             _T("UWM_SINGLE_INSTANCE-{90500F29-511B-47e5-BCB1-A762F2C1E64E}")
                                            DECLARE_USER_MESSAGE(UWM_SINGLE_INSTANCE)

#define UWM_AUTO_UPDATE_CHECK_MSG           _T("UWM_AUTO_UPDATE_CHECK-{686C4B80-B529-4bdd-B780-7052798F3500}")
                                            DECLARE_USER_MESSAGE(UWM_AUTO_UPDATE_CHECK)


// CPlaylistCreatorDlg dialog
class CPlaylistCreatorDlg : public CDialog, ITaskbarListProvider
{
///// types /////


///// members /////
protected:
    int             m_minDlgSizeX;
    int             m_minDlgSizeY;

    CStatusBar*     m_pStatusBar;

    CFont           m_font;

    CReBar*         m_pReBar;
    CToolBar*       m_pToolBar;
    CImageList*     m_pilToolBarCold;
    CImageList*     m_pilToolBarHot;

    CString         m_strToolTipText;

    CxImage         m_imgReBarBkgnd;            // image for the rebar background

    CFileDropListCtrl   m_listPlaylistContent;  // the playlist listctrl

    int             m_nSortColumn;              // defines the sort column
    BOOL            m_bSortAscending;           // defines if sorting is ascending or descending

    CButtonST       m_btnAddFolder;
    CButtonST       m_btnAddFiles;
    CButtonST       m_btnAddPlaylists;
    CButtonST       m_btnMoveSelectedItemsTop;
    CButtonST       m_btnMoveSelectedItemsUp;
    CButtonST       m_btnMoveSelectedItemsDown;
    CButtonST       m_btnMoveSelectedItemsBottom;
    CButtonST       m_btnDeleteSelectedItems;
    CButtonST       m_btnDeleteAllItems;
    CButtonST       m_btnSelectPlaylistSaveLocation;

    CXPStyleButtonST    m_btnCreatePlaylist;
    CThemeHelperST      m_ThemeHelper;

    CString         m_strPlaylistFolder;
    CPromptEdit     m_editPlaylistName;
    CComboBox       m_cbPlaylistTypes;

    CStringArray    m_listActiveFileTypes;

    CEdit*          m_pEditToFlash;
    CBrush          m_brushFlash;

    BOOL            m_bTransparency;
    int             m_iAlpha;
    lpfnSetLayeredWindowAttributes m_pSetLayeredWindowAttributes;
    lpfnChangeWindowMessageFilterEx m_pChangeWindowMessageFilterEx;

    CSnapperDialog  m_dlgSnap;
    BOOL            m_bSnapToScreenBorder;      // decides whether snap to screen border is active

    COLORREF        m_DefaultHighlightColor;    // default color for highlighting items

    BOOL            m_bHighlightMissingItems;
    COLORREF        m_crMissingItems;

    // ITaskbarList3
    CComPtr<ITaskbarList3> m_pTaskbarList;
    static const UINT m_uTaskbarBtnCreatedMsg;

    
///// construction /////
public:
    CPlaylistCreatorDlg(CWnd* pParent = NULL);  // standard constructor
    virtual ~CPlaylistCreatorDlg();

///// dialog data /////
    enum { IDD = IDD_PLAYLISTCREATOR_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


///// methods /////
public:
    CFileDropListCtrl* GetPlaylistListCtrl() { return &m_listPlaylistContent; }

    BOOL IsActiveFileType(const CString& strFileType);


protected:
    void __init__();
    void InitWindowMessageFilter();
    BOOL FirstTimeInit();
    void InitLanguage();
    void InitControls();
    void InitButtons();
    void InitPlaylistListCtrl();
    void InitWindowPlacement();
    void InitResizableDialog();
    void InitPlaylistColumnSizes();
    void InitColors();
    void InitThemeDependentControls();

    void UpdateFromRegistry();
    void UpdateToRegistry();

    void SavePlaylistColumnSizes();
    void SaveWindowPlacement();

    void ResetPlaylistSorting();

    void SetPlaylistSaveLocation(const CString& strLocation);
    void SetPlaylistSaveLocationFromItem(int nItem);

    void SetPlaylistName(const CString& strName);
    void ClearPlaylistName();

    CString GetPlaylistType();
    bool SetPlaylistType(const CString& strType);

    void UpdateItemStateDependentControls();

    void InitToolBar();
    void UpdateToolBar();
    void AttachToolBarImages();

    void InitStatusBar();
    void UpdateStatusBar();
    void SetStatusBarText(UINT nPane, const CString& strText, UINT nDuration = 0);

    CString GetBaseFolder(const CStringArray& arrayFolders);

    BOOL AutoPlaylistFolder(const CString& strFolder);
    BOOL AutoPlaylistFolder(const CStringArray& arrayFolders);

    BOOL AutoPlaylistName(const CString& strFolder);
    BOOL AutoPlaylistName(const CStringArray& arrayFolders);

    BOOL AutoClearPlaylist();
    void ClearPlaylist();

    void OpenPlaylist(const CString& strPlaylistFilePath);

    BOOL AddToPlaylist(const CString& strFileOrFolder, int customIndex = -1, bool bRecursive = false);
    BOOL AddToPlaylist(CStringArray& arrayFilesAndFolders, int customIndex = -1, bool bRecursive = false);

    void SelectAllItems();
    void DeselectAllItems();

    void RefreshItemState();

    void ShufflePlaylist();

    static HRESULT CALLBACK OnPlaylistItemsDropped(CDroppedItems* pDroppedItems);

    void FlashEdit(CEdit* pEdit, COLORREF color);

    // ITaskbarListProvider
    virtual HRESULT SetProgressState(TBPFLAG tbpFlags);
    virtual HRESULT SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal);

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void OnToolBarNewPlaylist();
    afx_msg void OnToolBarOpenPlaylist();
    afx_msg void OnToolBarSettings();
    afx_msg void OnToolBarHelp();
    afx_msg void OnToolBarAbout();
    afx_msg void OnClose();
    afx_msg void OnAddFolder();
    afx_msg void OnAddFiles();
    afx_msg void OnAddPlaylists();
    afx_msg void OnSelectPlaylistSaveLocation();
    afx_msg void OnMoveSelectedItemsToTop();
    afx_msg void OnMoveSelectedItemsUp();
    afx_msg void OnMoveSelectedItemsDown();
    afx_msg void OnMoveSelectedItemsToBottom();
    afx_msg void OnDeleteSelectedItems();
    afx_msg void OnDeleteAllItems();
    afx_msg void OnCreatePlaylist();
    afx_msg void OnPlaylistItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChangedPlaylistType();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
    afx_msg void OnNcDestroy();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
    afx_msg void OnSysColorChange();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg LRESULT OnSingleInstance(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnAutoUpdateCheck(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnFillPlaylist(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnSortPlaylistColumn(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnPlaylistItemToolTipNeedText(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnTaskbarBtnCreated(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
    DECLARE_EASYSIZE

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
