/*----------------------------------------------------------------------------
| File:    oddgravitysdk.stl.h
| Project: oddgravitysdk
|
| Description:
|   STL helper methods
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 482 $
| $Id: oddgravitysdk.stl.h 482 2010-01-08 14:34:33Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_STL_H__
#define __ODDGRAVITYSDK_STL_H__

namespace oddgravity
{
    /////////////////////////////////////////////////////////////////////////////
    // std::sort handlers
    //

    static bool SortAscendingString(const CString& s1, const CString& s2)
    {
        return s1 < s2;
    }

    static bool SortDescendingString(const CString& s1, const CString& s2)
    {
        return s1 > s2;
    }
}

#endif // __ODDGRAVITYSDK_STL_H__
