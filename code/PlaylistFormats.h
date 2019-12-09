/*----------------------------------------------------------------------------
| File:    PlaylistFormats.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the playlist formats.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 391 $
| $Id: PlaylistFormats.h 391 2009-04-30 16:03:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#define PLAYLIST_TYPE_PLS   _T(".pls")
#define PLAYLIST_TYPE_M3U   _T(".m3u")


class IPlaylistFormat
{
public:
    // destruction
    virtual ~IPlaylistFormat() {};

    // returns the format ID
    virtual CString GetID() = 0;

    // returns the format extenstion
    virtual CString GetExtension() = 0;

    // returns the header
    virtual CString GetHeader() = 0;

    // returns the plain header (i.e. minimalistic data, can be null)
    virtual CString GetHeaderPlain() = 0;

    // returns the item template
    virtual CString GetItem() = 0;

    // returns the item template with placeholders for tag data
    virtual CString GetItemWithTagData() = 0;

    // returns the plain item template (i.e. minimalistic data, can be null)
    virtual CString GetItemPlain() = 0;

    // returns the footer
    virtual CString GetFooter() = 0;

    // returns the plain footer (i.e. minimalistic data, can be null)
    virtual CString GetFooterPlain() = 0;
};


class CPlaylistFormatPls : public IPlaylistFormat
{
public:
    CPlaylistFormatPls() { }
    virtual ~CPlaylistFormatPls() { }

    virtual CString GetID()
    {
        return _T("PLS");
    }

    virtual CString GetExtension()
    {
        return _T(".pls");
    }

    virtual CString GetHeader()
    {
        return _T("[playlist]\n");
    }

    virtual CString GetHeaderPlain()
    {
        return _T("");
    }

    virtual CString GetItem()
    {
        return   CString(_T("File<TRACK/>=<FILEPATH/>\n"))
               + CString(_T("Title<TRACK/>=<FILENAME/>\n"));
    }

    virtual CString GetItemWithTagData()
    {
        return   CString(_T("File<TRACK/>=<FILEPATH/>\n"))
               + CString(_T("Title<TRACK/>=<TITLE/>\n"))
               + CString(_T("Length<TRACK/>=<LENGTH/>\n"));
    }

    virtual CString GetItemPlain()
    {
        return _T("<FILEPATH/>\n");
    }

    virtual CString GetFooter()
    {
        return   CString(_T("NumberOfEntries=<TRACKCOUNT/>\n"))
               + CString(_T("Version=2\n"));
    }

    virtual CString GetFooterPlain()
    {
        return _T("");
    }
};


class CPlaylistFormatM3u : public IPlaylistFormat
{
public:
    CPlaylistFormatM3u() { }
    virtual ~CPlaylistFormatM3u() { }

    virtual CString GetID()
    {
        return _T("M3U");
    }

    virtual CString GetExtension()
    {
        return _T(".m3u");
    }

    virtual CString GetHeader()
    {
        return _T("#EXTM3U\n");
    }

    virtual CString GetHeaderPlain()
    {
        return _T("");
    }

    virtual CString GetItem()
    {
        return   CString(_T("#EXTINF:-1,<FILENAME/>\n"))
               + CString(_T("<FILEPATH/>\n"));
    }

    virtual CString GetItemWithTagData()
    {
        return   CString(_T("#EXTINF:<LENGTH/>,<TITLE/>\n"))
               + CString(_T("<FILEPATH/>\n"));
    }

    virtual CString GetItemPlain()
    {
        return _T("<FILEPATH/>\n");
    }

    virtual CString GetFooter()
    {
        return _T("");
    }

    virtual CString GetFooterPlain()
    {
        return _T("");
    }
};
