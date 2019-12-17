/*----------------------------------------------------------------------------
| File: ReportListCtrl.h
| Project: oddgravitysdk
|
| Description:
|   Extended CListCtrl
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 275 $
| $Id: ReportListCtrl.h 275 2007-07-19 18:36:31Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#ifndef __REPORTLISTCTRL_H__
#define __REPORTLISTCTRL_H__

#pragma once

#include <vector>
#include <afxtempl.h>


/////////////////////////////////////////////////////////////////////////////
// user window messages

// sent to parent window when a column needs to be sorted
// wParam: int nColumn
// lParam: NULL
static const int UWM_SORT_COLUMN = ::RegisterWindowMessage(_T("UWM_SORT_COLUMN-{D3DC9476-F315-44a7-A934-BF7E6216AE0B}"));

// sent to parent window when an item is going to be edited
// (e.g. via combobox) and the items' options are needed
// wParam: CReportListCtrlItemOptions* pItemOptions
// lParam: NULL
static const int UWM_EDIT_NEEDOPTIONS = ::RegisterWindowMessage(_T("UWM_EDIT_NEEDOPTIONS-{027E6730-27CF-482a-9286-8E331D25E3EB}"));

// sent to parent window when an item text editing was committed
// wParam: The item index
// lParam: The column index
static const int UWM_EDIT_COMMITTED = ::RegisterWindowMessage(_T("UWM_EDIT_COMMITTED-{C8D09798-F734-476a-8736-8A12F3B8C451}"));

// sent to parent window when a (sub)item needs text for displaying a tooltip
// wParam: CReportListCtrlItemToolTip* pItemToolTip
// lParam: NULL
static const int UWM_LTTN_NEEDTEXT = ::RegisterWindowMessage(_T("UWM_LTTN_NEEDTEXT-{EE013E57-C97E-4b74-8DA7-79DFF10B5CD4}"));


///// defines /////
#define COLOR_INVALID   0xffffffff


/////////////////////////////////////////////////////////////////////////////
// types
typedef std::vector<int> CItemsArray;

typedef enum LISTTOOLTIPSTYLE
{
    LTTS_PER_ITEM_TIP,
    LTTS_PER_SUBITEM_TIP,

    LTTS_DEFAULT = LTTS_PER_ITEM_TIP
};


/////////////////////////////////////////////////////////////////////////////
// forward declarations

class CReportListCtrl;


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlItemData

class CReportListCtrlItemData
{
public:
  CReportListCtrlItemData();

  void InsertColumn(int nColumn);
  void DeleteColumn(int nColumn);

  DWORD_PTR dwData;                         // the actual 32-bit user data is stored here
  CArray<COLORREF, COLORREF> aTextColors;   // sub item text colors
  CArray<COLORREF, COLORREF> aBkColors;     // sub item backgroud colors
  BOOL bEnabled;                            // flag if item is enabled or disabled
};


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlItemOptions

class CReportListCtrlItemOptions
{
public:
  CReportListCtrlItemOptions();
  CReportListCtrlItemOptions(int nItem, int nSubItem);

  void Add(const CString& strOption);       // adds an option
  void Clear();                             // removes all options
  int GetCount();                           // returns the number of options
  CString GetAt(int nIndex);                // returns the option for the passed index

  int nItem;
  int nSubItem;

protected:
  CStringArray aOptions;
};


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlItemToolTip

class CReportListCtrlItemToolTip
{
public:
  CReportListCtrlItemToolTip();
  CReportListCtrlItemToolTip(int nItem, int nSubItem);

  int nItem;
  int nSubItem;

  CString strTooltipText;
};


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlComboBox

class CReportListCtrlComboBox : public CComboBox
{
public:
  ///// construction / destruction /////
  CReportListCtrlComboBox(CReportListCtrl* pParent = NULL);
  virtual ~CReportListCtrlComboBox();

  ///// methods /////
public:
  void SetParent(CReportListCtrl* pParent) { m_pParent = pParent; }

  ///// message handlers /////
public:
  afx_msg void OnSelectionChanged();

protected:
  DECLARE_MESSAGE_MAP()

  ///// members /////
protected:
  CReportListCtrl* m_pParent;
};


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrl

class CReportListCtrl : public CListCtrl
{
  DECLARE_DYNAMIC(CReportListCtrl)

public:
  ///// construction / destruction /////
  CReportListCtrl();
  virtual ~CReportListCtrl();

  virtual BOOL Create(CWnd* pParentWnd, UINT nID, LPCRECT lpRect = NULL, DWORD dwStyle = WS_BORDER | WS_TABSTOP);


  ///// methods /////
public:
  void EnableFullRowSelect(BOOL bEnable = TRUE);    // sets full row selection
  void EnableShowSelAlways(BOOL bEnable = TRUE);    // sets always show selection
  void EnableSortHeader(BOOL bEnable = TRUE);       // sets header buttons
  void EnableGridLines(BOOL bEnable = TRUE);        // sets display of gridlines
  void EnableCustomGridlines(BOOL bEnable = TRUE, BOOL bRedraw = TRUE); // sets painting of custom gridlines

  BOOL HasColumnHeader();
  int  GetColumnCount();
  CString GetHeaderText(int nColumn);
  BOOL SetHeaderText(int nColumn, LPCTSTR lpText);

  CImageList* SetSafeImageList(CImageList* pImageList, int nImageListType);

  void SetItemTextColor(int nItem = -1, int nSubItem = -1, COLORREF color = COLOR_INVALID, BOOL bRedraw = TRUE);
  COLORREF GetItemTextColor(int nItem, int nSubItem);
  void SetItemBkColor(int nItem = -1, int nSubItem = -1, COLORREF color = COLOR_INVALID, BOOL bRedraw = TRUE);
  COLORREF GetItemBkColor(int nItem, int nSubItem);

  void SetAlternatingRowColor(COLORREF color, BOOL bRedraw = TRUE);
  COLORREF GetAlternatingRowColor();
  void EnableAlternatingRowColor(BOOL bEnable, BOOL bRedraw = TRUE);

  void EnableItem(int nItem, BOOL bEnable = TRUE);  // enables or disabled the passed item
  BOOL IsItemEnabled(int nItem);                    // checks if passed item is enabled

  void SetEditable(BOOL bSet);                      // global switch if items can be edited or not
  BOOL IsEditable();                                // checks if global switch for item editing is set
  void SetEditableColumn(int nColumn, BOOL bSet);   // sets a column to be editable
  BOOL IsEditableColumn(int nColumn);               // checks if passed column is editable
  int  GetFirstEditableColumn();                    // returns the index of the first column that is editable

  BOOL StartEdit(int nItem, int nSubItem = -1, bool bShowDrop = false); // displays the combobox, previous combobox is committed
  BOOL EndEdit(BOOL bCommit = TRUE);                // commit/cancel edit, hide the combobox
  BOOL HotEdit();                                   // takes over the changes without ending to edit
  BOOL IsComboVisible();                            // checks if combobox is currently visible
  CComboBox* GetComboBoxControl();                  // returns the combobox control

  void SelectAllItems();                            // selects all items
  void DeselectAllItems();                          // deselects all items
  void InvertItemSelection();                       // inverts item selection
  bool IsItemSelected(int nItem);                   // returns whether the passed item is selected
  int  GetFirstSelectedIndex();                     // returns the index of the first selected item (returns -1 if no selection)
  int  GetLastSelectedIndex();                      // returns the index of the last selected item (returns -1 if no selection)
  int  GetTailIndex();                              // returns the index of the last item (returns -1 if empty)
  bool GetSelectedItems(CItemsArray* pArray);       // fills the passed array with all selected items

  int  GetFocusedAndSelectedItem();                 // returns the item that currently has the focus and is selected
  bool IsEmpty();                                   // returns whether list is empty

  // item moving support
  void SwapItems(int nItem1, int nItem2);
  bool MoveItem(int nItem, int nNewPosition);
  bool MoveSelectedItemsUp();
  bool MoveSelectedItemsDown();
  bool MoveSelectedItemsToTop();
  bool MoveSelectedItemsToBottom();

  BOOL IsValidItemIndex(int nIndex);
  BOOL IsValidColumnIndex(int nIndex);

  // empty list message
  void SetEmptyMessage(const CString& strMessage, BOOL bBold = FALSE, COLORREF color = RGB(0,0,0), BOOL bRedraw = TRUE);
  const CString& GetEmptyMessage()  { return m_strEmptyMessage; }
  void ClearEmptyMessage()          { SetEmptyMessage(_T("")); }
  void UpdateEmptyMessage();

  // sorting support
  void EnableSorting(BOOL bEnable = TRUE);
  BOOL IsSortingEnabled() { return m_bSortingEnabled; }

  ///// overrides: CWnd /////
  virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
  virtual BOOL EnableToolTips(BOOL bEnable = TRUE);
  virtual BOOL EnableToolTips(BOOL bEnable, LISTTOOLTIPSTYLE ttStyle);


protected:
  BOOL GetItemWrapped(UINT nMask, int nItem, int nSubItem, LVITEM* pItem, UINT nStateMask);
  BOOL SetItemWrapped(const LVITEM* pItem);
  BOOL DeleteItemWrapped(int nItem);                    // allocated item memory isn't deleted!
  int  InsertItemWrapped(int nItem, LPCTSTR lpszItem);  // no item memory is allocated!

  CReportListCtrlItemData* AllocItemMemory();
  void AllocItemMemory(int nItem);                  // !!!!! DEPRECATED !!!!!
  void FreeItemMemory(int nItem);

  void UpdateColumn(int nColumn, BOOL bInsert);

  void UpdateRowHeight();
  void DrawCustomGridlines(HDC hdc);
  void DrawEmptyMessage(HDC hdc);
  void DrawComboButtons(HDC hdc);
  void UpdateHeader();
  void UpdateCombo();
  void FillIconGap(HDC hdc, int nItem, COLORREF color);


  ///// overloaded methods /////
public:
  int InsertColumn(int nCol, const LVCOLUMN* pColumn);
  int InsertColumn(int nCol, LPCTSTR lpColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);
  BOOL DeleteColumn(int nCol);
  int InsertItem(const LVITEM* pItem);
  int InsertItem(int nItem, LPCTSTR lpszItem);
  int InsertItem(int nItem, LPCTSTR lpszItem, int nImage);
  int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam);
  BOOL SetItem(const LVITEM* pItem);
  BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam);
  BOOL GetItem(LVITEM* pItem);
  BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpText);
  DWORD_PTR GetItemData(int nItem);
  BOOL SetItemData(int nItem, DWORD_PTR dwData);
  BOOL DeleteItem(int nItem);
  BOOL DeleteAllItems();


  ///// members /////
protected:
  BOOL          m_bAllowEdit;               // is subitem edit allowed
  CReportListCtrlComboBox* m_pComboBox;     // combobox for editing values
  POINT         m_ptEditting;               // position of the subitem that is currently being edited
  CImageList    m_ImageList;                // needed for modifying row height
  CByteArray    m_EditableColumns;          // tracks which columns can be edited
  CPen          m_GridLinePen;              // pen for painting custom gridlines
  BOOL          m_bCustomGridlines;         // is custom gridline painting activated
  int           m_nButtonWidth;             // width of an item combo button
  int           m_nButtonHeight;            // height of an item combo button

  COLORREF      m_clrAlternatingRowColor;   // color of alternate row coloring
  BOOL          m_bAlternatingRowColor;     // sets whether alternate row coloring is on

  CString       m_strEmptyMessage;          // empty list message
  BOOL          m_bBoldEmptyMessage;        // sets whether empty message is drawn bold
  COLORREF      m_clrEmptyMessage;          // sets the color of the empty message
  CFont*        m_pBoldFont;                // bold version of parents' font

  BOOL          m_bSortingEnabled;          // defines whether sorting support is enabled

  CStringA      m_astrToolTipText;          // tooltip text buffer (due to 80 chararcter limit)
  CStringW      m_wstrToolTipText;          // tooltip text buffer (due to 80 chararcter limit)
  LISTTOOLTIPSTYLE m_ToolTipStyle;          // ListCtrl tooltip style

  ///// message handlers / overrides /////
public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg BOOL OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
  afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM);
  afx_msg void OnDestroy();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnHdnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHdnTrack(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHdnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHdnEndTrack(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnHdnItemClick(NMHDR* pNMHDR, LRESULT* pResult);

  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  virtual afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

protected:
  DECLARE_MESSAGE_MAP()
};


#endif // __REPORTLISTCTRL_H__
