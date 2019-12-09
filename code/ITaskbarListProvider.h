/*----------------------------------------------------------------------------
| File:    ITaskbarListProvider.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the ITaskbarListProvider interface.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 468 $
| $Id: ITaskbarListProvider.h 468 2009-11-12 16:48:36Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once


class ITaskbarListProvider
{
public:
    // destruction
    virtual ~ITaskbarListProvider() {};

    // sets the progress state (e.g. normal, paused, indeterminate)
    virtual HRESULT SetProgressState(TBPFLAG tbpFlags) = 0;

    // sets the progress value
    virtual HRESULT SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal) = 0;
};
