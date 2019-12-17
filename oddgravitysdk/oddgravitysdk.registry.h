/*----------------------------------------------------------------------------
| File:    oddgravitysdk.registry.h
| Project: oddgravitysdk
|
| Description:
|   Registry functions
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 18 $
| $Id: oddgravitysdk.registry.h 18 2006-10-12 19:25:07Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_REGISTRY_H__
#define __ODDGRAVITYSDK_REGISTRY_H__

#include <atlbase.h>


namespace oddgravity
{
#define REGISTRY_MAX_VALUENAME_LENGTH  256

// creates a string value
static BOOL RegistryCreateStringValue(HKEY hRootKey, const CString& strKey, const CString& strValue, const CString& strData = _T(""))
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Create(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = regKey.SetStringValue(strValue, strData);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// sets a string value
static BOOL RegistrySetStringValue(HKEY hRootKey, const CString& strKey, const CString& strValue, const CString& strData = _T(""))
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = regKey.SetStringValue(strValue, strData);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// returns the data of a string value
static CString RegistryGetStringValue(HKEY hRootKey, const CString& strKey, const CString& strValue)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return _T("");
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        TCHAR buffer[256] = _T("");
        ULONG bufferSize = sizeof(buffer);
        lRes = regKey.QueryStringValue(strValue, buffer, &bufferSize);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return CString(buffer);
        }
    }
    return _T("");
}

// deletes a registry value
static BOOL RegistryDeleteValue(HKEY hRootKey, const CString& strKey, const CString& strValue)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = regKey.DeleteValue(strValue);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// enumerates all registry entries of a given key.
// returns the number of found entries.
static unsigned int RegistryEnumerateValues(HKEY hRootKey, const CString& strKey, CStringArray& arrayValues)
{
    if(!arrayValues.IsEmpty())
        arrayValues.RemoveAll();

    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return 0;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey, KEY_READ);
    if (lRes != ERROR_SUCCESS)
    {
        return 0;
    }

    DWORD dwValueCount = 0;
    TCHAR *lpstrValueName;
    lpstrValueName = (TCHAR*)malloc(sizeof(TCHAR) * REGISTRY_MAX_VALUENAME_LENGTH);
    if(lpstrValueName == NULL)
        return 0;
    unsigned long nValueNameLength;
    DWORD dwType = 0;
    do 
    {
        nValueNameLength = REGISTRY_MAX_VALUENAME_LENGTH;
        lRes = RegEnumValue(
            regKey.m_hKey,
            dwValueCount,
            lpstrValueName,
            &nValueNameLength,
            NULL,
            &dwType,
            NULL,
            NULL);
        if (lRes == ERROR_SUCCESS)
        {
            dwValueCount++;
            arrayValues.Add(CString(lpstrValueName));
        }
    } while(lRes == ERROR_SUCCESS);

    free(lpstrValueName);
    regKey.Close();
    return dwValueCount;
}

// deletes a sub key recursively
static BOOL RegistryDeleteSubKey(HKEY hRootKey, const CString& strKey, const CString& strSubKey)
{
    if(!hRootKey || strKey.IsEmpty() || strSubKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = regKey.RecurseDeleteKey(strSubKey);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// returns TRUE if a certain string value is present
static BOOL RegistryIsStringValuePresent(HKEY hRootKey, const CString& strKey, const CString& strValue)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        TCHAR buffer[256] = _T("");
        ULONG bufferSize = sizeof(buffer);
        lRes = regKey.QueryStringValue(strValue, buffer, &bufferSize);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// returns TRUE if a certain DWORD value is present
static BOOL RegistryIsDwordValuePresent(HKEY hRootKey, const CString& strKey, const CString& strValue)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        DWORD dwValue;
        lRes = regKey.QueryDWORDValue(strValue, dwValue);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL RegistryIsKeyPresent(HKEY hRootKey, const CString& strKey)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        regKey.Close();
        return TRUE;
    }
    return FALSE;
}

// creates a dword value
static BOOL RegistryCreateDwordValue(HKEY hRootKey, const CString& strKey, const CString& strValue, DWORD dwData = 0)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Create(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = regKey.SetDWORDValue(strValue, dwData);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// sets a dword value
static BOOL RegistrySetDwordValue(HKEY hRootKey, const CString& strKey, const CString& strValue, DWORD dwData = 0)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return FALSE;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        lRes = regKey.SetDWORDValue(strValue, dwData);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// returns the data of a dword value
static DWORD RegistryGetDwordValue(HKEY hRootKey, const CString& strKey, const CString& strValue)
{
    if(!hRootKey || strKey.IsEmpty())
    {
        ASSERT(0);
        return 0;
    }

    CRegKey regKey;
    LONG lRes = regKey.Open(hRootKey, strKey);
    if(lRes == ERROR_SUCCESS)
    {
        DWORD dwData = 0;
        lRes = regKey.QueryDWORDValue(strValue, dwData);
        regKey.Close();
        if(lRes == ERROR_SUCCESS)
        {
            return dwData;
        }
    }
    return 0;
}





}

#endif // __ODDGRAVITYSDK_REGISTRY_H__
