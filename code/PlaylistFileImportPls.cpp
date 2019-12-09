/*----------------------------------------------------------------------------
| File:    PlaylistFileImportPls.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPlaylistFileImportPls class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 255 $
| $Id: PlaylistFileImportPls.cpp 255 2007-07-05 20:09:40Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "PlaylistFileImportPls.h"
#include <filesystem/TextFile.h>
#include <filesystem/Path.h>


CPlaylistFileImportPls::CPlaylistFileImportPls()
{
    __init__();
}


CPlaylistFileImportPls::~CPlaylistFileImportPls()
{
}


void CPlaylistFileImportPls::__init__()
{
    m_strPlaylistFilePath   = _T("");
    m_strPlaylistFolder     = _T("");

    m_aPlaylistFile.RemoveAll();
    m_currentLineIndex      = -1;
}


CString CPlaylistFileImportPls::GetExtension()
{
    return _T("pls");
}


bool CPlaylistFileImportPls::Init(const CString& strFilePath)
{
    // reset members
    __init__();

    // does the passed file exist?
    if(!IsFile(strFilePath))
        return false;

    // remember playlist file path and folder
    m_strPlaylistFilePath = strFilePath;
    m_strPlaylistFolder   = GetFolderComponent(strFilePath);

    // import file
    CTextFileRead file(m_strPlaylistFilePath);                                      ASSERT(file.IsOpen());
    file.Read(m_aPlaylistFile);

    return true;
}


bool CPlaylistFileImportPls::IsValidPlaylist()
{
    if(m_strPlaylistFilePath.IsEmpty())
        return false; // error: no playlist file set

    if(GetExtension().CompareNoCase(GetFileExtensionComponent(m_strPlaylistFilePath)) != 0)
        return false; // error: invalid file type

    return true;
}


CString CPlaylistFileImportPls::GetFirstItem()
{
    // init current line index
    m_currentLineIndex = 0;

    // return the first item
    return GetNext(m_currentLineIndex);
}


CString CPlaylistFileImportPls::GetNextItem()
{
    // increment current line index
    m_currentLineIndex++;

    // return the next item
    return GetNext(m_currentLineIndex);
}


CString CPlaylistFileImportPls::GetNext(int& indexStart)
{
    // check bounds of passed param
    if(indexStart < 0)
        indexStart = 0;

    // get line count of current playlist file
    int lineCount = (int)m_aPlaylistFile.GetCount();
    if(indexStart >= lineCount)
        return _T("");

    // assemble list of extended info which this format can have
    CStringArray aExInfo;
    aExInfo.Add(_T("[playlist]"));
    aExInfo.Add(_T("Title"));
    aExInfo.Add(_T("Length"));
    aExInfo.Add(_T("NumberOfEntries"));
    aExInfo.Add(_T("Version"));

    // skip extended info lines
    CString strCurrentLine;
    while(indexStart < lineCount)
    {
        strCurrentLine = m_aPlaylistFile.GetAt(indexStart);
        strCurrentLine.Trim();

        bool bIsExLine = false;
        for(int i = 0; i < aExInfo.GetCount(); i++)
        {
            if(strCurrentLine.Left(aExInfo.GetAt(i).GetLength()).CompareNoCase(aExInfo.GetAt(i)) == 0)
            {
                // extended info line found
                bIsExLine = true;
                break;
            }
        }

        if(bIsExLine)
            indexStart++;
        else // should be a track!
            break;
    }

    if(indexStart >= lineCount) // EOF
        return _T("");

    // create buffer for processing and assure that only back slashes are used from now on
    CString strItem = strCurrentLine;
    strItem.Replace('/', '\\');

    // extended PLS files can have a leading 'File' tag, strip it!
    if(strItem.Left(4).CompareNoCase(_T("File")) == 0)
    {
        int posSeparator = strItem.Find('=');
        if(posSeparator > 0)
            strItem.Delete(0, posSeparator + 1);
        else
            ASSERT(0); // error: corrupt playlist or stupit file name
    }

    // split playlist file path into its parts
    CString strPlaylistDrive, strPlaylistDir, strPlaylistName, strPlaylistExt;
    SplitPath(m_strPlaylistFilePath, strPlaylistDrive, strPlaylistDir, strPlaylistName, strPlaylistExt);

    // split the current line into its parts
    CString strItemDrive, strItemDir, strItemName, strItemExt;
    SplitPath(strItem, strItemDrive, strItemDir, strItemName, strItemExt);

    // current line can be an absolute or relative file path or
    // a web link. all file paths must be converted to absolute
    // file paths, web links must be filtered.
    if(strItemDrive.IsEmpty()) // no absolute path
    {
        if(strItemDir.Left(1) == '/' || strItemDir.Left(1) == '\\') // absolute path without drive
        {
            strItem = strPlaylistDrive + strItemDir + strItemName + strItemExt;
        }
        else // relative path
        {
            if(strItemDir.IsEmpty()) // same folder as playlist
            {
                strItem = strPlaylistDrive + strPlaylistDir + strItemName + strItemExt;
            }
            else // not the same folder as playlist
            {
                CPath path(strItem);
                strItem = path.GetAbsolutePath(strPlaylistDrive + strPlaylistDir);
            }
        }
    }
    //else  // absolute path

    return strItem;
}


