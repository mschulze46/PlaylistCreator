/*----------------------------------------------------------------------------
| File:    PlaylistCreator.cpp
| Project: Playlist Creator
|
| Description:
|   Defines the class behaviors for the application.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 448 $
| $Id: PlaylistCreator.cpp 448 2009-10-19 14:24:33Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistCreator.h"
#include "PlaylistCreatorDlg.h"
#include <system/SingleInstance.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPlaylistCreatorApp

BEGIN_MESSAGE_MAP(CPlaylistCreatorApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPlaylistCreatorApp construction

CPlaylistCreatorApp::CPlaylistCreatorApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance

    EnableHtmlHelp();

    m_hMutexSingleInstance = NULL;
}


CPlaylistCreatorApp::~CPlaylistCreatorApp()
{
}

// The one and only CPlaylistCreatorApp object

CPlaylistCreatorApp theApp;


// CPlaylistCreatorApp initialization

BOOL CPlaylistCreatorApp::InitInstance()
{
    CWinApp::InitInstance();

    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization


    // hack registry path
    LPCTSTR pszBuffer = theApp.m_pszAppName;
    char* pszNewAppName = _tcsdup(CAppSettings::GetRegistryKey());
    theApp.m_pszAppName = pszNewAppName;
    SetRegistryKey(CAppSettings::GetCompanyName());
    theApp.m_pszAppName = pszBuffer;
    free(pszNewAppName);


    // single instance
    BOOL bSingleInstance = CAppSettings::GetIntProperty(PROP_SINGLEINSTANCE, (int)TRUE);
    if(bSingleInstance)
    {
        // create unique name for mutex
        CString sUniqueName;
        CreateUniqueName(_T("PC3-{B9CA50FC-8C22-4ced-8C64-E98672FFBF25}"), sUniqueName, SI_DESKTOP_UNIQUE);

        // CreateMutex fails with ERROR_ACCESS_DENIED if the mutex was created in a different users
        // session because of passing NULL for the SECURITY_ATTRIBUTES on mutex creation
        m_hMutexSingleInstance = ::CreateMutex(NULL, FALSE, sUniqueName);
        BOOL bAlreadyRunning = (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED);

        if(bAlreadyRunning)
        {
            HWND hOther = NULL;
            EnumWindows(WndIterator, (LPARAM)&hOther);

            if(hOther != NULL)
            {
                ::ShowWindow(hOther, ::IsIconic(hOther) ? SW_RESTORE : SW_SHOW);
                ::SetForegroundWindow(hOther);
            }

            return FALSE;
        }
    }

    // launch main window
    CPlaylistCreatorDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();

    if (nResponse == IDOK)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }

    if(m_hMutexSingleInstance)
    {
        ReleaseMutex(m_hMutexSingleInstance);
        CloseHandle(m_hMutexSingleInstance);
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}


BOOL CALLBACK CPlaylistCreatorApp::WndIterator(HWND hWnd, LPARAM lParam)
{
    DWORD result;
    LRESULT ok = ::SendMessageTimeout(
        hWnd,
        UWM_SINGLE_INSTANCE,
        0, 0, 
        SMTO_BLOCK|SMTO_ABORTIFHUNG,
        200,
        &result);
    if(ok == 0)
        return TRUE;
    if(result == UWM_SINGLE_INSTANCE)
    {
        // found it
        HWND* target = (HWND*)lParam;
        *target = hWnd;
        return FALSE;
    }
    return TRUE;
}


