/*----------------------------------------------------------------------------
| File:    oddgravitysdk.constants.h
| Project: oddgravitysdk
|
| Description:
|   Constants / defines
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 477 $
| $Id: oddgravitysdk.constants.h 477 2009-11-23 18:00:30Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_CONSTANTS_H__
#define __ODDGRAVITYSDK_CONSTANTS_H__


namespace oddgravity
{

//-------------------------------------
// UpdateData helpers
//-------------------------------------

#define FROM_DIALOG     TRUE
#define TO_DIALOG       FALSE


//-------------------------------------
// result values
//-------------------------------------

enum RESULT
{
    RESULT_SUCCESS              = 0,
    RESULT_ERROR                = 1,
    RESULT_NOT_IMPLEMENTED      = 100,
    RESULT_NOT_SUPPORTED        = 101,
    RESULT_OUT_OF_BOUNDS        = 102,
    RESULT_EXECUTE_FAILED       = 103,
    RESULT_SHUTDOWN_THREAD      = 104,
    RESULT_ACCESS_DENIED        = 105,
};


//-------------------------------------
// hand cursor
//   Minimum OS:    Windows 2000
//   Reference:     Windows SDK, WinUser.h
//-------------------------------------

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif


//-------------------------------------
// SetLayeredWindowAttributes API
//   Minimum DLL:   user32.dll
//   Minimum OS:    Windows 2000
//   Reference:     Windows SDK, WinUser.h
//-------------------------------------

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED       0x80000
#endif
#ifndef LWA_COLORKEY
#define LWA_COLORKEY        0x00001
#endif
#ifndef LWA_ALPHA
#define LWA_ALPHA           0x00002
#endif

typedef BOOL (WINAPI* lpfnSetLayeredWindowAttributes) (HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);


//-------------------------------------
// ChangeWindowMessageFilter API
//   Minimum DLL:   user32.dll
//   Minimum OS:    Windows Vista
//   Reference:     Windows SDK, WinUser.h
//-------------------------------------

#ifndef MSGFLT_ADD
#define MSGFLT_ADD          1
#endif
#ifndef MSGFLT_REMOVE
#define MSGFLT_REMOVE       2
#endif

typedef BOOL (WINAPI* lpfnChangeWindowMessageFilter) (UINT message, DWORD dwFlag);


//-------------------------------------
// ChangeWindowMessageFilterEx API
//   Minimum DLL:   user32.dll
//   Minimum OS:    Windows 7
//   Reference:     Windows SDK, WinUser.h
//-------------------------------------

#ifndef MSGFLTINFO_NONE
#define MSGFLTINFO_NONE                         (0)
#endif
#ifndef MSGFLTINFO_ALREADYALLOWED_FORWND
#define MSGFLTINFO_ALREADYALLOWED_FORWND        (1)
#endif
#ifndef MSGFLTINFO_ALREADYDISALLOWED_FORWND
#define MSGFLTINFO_ALREADYDISALLOWED_FORWND     (2)
#endif
#ifndef MSGFLTINFO_ALLOWED_HIGHER
#define MSGFLTINFO_ALLOWED_HIGHER               (3)
#endif

#ifndef PCHANGEFILTERSTRUCT
typedef struct tagCHANGEFILTERSTRUCT {
    DWORD cbSize;
    DWORD ExtStatus;
} CHANGEFILTERSTRUCT, *PCHANGEFILTERSTRUCT;
#endif

#ifndef MSGFLT_RESET
#define MSGFLT_RESET                            (0)
#endif
#ifndef MSGFLT_ALLOW
#define MSGFLT_ALLOW                            (1)
#endif
#ifndef MSGFLT_DISALLOW
#define MSGFLT_DISALLOW                         (2)
#endif

typedef BOOL (WINAPI* lpfnChangeWindowMessageFilterEx) (HWND hWnd, UINT message, DWORD action, PCHANGEFILTERSTRUCT pChangeFilterStruct);


}

#endif // __ODDGRAVITYSDK_CONSTANTS_H__
