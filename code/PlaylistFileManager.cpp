/*----------------------------------------------------------------------------
| File:    PlaylistFileManager.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPlaylistFileManager class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 261 $
| $Id: PlaylistFileManager.cpp 261 2007-07-08 14:22:07Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "PlaylistFileManager.h"
#include "IPlaylistFileImport.h"
#include "PlaylistFileImportM3u.h"
#include "PlaylistFileImportPls.h"


CPlaylistFileManager::CPlaylistFileManager()
{
    __init__();
}


CPlaylistFileManager::~CPlaylistFileManager()
{
}


void CPlaylistFileManager::__init__()
{
    GetSupportedFileTypes(m_arraySupportedFileTypes);
}


void CPlaylistFileManager::GetSupportedFileTypes(CStringArray& arrayFileTypes)
{
    // clear array
    arrayFileTypes.RemoveAll();

    // add all supported playlist formats
    arrayFileTypes.Add(CPlaylistFileImportM3u().GetExtension());
    arrayFileTypes.Add(CPlaylistFileImportPls().GetExtension());
}


bool CPlaylistFileManager::IsPlaylistFilePath(const CString& strFilePath)
{
    ASSERT(!m_arraySupportedFileTypes.IsEmpty());

    CString strExtension = GetFileExtensionComponent(strFilePath);
    for(int i = 0; i < m_arraySupportedFileTypes.GetCount(); i++)
    {
        if(m_arraySupportedFileTypes.GetAt(i).CompareNoCase(strExtension) == 0)
            return true;
    }

    return false;
}


bool CPlaylistFileManager::IsPlaylistFileExtension(const CString& strExtension)
{
    ASSERT(!m_arraySupportedFileTypes.IsEmpty());

    CString strExtFormatted = strExtension;
    if(strExtFormatted.Left(1) == '.')
        strExtFormatted.Delete(0, 1); // remove the dot

    for(int i = 0; i < m_arraySupportedFileTypes.GetCount(); i++)
    {
        if(m_arraySupportedFileTypes.GetAt(i).CompareNoCase(strExtFormatted) == 0)
            return true;
    }

    return false;
}


bool CPlaylistFileManager::ImportPlaylist(const CString& strPlaylistFilePath, CStringArray& aImportedFiles)
{
    // reset passed array
    aImportedFiles.RemoveAll();

    // supported and existent playlist passed?
    if(!IsFile(strPlaylistFilePath) || !IsPlaylistFilePath(strPlaylistFilePath))
        return false;

    // create playlist file importer for the passed type
    IPlaylistFileImport* pPlaylistFileImport = NULL;
    CString strPlaylistType = GetFileExtensionComponent(strPlaylistFilePath);
    
    if(CPlaylistFileImportM3u().GetExtension().CompareNoCase(strPlaylistType) == 0)
    {
        pPlaylistFileImport = new CPlaylistFileImportM3u();
    }
    else if(CPlaylistFileImportPls().GetExtension().CompareNoCase(strPlaylistType) == 0)
    {
        pPlaylistFileImport = new CPlaylistFileImportPls();
    }

    // import playlist
    if(pPlaylistFileImport)
    {
        pPlaylistFileImport->Init(strPlaylistFilePath);

        if(pPlaylistFileImport->IsValidPlaylist())
        {
            CString strItem = pPlaylistFileImport->GetFirstItem();
            while(!strItem.IsEmpty())
            {
                aImportedFiles.Add(strItem);
                strItem = pPlaylistFileImport->GetNextItem();
            }
        }
        else
        {
            ASSERT(0); // error: invalid playlist
            #pragma message(Reminder "ImportPlaylist(): Invalid playlist - Implement error reporting and/or try to open with other importers")
        }

        delete pPlaylistFileImport;
        pPlaylistFileImport = NULL;
    }
    else
    {
        ASSERT(0); // error: no playlist file importer
    }

    return (aImportedFiles.GetCount() > 0) ? true : false;
}


