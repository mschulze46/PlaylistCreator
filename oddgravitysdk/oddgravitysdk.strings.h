/*----------------------------------------------------------------------------
| File:    oddgravitysdk.strings.h
| Project: oddgravitysdk
|
| Description:
|   String functions
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 482 $
| $Id: oddgravitysdk.strings.h 482 2010-01-08 14:34:33Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_STRINGS_H__
#define __ODDGRAVITYSDK_STRINGS_H__

namespace oddgravity
{
    // returns a truncated string ending with "..."
    static CString StrTruncate(LPCTSTR lpcstrSource, int nMaxChars)
    {
        if(!lpcstrSource)
        {
            ASSERT(0);
            return _T("");
        }

        CString strSource = lpcstrSource;
        if(strSource.IsEmpty() || nMaxChars <= 0)
            return _T("");

        if((strSource.GetLength() > nMaxChars) && (nMaxChars > 3))
        {
            return strSource.Left(max(0,nMaxChars-3)) + _T("...");
        }
        return strSource;
    }

    // returns the string equivalent of the number parameter
    static CString IntegerToString(int iNumber)
    {
        CString strNumber;
        strNumber.Format(_T("%i"), iNumber);
        return strNumber;
    }

    // returns the int equivalent of the string parameter
    static int StringToInteger(CString strNumber)
    {
        return _ttoi(strNumber);
    }

    // returns the upper string
    static CString MakeUpper(const CString& str)
    {
        CString strUpper = str;
        return strUpper.MakeUpper();
    }

    // returns the lower string
    static CString MakeLower(const CString& str)
    {
        CString strLower = str;
        return strLower.MakeLower();
    }

    // returns the reverse string
    static CString MakeReverse(const CString& str)
    {
        CString strReverse = str;
        return strReverse.MakeReverse();
    }

    // returns TRUE if at least one whitespace was repaired
    static BOOL RepairWhitespaces(CString& str)
    {
        if(str.IsEmpty())
            return FALSE;

        // repair line breaks
        while(str.Find(_T("\\n")) != -1)
        {
            int iPos = str.Find(_T("\\n"));
            str.Delete(iPos, 2);
            str.Insert(iPos, _T('\n'));
        }

        // repair tabs
        while(str.Find(_T("\\t")) != -1)
        {
            int iPos = str.Find(_T("\\t"));
            str.Delete(iPos, 2);
            str.Insert(iPos, _T('\t'));
        }

        return TRUE;
    }

    // replaces the tags in a string (e.g. <FILE/>)
    static CString ReplaceTags(const CString& strWithTags,
                               const CString& strTag1,          const CString& strValue1,
                               const CString& strTag2 = _T(""), const CString& strValue2 = _T(""),
                               const CString& strTag3 = _T(""), const CString& strValue3 = _T(""),
                               const CString& strTag4 = _T(""), const CString& strValue4 = _T(""),
                               const CString& strTag5 = _T(""), const CString& strValue5 = _T(""))
    {
        if(strWithTags.IsEmpty())
            return _T("");

        // stuff keys and values in arrays
        CStringArray listTags;
        CStringArray listValues;

        if(strTag1.GetLength())
        {
            listTags.Add(MakeUpper(strTag1));
            listValues.Add(strValue1);
        }
        if(strTag2.GetLength())
        {
            listTags.Add(MakeUpper(strTag2));
            listValues.Add(strValue2);
        }
        if(strTag3.GetLength())
        {
            listTags.Add(MakeUpper(strTag3));
            listValues.Add(strValue3);
        }
        if(strTag4.GetLength())
        {
            listTags.Add(MakeUpper(strTag4));
            listValues.Add(strValue4);
        }
        if(strTag5.GetLength())
        {
            listTags.Add(MakeUpper(strTag5));
            listValues.Add(strValue5);
        }

        // replace all tags
        CString strTagsReplaced = strWithTags;
        for(int i=0; i < listTags.GetSize(); i++)
        {
            strTagsReplaced.Replace(_T("<") + listTags.GetAt(i) + _T("/>"), listValues.GetAt(i));
        }
        return strTagsReplaced;
    }
}

#endif // __ODDGRAVITYSDK_STRINGS_H__
