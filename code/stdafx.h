/*----------------------------------------------------------------------------
| File:    stdafx.h
| Project: Playlist Creator
|
| Description:
|   Include file for standard system include files,
|   or project specific include files that are used
|   frequently, but are changed infrequently.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: stdafx.h 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


//#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
//#endif



//---------- <oddgravity> -----------//
#ifdef _DEBUG
#define _SLOW_FILE_IMPORT
#define _SLOW_PLAYLIST_CREATION
#define _SLOW_CHECK_FOR_UPDATES
#endif

#include <HtmlHelp.h>
#include <afxdlgs.h>
#include <afxmt.h>                      // CTreePropSheetEx: CriticalSection
#include <MMSystem.h>                   // PlaySound
#include <Shlwapi.h>                    // PathIsDirectory

#include "ximacfg.h"                    // CxImage library
#include <CxImage/ximage.h>             // CxImage library
#include <system/XWinVer.h>
#include <dialogs/XMessageBox.h>

#include <oddgravitysdk.h>
#include "AppSettings.h"

using namespace PlaylistCreator;
//---------- </oddgravity> ----------//
