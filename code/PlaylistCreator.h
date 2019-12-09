/*----------------------------------------------------------------------------
| File:    PlaylistCreator.h
| Project: Playlist Creator
|
| Description:
|   Main header file for the PlaylistCreator application.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 158 $
| $Id: PlaylistCreator.h 158 2007-05-14 21:20:37Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CPlaylistCreatorApp:
// See PlaylistCreator.cpp for the implementation of this class
//

class CPlaylistCreatorApp : public CWinApp
{
	///// members /////
	HANDLE	m_hMutexSingleInstance;


	///// methods /////
public:
	CPlaylistCreatorApp();
	virtual ~CPlaylistCreatorApp();

	static BOOL CALLBACK WndIterator(HWND hWnd, LPARAM lParam);

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPlaylistCreatorApp theApp;