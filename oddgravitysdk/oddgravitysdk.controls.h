/*----------------------------------------------------------------------------
| File:    oddgravitysdk.controls.h
| Project: oddgravitysdk
|
| Description:
|   GUI controls functions
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 467 $
| $Id: oddgravitysdk.controls.h 467 2009-11-12 15:19:25Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_CONTROLS_H__
#define __ODDGRAVITYSDK_CONTROLS_H__

#include "oddgravitysdk.windows.h"
#include "system/XWinVer.h"

// header defines (>= Windows XP)
#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#endif
#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN            0x0200
#endif

// progress bar defines (>= Windows Vista)
#ifndef PBM_GETSTEP
#define PBM_GETSTEP             (WM_USER+13)
#endif
#ifndef PBM_GETBKCOLOR
#define PBM_GETBKCOLOR          (WM_USER+14)
#endif
#ifndef PBM_GETBARCOLOR
#define PBM_GETBARCOLOR         (WM_USER+15)
#endif
#ifndef PBM_SETSTATE
#define PBM_SETSTATE            (WM_USER+16) // wParam = PBST_[State] (NORMAL, ERROR, PAUSED)
#endif
#ifndef PBM_GETSTATE
#define PBM_GETSTATE            (WM_USER+17)
#endif
#ifndef PBST_NORMAL
#define PBST_NORMAL             0x0001
#endif
#ifndef PBST_ERROR
#define PBST_ERROR              0x0002
#endif
#ifndef PBST_PAUSED
#define PBST_PAUSED             0x0003
#endif


namespace oddgravity
{

// refreshes the selection of a CListBox if an item was removed.
// returns the index of the new selected item or -1 if failed.
static int ListBox_RefreshSelectionAfterItemRemoval(CListBox& listBox, int nSelectionBefore)
{
    if(listBox.GetCount() <= 0 || nSelectionBefore < 0)
        return -1;

    int nSelectionNow = 0;
    if(nSelectionBefore == listBox.GetCount())
    {
        nSelectionNow = nSelectionBefore - 1;
    }
    else
    {
        nSelectionNow = nSelectionBefore;
    }
    int iRetVal = listBox.SetCurSel(nSelectionNow);
    return (iRetVal == LB_ERR) ? -1 : nSelectionNow;
}

// refreshes the selection of a CListCtrl if an item was removed.
// returns the index of the new selected item.
static int ListCtrl_RefreshSelectionAfterItemRemoval(CListCtrl& listCtrl, int nSelectionBefore)
{
    if(listCtrl.GetItemCount() <= 0 || nSelectionBefore < 0)
        return -1;

    int nSelectionNow = 0;
    if(nSelectionBefore == listCtrl.GetItemCount())
    {
        nSelectionNow = nSelectionBefore - 1;
    }
    else
    {
        nSelectionNow = nSelectionBefore;
    }
    BOOL bOkay = listCtrl.SetItemState(nSelectionNow, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
    return (bOkay) ? nSelectionNow : -1;
}

// ensures that nItem is visible. set a positive or negative iOffset
// to make a range around nItem visible.
static BOOL ListCtrl_EnsureVisibleOffset(CListCtrl& listCtrl, int nItem, BOOL bPartialOK, int iOffset = 0)
{
    if(listCtrl.GetItemCount() <= 0 || nItem < 0)
        return FALSE;

    int iListBottomIndex = listCtrl.GetItemCount()-1;
    if(nItem > iListBottomIndex)
        return FALSE;

    int iVisibleItem = 0;
    if(iOffset == 0)
    {
        iVisibleItem = nItem;
    }
    else if(iOffset > 0)
    {
        iVisibleItem = min(nItem + iOffset, iListBottomIndex);
    }
    else if(iOffset < 0)
    {
        iVisibleItem = max(nItem + iOffset, 0);
    }
    return listCtrl.EnsureVisible(iVisibleItem, bPartialOK);
}

// scrolls to the end of the text of an edit control
static void Edit_ScrollToEnd(CEdit& edit)
{
    DWORD dwSel = edit.GetSel();
    edit.SetSel(HIWORD(dwSel), -1);
    edit.SetSel(-1, -1);
}

// sets a sort arrow to the header of the passed ListView
// note: separate bitmaps are only needed on legacy sytems (< Windows XP)
static void ListView_SetHeaderSortArrow(HWND listView, int columnIndex, BOOL isAscending, UINT bmpIdSortUp, UINT bmpIdSortDown)
{
    HWND header = ListView_GetHeader(listView);
    if(!header)
    {
        ASSERT(0); // error: list has no header control
        return;
    }

    BOOL isCommonControlVersion6 = IsCommCtrlVersion6();
    
    int columnCount = Header_GetItemCount(header);
    for(int i = 0; i < columnCount; i++)
    {
        HDITEM hi = {0};
        
        // for Windows XP and above only the format must be retrieved.
        // otherwise, the bitmap must be retrieved as well.
        hi.mask = HDI_FORMAT | (isCommonControlVersion6 ? 0 : HDI_BITMAP);
        
        Header_GetItem(header, i, &hi);
        
        // set sort image to this column?
        if(i == columnIndex)
        {
            // support for header sort image available?
            if(isCommonControlVersion6)
            {
                hi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
                hi.fmt |= isAscending ? HDF_SORTUP : HDF_SORTDOWN;
            }
            else
            {
                UINT bitmapID = isAscending ? bmpIdSortUp : bmpIdSortDown;
                
                // if there is a bitmap, delete it
                if(hi.hbm)
                    DeleteObject(hi.hbm);
                
                hi.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
                hi.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
            }
        }
        else // not the sort column
        {
            // remove sort image from other columns
            if(isCommonControlVersion6)
            {
                hi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
            }
            else
            {
                // if there is a bitmap, delete it
                if(hi.hbm)
                    DeleteObject(hi.hbm);
                
                hi.mask &= ~HDI_BITMAP;
                hi.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
            }
        }
        
        Header_SetItem(header, i, &hi);
    }
}


// removes the sort arrow from the header of the passed ListView
static void ListView_RemoveHeaderSortArrow(HWND listView)
{
    ListView_SetHeaderSortArrow(listView, -1, TRUE, 0, 0);
}


// sets the state of a progress control (>= Windows Vista)
static void CProgressCtrl_SetState(CProgressCtrl& progressCtrl, int iState)
{
    // only send message if Windows Vista or later
    if(WinVersion.IsVistaOrLater())
    {
        // reset to normal state (required for values to be updated properly, but causes a slight flicker)
        SendMessage(progressCtrl.GetSafeHwnd(), PBM_SETSTATE, (WPARAM)PBST_NORMAL, 0);

        // set state
        SendMessage(progressCtrl.GetSafeHwnd(), PBM_SETSTATE, (WPARAM)iState, 0);
    }
}


}

#endif // __ODDGRAVITYSDK_CONTROLS_H__
