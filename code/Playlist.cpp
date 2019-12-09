/*----------------------------------------------------------------------------
| File:    Playlist.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPlaylist class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 28 $
| $Id: Playlist.cpp 28 2006-10-15 16:57:50Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Playlist.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlaylist::CPlaylist()
{
    __init__();
}


CPlaylist::~CPlaylist()
{
    Destroy();
}


//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

void CPlaylist::__init__()
{
    m_strFolder     = _T("");
    m_strName       = _T("");

    m_Type          = PT_PLS;    // default: PLS

    m_Elements.clear();
}


void CPlaylist::Destroy()
{
    m_Elements.clear();
}


CString CPlaylist::GetFilePath()
{
    ASSERT(m_strFolder.GetLength());
    ASSERT(m_strName.GetLength());

    CString strFilePath = m_strFolder + m_strName;

    if(m_Type == PT_PLS)
    {
        strFilePath += PLAYLIST_EXT_PLS;
    }
    else if(m_Type == PT_M3U)
    {
        strFilePath += PLAYLIST_EXT_M3U;
    }
    else
    {
        ASSERT(0); // error: unknown type
        return _T("");
    }

    return strFilePath;
}


void CPlaylist::SetTypeAsString(const CString& strType)
{
    if(strType.CompareNoCase(PLAYLIST_EXT_PLS) == 0)
    {
        m_Type = PT_PLS;
    }
    else if(strType.CompareNoCase(PLAYLIST_EXT_M3U) == 0)
    {
        m_Type = PT_M3U;
    }
    else
    {
        ASSERT(0); // error: unknown type
    }
}


void CPlaylist::Add(CPlaylistElement* pElement)
{
    if(!pElement)
    {
        ASSERT(0);
        return;
    }

    m_Elements.push_back(pElement);
}


CPlaylistElement* CPlaylist::GetAt(int nIndex)
{
    if(IsEmpty() || nIndex < 0 || nIndex >= GetCount())
        return NULL;

    try
    {
        return m_Elements.at(nIndex);
    }
    catch(std::out_of_range &ex)
    {
#ifdef _DEBUG
        MessageBox(GetDesktopWindow(), ex.what(), _T("Exception catched..."), MB_OK | MB_ICONERROR);
#endif
        ex; // prevents compiler warning C4101 (unreferenced local variable)
        return NULL;
    }
    catch(...)
    {
        ASSERT(0);
        return NULL;
    }
}
