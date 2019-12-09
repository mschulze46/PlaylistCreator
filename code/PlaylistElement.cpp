/*----------------------------------------------------------------------------
| File:    PlaylistElement.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CPlaylistElement class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 448 $
| $Id: PlaylistElement.cpp 448 2009-10-19 14:24:33Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "PlaylistElement.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlaylistElement::CPlaylistElement()
{
    m_strFolder     = _T("");
    m_strFileName   = _T("");
    m_strExtension  = _T("");
}


CPlaylistElement::CPlaylistElement(CString strFolder, CString strFileNameDotExtension)
{
    SetFolder(strFolder);
    SetFileNameDotExtension(strFileNameDotExtension);
}


CPlaylistElement::CPlaylistElement(CString strFolder, CString strFileName, CString strExtension)
{
    SetFolder(strFolder);
    SetFileName(strFileName);
    SetExtension(strExtension);
}


CPlaylistElement::~CPlaylistElement()
{
}


//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

void CPlaylistElement::SetFileNameDotExtension(CString strFileNameDotExtension)
{
    // find dot that separates file name and extension
    int dotIndex = strFileNameDotExtension.ReverseFind( _T('.') );

    // no dot?
    if(dotIndex == -1)
    {
        m_strFileName = strFileNameDotExtension;
        m_strExtension = _T("");
        return;
    }

    // extract file name
    m_strFileName = strFileNameDotExtension.Left(dotIndex);

    // extract extension
    strFileNameDotExtension.Delete(0, dotIndex+1);
    m_strExtension = strFileNameDotExtension;
}


void CPlaylistElement::SetFilePath(CString strFilePath)
{
    // split into components
    CString strDrive, strDir, strName, strExt;
    SplitPath(strFilePath, strDrive, strDir, strName, strExt);

    // concatenate strings
    CString strFolder = strDrive + strDir;
    CString strFileNameDotExtension = strName + strExt;

    // set members
    SetFolder(strFolder);
    SetFileNameDotExtension(strFileNameDotExtension);
}
