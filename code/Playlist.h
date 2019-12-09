/*----------------------------------------------------------------------------
| File:    Playlist.h
| Project: Playlist Creator
|
| Description:
|   Interface for the CPlaylist class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 28 $
| $Id: Playlist.h 28 2006-10-15 16:57:50Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#if !defined(AFX_PLAYLIST_H__8397DB22_CA15_4356_92DD_EE2279BDFD3B__INCLUDED_)
#define AFX_PLAYLIST_H__8397DB22_CA15_4356_92DD_EE2279BDFD3B__INCLUDED_

#include <stdexcept>
#include <vector>
#include "PlaylistElement.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


///// forwards /////
class CPlaylistElement;


///// defines /////

#define PLAYLIST_EXT_PLS        _T(".pls")
#define PLAYLIST_EXT_M3U        _T(".m3u")


///// types /////

enum PLAYLIST_TYPE
{
    PT_PLS = 0,
    PT_M3U,

    PT_COUNT    // must remain the last item
};

typedef std::vector<CPlaylistElement*> CPlaylistElements;


///// classes /////

class CPlaylist
{
    ///// methods /////
public:

    CPlaylist();
    virtual ~CPlaylist();    

    void __init__();
    void Destroy();

    int GetCount() { return (int)m_Elements.size(); }
    BOOL IsEmpty() { return m_Elements.empty(); };

    CString GetFolder() { return m_strFolder; }
    void SetFolder(CString strFolder) { m_strFolder = PathAddTrailingBackSlash(strFolder); };

    CString GetName() { return m_strName; }
    void SetName(CString strPlaylistName) { m_strName = strPlaylistName; }

    CString GetFilePath();

    PLAYLIST_TYPE GetType() { return m_Type; }
    void SetType(PLAYLIST_TYPE type) { m_Type = type; }
	void SetTypeAsString(const CString& strType);

    void Add(CPlaylistElement* pElement);

    CPlaylistElement* GetAt(int nIndex);


    ///// attributes /////
protected:
    CString m_strFolder;            // folder
    CString m_strName;              // file name

    PLAYLIST_TYPE m_Type;           // type

    CPlaylistElements m_Elements;   // elements
};

#endif // !defined(AFX_PLAYLIST_H__8397DB22_CA15_4356_92DD_EE2279BDFD3B__INCLUDED_)
