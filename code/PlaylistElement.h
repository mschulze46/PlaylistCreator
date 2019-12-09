/*----------------------------------------------------------------------------
| File:    PlaylistElement.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPlaylistElement class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 28 $
| $Id: PlaylistElement.h 28 2006-10-15 16:57:50Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#if !defined(AFX_PLAYLISTELEMENT_H__4841C50C_FEBA_4C6E_9645_1399A73F5FA3__INCLUDED_)
#define AFX_PLAYLISTELEMENT_H__4841C50C_FEBA_4C6E_9645_1399A73F5FA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPlaylistElement  
{
    ///// methods /////
public:
    CPlaylistElement();
    CPlaylistElement(CString strFolder, CString strFileNameDotExtension);
    CPlaylistElement(CString strFolder, CString strFileName, CString strExtension);
    virtual ~CPlaylistElement();    

    CString GetFolder() { return m_strFolder; }
    void SetFolder(CString strFolder) { m_strFolder = PathAddTrailingBackSlash(strFolder); }

    CString GetFileName() { return m_strFileName; }
    void SetFileName(CString strFileName) { m_strFileName = strFileName; }

    CString GetExtension() { return m_strExtension; }
    void SetExtension(CString strExtension) { m_strExtension = strExtension; }

    CString GetFileNameDotExtension() { return m_strFileName + _T(".") + m_strExtension; }
    void SetFileNameDotExtension(CString strFileNameDotExtension);

    CString GetFilePath() { return m_strFolder + m_strFileName + _T(".") + m_strExtension; }
    void SetFilePath(CString strFilePath);


    ///// attributes /////
protected:    
    CString m_strFolder;
    CString m_strFileName;
    CString m_strExtension;
};

#endif // !defined(AFX_PLAYLISTELEMENT_H__4841C50C_FEBA_4C6E_9645_1399A73F5FA3__INCLUDED_)
