/*----------------------------------------------------------------------------
| File:    PlaylistFileImportPls.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPlaylistFileImportPls class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 254 $
| $Id: PlaylistFileImportPls.h 254 2007-07-05 16:54:51Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once
#include "IPlaylistFileImport.h"


class CPlaylistFileImportPls : public IPlaylistFileImport
{
    ///// construction / destruction /////
public:
    CPlaylistFileImportPls();
    virtual ~CPlaylistFileImportPls();


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
