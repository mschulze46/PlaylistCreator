/*----------------------------------------------------------------------------
| File:    oddgravitysdk.internet.h
| Project: oddgravitysdk
|
| Description:
|   Internet related stuff
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 439 $
| $Id: oddgravitysdk.internet.h 439 2009-10-17 10:35:19Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_INTERNET_H__
#define __ODDGRAVITYSDK_INTERNET_H__


namespace oddgravity
{

// gets the passed registry key (helper function for GotoURL())
static LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS)
    {
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata, data);
        RegCloseKey(hkey);
    }

    return retval;
}


// opens the passed url in the default webbrowser
static HINSTANCE GotoURL(LPCTSTR url, int showcmd = SW_SHOW, BOOL bAlwaysOpenNew = FALSE)
{
    // if no url then this is not an internet link
    if (!url || url[0] == _T('\0'))
        return (HINSTANCE) HINSTANCE_ERROR + 1;

    TCHAR key[MAX_PATH * 2];

    // First try ShellExecute()
    TCHAR *verb = _T("open");
    if (bAlwaysOpenNew)
        verb = _T("new");
    HINSTANCE result = ShellExecute(NULL, verb, url, NULL, NULL, showcmd);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT_PTR)result <= HINSTANCE_ERROR) 
    {
        if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) 
        {
#if _MSC_VER >= 1400  // VS2005
            _tcscat_s(key, _countof(key), _T("\\shell\\open\\command"));
#else
            _tcscat(key, _T("\\shell\\open\\command"));
#endif

            if (GetRegKey(HKEY_CLASSES_ROOT, key, key) == ERROR_SUCCESS)
            {
                TCHAR *pos;
                pos = _tcsstr(key, _T("\"%1\""));   // search for "%1"
                if (pos == NULL) // no quotes found
                {
                    pos = _tcsstr(key, _T("%1"));   // search for %1, without quotes
                    if (pos == NULL)                // no parameter at all
                        pos = key + _tcslen(key)-1;
                    else
                        *pos = _T('\0');            // remove the parameter
                }
                else // quotes found
                {
                    *pos = _T('\0');                // remove the parameter
                }

#if _MSC_VER >= 1400  // VS2005
                _tcscat_s(key, _countof(key), _T(" "));
                _tcscat_s(key, _countof(key), url);
#else
                _tcscat(pos, _T(" "));
                _tcscat(pos, url);
#endif

                USES_CONVERSION;
                result = (HINSTANCE)(UINT_PTR)WinExec(T2A(key),showcmd);
            }
        }
    }

    return result;
}





}

#endif // __ODDGRAVITYSDK_INTERNET_H__
