/*----------------------------------------------------------------------------
| File:    oddgravitysdk.windows.h
| Project: oddgravitysdk
|
| Description:
|   Windows functions
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 474 $
| $Id: oddgravitysdk.windows.h 474 2009-11-19 11:20:23Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_WINDOWS_H__
#define __ODDGRAVITYSDK_WINDOWS_H__

#include <MMSystem.h>       // PlaySound
#include <shlwapi.h>        // DLLGETVERSIONPROC, DLLVERSIONINFO


namespace oddgravity
{

enum SYSTEM_SOUND
{
    SOUND_ASTERISK,
    SOUND_EXCLAMATION,
    SOUND_EXIT,
    SOUND_HAND,
    SOUND_QUESTION,
    SOUND_START,
};

// gets the invalid file name characters
static CString GetInvalidFileNameCharacters()
{
    return _T("\\/:*?\"<>|");
}

// gets the invalid file extension characters
static CString GetInvalidFileExtensionCharacters()
{
    return _T("\\/:*?\"<>|.");
}

// gets the invalid folder characters
static CString GetInvalidFolderCharacters()
{
    return _T("*?\"<>|");
}

// checks if a file name is valid
static BOOL IsValidFileName(const CString& strFileName)
{
    if(strFileName.IsEmpty())
        return FALSE;

    if(strFileName.FindOneOf(GetInvalidFileNameCharacters()) == -1)
        return TRUE;

    return FALSE;
}

// checks if a file extension is valid
static BOOL IsValidFileExtension(const CString& strFileExtension)
{
    if(strFileExtension.IsEmpty())
        return FALSE;

    if(strFileExtension.FindOneOf(GetInvalidFileExtensionCharacters()) == -1)
        return TRUE;

    return FALSE;
}

// checks if a folder name is valid
static BOOL IsValidFolderName(const CString& strFolder)
{
    if(strFolder.IsEmpty())
        return FALSE;

    if(strFolder.FindOneOf(GetInvalidFolderCharacters()) == -1)
        return TRUE;

    return FALSE;
}

// strips all invalid characters from a file name
static CString StripInvalidFileNameCharacters(const CString& strFileName)
{
    if(strFileName.IsEmpty())
        return _T("");

    if(IsValidFileName(strFileName))
        return strFileName;

    CString strStrippedFileName = strFileName;
    while(1)
    {
        int nPos = strStrippedFileName.FindOneOf(GetInvalidFileNameCharacters());
        if(nPos == -1)
        {
            break;
        }
        strStrippedFileName.Delete(nPos);
    }
    return strStrippedFileName;
}

// strips all invalid characters from a file extension
static CString StripInvalidFileExtensionCharacters(const CString& strFileExtension)
{
    if(strFileExtension.IsEmpty())
        return _T("");

    if(IsValidFileExtension(strFileExtension))
        return strFileExtension;

    CString strStrippedFileExtension = strFileExtension;
    while(1)
    {
        int nPos = strStrippedFileExtension.FindOneOf(GetInvalidFileExtensionCharacters());
        if(nPos == -1)
        {
            break;
        }
        strStrippedFileExtension.Delete(nPos);
    }
    return strStrippedFileExtension;
}

#if(WINVER >= 0x0500)
// flashes a window
static BOOL FlashWindow(HWND hWnd, UINT uCount = 1, DWORD dwTimeout = 0, DWORD dwFlags = FLASHW_CAPTION)
{
    if(!IsWindow(hWnd))
    {
        ASSERT(0);
        return FALSE;
    }
    FLASHWINFO flashwinfo;
    ZeroMemory(&flashwinfo, sizeof(FLASHWINFO));
    flashwinfo.cbSize = sizeof(FLASHWINFO);
    flashwinfo.hwnd = hWnd;
    flashwinfo.uCount = uCount;
    flashwinfo.dwFlags = dwFlags;
    flashwinfo.dwTimeout = dwTimeout;
    return ::FlashWindowEx(&flashwinfo);
}
#endif /* WINVER >= 0x0500 */

// forces a window to redraw
static void ForceRedrawWindow(HWND hWnd)
{
    if(!IsWindow(hWnd))
    {
        ASSERT(0);
        return;
    }
    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);
    RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_INTERNALPAINT);
}

// gets the position of a window
static void GetWindowPosition(HWND hWnd, POINT* pPointPosition)
{
    if(!IsWindow(hWnd) || !pPointPosition)
    {
        ASSERT(0);
        return;
    }
    RECT rectWindow;
    GetWindowRect(hWnd, &rectWindow);
    pPointPosition->x = rectWindow.left;
    pPointPosition->y = rectWindow.top;
}

// gets the x-position of a window
static int GetWindowPositionX(HWND hWnd)
{
    if(!IsWindow(hWnd))
    {
        ASSERT(0);
        return 0;
    }
    POINT pointPosition;
    GetWindowPosition(hWnd, &pointPosition);
    return pointPosition.x;
}

// gets the y-position of a window
static int GetWindowPositionY(HWND hWnd)
{
    if(!IsWindow(hWnd))
    {
        ASSERT(0);
        return 0;
    }
    POINT pointPosition;
    GetWindowPosition(hWnd, &pointPosition);
    return pointPosition.y;
}

// gets the style of a window
static DWORD GetWindowStyle(HWND hWnd)
{
    if(!hWnd)
    {
        ASSERT(0);
        return 0;
    }
    return GetWindowLong(
        hWnd,
        GWL_STYLE);
}

// sets the style of a window
static void SetWindowStyle(HWND hWnd, DWORD dwStyle)
{
    if(!hWnd)
    {
        ASSERT(0);
        return;
    }
    SetWindowLong(
        hWnd,
        GWL_STYLE,
        dwStyle);
}

// adds a style to a window
static void AddWindowStyle(HWND hWnd, DWORD dwStyle)
{
    if(!hWnd)
    {
        ASSERT(0);
        return;
    }
    DWORD dwStyleNow = GetWindowStyle(hWnd);
    dwStyleNow |= dwStyle;
    SetWindowStyle(hWnd, dwStyleNow);
}

// removes a style from a window
static void RemoveWindowStyle(HWND hWnd, DWORD dwStyle)
{
    if(!hWnd)
    {
        ASSERT(0);
        return;
    }
    DWORD dwStyleNow = GetWindowStyle(hWnd);
    dwStyleNow &= ~dwStyle;
    SetWindowStyle(hWnd, dwStyleNow);
}

// gets the extended style of a window
static DWORD GetWindowExStyle(HWND hWnd)
{
    if(!hWnd)
    {
        ASSERT(0);
        return 0;
    }
    return GetWindowLong(
        hWnd,
        GWL_EXSTYLE);
}

// sets the extended style of a window
static void SetWindowExStyle(HWND hWnd, DWORD dwExStyle)
{
    if(!hWnd)
    {
        ASSERT(0);
        return;
    }
    SetWindowLong(
        hWnd,
        GWL_EXSTYLE,
        dwExStyle);
}

// adds an extended style to a window
static void AddWindowExStyle(HWND hWnd, DWORD dwExStyle)
{
    if(!hWnd)
    {
        ASSERT(0);
        return;
    }
    DWORD dwExStyleNow = GetWindowExStyle(hWnd);
    dwExStyleNow |= dwExStyle;
    SetWindowExStyle(hWnd, dwExStyleNow);
}

// removes an extended style from a window
static void RemoveWindowExStyle(HWND hWnd, DWORD dwExStyle)
{
    if(!hWnd)
    {
        ASSERT(0);
        return;
    }
    DWORD dwExStyleNow = GetWindowExStyle(hWnd);
    dwExStyleNow &= ~dwExStyle;
    SetWindowExStyle(hWnd, dwExStyleNow);
}

// plays a system sound
static BOOL PlaySystemSound(SYSTEM_SOUND eSound = SOUND_ASTERISK)
{
    CString strSound;
    switch(eSound)
    {
        case SOUND_ASTERISK:
        default:
            strSound = _T("SystemAsterisk");
            break;
        case SOUND_EXCLAMATION:
            strSound = _T("SystemExclamation");
            break;
        case SOUND_EXIT:
            strSound = _T("SystemExit");
            break;
        case SOUND_HAND:
            strSound = _T("SystemHand");
            break;
        case SOUND_QUESTION:
            strSound = _T("SystemQuestion");
            break;
        case SOUND_START:
            strSound = _T("SystemStart");
            break;
    }
    return PlaySound(strSound, NULL, SND_ALIAS | SND_ASYNC | SND_NOWAIT);
}

// plays a sound from resource (linking to 'winmm.lib' is mandatory)
static BOOL PlaySoundFromResource(int nID, bool bAsync = true)
{
    if(nID <= 0)
    {
        ASSERT(0);
        return FALSE;
    }

    DWORD dwFlags = SND_RESOURCE | SND_NOWAIT | ((bAsync) ? SND_ASYNC : SND_SYNC);
    return PlaySound(MAKEINTRESOURCE(nID), AfxGetInstanceHandle(), dwFlags);
}

// checks if a specific key is pressed (VK_CONTROL, VK_SHIFT, etc.)
static BOOL IsKeyPressed(int nVirtKey)
{
    if((GetKeyState(nVirtKey) & 0x8000) != 0)
    {
        return TRUE;
    }
    return FALSE;
}

static void WaitAndCheckMessages(DWORD dwTime)
{
    DWORD dwTimeStart = timeGetTime();
    DWORD dwTimeNow   = dwTimeStart;

    while(dwTimeNow < dwTimeStart + dwTime)
    {
        MSG msg;
        while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            AfxGetThread()->PumpMessage();
        }
        Sleep(25);
        dwTimeNow = timeGetTime();
    }
}

static CString GetUserDefaultLanguage()
{
    LANGID langID = GetUserDefaultLangID();

    // 0x0407 German (Standard) 
    // 0x0807 German (Switzerland) 
    // 0x0c07 German (Austria) 
    // 0x1007 German (Luxembourg) 
    // 0x1407 German (Liechtenstein) 
    if(langID == 0x0407 || langID == 0x0807 ||
        langID == 0x0c07 || langID == 0x1007 ||
        langID == 0x1407)
        return _T("DE");

    // 0x040c French (Standard) 
    // 0x080c French (Belgian) 
    // 0x0c0c French (Canadian) 
    // 0x100c French (Switzerland) 
    // 0x140c French (Luxembourg) 
    // 0x180c French (Monaco) 
    if(langID == 0x040c || langID == 0x080c ||
        langID == 0x0c0c || langID == 0x100c ||
        langID == 0x140c || langID == 0x180c)
        return _T("FR");

    // 0x0410 Italian (Standard) 
    // 0x0810 Italian (Switzerland) 
    if(langID == 0x0410 || langID == 0x0810)
        return _T("IT");

    // 0x0405 Czech 
    if(langID == 0x0405)
        return _T("CZ");

    //  0x040a Spanish (Traditional Sort) 
    //  0x080a Spanish (Mexican) 
    //  0x0c0a Spanish (Modern Sort) 
    //  0x100a Spanish (Guatemala) 
    //  0x140a Spanish (Costa Rica) 
    //  0x180a Spanish (Panama) 
    //  0x1c0a Spanish (Dominican Republic) 
    //  0x200a Spanish (Venezuela) 
    //  0x240a Spanish (Colombia) 
    //  0x280a Spanish (Peru) 
    //  0x2c0a Spanish (Argentina) 
    //  0x300a Spanish (Ecuador) 
    //  0x340a Spanish (Chile) 
    //  0x380a Spanish (Uruguay) 
    //  0x3c0a Spanish (Paraguay) 
    //  0x400a Spanish (Bolivia) 
    //  0x440a Spanish (El Salvador) 
    //  0x480a Spanish (Honduras) 
    //  0x4c0a Spanish (Nicaragua) 
    //  0x500a Spanish (Puerto Rico) 
    if(langID == 0x040a || langID == 0x080a ||
        langID == 0x0c0a || langID == 0x100a ||
        langID == 0x140a || langID == 0x180a ||
        langID == 0x1c0a || langID == 0x200a ||
        langID == 0x240a || langID == 0x280a ||
        langID == 0x2c0a || langID == 0x300a ||
        langID == 0x340a || langID == 0x380a ||
        langID == 0x3c0a || langID == 0x400a ||
        langID == 0x440a || langID == 0x480a ||
        langID == 0x4c0a || langID == 0x500a)
        return _T("ES");

    //  0x0416 Portuguese (Brazil) 
    //  0x0816 Portuguese (Standard) 
    if( langID == 0x0416 || langID == 0x0816 )
        return _T("PT");

    //  0x0415 Polish 
    if( langID == 0x0415 )
        return _T("PL");

    // 0x040e Hungarian
    if( langID == 0x040e )
        return _T("HU");

    //0x0413 Dutch (Netherlands) 
    //0x0813 Dutch (Belgium) 
    if( langID == 0x0413 || langID == 0x0813 )
        return _T("NL");

    //  0x0406 Danish 
    if( langID == 0x0406 )
        return _T("DK");

    return _T("EN");
}


// callback for child enumeration of SetFontToDialog
static BOOL CALLBACK EnumChildProc_SetFontToDialog(HWND hWnd, LPARAM lParam)
{
    if(!::IsWindow(hWnd))
        return TRUE;

    LONG nStyle = ::GetWindowLong(hWnd, GWL_STYLE);
    if(!(nStyle & WS_POPUP))
    {
        ::SendMessage(hWnd, WM_SETFONT, lParam, 0);
    }
    return TRUE;
}


// sets a font to a dialog and its child controls
static void SetFontToDialog(HWND hWnd, HFONT hFont)
{
    if(!IsWindow(hWnd) || !hFont)
    {
        ASSERT(0);
        return;
    }
    ::EnumChildWindows(
        hWnd,
        &EnumChildProc_SetFontToDialog,
        (LPARAM)hFont);
}


// returns the number of pixels it takes to display the given text on the window's device context.
// if pFont is not NULL, it is temporarily selected into the window's device context.
static int GetTextExtent(CWnd* pWnd, const CString& strText, CFont* pFont = NULL)
{
    ASSERT(IsWindow(pWnd->GetSafeHwnd()));

    int nExtent = 0;
    int nLen = strText.GetLength();

    if(nLen)
    {
        TEXTMETRIC tm;
        
        CDC* pDC = pWnd->GetDC();
        CFont* pFontOld = pDC->SelectObject(pFont ? pFont : pWnd->GetFont()); 
        pDC->GetTextMetrics(&tm);
        CSize size = pDC->GetTextExtent(strText, nLen);
        nExtent = size.cx + tm.tmAveCharWidth;
        
        pDC->SelectObject(pFontOld);
        pWnd->ReleaseDC(pDC);
    }
    return nExtent;
}


// returns whether the OS contains Common Controls Version 6.0 or above (>= Windows XP)
static BOOL IsCommCtrlVersion6()
{
    static BOOL isCommCtrlVersion6 = -1;
    if(isCommCtrlVersion6 != -1)
        return isCommCtrlVersion6;
    
    // set default value
    isCommCtrlVersion6 = FALSE;
    
    HINSTANCE commCtrlDll = LoadLibrary(_T("comctl32.dll"));
    if(commCtrlDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(commCtrlDll, "DllGetVersion");
        
        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi = {0};
            dvi.cbSize = sizeof(DLLVERSIONINFO);
            (*pDllGetVersion)(&dvi);
            
            isCommCtrlVersion6 = (dvi.dwMajorVersion == 6);
        }
        
        FreeLibrary(commCtrlDll);
    }
    
    return isCommCtrlVersion6;
}



}

#endif // __ODDGRAVITYSDK_WINDOWS_H__
