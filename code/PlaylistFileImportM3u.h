/*----------------------------------------------------------------------------
| File:    PlaylistFileImportM3u.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPlaylistFileImportM3u class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 254 $
| $Id: PlaylistFileImportM3u.h 254 2007-07-05 16:54:51Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once
#include "IPlaylistFileImport.h"


class CPlaylistFileImportM3u : public IPlaylistFileImport
{
    ///// construction / destruction /////
public:
    CPlaylistFileImportM3u();
    virtual ~CPlaylistFileImportM3u();


    ///// attributes /////
protected:
    CString      m_strPlaylistFilePath;
    CString      m_strPlaylistFolder;

    CStringArray m_aPlaylistFile;
    int          m_currentLineIndex;


    ///// methods /////
protected:
    void __init__();

    CString GetNext(int& indexStart);

public:
    virtual CString GetExtension();

    virtual bool Init(const CString& strFilePath);

    virtual bool IsValidPlaylist();

    virtual CString GetFirstItem();
    virtual CString GetNextItem();

};
