/*----------------------------------------------------------------------------
| File:    PlaylistFileManager.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPlaylistFileManager class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 261 $
| $Id: PlaylistFileManager.h 261 2007-07-08 14:22:07Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once


class CPlaylistFileManager
{
    ///// construction / destruction /////
public:
    CPlaylistFileManager();
    virtual ~CPlaylistFileManager();


    ///// attributes /////
protected:
    CStringArray m_arraySupportedFileTypes;


    ///// methods /////
protected:
    void __init__();

public:
    void GetSupportedFileTypes(CStringArray& arrayFileTypes);
    bool IsPlaylistFilePath(const CString& strFilePath);
    bool IsPlaylistFileExtension(const CString& strExtension);

    bool ImportPlaylist(const CString& strPlaylistFilePath, CStringArray& aImportedFiles);

};
