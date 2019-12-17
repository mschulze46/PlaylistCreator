/*----------------------------------------------------------------------------
| File:    oddgravitysdk.datetime.h
| Project: oddgravitysdk
|
| Description:
|   Date and time functions
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 18 $
| $Id: oddgravitysdk.datetime.h 18 2006-10-12 19:25:07Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_DATETIME_H__
#define __ODDGRAVITYSDK_DATETIME_H__


namespace oddgravity
{

// returns the time as a formatted string of the passed time
static CString GetFormattedTime(
    const SYSTEMTIME& time,
    BOOL bMinutes       = TRUE,
    BOOL bSeconds       = TRUE,
    BOOL b24HourFormat  = TRUE,
    BOOL bTimeMarker    = FALSE,
    const CString& strSeparator = _T(":"))
{
    DWORD dwFlags = 0;
    CString strFormat = _T("hh");

    if(!bMinutes && !bSeconds)
    {
        dwFlags |= TIME_NOMINUTESORSECONDS;
    }
    else if(!bSeconds)
    {
        dwFlags |= TIME_NOSECONDS;
        strFormat += _T("\'") + strSeparator + _T("\'mm");
    }
    else
    {
        strFormat += _T("\'") + strSeparator + _T("\'mm\'") + strSeparator + _T("\'ss");
    }

    if(b24HourFormat)
    {
        dwFlags |= TIME_FORCE24HOURFORMAT;
        strFormat.Replace(_T("hh"), _T("HH"));
    }

    if(bTimeMarker)
    {
        strFormat += _T(" tt");
    }
    else
    {
        dwFlags |= TIME_NOTIMEMARKER;
    }

    TCHAR buffer[MAX_PATH];
    int iResult = GetTimeFormat(LOCALE_USER_DEFAULT, dwFlags, &time, strFormat, buffer, sizeof(buffer));
    if(iResult == 0)
    {
        ASSERT(0); // u suck!
        return _T("");
    }

    return CString(buffer);
}


// returns the current local time as a formatted string
static CString GetFormattedLocalTime(
    BOOL bMinutes       = TRUE,
    BOOL bSeconds       = TRUE,
    BOOL b24HourFormat  = TRUE,
    BOOL bTimeMarker    = FALSE,
    const CString& strSeparator = _T(":"))
{
    SYSTEMTIME time;
    GetLocalTime(&time);
    return GetFormattedTime(time, bMinutes, bSeconds, b24HourFormat, bTimeMarker, strSeparator);
}


// returns the current system time as a formatted string.
// note: the system time is expressed in Coordinated Universal Time (UTC) = Greenwich Mean Time (GMT) !
static CString GetFormattedSystemTime(
    BOOL bMinutes       = TRUE,
    BOOL bSeconds       = TRUE,
    BOOL b24HourFormat  = TRUE,
    BOOL bTimeMarker    = FALSE,
    const CString& strSeparator = _T(":"))
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    return GetFormattedTime(time, bMinutes, bSeconds, b24HourFormat, bTimeMarker, strSeparator);
}


// returns the date as a formatted string of the passed time
static CString GetFormattedDate(
    const SYSTEMTIME& time,
    BOOL bYear      = TRUE,
    BOOL bMonth     = TRUE,
    BOOL bDay       = TRUE,
    const CString& strSeparator = _T("-"))
{
    // %TODO%: add support for choosing between DATE_SHORTDATE / DATE_LONGDATE / DATE_YEARMONTH
    //         note: either dwFlags can be used or strFormat !
    DWORD dwFlags = 0;
    CString strFormat = _T("");

    if(bYear)
    {
        strFormat = _T("yyyy");
    }

    if(bMonth)
    {
        if(!strFormat.IsEmpty())
        {
            strFormat += _T("\'") + strSeparator + _T("\'");
        }
        strFormat += _T("MM");
    }

    if(bDay)
    {
        if(!strFormat.IsEmpty())
        {
            strFormat += _T("\'") + strSeparator + _T("\'");
        }
        strFormat += _T("dd");
    }

    TCHAR buffer[MAX_PATH];
    int iResult = GetDateFormat(LOCALE_USER_DEFAULT, dwFlags, &time, strFormat, buffer, sizeof(buffer));
    if(iResult == 0)
    {
        ASSERT(0); // u suck!
        return _T("");
    }

    return CString(buffer);
}


// returns the current local date as a formatted string
static CString GetFormattedLocalDate(
    BOOL bYear      = TRUE,
    BOOL bMonth     = TRUE,
    BOOL bDay       = TRUE,
    const CString& strSeparator = _T("-"))
{
    SYSTEMTIME time;
    GetLocalTime(&time);
    return GetFormattedDate(time, bYear, bMonth, bDay, strSeparator);
}


// returns the current system date as a formatted string.
// note: the system date is expressed in Coordinated Universal Time (UTC) = Greenwich Mean Time (GMT) !
static CString GetFormattedSystemDate(
    BOOL bYear      = TRUE,
    BOOL bMonth     = TRUE,
    BOOL bDay       = TRUE,
    const CString& strSeparator = _T("-"))
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    return GetFormattedDate(time, bYear, bMonth, bDay, strSeparator);
}





}

#endif // __ODDGRAVITYSDK_DATETIME_H__
