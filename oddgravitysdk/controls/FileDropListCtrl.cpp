///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CFileDropListCtrl - Enhanced CListCtrl that accepts and filters dropped files/folders.
//
//  Jan 2000, Stuart Carter, stuart.carter@hotmail.com
//  You're free to use, modify and distribute this code as long as credit is given...
//
//      Thanks to:
//      Handling of droppped files modified from:
//          CDropEdit, 1997 Chris Losinger
//          http://www.codeguru.com/editctrl/filedragedit.shtml
//
//      Shortcut expansion code modified from:
//          CShortcut, 1996 Rob Warner
//
//
//  History:
//
//  Version Date        Author              Description
//  ------- ----------  ------------------- --------------------------------
//  1.0     2000-01-20  Stuart Carter       initial release
//

#include "stdafx.h"
#include "FileDropListCtrl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <afxdisp.h>        // OLE stuff
#include <shlwapi.h>        // Shell functions (PathFindExtension() in this case)
#include <afxpriv.h>        // ANSI to/from Unicode conversion macros
#include <MMSystem.h>       // PlaySound

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// defines
#define DRAG_DROP_LINE_COLOR        RGB(0,0,0)
#define DRAG_DROP_LINE_WIDTH        2


CFileDropListCtrl::CFileDropListCtrl()
    : inherited()
{
    // default drop mode
    m_dropMode.nMask        = DL_ACCEPT_FILES | DL_ACCEPT_FOLDERS;
    m_dropMode.pfnCallback  = NULL;

    // initialize OLE libraries
    m_bMustUninitOLE = FALSE;
    _AFX_THREAD_STATE* pState = AfxGetThreadState();
    if(!pState->m_bNeedTerm) // TRUE if OleUninitialize needs to be called
    {
        HRESULT hr = ::OleInitialize(NULL);
        if(FAILED(hr))
        {
            ASSERT(0);
            AfxMessageBox(_T("OLE initialization failed.\n\nMake sure that the OLE libraries have the correct version."));
        }
        else
        {
            m_bMustUninitOLE = TRUE;
        }
    }

    // initialize IDropTargetHelper for shaded drag&drop
    m_piDropTargetHelper = NULL;
    m_bUseDropTargetHelper = FALSE;
    if( SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
        IID_IDropTargetHelper, (LPVOID*)&m_piDropTargetHelper)) )
    {
        m_bUseDropTargetHelper = TRUE;
    }

    // init members
    m_bIsDroppableData          = FALSE;
    m_bIsSpecialDropKeyPressed  = FALSE;
    m_dropIndex                 = -1;
}

CFileDropListCtrl::~CFileDropListCtrl()
{
    if(m_piDropTargetHelper)
    {
        m_piDropTargetHelper->Release();
    }

    if(m_bMustUninitOLE)
    {
        ::OleUninitialize();
    }
}

BEGIN_MESSAGE_MAP(CFileDropListCtrl, inherited)
    //{{AFX_MSG_MAP(CFileDropListCtrl)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//  PUBLIC SetDropMode()
//
//  Specify how the control will react to dropped files/folders.
//
//  Return value:
//      FALSE:  the structure was not populated correctly, 
//              the default settings will be used.
//      TRUE:   changes to the drop mode accepted
//

BOOL CFileDropListCtrl::SetDropMode(const CFileDropListCtrl::DROPLISTMODE& dropMode)
{
    // if we use a callback, ensure that we have a valid function pointer
    if(dropMode.nMask & DL_USE_CALLBACK && dropMode.pfnCallback == NULL)
    {
        ASSERT(0);
        return FALSE;
    }

    m_dropMode = dropMode;
    return TRUE;
}


//////////////////////////////////////////////////////////////////
//
//  ExpandShortcut()
//
//  Uses IShellLink to expand a shortcut.
//
//  Return value:
//      the expanded filename, or "" on error or if filename
//      wasn't a shortcut
//
//  Adapted from CShortcut, 1996 by Rob Warner
//  rhwarner@southeast.net
//  http://users.southeast.net/~rhwarner
//

CString CFileDropListCtrl::ExpandShortcut(CString& strFilename) const
{
    USES_CONVERSION; // for T2COLE() below
    CString strExpandedFile;

    //
    // Make sure we have a path
    //
    if(strFilename.IsEmpty())
    {
        ASSERT(0);
        return strExpandedFile;
    }

    //
    // Get a pointer to the IShellLink interface
    //
    HRESULT hr;
    IShellLink* pIShellLink;

    hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*) &pIShellLink);

    if (SUCCEEDED(hr))
    {
        //
        // Get a pointer to the persist file interface
        //
        IPersistFile* pIPersistFile;
        hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*) &pIPersistFile);

        if (SUCCEEDED(hr))
        {
            //
            // Load the shortcut and resolve the path
            //
            // IPersistFile::Load() expects a UNICODE string
            // so we're using the T2COLE macro for the conversion
            //
            // For more info, check out MFC Technical note TN059
            // (these macros are also supported in ATL and are
            // so much better than the ::MultiByteToWideChar() family)
            //
            hr = pIPersistFile->Load(T2COLE(strFilename), STGM_READ);
            
            if (SUCCEEDED(hr))
            {
                WIN32_FIND_DATA wfd;
                hr = pIShellLink->GetPath(strExpandedFile.GetBuffer(MAX_PATH),
                                          MAX_PATH,
                                          &wfd,
                                          SLGP_UNCPRIORITY);

                strExpandedFile.ReleaseBuffer(-1);
            }
            pIPersistFile->Release();
        }
        pIShellLink->Release();
    }

    return strExpandedFile;
}


//////////////////////////////////////////////////////////////////
//
//  ValidatePathname()
//
//  Checks if a pathname is valid based on these options set:
//      Allow directories to be dropped
//      Allow files to be dropped
//      Only allow files with a certain extension to be dropped
//
//  Return value:
//      TRUE:   the pathname is suitable for selection, or
//      FALSE:  the pathname failed the checks.
//
//      If successful, iPathType specifies the type of path
//      validated - either a file or a folder.

BOOL CFileDropListCtrl::ValidatePathname(const CString& strPathname, UINT& iPathType) const
{
    //
    // Get some info about that path so we can filter out dirs
    // and files if need be
    //
    BOOL bValid = FALSE;

    struct _stat buf;
    int result = _tstat(strPathname, &buf);
    if(result == 0)
    {
        //
        // Do we have a directory? (if we want dirs)
        //
        if((m_dropMode.nMask & DL_ACCEPT_FOLDERS)
            && ((buf.st_mode & _S_IFDIR) == _S_IFDIR))
        {
            bValid = TRUE;
            iPathType = DL_TYPE_FOLDER;
        } 
        else if ((m_dropMode.nMask & DL_ACCEPT_FILES)
            && ((buf.st_mode & _S_IFREG) == _S_IFREG))
        {
            // it's a file and files are allowed
            iPathType = DL_TYPE_FILE;

            // check the file extension, if selected
            if(m_dropMode.nMask & DL_FILTER_EXTENSION)
            {
                LPTSTR pszFileExt = PathFindExtension(strPathname);
                int i=0;
                CString strExt;
                while(bValid == false && AfxExtractSubString(strExt, m_dropMode.strFileExt, i, ';'))
                {
                    if(CString(pszFileExt).CompareNoCase(strExt) == 0)
                    {
                        bValid = true;
                    }
                    i++;
                }
            }
            else
            {
                bValid = true;
            } 
        }
    }

    return bValid;
}


//////////////////////////////////////////////////////////////////
//
//  InsertPathname()
//
//  This is used to insert a dropped item when a callback function
//  hasn't been specified.
//
//  It also checks if duplicate files are allowed to be inserted
//  and does the necessary.
//

int CFileDropListCtrl::InsertPathname(const CString& strFilename)
{
    if(!(m_dropMode.nMask & DL_ALLOW_DUPLICATES))
    {
        //
        // We don't allow duplicate pathnames, so
        // see if this one is already in the list.
        //
        LVFINDINFO lvInfo;
        lvInfo.flags = LVFI_STRING;
        lvInfo.psz = strFilename;

        if(FindItem(&lvInfo, -1) != -1)
            return -1;
    }

    return InsertItem(0, strFilename);
}


// selects the passed item
void CFileDropListCtrl::SelectItem(int index)
{
    if(index < 0 || index >= GetItemCount())
    {
        ASSERT(0);
        return;
    }

    SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}


// activates the parent window
void CFileDropListCtrl::ActivateWindow()
{
   CWnd* pWnd = GetParent();
   if(pWnd)
   {
      pWnd->SetForegroundWindow();
      pWnd->BringWindowToTop();
      pWnd->SetActiveWindow();
   }
}


//
// called when the cursor first enters the window
//
DROPEFFECT CFileDropListCtrl::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
    // default action: no dropping allowed
    DROPEFFECT dwEffect = DROPEFFECT_NONE;

    // check if dragged data can be dropped here
    HGLOBAL hGlobal = pDataObject->GetGlobalData(CF_HDROP);
    if(hGlobal != NULL)
    {
        m_bIsDroppableData = TRUE;

        // bring window to top
        ActivateWindow();

        // deselect items
        inherited::DeselectAllItems();

        // use the copy cursor
        dwEffect = DROPEFFECT_COPY;
    }
    else
    {
        m_bIsDroppableData = FALSE;
    }

    // call the drag and drop helper
    if(m_bUseDropTargetHelper)
    {
        IDataObject* piDataObj = pDataObject->GetIDataObject(FALSE); // no AddRef() is done
        m_piDropTargetHelper->DragEnter(GetSafeHwnd(), piDataObj, &point, dwEffect);
    }

    return dwEffect;
}


//
// called repeatedly when the cursor is dragged over the window
//
DROPEFFECT CFileDropListCtrl::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
    // default action: no dropping allowed
    DROPEFFECT dwEffect = DROPEFFECT_NONE;

    // only crunch the data if it can be dropped
    if(m_bIsDroppableData)
    {
        // update special drop key press state
        if(IsKeyPressed(VK_LCONTROL) || IsKeyPressed(VK_RCONTROL))
        {
            m_bIsSpecialDropKeyPressed = TRUE;
            dwEffect = DROPEFFECT_COPY;
        }
        else
        {
            m_bIsSpecialDropKeyPressed = FALSE;
            dwEffect = DROPEFFECT_MOVE;
        }

        // are we dragging over an item?
        int index = HitTestVCenter(point);
        if(index != -1)
        {
            // scroll if necessary
            DoDragDropScrolling(point, index);

            // redraw insertion line if necessary
            if(index != m_dropIndex)
            {
                // draw insertion line
                DrawDragDropLine(index);
            }
        }
    }

    // call the drag and drop helper
    if(m_bUseDropTargetHelper)
    {
        // check if GUI option "Show window contents while dragging" is off
        BOOL bDragFullWindows = TRUE;
        BOOL bSuccess = ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &bDragFullWindows, 0);
        if(bSuccess && !bDragFullWindows)
        {
            // point must be converted to screen coordinates, otherwise
            // there will be an offset in the drop graphic
            ClientToScreen(&point);
        }

        m_piDropTargetHelper->DragOver(&point, dwEffect);
    }

    return dwEffect;
}


//
// called when data is dropped into the window, default handler
//
BOOL CFileDropListCtrl::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
    // call the drag and drop helper
    // note: drop graphic will not be removed until this call
    DROPEFFECT dwEffect = DROPEFFECT_COPY;
    if(m_bUseDropTargetHelper)
    {
        IDataObject* piDataObj = pDataObject->GetIDataObject(FALSE); // no AddRef() is done
        m_piDropTargetHelper->Drop(piDataObj, &point, dwEffect);
    }

    // reset members
    m_DroppedItems.arrayDroppedItems.RemoveAll();
    m_bIsDroppableData = FALSE;

    // clear the last line that was drawn
    ClearOldDragDropLine(m_dropIndex);

    // buffers
    BOOL bDroppedDataValid = FALSE;
    HGLOBAL hGlobal        = NULL;
    HDROP hDrop            = NULL;
    UINT nFilesDropped     = 0;

    // get the HDROP data from the data object
    hGlobal = pDataObject->GetGlobalData(CF_HDROP);
    if(!hGlobal)
    {
        ASSERT(0);
        return FALSE;
    }

    hDrop = (HDROP)GlobalLock(hGlobal);
    if(!hDrop)
    {
        GlobalUnlock(hGlobal);
        ASSERT(0);
        return FALSE;
    }

    // get the number of files being dropped
    nFilesDropped = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

    // iterate over dropped files
    TCHAR szFilename[MAX_PATH + 1];
    CString strPathname;
    //CString strExpandedFilename;

    // put all dropped items in a list
    for(UINT nFile = 0; nFile < nFilesDropped; nFile++)
    {
        // get the fully qualified pathname
        DragQueryFile(hDrop, nFile, szFilename, MAX_PATH + 1);

        // it might be a shortcut, so try to expand it
        strPathname = szFilename;
        //strExpandedFilename = ExpandShortcut(strPathname);
        //if(!strExpandedFilename.IsEmpty())
        //{
        //    strPathname = strExpandedFilename;
        //}

        // now see if it is something we allow to be dropped
        UINT iPathType = 0;
        if(ValidatePathname(strPathname, iPathType))
        {
            bDroppedDataValid = TRUE; // we found at least one valid dropped item

            // using a callback?
            if(m_dropMode.nMask & DL_USE_CALLBACK)
                m_DroppedItems.arrayDroppedItems.Add(strPathname);
            else
                InsertPathname(strPathname);
        }
    }

    // call the provided callback
    if((m_dropMode.nMask & DL_USE_CALLBACK) && m_dropMode.pfnCallback)
    {
        m_DroppedItems.pList      = this;
        m_DroppedItems.iDropIndex = m_dropIndex;
        m_DroppedItems.bRecursive = (m_bIsSpecialDropKeyPressed) ? true : false;
        m_dropMode.pfnCallback(&m_DroppedItems);
    }

    // release global memory object
    GlobalUnlock(hGlobal);

    // reset drop index
    m_dropIndex = LB_ERR;

    // check if dropped data is valid
    if(!bDroppedDataValid)
    {
        PlaySound(_T("SystemQuestion"), NULL, SND_ALIAS | SND_ASYNC | SND_NOWAIT);
        return FALSE;
    }

    return TRUE;
}


//
// Called when the cursor is dragged out of the window
//
void CFileDropListCtrl::OnDragLeave(CWnd* pWnd)
{
    // reset drop flag
    m_bIsDroppableData = FALSE;

    // deselect items
    inherited::DeselectAllItems();

    // clear the last line that was drawn
    ClearOldDragDropLine(m_dropIndex);

    // reset drop index
    m_dropIndex = LB_ERR;

    // call the drag and drop helper
    if(m_bUseDropTargetHelper)
    {
        m_piDropTargetHelper->DragLeave();
    }
}


//-------------------------------------
// drag and drop helpers
//-------------------------------------

void CFileDropListCtrl::DrawDragDropLine(int index)
{
    CDC* pDC = GetDC();

    if(!(index == GetItemCount() && m_dropIndex == LB_ERR)) // fixes that initially no bottom line is drawn
    {
        ClearOldDragDropLine(m_dropIndex);
    }

    m_dropIndex = index;

    DrawNewDragDropLine(pDC, m_dropIndex);

    ReleaseDC(pDC);
}


void CFileDropListCtrl::ClearOldDragDropLine(int index)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    CRect rect;
    CRect rectTriangle(0, 0, 2, 6);

    if((index != LB_ERR) && (index < GetItemCount()))
    {
        GetVisibleItemRect(index, &rect, LVIR_BOUNDS);
        if(clientRect.PtInRect(rect.TopLeft()))
        {
            // main line
            rect.bottom = rect.top + DRAG_DROP_LINE_WIDTH; // adjust invalidate rect
            InvalidateRect(&rect);

            // left triangle
            rectTriangle.MoveToXY(rect.left, rect.top - 2);
            InvalidateRect(rectTriangle);

            // right triangle
            rectTriangle.MoveToXY(rect.right - 2, rect.top - 2);
            InvalidateRect(rectTriangle);
        }
    }
    else
    {
        GetVisibleItemRect(GetItemCount() - 1, &rect, LVIR_BOUNDS);
        if(clientRect.PtInRect(CPoint(0, rect.bottom)))
        {
            // main line
            rect.bottom = rect.bottom + (DRAG_DROP_LINE_WIDTH / 2);
            rect.top    = rect.bottom - DRAG_DROP_LINE_WIDTH;
            InvalidateRect(&rect);

            // left triangle
            rectTriangle.MoveToXY(rect.left, rect.top - 2);
            InvalidateRect(rectTriangle);

            // right triangle
            rectTriangle.MoveToXY(rect.right - 2, rect.top - 2);
            InvalidateRect(rectTriangle);
        }
    }
}


void CFileDropListCtrl::DrawNewDragDropLine(CDC* pDC, int index)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    CRect rect;

    CPen pen(PS_SOLID, DRAG_DROP_LINE_WIDTH, DRAG_DROP_LINE_COLOR);
    CPen* pOldPen = pDC->SelectObject(&pen);

    if((index != LB_ERR) && (index < GetItemCount()))
    {
        GetVisibleItemRect(index, &rect, LVIR_BOUNDS);
        if(clientRect.PtInRect(rect.TopLeft()))
        {
            // main line
            pDC->MoveTo(rect.left, rect.top + 1);
            pDC->LineTo(rect.right - (DRAG_DROP_LINE_WIDTH / 2), rect.top + 1);

            // left triangle
            pDC->FillSolidRect(rect.left, rect.top - 2, 1, 6, DRAG_DROP_LINE_COLOR);
            pDC->FillSolidRect(rect.left, rect.top - 1, 2, 4, DRAG_DROP_LINE_COLOR);

            // right triangle
            pDC->FillSolidRect(rect.right - 1, rect.top - 2, 1, 6, DRAG_DROP_LINE_COLOR);
            pDC->FillSolidRect(rect.right - 2, rect.top - 1, 2, 4, DRAG_DROP_LINE_COLOR);
        }
    }
    else
    {
        GetVisibleItemRect(GetItemCount() - 1, &rect, LVIR_BOUNDS);
        if(clientRect.PtInRect(CPoint(0, rect.bottom)))
        {
            // main line
            pDC->MoveTo(rect.left, rect.bottom);
            pDC->LineTo(rect.right - (DRAG_DROP_LINE_WIDTH / 2), rect.bottom);

            // left triangle
            pDC->FillSolidRect(rect.left, rect.bottom - 3, 1, 6, DRAG_DROP_LINE_COLOR);
            pDC->FillSolidRect(rect.left, rect.bottom - 2, 2, 4, DRAG_DROP_LINE_COLOR);

            // right triangle
            pDC->FillSolidRect(rect.right - 1, rect.bottom - 3, 1, 6, DRAG_DROP_LINE_COLOR);
            pDC->FillSolidRect(rect.right - 2, rect.bottom - 2, 2, 4, DRAG_DROP_LINE_COLOR);
        }
    }

    pDC->SelectObject(pOldPen);
}


void CFileDropListCtrl::DoDragDropScrolling(const CPoint& point, int index)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    int scrollHeight = 20; // start scrolling if cursor is +/- 20px from top or bottom

    if(point.y < scrollHeight && index > 0)
    {
        EnsureVisible(index - 1, FALSE);

        CHeaderCtrl* pHeader = GetHeaderCtrl();
        if(pHeader)
        {
            pHeader->Invalidate(); // first fix for dropline fragments when scrolling upwards
        }
    }
    else if(point.y > (clientRect.bottom - scrollHeight) && index < GetItemCount() - 1)
    {
        EnsureVisible(index + 1, FALSE);
    }
    else
    {
        EnsureVisible(index, FALSE);
    }
}


// determines which list view item, if any, is at a specified position.
// item's vcenter is the border!
int CFileDropListCtrl::HitTestVCenter(CPoint pt, UINT* pFlags /*NULL*/)
{
    // get item from point
    int nItem = HitTest(pt, pFlags);

    // are we over an item?
    if(nItem != -1)
    {
        CRect itemRect;
        GetItemRect(nItem, &itemRect, LVIR_BOUNDS);

        if(pt.y > itemRect.top + (itemRect.Height() / 2))
        {
            // hit test point is below middle of item
            nItem++;
        }
    }

    return nItem;
}


BOOL CFileDropListCtrl::GetVisibleItemRect(int nItem, LPRECT lpRect, UINT nCode)
{
    BOOL result = FALSE;

    // get client rect
    CRect clientRect;
    GetClientRect(&clientRect);

    // get item rect
    result = GetItemRect(nItem, lpRect, nCode);

    if(result)
    {
        // clip item rect to client rect if necessary
        if(lpRect->left < 0) lpRect->left = clientRect.left;
        if(lpRect->right > clientRect.Width()) lpRect->right = clientRect.Width();
    }

    return result;
}


