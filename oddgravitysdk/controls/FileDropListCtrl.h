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

#pragma once
#include <afxole.h>                     // COleDropTarget
#include <ShlObj.h>                     // IDropTargetHelper
#include <controls/ReportListCtrl.h>


// struct for dropped items
class CDroppedItems
{
    ////// members /////
public:
    CListCtrl*   pList;             // pointer to the list control calling you
    CStringArray arrayDroppedItems; // array with fully qualified paths to dropped items (file/folder)
    int          iDropIndex;        // index of the item that was dropped onto (-1: dropped on no item)
    bool         bRecursive;        // decides whether all folders are added recursively

    ///// methods /////
public:
    CDroppedItems() {};
};


// declare your (optional) callback function like this:
//   static HRESULT CALLBACK MyDragDropCallback(CDroppedItems* pDroppedItems)
typedef HRESULT (CALLBACK FAR * LPFN_DROP_FILES_CALLBACK)(CDroppedItems*);


//------------------------------------------------------
// class CFileDropListCtrl
//------------------------------------------------------
class CFileDropListCtrl : public CReportListCtrl, public COleDropTarget
{
    ///// types /////
public:
    enum DLDropFlags
    {
        DL_ACCEPT_FILES     =   0x01,   // Allow files to be dropped
        DL_ACCEPT_FOLDERS   =   0x02,   // Allow folders to be droppped
        DL_ALLOW_DUPLICATES =   0x04,   // Allow a pathname to be dropped even if its already in the list (ignored if you specify a callback function)
        DL_FILTER_EXTENSION =   0x10,   // Only accept files with the specified extension. Specify in strFileExt
        DL_USE_CALLBACK     =   0x20,   // Receive a callback for each item dropped, specified in pfnCallback (you have responsibility for inserting items into the list)

        DL_TYPE_FOLDER      =   0x40,   // Returned to the callback function - indicating the type of dropped item
        DL_TYPE_FILE        =   0x80
    };

    struct DROPLISTMODE
    { 
        UINT nMask;                             // Specifies what type of items to accept - a combination of the above flags
        CString strFileExt;                     // The file extension on which to filter. Use the format ".extension". Ignored unless DL_FILTER_EXTENSION is specified
        LPFN_DROP_FILES_CALLBACK pfnCallback;   // Address of your callback function. Ignored unless DL_USE_CALLBACK is specified
    };

protected:
    typedef CReportListCtrl inherited;


    ///// members /////
protected:
    BOOL                m_bMustUninitOLE;
    IDropTargetHelper*  m_piDropTargetHelper;   // enables Win2K and later to use new style drag&drop (shaded)
    CDroppedItems       m_DroppedItems;
    DROPLISTMODE        m_dropMode;
    BOOL                m_bUseDropTargetHelper;
    BOOL                m_bIsDroppableData;     // defines whether the dragged data can be dropped
    BOOL                m_bIsSpecialDropKeyPressed;
    int                 m_dropIndex;


    ///// methods /////
public:
    CFileDropListCtrl();
    virtual ~CFileDropListCtrl();

    BOOL SetDropMode(const CFileDropListCtrl::DROPLISTMODE& dropMode);
    DROPLISTMODE GetDropMode() const { return m_dropMode; };

    BOOL GetVisibleItemRect(int nItem, LPRECT lpRect, UINT nCode); // returns the item rect clipped to the client rect

protected:
    // prevent default compiler generation of these copy constructors (no bitwise copy)
    CFileDropListCtrl& operator=(const CFileDropListCtrl& x);
    CFileDropListCtrl(const CFileDropListCtrl& x);

    CString ExpandShortcut(CString& strFilename) const;
    BOOL ValidatePathname(const CString& strPathname, UINT& iPathType) const;
    virtual int InsertPathname(const CString& strFilename);

    void SelectItem(int index);
    void ActivateWindow();

    // COleDropTarget
    DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
    void OnDragLeave(CWnd* pWnd);

    // drag and drop helpers
    void DrawDragDropLine(int index);
    void ClearOldDragDropLine(int index);
    void DrawNewDragDropLine(CDC* pDC, int index);
    void DoDragDropScrolling(const CPoint& point, int index);
    int  HitTestVCenter(CPoint pt, UINT* pFlags = NULL);


    ///// message handlers / overrides /////
protected:
    //{{AFX_MSG(CFileDropListCtrl)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};
