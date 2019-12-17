/*----------------------------------------------------------------------------
| File:    oddgravitysdk.macros.h
| Project: oddgravitysdk
|
| Description:
|   Macros
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 18 $
| $Id: oddgravitysdk.macros.h 18 2006-10-12 19:25:07Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_MACROS_H__
#define __ODDGRAVITYSDK_MACROS_H__


namespace oddgravity
{

// nice macro for defining user messages
#define DECLARE_USER_MESSAGE(name)      static const UINT name = ::RegisterWindowMessage(name##_MSG);

// compile reminder
//
// Statements like:
// #pragma message(Reminder "Fix this problem!")
//
// Which will cause messages like:
// C:\Source\Project\main.cpp(47): Reminder: Fix this problem!
//
// to show up during compiles. Note that you can NOT use the
// words "error" or "warning" in your reminders, since it will
// make the IDE think it should abort execution. You can double
// click on these messages and jump to the line in question.
#define Stringize( L )          #L
#define MakeString( M, L )      M(L)
#define $Line                   MakeString( Stringize, __LINE__ )
#define Reminder                __FILE__ "(" $Line ") : Reminder: "



}

#endif // __ODDGRAVITYSDK_MACROS_H__
