/*----------------------------------------------------------------------------
| File:    IPlaylistFileImport.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the IPlaylistFileImport interface.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 253 $
| $Id: IPlaylistFileImport.h 253 2007-07-04 22:56:12Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once


class IPlaylistFileImport
{
public:
    // destruction
    virtual ~IPlaylistFileImport() {};

    // returns the file extension of the playlist format
    virtual CString GetExtension() = 0;

    // initializes the instance with a playlist
    virtual bool Init(const CString& strFilePath) = 0;

    // returns whether the playlist is valid
    virtual bool IsValidPlaylist() = 0;

    // returns the first item in the playlist
    virtual CString GetFirstItem() = 0;

    // returns the next item in the playlist (returns an empty string if the end is reached)
    virtual CString GetNextItem() = 0;

};
