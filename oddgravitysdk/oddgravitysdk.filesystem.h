/*----------------------------------------------------------------------------
| File:    oddgravitysdk.filesystem.h
| Project: oddgravitysdk
|
| Description:
|   File system functions
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 438 $
| $Id: oddgravitysdk.filesystem.h 438 2009-10-17 10:12:49Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_FILESYSTEM_H__
#define __ODDGRAVITYSDK_FILESYSTEM_H__

#include "oddgravitysdk.strings.h"


namespace oddgravity
{

// adds a trailing backslash to a path if there is none
static CString PathAddTrailingBackSlash(const CString& strPath)
{
    if(strPath.IsEmpty())
        return _T("");

    if(strPath.Right(1) == _T("\\"))
        return strPath;

    return strPath + _T("\\");
}


// removes the trailing backslash of a path if there is one
static CString PathRemoveTrailingBackSlash(const CString& strPath)
{
    if(strPath.IsEmpty())
        return _T("");

    CString strBuffer = strPath;
    if(strBuffer.Right(1) != _T("\\"))
        return strBuffer;

    strBuffer.Delete(strBuffer.GetLength()-1, 1);
    return strBuffer;
}


// splits a fully qualified file path into its components
static void SplitPath(const CString& strFilePath, CString& strDrive, CString& strDir, CString& strName, CString& strExt)
{
    if(strFilePath.IsEmpty())
    {
        strDrive = strDir = strName = strExt = _T("");
        return;
    }

    // buffers
    TCHAR szDrive[_MAX_DRIVE];  // drive
    TCHAR szDir[_MAX_DIR];      // directory
    TCHAR szName[_MAX_FNAME];   // file name
    TCHAR szExt[_MAX_EXT];      // extension

    // crunch file path
#if _MSC_VER >= 1400  // VS2005
    _tsplitpath_s(
        strFilePath,
        szDrive, _MAX_DRIVE,
        szDir, _MAX_DIR,
        szName, _MAX_FNAME,
        szExt, _MAX_EXT);
#else
    _tsplitpath(
        strFilePath,
        szDrive,
        szDir,
        szName,
        szExt);
#endif

    // get results
    strDrive = szDrive;
    strDir   = szDir;
    strName  = szName;
    strExt   = szExt;
}


// extracts the folder from a fully qualified file path
static CString GetFolderComponent(const CString& strFilePath)
{
    if(strFilePath.IsEmpty())
        return _T("");

    int iPos = strFilePath.ReverseFind('\\');
    if(iPos == -1) // no backslash found
        return _T("");

    return strFilePath.Left(iPos + 1);
}


// extracts the file name and extension from a fully qualified file path
static CString GetFileNameAndExtension(const CString& strFilePath)
{
    if(strFilePath.IsEmpty())
        return _T("");

    int iPos = strFilePath.ReverseFind('\\');
    if(iPos == -1) // no backslash found
        return _T("");

    return strFilePath.Right(strFilePath.GetLength() - iPos - 1);
}


// extracts the file name from a fully qualified file path
static CString GetFileNameComponent(const CString& strFilePath)
{
    if(strFilePath.IsEmpty())
        return _T("");

    int iPosDot = strFilePath.ReverseFind('.');
    int iPosDir = strFilePath.ReverseFind('\\');

    if(iPosDir > iPosDot) // no dot or backslash after dot (dot in path)
        iPosDot = -1;

    int lengthFilePath = strFilePath.GetLength();
    int lengthFileName = lengthFilePath
                          - (iPosDir + 1)
                          - ((iPosDot > 0) ? (lengthFilePath - iPosDot) : 0);

    return strFilePath.Mid(iPosDir + 1, lengthFileName);
}


// extracts the file extension from a fully qualified file path
static CString GetFileExtensionComponent(const CString& strFilePath)
{
    if(strFilePath.IsEmpty())
        return _T("");

    int iPosDot = strFilePath.ReverseFind('.');
    int iPosDir = strFilePath.ReverseFind('\\');

    if(iPosDot == -1 || iPosDir > iPosDot) // no dot or backslash after dot (dot in path)
        return _T("");

    return strFilePath.Right(strFilePath.GetLength() - iPosDot - 1);
}


// returns the parent folder
static CString GetParentFolder(const CString& strChildFolder)
{
    if(strChildFolder.IsEmpty())
        return _T("");

    CString strParentFolder = PathRemoveTrailingBackSlash(strChildFolder);
    int iPos = strParentFolder.ReverseFind('\\');
    
    if(iPos == -1) // folder has no parent
        return strChildFolder;

    strParentFolder = strParentFolder.Left(iPos);

    if(strParentFolder.GetLength() == 2) // root folder
    {
        strParentFolder = PathAddTrailingBackSlash(strParentFolder);
    }
    else if(strParentFolder.GetLength() < 2) // shit happened
    {
        ASSERT(0);
        return strChildFolder;
    }

    return strParentFolder;
}


// enumerates all files in the given folder
static BOOL EnumerateFilesInFolder(const CString& strFolder, CStringArray& arrayFiles)
{
    // reset file array
    arrayFiles.RemoveAll();

    if(strFolder.IsEmpty())
        return FALSE;

    // buffer
    CString strTargetFriendlyFolder = PathAddTrailingBackSlash(strFolder);

    // init file finder
    CFileFind finder;
    BOOL bWorking = finder.FindFile(strTargetFriendlyFolder + _T("*.*"));
    if(!bWorking) // folder is empty
        return FALSE;

    // add all files to array
    while(bWorking)
    {
        bWorking = finder.FindNextFile();
        if(finder.IsDots() || finder.IsDirectory())
            continue;

        arrayFiles.Add(strTargetFriendlyFolder + finder.GetFileName());
    }

    return (arrayFiles.GetCount() > 0) ? TRUE : FALSE;
}


// returns the file path of the module that calls this function
static CString GetExeFilePath()
{
    CString strExeFilePath;
    DWORD result = GetModuleFileName(NULL, strExeFilePath.GetBuffer(_MAX_PATH), _MAX_PATH);
    strExeFilePath.ReleaseBuffer();

    if(result == 0)
    {
        ASSERT(0);
        return _T("");
    }

    return strExeFilePath;
}


// returns the directory of the module that calls this function
static CString GetExeFolder()
{
    return GetFolderComponent(GetExeFilePath());
}


// returns the filename of the module that calls this function
static CString GetExeFileName()
{
    return GetFileNameComponent(GetExeFilePath());
}


// checks if the passed file exists
static bool IsFile(const CString& strFilePath)
{
    if(strFilePath.IsEmpty())
    {
        return false;
    }

    DWORD dwAttr = GetFileAttributes(strFilePath);
    if((dwAttr != 0xffffffff) && ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
        return true;
    }
    else 
    {
        return false;
    }
}


// checks if the passed folder exists
static bool IsFolder(const CString& strFolder)
{
    if(strFolder.IsEmpty())
    {
        return false;
    }

    DWORD dwAttr = GetFileAttributes(strFolder);
    if((dwAttr != 0xffffffff) && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
        return true;
    }
    else
    {
        return false;
    }
}


// creates the passed folder if it doesn't exist (creates nested folders too)
static void CreateFolder(const CString& strFolder)
{
    if(strFolder.IsEmpty()          // no folder passed
        || IsFolder(strFolder))     // folder already exists
    {
        return;
    }

    CString strFolderBuffer = strFolder;
    char* pFolder = strFolderBuffer.GetBuffer();
    {
        char DirName[256];
        char* p = pFolder;
        char* q = DirName; 
        while(*p)
        {
            if (('\\' == *p) || ('/' == *p))
            {
                if (':' != *(p-1))
                {
                    CreateDirectory(DirName, NULL);
                }
            }
            *q++ = *p++;
            *q = '\0';
        }
        CreateDirectory(DirName, NULL);
    }
    strFolderBuffer.ReleaseBuffer();
}


// returns the system temp folder
static CString GetSystemTempDirectory()
{
    TCHAR lpBuffer[MAX_PATH + 100];
    GetTempPath(MAX_PATH + 100, lpBuffer);
    CString strReturn = (TCHAR*)lpBuffer;
    return PathAddTrailingBackSlash(strReturn);
}


// returns the name of a temporary file name
static CString MakeTempFileName(LPCTSTR lpcstrRootDir, LPCTSTR lpcstrPrefix, LPCTSTR lpcstrExtension)
{
    if (!IsFolder(lpcstrRootDir))
    {
        ASSERT(0);
        return _T("");
    }

    CString strTempFile;
    srand((unsigned)time(NULL));

    do
    {
        strTempFile  = CString(lpcstrPrefix);
        strTempFile += IntegerToString(rand());
        strTempFile += ".";
        strTempFile += lpcstrExtension;
    } while (IsFile(CString(lpcstrRootDir) + strTempFile));

    return strTempFile;
}


// returns the name of a temporary file name
static CString MakeTempFileName(LPCTSTR lpcstrRootDir)
{
    return MakeTempFileName(lpcstrRootDir, _T("~"), _T("tmp"));
}


// returns the file path of a temporary file
static CString MakeTempFilePath(LPCTSTR lpcstrRootDir, LPCTSTR lpcstrPrefix, LPCTSTR lpcstrExtension)
{
    if (!IsFolder(lpcstrRootDir))
    {
        ASSERT(0);
        return _T("");
    }

    CString strTempFilePath = CString(lpcstrRootDir) +
        MakeTempFileName(lpcstrRootDir, lpcstrPrefix, lpcstrExtension);

    return strTempFilePath;
}


// returns the file path of a temporary file
static CString MakeTempFilePath(LPCTSTR lpcstrRootDir)
{
    return MakeTempFilePath(lpcstrRootDir, _T("~"), _T("tmp"));
}


// the one and only working one
static CString MakeWindowsTempFilePath(LPCTSTR lpctstrFolderPath)
{
    CString strFileName;

    bool bOk = (GetTempFileName(lpctstrFolderPath, _T("ODD"), 0, strFileName.GetBuffer(MAX_PATH)) > 0);
    strFileName.ReleaseBuffer();

    if (!bOk)
    {
        ASSERT(0);
        return _T("");
    }

    return strFileName;
}




}

#endif // __ODDGRAVITYSDK_FILESYSTEM_H__
