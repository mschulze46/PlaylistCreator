/*----------------------------------------------------------------------------
| File: ReportListCtrl.cpp
| Project: oddgravitysdk
|
| Description:
|   Extended CListCtrl
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 440 $
| $Id: ReportListCtrl.cpp 440 2009-10-17 10:47:55Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "ReportListCtrl.h"

/*
// %TODO%: support windows xp styles
#ifndef __VUXTHEME_H__
#  include "VUXTheme.h"
#endif
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlItemData class is used for store extra information

CReportListCtrlItemData::CReportListCtrlItemData()
{
  dwData = 0;
  bEnabled = TRUE;
}


void CReportListCtrlItemData::InsertColumn(int nColumn)
{
  aTextColors.InsertAt(nColumn, ::GetSysColor(COLOR_WINDOWTEXT));
  aBkColors.InsertAt(nColumn, COLOR_INVALID);
}


void CReportListCtrlItemData::DeleteColumn(int nColumn)
{
  aTextColors.RemoveAt(nColumn);
  aBkColors.RemoveAt(nColumn);
}


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlItemOptions class is used to retrieve the possible
// options of an item from the parent window

CReportListCtrlItemOptions::CReportListCtrlItemOptions()
{
  nItem = -1;
  nSubItem = -1;
}


CReportListCtrlItemOptions::CReportListCtrlItemOptions(int nItem, int nSubItem)
{
  this->nItem = nItem;
  this->nSubItem = nSubItem;
}


void CReportListCtrlItemOptions::Add(const CString& strOption)
{
  aOptions.Add(strOption);
}


void CReportListCtrlItemOptions::Clear()
{
  aOptions.RemoveAll();
}


int CReportListCtrlItemOptions::GetCount()
{
  return (int)aOptions.GetSize();
}


CString CReportListCtrlItemOptions::GetAt(int nIndex)
{
  if(nIndex < 0 || nIndex >= (int)aOptions.GetCount())
    return _T("");

  return aOptions.GetAt(nIndex);
}


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlItemToolTip class is used to retrieve the
// (sub)item tooltip text from the parent window

CReportListCtrlItemToolTip::CReportListCtrlItemToolTip()
{
  nItem = -1;
  nSubItem = -1;

  strTooltipText = _T("");
}


CReportListCtrlItemToolTip::CReportListCtrlItemToolTip(int nItem, int nSubItem)
{
  this->nItem = nItem;
  this->nSubItem = nSubItem;

  this->strTooltipText = _T("");
}


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrlComboBox

CReportListCtrlComboBox::CReportListCtrlComboBox(CReportListCtrl* pParent /*=NULL*/)
{
  m_pParent = pParent;
}


CReportListCtrlComboBox::~CReportListCtrlComboBox()
{
}


BEGIN_MESSAGE_MAP(CReportListCtrlComboBox, CComboBox)
  ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelectionChanged)
END_MESSAGE_MAP()


void CReportListCtrlComboBox::OnSelectionChanged()
{
  if(!m_pParent)
  {
    ASSERT(0);
    return;
  }

  // notify parent that selection changed
  m_pParent->HotEdit();
}


/////////////////////////////////////////////////////////////////////////////
// CReportListCtrl

IMPLEMENT_DYNAMIC(CReportListCtrl, CListCtrl)

CReportListCtrl::CReportListCtrl()
{
    m_bAllowEdit            = FALSE;
    m_pComboBox             = new CReportListCtrlComboBox(this);
    m_ptEditting.x          = -1;
    m_ptEditting.y          = -1;
    m_GridLinePen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNFACE));
    m_bCustomGridlines      = FALSE;
    m_nButtonWidth          = ::GetSystemMetrics(SM_CXVSCROLL);
    m_nButtonHeight         = ::GetSystemMetrics(SM_CYVSCROLL);

    m_clrAlternatingRowColor = COLOR_INVALID;
    m_bAlternatingRowColor   = FALSE;

    m_strEmptyMessage       = _T("");
    m_bBoldEmptyMessage     = FALSE;
    m_clrEmptyMessage       = RGB(0,0,0);
    m_pBoldFont             = NULL;

    m_bSortingEnabled       = FALSE;

    m_astrToolTipText       = _T("");
    m_wstrToolTipText       = _T("");
    m_ToolTipStyle          = LTTS_DEFAULT;
}


CReportListCtrl::~CReportListCtrl()
{
    if(m_pComboBox)
    {
        delete m_pComboBox;
        m_pComboBox = NULL;
    }

    if(m_GridLinePen.m_hObject)
    {
        m_GridLinePen.DeleteObject();
    }

    if(m_pBoldFont)
    {
        m_pBoldFont->DeleteObject();
        delete m_pBoldFont;
        m_pBoldFont = NULL;
    }
}


BOOL CReportListCtrl::Create(CWnd* pParentWnd, UINT nID, LPCRECT lpRect, DWORD dwStyle) 
{
  ASSERT(pParentWnd != NULL);

  dwStyle &= ~(LVS_EDITLABELS | LVS_ICON | LVS_SMALLICON | LVS_LIST | LVS_NOSCROLL);  // remove these styles by default
  dwStyle |= (WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LVS_REPORT);          // add these styles by default

  CRect rect;
  if(lpRect == NULL)
    pParentWnd->GetClientRect(&rect);
  else
    rect = *lpRect;

  return CListCtrl::Create(dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CReportListCtrl, CListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnSelectionChanged)
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_RBUTTONDOWN()
  ON_WM_RBUTTONDBLCLK()
  ON_WM_NCLBUTTONDOWN()
  ON_WM_NCRBUTTONDOWN()
  ON_WM_MOUSEWHEEL()
  ON_MESSAGE(WM_SETFONT, OnSetFont)
  ON_WM_DESTROY()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_SIZE()
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// CReportListCtrl message handlers

void CReportListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMLVCUSTOMDRAW lplvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

  switch(lplvcd->nmcd.dwDrawStage)
  {
    case CDDS_PREPAINT:
      {
        *pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
      }
      break;

    case CDDS_ITEMPREPAINT:
      {
        *pResult = CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;
      }
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      {
        CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData((int)lplvcd->nmcd.dwItemSpec)); ASSERT(pData);
        if(pData)
        {
          if(lplvcd->iSubItem >= 0 && lplvcd->iSubItem < pData->aTextColors.GetSize())
          {
            // set text color
            lplvcd->clrText = pData->aTextColors[lplvcd->iSubItem];

            // set background color
            if(m_bAlternatingRowColor && ((int)lplvcd->nmcd.dwItemSpec % 2) == 0) // alternate row coloring on and it's the colored row
            {
              lplvcd->clrTextBk = (pData->aBkColors[lplvcd->iSubItem] != COLOR_INVALID)
                                    ? pData->aBkColors[lplvcd->iSubItem]
                                    : m_clrAlternatingRowColor;
            }
            else // alternate row coloring off or not the colored row
            {
              lplvcd->clrTextBk = (pData->aBkColors[lplvcd->iSubItem] != COLOR_INVALID)
                                    ? pData->aBkColors[lplvcd->iSubItem]
                                    : ::GetSysColor(COLOR_WINDOW);
            }
          }
        }
        *pResult = CDRF_DODEFAULT;
      }
      break;

    case CDDS_ITEMPOSTPAINT:
      {
        FillIconGap(lplvcd->nmcd.hdc, (int)lplvcd->nmcd.dwItemSpec, lplvcd->clrTextBk);

        *pResult = CDRF_DODEFAULT;
      }
      break;

    case CDDS_POSTPAINT:
      {
        // custom drawing
        DrawCustomGridlines(lplvcd->nmcd.hdc);
        DrawEmptyMessage(lplvcd->nmcd.hdc);
        DrawComboButtons(lplvcd->nmcd.hdc);

        // update attached controls
        UpdateHeader();
        UpdateCombo();

        *pResult = CDRF_DODEFAULT;
      }
      break;

    default:
      ASSERT(0); // all custom draw steps should be handled!
      break;
  }
}


int CReportListCtrl::InsertColumn(int nCol, const LVCOLUMN* pColumn)
{
  EndEdit(TRUE);
  const int index = CListCtrl::InsertColumn(nCol, pColumn);
  if(index >= 0)
    UpdateColumn(index, TRUE);
  return index;
}


int CReportListCtrl::InsertColumn(int nCol, LPCTSTR lpColumnHeading, int nFormat, int nWidth, int nSubItem)
{
  EndEdit(TRUE);
  const int index = CListCtrl::InsertColumn(nCol, lpColumnHeading, nFormat, nWidth, nSubItem);
  if (index >= 0)
    UpdateColumn(index, TRUE);
  return index;
}


BOOL CReportListCtrl::DeleteColumn(int nCol)
{
  if(!IsValidColumnIndex(nCol))
    return FALSE;

  EndEdit(TRUE);
  BOOL result = CListCtrl::DeleteColumn(nCol);
  if(result)
    UpdateColumn(nCol, FALSE);
  return result;
}


int CReportListCtrl::InsertItem(const LVITEM* pItem)
{
  EndEdit(TRUE);

  CReportListCtrlItemData* pData = AllocItemMemory();                               ASSERT(pData);
  if(pItem && pData) pData->dwData = (DWORD_PTR)pItem->lParam;

  LVITEM lvi;
  memcpy(&lvi, pItem, sizeof(LVITEM));
  lvi.lParam = (LPARAM)pData;

  const int index = CListCtrl::InsertItem(&lvi);
  if(index < 0)
    delete pData;

  return index;
}


int CReportListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
  EndEdit(TRUE);

  CReportListCtrlItemData* pData = AllocItemMemory();                               ASSERT(pData);

  LVITEM lvi;
  lvi.mask      = LVIF_TEXT | LVIF_PARAM;
  lvi.iItem     = nItem;
  lvi.iSubItem  = 0;
  lvi.pszText   = (LPTSTR)lpszItem;
  lvi.lParam    = (LPARAM)pData;

  const int index = CListCtrl::InsertItem(&lvi);
  if(index < 0)
    delete pData;

  return index;
}


int CReportListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
  EndEdit(TRUE);

  CReportListCtrlItemData* pData = AllocItemMemory();                               ASSERT(pData);

  LVITEM lvi;
  lvi.mask      = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
  lvi.iItem     = nItem;
  lvi.iSubItem  = 0;
  lvi.pszText   = (LPTSTR)lpszItem;
  lvi.iImage    = nImage;
  lvi.lParam    = (LPARAM)pData;

  const int index = CListCtrl::InsertItem(&lvi);
  if(index < 0)
    delete pData;

  return index;
}


int CReportListCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam)
{
  EndEdit(TRUE);

  CReportListCtrlItemData* pData = AllocItemMemory();                               ASSERT(pData);
  if(pData) pData->dwData = (DWORD_PTR)lParam;

  LVITEM lvi;
  lvi.mask      = nMask | LVIF_PARAM;
  lvi.iItem     = nItem;
  lvi.iSubItem  = 0;
  lvi.pszText   = (LPTSTR)lpszItem;
  lvi.state     = nState;
  lvi.stateMask = nStateMask;
  lvi.iImage    = nImage;
  lvi.lParam    = (LPARAM)pData;

  const int index = CListCtrl::InsertItem(&lvi);
  if(index < 0)
    delete pData;

  return index;
}


BOOL CReportListCtrl::SetItem(const LVITEM* pItem)
{
  if((pItem == NULL) || !IsValidItemIndex(pItem->iItem) || !IsValidColumnIndex(pItem->iSubItem))
    return FALSE;

  LVITEM li;
  memcpy(&li, pItem, sizeof(LVITEM));
  if((li.mask & LVIF_PARAM) != 0)
  {
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(li.iItem)); ASSERT(pData);
    if(pData)
    {
      pData->dwData = (DWORD)li.lParam;
      li.lParam = (LPARAM)pData;
    }   
  }
  return CListCtrl::SetItem(&li);
}


BOOL CReportListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
{
  if(!IsValidItemIndex(nItem) || !IsValidColumnIndex(nSubItem))
    return FALSE;

  if((nMask & LVIF_PARAM) != 0)
  {
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
    if(pData)
    {
      pData->dwData = (DWORD)lParam;
      lParam = (LPARAM)pData;
    }   
  }
  return CListCtrl::SetItem(nItem, nSubItem, nMask, lpszItem, nImage, nState, nStateMask, lParam);
}


BOOL CReportListCtrl::GetItem(LVITEM* pItem)
{
  if((pItem == NULL) || !IsValidItemIndex(pItem->iItem))
    return FALSE;

  BOOL result = CListCtrl::GetItem(pItem);
  if(result && (pItem->mask & LVIF_PARAM) != 0)
  {
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(pItem->lParam); ASSERT(pData);
    if(pData)
      pItem->lParam = pData->dwData;
  }
  return result;
}


BOOL CReportListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpText)
{
  if(!IsValidItemIndex(nItem) || !IsValidColumnIndex(nSubItem))
    return FALSE;

  EndEdit(TRUE);
  return CListCtrl::SetItemText(nItem, nSubItem, lpText);
}


DWORD_PTR CReportListCtrl::GetItemData(int nItem)
{
  if(!IsValidItemIndex(nItem))
    return 0;

  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  return (pData == NULL) ? 0 : pData->dwData;
}


BOOL CReportListCtrl::SetItemData(int nItem, DWORD_PTR dwData)
{
  if(!IsValidItemIndex(nItem))
    return FALSE;

  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  if(!pData)
    return FALSE;
  pData->dwData = dwData;
  return CListCtrl::SetItemData(nItem, (DWORD_PTR)pData);
}


BOOL CReportListCtrl::DeleteItem(int nItem)
{
  if(!IsValidItemIndex(nItem))
    return FALSE;

  EndEdit(m_ptEditting.x != nItem);

  FreeItemMemory(nItem);
  return CListCtrl::DeleteItem(nItem);
}


BOOL CReportListCtrl::DeleteAllItems()
{
  BOOL result = FALSE;

  EndEdit(FALSE);

  const int nItems = GetItemCount();

  SetRedraw(FALSE); // no redraw while deleting
  {
    for(int item = 0; item < nItems; item++)
      FreeItemMemory(item);

    result = CListCtrl::DeleteAllItems();
  }
  SetRedraw(TRUE);

  return result;
}


BOOL CReportListCtrl::HasColumnHeader()
{
  return (GetStyle() & LVS_NOCOLUMNHEADER) == 0;
}


int CReportListCtrl::GetColumnCount()
{
  CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
  return (pHeader) ? pHeader->GetItemCount() : 1;
}


CString CReportListCtrl::GetHeaderText(int nColumn)
{
  if(!HasColumnHeader() || !IsValidColumnIndex(nColumn))
    return _T("");

  HDITEM hd;
  TCHAR szBuffer[256] = _T("");
  hd.mask       = HDI_TEXT;
  hd.pszText    = szBuffer;
  hd.cchTextMax = 255;
  return CListCtrl::GetHeaderCtrl()->GetItem(nColumn, &hd) ? hd.pszText : _T("");
}


BOOL CReportListCtrl::SetHeaderText(int nColumn, LPCTSTR lpText)
{
  if(!HasColumnHeader() || !IsValidColumnIndex(nColumn))
    return FALSE;

  LPTSTR psz = NULL;
  if(lpText == NULL)
  {
    psz = new TCHAR[1];
    *psz = _T('\0');
  }
  else
  {
    psz = _tcsdup(lpText);
  }
  HDITEM hd;
  hd.mask       = HDI_TEXT;
  hd.pszText    = psz;
  hd.cchTextMax = (int)_tcslen(psz);
  BOOL result = CListCtrl::GetHeaderCtrl()->SetItem(nColumn, &hd);
  delete[] psz;
  return result;
}


CImageList* CReportListCtrl::SetSafeImageList(CImageList* pImageList, int nImageListType)
{
    // image list type LVSIL_SMALL needs special handling:
    // if the header control has an image list assigned it must
    // be buffered while setting the ListCtrl's small image list,
    // otherwise the ListCtrl's small image list gets also attached
    // to the header control! makes me sick!

    CImageList* retVal = NULL;

    CHeaderCtrl* pHeader = NULL;
    CImageList* pHeaderImageList = NULL;

    if(nImageListType == LVSIL_SMALL)
    {
        pHeader = CListCtrl::GetHeaderCtrl();
        if(pHeader)
            pHeaderImageList = pHeader->GetImageList();
    }

    retVal = CListCtrl::SetImageList(pImageList, nImageListType);

    if(nImageListType == LVSIL_SMALL)
    {
        if(pHeader && pHeaderImageList)
            pHeader->SetImageList(pHeaderImageList);
    }

    return retVal;
}


void CReportListCtrl::SetItemTextColor(int nItem, int nSubItem, COLORREF color, BOOL bRedraw)
{
  if(color == COLOR_INVALID)
    color = ::GetSysColor(COLOR_WINDOWTEXT);
  const int nRows = GetItemCount();
  const int nCols = GetColumnCount();
  BOOL bRowValid = nItem >= 0 && nItem < nRows;
  BOOL bColValid = nSubItem >= 0 && nSubItem < nCols;

  if(bRowValid && bColValid)
  {
    // apply to individual grid
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
    if(pData)
      pData->aTextColors[nSubItem] = color;
  }
  else if(bRowValid && !bColValid)
  {
    // apply to whole row for the existing item
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
    if(pData)
    {
      for(int i = 0; i < nCols; i++)
        pData->aTextColors[i] = color;
    }
  }
  else if(!bRowValid && bColValid)
  {
    // apply to whole column for all existing items
    for(int i = 0; i < nRows; i++)
    {
      CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(i)); ASSERT(pData);
      if(pData)
        pData->aTextColors[nSubItem] = color;
    }
  }
  else
  {
    // apply to whole table for all existing items
    for(int i = 0; i < nRows; i++)
    {
      CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(i)); ASSERT(pData);
      if(pData)
      {
        for(int j = 0; j < nCols; j++)
          pData->aTextColors[j] = color;
      }
    }
  }

  if(bRedraw)
    RedrawWindow();
}


COLORREF CReportListCtrl::GetItemTextColor(int nItem, int nSubItem)
{
  if(!IsValidItemIndex(nItem) || !IsValidColumnIndex(nSubItem))
    return COLOR_INVALID;
  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  return (pData != NULL) ? pData->aTextColors[nSubItem] : COLOR_INVALID;
}


void CReportListCtrl::SetItemBkColor(int nItem, int nSubItem, COLORREF color, BOOL bRedraw)
{
  const int nRows = GetItemCount();
  const int nCols = GetColumnCount();
  BOOL bRowValid = nItem >= 0 && nItem < nRows;
  BOOL bColValid = nSubItem >= 0 && nSubItem < nCols;

  if(bRowValid && bColValid)
  {
    // apply to individual grid
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
    if(pData)
      pData->aBkColors[nSubItem] = color;
  }
  else if(bRowValid && !bColValid)
  {
    // apply to whole row for the existing item
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
    if(pData)
    {
      for(int i = 0; i < nCols; i++)
        pData->aBkColors[i] = color;
    }
  }
  else if(!bRowValid && bColValid)
  {
    // apply to whole column for all existing items
    for(int i = 0; i < nRows; i++)
    {
      CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(i)); ASSERT(pData);
      if(pData)
        pData->aBkColors[nSubItem] = color;
    }
  }
  else
  {
    // apply to whole table for all existing items
    for(int i = 0; i < nRows; i++)
    {
      CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(i)); ASSERT(pData);
      if(pData)
      {
        for(int j = 0; j < nCols; j++)
          pData->aBkColors[j] = color;
      }
    }
  }

  if(bRedraw)
    RedrawWindow();
}


COLORREF CReportListCtrl::GetItemBkColor(int nItem, int nSubItem)
{
  if(!IsValidItemIndex(nItem) || !IsValidColumnIndex(nSubItem))
    return COLOR_INVALID;
  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  return (pData != NULL) ? pData->aBkColors[nSubItem] : COLOR_INVALID;
}


void CReportListCtrl::SetAlternatingRowColor(COLORREF color, BOOL bRedraw /*TRUE*/)
{
  m_clrAlternatingRowColor = color;
  m_bAlternatingRowColor = TRUE;

  if(bRedraw)
    RedrawWindow();
}


COLORREF CReportListCtrl::GetAlternatingRowColor()
{
  return m_clrAlternatingRowColor;
}


void CReportListCtrl::EnableAlternatingRowColor(BOOL bEnable, BOOL bRedraw /*TRUE*/)
{
  m_bAlternatingRowColor = bEnable;

  if(bRedraw)
    RedrawWindow();
}


void CReportListCtrl::EnableItem(int nItem, BOOL bEnable /*TRUE*/)
{
  if(!IsValidItemIndex(nItem))
    return;

  // set enable flag
  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  if(pData)
    pData->bEnabled = bEnable;

  // set text and bkgnd color
  COLORREF colorText = COLOR_INVALID;
  COLORREF colorBkgnd = COLOR_INVALID;

  if(bEnable)
  {
    colorText = ::GetSysColor(COLOR_WINDOWTEXT);
    colorBkgnd = ::GetSysColor(COLOR_WINDOW);
  }
  else
  {
    colorText = ::GetSysColor(COLOR_GRAYTEXT);
    colorBkgnd = ::GetSysColor(COLOR_WINDOW);
  }

  SetItemTextColor(nItem, -1, colorText, FALSE);
  SetItemBkColor(nItem, -1, colorBkgnd, FALSE);

  RedrawWindow();
}


BOOL CReportListCtrl::IsItemEnabled(int nItem)
{
  if(!IsValidItemIndex(nItem))
    return FALSE;

  BOOL result = FALSE;

  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  if(pData)
    result = pData->bEnabled;

  return result;
}


void CReportListCtrl::EnableFullRowSelect(BOOL bEnable /*TRUE*/)
{
  DWORD dwExStyle = GetExtendedStyle();
  if(bEnable)
    dwExStyle |= LVS_EX_FULLROWSELECT;
  else
    dwExStyle &= ~LVS_EX_FULLROWSELECT;

  SetExtendedStyle(dwExStyle);
}


void CReportListCtrl::EnableShowSelAlways(BOOL bEnable /*TRUE*/)
{
  if(bEnable)
    ModifyStyle(0, LVS_SHOWSELALWAYS);
  else
    ModifyStyle(LVS_SHOWSELALWAYS, 0);
}


void CReportListCtrl::EnableSortHeader(BOOL bEnable /*TRUE*/)
{
  CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
  if(pHeader)
  {
    if(bEnable)
      pHeader->ModifyStyle(0, HDS_BUTTONS);
    else
      pHeader->ModifyStyle(HDS_BUTTONS, 0);
  }
}


void CReportListCtrl::EnableGridLines(BOOL bEnable /*TRUE*/)
{
  DWORD dwExStyle = GetExtendedStyle();
  if(bEnable)
  {
    dwExStyle |= LVS_EX_GRIDLINES;
    m_bCustomGridlines = FALSE; // turn off custom gridlines
  }
  else
  {
    dwExStyle &= ~LVS_EX_GRIDLINES;
  }

  SetExtendedStyle(dwExStyle);
}


void CReportListCtrl::EnableCustomGridlines(BOOL bEnable /*TRUE*/, BOOL bRedraw /*TRUE*/)
{
  if(bEnable)
  {
    EnableGridLines(FALSE); // turn off standard gridlines
  }

  m_bCustomGridlines = bEnable;

  if(bRedraw)
    RedrawWindow();
}


BOOL CReportListCtrl::IsValidItemIndex(int nIndex)
{
  return (nIndex >= 0) && (nIndex < CListCtrl::GetItemCount());
}


BOOL CReportListCtrl::IsValidColumnIndex(int nIndex)
{
  return (nIndex >= 0) && (nIndex < GetColumnCount());
}


void CReportListCtrl::SetEmptyMessage(const CString& strMessage,
                                      BOOL bBold /*FALSE*/,
                                      COLORREF color /*RGB(0,0,0)*/,
                                      BOOL bRedraw /*TRUE*/)
{
    // get parameters
    m_strEmptyMessage   = strMessage;
    m_bBoldEmptyMessage = bBold;
    m_clrEmptyMessage   = color;

    // create bold font if necessary
    if(!m_pBoldFont)
    {
        CFont* pListFont = CListCtrl::GetFont();
        if(pListFont)
        {
            m_pBoldFont = new CFont();

            LOGFONT lf;
            pListFont->GetLogFont(&lf);
            lf.lfWeight = FW_BOLD;

            m_pBoldFont->CreateFontIndirect(&lf);
        }
    }

    if(bRedraw)
        RedrawWindow();
}


void CReportListCtrl::UpdateEmptyMessage()
{
    // only update if the list is empty and we have an empty message
    if(IsEmpty() && !m_strEmptyMessage.IsEmpty())
    {
        Invalidate();
    }
}


void CReportListCtrl::EnableSorting(BOOL bEnable /*TRUE*/)
{
    m_bSortingEnabled = bEnable;
    EnableSortHeader(bEnable);
}


INT_PTR CReportListCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
  // perform hit test for the passed point
  LVHITTESTINFO lvhti;
  lvhti.pt = point;
  int nItem    = ListView_SubItemHitTest(m_hWnd, &lvhti);
  int nSubItem = lvhti.iSubItem;

  // was a ListCtrl item hit?
  if(nItem >= 0)
  {
    // get client rect
    RECT rcClient;
    GetClientRect(&rcClient);

    // fill in TOOLINFO struct
    pTI->hwnd     = m_hWnd;
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->rect     = rcClient;
    pTI->uId      = (UINT_PTR)(nItem * 100 + ((m_ToolTipStyle == LTTS_PER_SUBITEM_TIP) ? nSubItem : 0)); // bogus child window handle

    return pTI->uId; // by returning a unique value per list item, we ensure that when
                     // the mouse moves over another list item, the tooltip changes
  }
  else // ListCtrl wasn't hit
  {
    return -1; // somebody else's problem!
  }
}


BOOL CReportListCtrl::EnableToolTips(BOOL bEnable /*TRUE*/)
{
    m_ToolTipStyle = LTTS_DEFAULT;
    return CListCtrl::EnableToolTips(bEnable);
}


BOOL CReportListCtrl::EnableToolTips(BOOL bEnable, LISTTOOLTIPSTYLE ttStyle)
{
    m_ToolTipStyle = ttStyle;
    return CListCtrl::EnableToolTips(bEnable);
}


BOOL CReportListCtrl::GetItemWrapped(UINT nMask, int nItem, int nSubItem, LVITEM* pItem, UINT nStateMask)
{
  pItem->mask      = nMask;
  pItem->stateMask = nStateMask;
  pItem->iItem     = nItem;
  pItem->iSubItem  = nSubItem;

  return CListCtrl::GetItem(pItem);
}


BOOL CReportListCtrl::SetItemWrapped(const LVITEM* pItem)
{
  if((pItem == NULL) || !IsValidItemIndex(pItem->iItem) || !IsValidColumnIndex(pItem->iSubItem))
    return FALSE;

  return CListCtrl::SetItem(pItem);
}


BOOL CReportListCtrl::DeleteItemWrapped(int nItem)
{
  if(!IsValidItemIndex(nItem))
    return FALSE;

  return CListCtrl::DeleteItem(nItem);
}


int CReportListCtrl::InsertItemWrapped(int nItem, LPCTSTR lpszItem)
{
  LVITEM lvi;
  lvi.mask      = LVIF_TEXT | LVIF_PARAM;
  lvi.iItem     = nItem;
  lvi.iSubItem  = 0;
  lvi.pszText   = (LPTSTR)lpszItem;
  lvi.lParam    = 0;

  return CListCtrl::InsertItem(&lvi);
}


CReportListCtrlItemData* CReportListCtrl::AllocItemMemory()
{
  const int nCols = GetColumnCount();                                               ASSERT(nCols > 0);
  CReportListCtrlItemData* pData = new CReportListCtrlItemData();
  pData->dwData = 0;
  pData->aTextColors.SetSize(nCols);
  pData->aBkColors.SetSize(nCols);
  for(int i = 0; i < nCols; i++)
  {
    pData->aTextColors[i] = ::GetSysColor(COLOR_WINDOWTEXT);
    pData->aBkColors[i]   = COLOR_INVALID;
  }
  return pData;
}


void CReportListCtrl::AllocItemMemory(int nItem)
{
  TRACE(">>> CReportListCtrl::AllocItemMemory(int nItem) is deprecated! Don't use it anymore!\n");
  ASSERT(0);

  if(!IsValidItemIndex(nItem))
  {
    ASSERT(0);
    return;
  }

  const int nCols = GetColumnCount();                                               ASSERT(nCols > 0);
  CReportListCtrlItemData* pData = new CReportListCtrlItemData();
  pData->dwData = CListCtrl::GetItemData(nItem);
  pData->aTextColors.SetSize(nCols);
  pData->aBkColors.SetSize(nCols);
  for(int i = 0; i < nCols; i++)
  {
    pData->aTextColors[i] = ::GetSysColor(COLOR_WINDOWTEXT);
    pData->aBkColors[i]   = COLOR_INVALID;
  }
  CListCtrl::SetItemData(nItem, (DWORD_PTR)pData);
}


void CReportListCtrl::FreeItemMemory(int nItem)
{
  if(!IsValidItemIndex(nItem))
    return;

  CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
  if(pData)
  {
    CListCtrl::SetItemData(nItem, 0);
    delete pData;
  }
}


void CReportListCtrl::UpdateColumn(int nColumn, BOOL bInsert)
{
  const int nItems = GetItemCount();
  for(int item = 0; item < nItems; item++)
  {
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(item)); ASSERT(pData);
    if(pData)
    {
      if(bInsert)
        pData->InsertColumn(nColumn);
      else
        pData->DeleteColumn(nColumn);
    }
  }
}


BOOL CReportListCtrl::IsComboVisible()
{
  BOOL result = FALSE;

  if(m_pComboBox && IsWindow(m_pComboBox->m_hWnd))
    result = m_pComboBox->IsWindowVisible();

  return result;
}


void CReportListCtrl::SetEditable(BOOL bSet)
{
  if(!bSet)
    EndEdit(TRUE);
  m_bAllowEdit = bSet;
}


BOOL CReportListCtrl::IsEditable()
{
  return m_bAllowEdit;
}


void CReportListCtrl::SetEditableColumn(int nColumn, BOOL bSet)
{
  int oldsize = (int)m_EditableColumns.GetSize();
  if(oldsize <= nColumn)
  {
    m_EditableColumns.SetSize(nColumn + 1);
    for(int x = oldsize; x < nColumn; x++)
      m_EditableColumns[x] = FALSE;
  }
  m_EditableColumns[nColumn] = bSet;
}


BOOL CReportListCtrl::IsEditableColumn(int nColumn)
{
  if(m_EditableColumns.GetSize() <= nColumn)
    return FALSE;

  return (m_EditableColumns[nColumn] == 0) ? FALSE : TRUE;
}


int CReportListCtrl::GetFirstEditableColumn()
{
  int nSize = (int)m_EditableColumns.GetSize();
  for(int index = 0; index < nSize; index++)
  {
    if(m_EditableColumns[index] == TRUE)
      return index;
  }
  return -1;
}


BOOL CReportListCtrl::PreTranslateMessage(MSG* pMsg) 
{
  if(pMsg->message == WM_KEYDOWN)
  {
    if(!IsComboVisible()) // combobox isn't visible
    {
      switch(pMsg->wParam)
      {
        case VK_SPACE:
          // disable user from check/uncheck the checkboxes using space key
          // things get nasty if the user is HOLDING the space bar down
          return TRUE;

        case VK_RETURN:
          {
            // if we have a focused and seleted item then start to edit it
            int nItem = GetFocusedAndSelectedItem();
            if(nItem >= 0)
              StartEdit(nItem);
          }
          return TRUE;

        default:
          break;
      }
    }
    else // combobox is visible
    {
      POINT pt = m_ptEditting;
      switch(pMsg->wParam)
      {
        case VK_ESCAPE: // cancel edit
          EndEdit(FALSE);
          return TRUE;

        case VK_RETURN:
          if((GetFocus() != m_pComboBox) && !m_pComboBox->GetDroppedState())
          {
            m_pComboBox->SetFocus();
            m_pComboBox->ShowDropDown(TRUE);
          }
          else
          {
            EndEdit(TRUE);
          }
          return TRUE;

        case VK_TAB:
          {
            if(GetFocus() != m_pComboBox)
            {
              m_pComboBox->SetFocus();
              return TRUE;
            }
          }
          break;

        default:
          break;        
      }
    }
  }

  return CListCtrl::PreTranslateMessage(pMsg);
}


CComboBox* CReportListCtrl::GetComboBoxControl()
{
  return m_pComboBox;
}


void CReportListCtrl::SelectAllItems()
{
  const int nItems = CListCtrl::GetItemCount();

  for(int item = 0; item < nItems; item++)
    CListCtrl::SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
}


void CReportListCtrl::DeselectAllItems()
{
  POSITION pos = CListCtrl::GetFirstSelectedItemPosition();
  while(pos)
  {
    int item = CListCtrl::GetNextSelectedItem(pos);
    CListCtrl::SetItemState(item, 0, LVIS_SELECTED | LVIS_FOCUSED);
  }
}


void CReportListCtrl::InvertItemSelection()
{
  const int nItems = CListCtrl::GetItemCount();

  for(int item = 0; item < nItems; item++)
  {
    UINT nCurState = CListCtrl::GetItemState(item, LVIS_SELECTED);
    CListCtrl::SetItemState(item, (nCurState == LVIS_SELECTED) ? 0 : LVIS_SELECTED, LVIS_SELECTED);
  }
}


bool CReportListCtrl::IsItemSelected(int nItem)
{
    if(!IsValidItemIndex(nItem))
        return false;

    if((CListCtrl::GetItemState(nItem, LVIS_SELECTED) & LVIS_SELECTED) != 0)
        return true;

    return false;
}


int CReportListCtrl::GetFirstSelectedIndex()
{
    POSITION pos = CListCtrl::GetFirstSelectedItemPosition();
    if(pos == NULL) // no selection
        return -1;

    return CListCtrl::GetNextSelectedItem(pos);
}


int CReportListCtrl::GetLastSelectedIndex()
{
    CItemsArray arraySelectedItems;
    GetSelectedItems(&arraySelectedItems);

    if(arraySelectedItems.empty()) // no selection
        return -1;

    return arraySelectedItems.back();
}


int CReportListCtrl::GetTailIndex()
{
    if(IsEmpty())
        return -1;

    return CListCtrl::GetItemCount() - 1;
}


bool CReportListCtrl::GetSelectedItems(CItemsArray* pArray)
{
    if(!pArray)
    {
        ASSERT(0); // error: no array passed
        return false;
    }

    // clear the passed array
    pArray->clear();

    // fill array with selected items
    POSITION pos = CListCtrl::GetFirstSelectedItemPosition();
    while(pos)
    {
        int nItem = CListCtrl::GetNextSelectedItem(pos);
        pArray->push_back(nItem);
    }

    return (pArray->empty()) ? false : true;
}


int CReportListCtrl::GetFocusedAndSelectedItem()
{
  POSITION pos = GetFirstSelectedItemPosition();
  while(pos)
  {
    int nItem = GetNextSelectedItem(pos);
    UINT nState = GetItemState(nItem, LVIS_FOCUSED);
    if(nState & LVIS_FOCUSED)
      return nItem;
  }
  return -1;
}


bool CReportListCtrl::IsEmpty()
{
    return (CListCtrl::GetItemCount() > 0) ? false : true;
}


void CReportListCtrl::SwapItems(int nItem1, int nItem2)
{
    if(!IsValidItemIndex(nItem1) || !IsValidItemIndex(nItem2))
    {
        ASSERT(0); // u suck!
        return;
    }

    // swapping needed?
    if(nItem1 == nItem2)
    {
        return;
    }

    EndEdit(TRUE);

    // assuming that a 4K buffer is really enough for storing the content of a single column
    const int LOCAL_BUFFER_SIZE = 4096;
    LVITEM lvi1, lvi2;
    UINT uMask = LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM  | LVIF_STATE;
    char szBuffer1[LOCAL_BUFFER_SIZE + 1], szBuffer2[LOCAL_BUFFER_SIZE + 1];
    lvi1.pszText = szBuffer1;
    lvi2.pszText = szBuffer2;
    lvi1.cchTextMax  = sizeof(szBuffer1);
    lvi2.cchTextMax  = sizeof(szBuffer2);

    BOOL bResult1 = GetItemWrapped(uMask, nItem1, 0, &lvi1, (UINT)-1);
    BOOL bResult2 = GetItemWrapped(uMask, nItem2, 0, &lvi2, (UINT)-1);

    if(bResult1 && bResult2)
    {
        lvi1.iItem = nItem2;
        lvi2.iItem = nItem1;
        lvi1.mask = uMask;
        lvi2.mask = uMask;
        lvi1.stateMask = (UINT)-1;
        lvi2.stateMask = (UINT)-1;

        // swap the items
        SetItemWrapped(&lvi1);
        SetItemWrapped(&lvi2);

        int nColumns = GetColumnCount();

        // loop over all columns
        for(int index = 1; index < nColumns; index++)
        {
            szBuffer1[0] = '\0';
            szBuffer2[0] = '\0';

            CListCtrl::GetItemText(nItem1, index, szBuffer1, LOCAL_BUFFER_SIZE);
            CListCtrl::GetItemText(nItem2, index, szBuffer2, LOCAL_BUFFER_SIZE);

            CListCtrl::SetItemText(nItem2, index, szBuffer1);
            CListCtrl::SetItemText(nItem1, index, szBuffer2);
        }
    }
}


bool CReportListCtrl::MoveItem(int nItem, int nNewPosition)
{
    if(!IsValidItemIndex(nItem))
    {
        ASSERT(0);
        return false;
    }

    EndEdit(TRUE);

    // verify that new position is valid
    nNewPosition = max(0, nNewPosition);
    nNewPosition = min(GetTailIndex(), nNewPosition);

    // moving needed?
    if(nItem == nNewPosition)
    {
        return true;
    }

    // assuming that a 4K buffer is really enough for storing the content of a single column
    const int LOCAL_BUFFER_SIZE = 4096;
    LVITEM lvi;
    UINT uMask = LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM  | LVIF_STATE;
    char szBuffer[LOCAL_BUFFER_SIZE + 1];
    lvi.pszText = szBuffer;
    lvi.cchTextMax = sizeof(szBuffer);

    if(GetItemWrapped(uMask, nItem, 0, &lvi, (UINT)-1))
    {
        int nColumns = GetColumnCount();

        // get text of all columns
        CStringArray arrayColumnTexts;
        for(int i = 0; i < nColumns; i++)
        {
            arrayColumnTexts.Add(CListCtrl::GetItemText(nItem, i));
        }

        // remove old item
        VERIFY(DeleteItemWrapped(nItem));

        // insert new item
        int index = InsertItemWrapped(nNewPosition, _T(""));

        // restore column texts
        for(int i = 0; i < nColumns; i++)
        {
            CListCtrl::SetItemText(index, i, arrayColumnTexts.GetAt(i));
        }

        // restore all other attributes and states
        lvi.iItem = index;
        SetItemWrapped(&lvi);

        return true;
    }
    else
    {
        ASSERT(0); // something broke!
    }

    return false;
}


bool CReportListCtrl::MoveSelectedItemsUp()
{
    if(GetItemCount() <= 1          // list contains only one item or less
        || IsItemSelected(0))       // head item is selected
    {
        return false;
    }

    // get selected items
    CItemsArray arraySelectedItems;
    GetSelectedItems(&arraySelectedItems);
    if(arraySelectedItems.empty()) // no item selected
        return false;

    // move items up
    CItemsArray::iterator iter;
    for(iter = arraySelectedItems.begin(); iter != arraySelectedItems.end(); iter++)
    {
        int nItem = *iter;
        SwapItems(nItem, nItem - 1);
    }

    return true;
}


bool CReportListCtrl::MoveSelectedItemsDown()
{
    if(GetItemCount() <= 1                  // list contains only one item or less
        || IsItemSelected(GetTailIndex()))  // tail item selected
    {
        return false;
    }

    // get selected items
    CItemsArray arraySelectedItems;
    GetSelectedItems(&arraySelectedItems);
    if(arraySelectedItems.empty()) // no item selected
        return false;

    // move items down (in reverse order!)
    CItemsArray::reverse_iterator rvIter;
    for(rvIter = arraySelectedItems.rbegin(); rvIter != arraySelectedItems.rend(); rvIter++)
    {
        int nItem = *rvIter;
        SwapItems(nItem, nItem + 1);
    }

    return true;
}


bool CReportListCtrl::MoveSelectedItemsToTop()
{
    if(GetItemCount() <= 1          // list contains only one item or less
        || IsItemSelected(0))       // head item is selected
    {
        return false;
    }

    // get selected items
    CItemsArray arraySelectedItems;
    GetSelectedItems(&arraySelectedItems);
    if(arraySelectedItems.empty()) // no item selected
        return false;

    // move items to top
    CItemsArray::iterator iter;
    for(iter = arraySelectedItems.begin(); iter != arraySelectedItems.end(); iter++)
    {
        int nItem = *iter;      // current position
        int nNewPosition = 0;   // new position

        if(iter != arraySelectedItems.begin())
        {
            // calculate offset from first selected item
            int nOffset = abs(nItem - arraySelectedItems.front());
            nNewPosition += nOffset;
        }

        VERIFY(MoveItem(nItem, nNewPosition));
    }

    return true;
}


bool CReportListCtrl::MoveSelectedItemsToBottom()
{
    if(GetItemCount() <= 1                  // list contains only one item or less
        || IsItemSelected(GetTailIndex()))  // tail item selected
    {
        return false;
    }

    // get selected items
    CItemsArray arraySelectedItems;
    GetSelectedItems(&arraySelectedItems);
    if(arraySelectedItems.empty()) // no item selected
        return false;

    // get the tail index
    int nTailIndex = GetTailIndex();

    // move items to bottom (in reverse order!)
    CItemsArray::reverse_iterator rvIter;
    for(rvIter = arraySelectedItems.rbegin(); rvIter != arraySelectedItems.rend(); rvIter++)
    {
        int nItem = *rvIter;            // current position
        int nNewPosition = nTailIndex;  // new position

        if(rvIter != arraySelectedItems.rbegin())
        {
            // calculate offset to last selected item
            int nOffset = abs(arraySelectedItems.back() - nItem);
            nNewPosition -= nOffset;
        }

        VERIFY(MoveItem(nItem, nNewPosition));
    }

    // assure that item bkgnd color isn't messed up
    RedrawWindow();

    return true;
}


LRESULT CReportListCtrl::OnSetFont(WPARAM wParam, LPARAM)
{
    LRESULT res = Default();

    // when the font changes the row height should be recalculated because standard behavior
    // doesn't regard any external leading of the new font(see MSDN - TEXTMETRIC).
    UpdateRowHeight();

    return res;
}


void CReportListCtrl::UpdateRowHeight()
{
    // only update row height if no image list is set
    if(GetImageList(LVSIL_SMALL) != NULL)
        return;

    TEXTMETRIC tm;
    HDC hDC = ::GetDC(NULL);
    CFont* pFont = GetFont();
    if(!hDC || !pFont)
        return;
    HFONT hFontOld = (HFONT)SelectObject(hDC, pFont->GetSafeHandle());
    GetTextMetrics(hDC, &tm);
    int nHeight = tm.tmHeight + tm.tmExternalLeading + 1;
    SelectObject(hDC, hFontOld);
    ::ReleaseDC(NULL, hDC);

    nHeight = max(nHeight, m_nButtonHeight); // take the bigger one of text height and button height

    if(nHeight > 0)
    {
        if(m_ImageList.m_hImageList) m_ImageList.DeleteImageList();
        m_ImageList.Create(1, nHeight, ILC_COLOR, 1, 1);
        SetSafeImageList(&m_ImageList, LVSIL_SMALL);
    }
}


void CReportListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  CListCtrl::OnLButtonDown(nFlags, point);

  LVHITTESTINFO lvhti;
  lvhti.pt = point;

  if(SubItemHitTest(&lvhti) != -1)
  {
    if(lvhti.iSubItem == GetFirstEditableColumn()) // clicked on an editable column
      StartEdit(lvhti.iItem, lvhti.iSubItem, true); // start edit and expand combobox list
  }
}


void CReportListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  CListCtrl::OnLButtonDblClk(nFlags, point);
}


void CReportListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
  CListCtrl::OnRButtonDown(nFlags, point);
}


void CReportListCtrl::OnRButtonDblClk(UINT nFlags, CPoint point)
{
  CListCtrl::OnRButtonDblClk(nFlags, point);
}


void CReportListCtrl::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
  EndEdit(FALSE);
  CListCtrl::OnNcLButtonDown(nHitTest, point);
}


void CReportListCtrl::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
  EndEdit(FALSE);
  CListCtrl::OnNcRButtonDown(nHitTest, point);
}


BOOL CReportListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  EndEdit(FALSE);
  return CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}


BOOL CReportListCtrl::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMLISTVIEW pNMListView = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

  if((pNMListView->uNewState & LVIS_SELECTED) && !(pNMListView->uOldState & LVIS_SELECTED))
  {
    StartEdit(pNMListView->iItem);
  }
  else if((pNMListView->uOldState & LVIS_SELECTED) && !(pNMListView->uNewState & LVIS_SELECTED))
  {
    EndEdit(FALSE);
  }

  *pResult = 0;
  return FALSE; // return FALSE to send message to parent also
}


BOOL CReportListCtrl::StartEdit(int nItem, int nSubItem /*-1*/, bool bShowDrop /*false*/)
{
  if(nSubItem == -1) // we must get the editable column by ourselves
    nSubItem = GetFirstEditableColumn();

  if(!IsValidItemIndex(nItem) || !IsValidColumnIndex(nSubItem) || !IsItemEnabled(nItem))
    return FALSE;

  if(m_ptEditting.x == nItem && m_ptEditting.y == nSubItem) // we are already editting the specified item
  {
    if(bShowDrop)
    {
      if(!m_pComboBox->GetDroppedState())
        m_pComboBox->ShowDropDown();
    }
    return TRUE;
  }

  // end previous edit and update editting point
  EndEdit(FALSE);
  m_ptEditting.x = nItem;
  m_ptEditting.y = nSubItem;
  SetItemState(m_ptEditting.x, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

  // create combobox and place it on item to edit
  if(m_pComboBox->m_hWnd)
    m_pComboBox->DestroyWindow();

  DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL;
  if(!m_pComboBox->Create(dwStyle, CRect(0, 0, 1, 200), this, 0))
    return FALSE;

  CRect comboRect;
  GetSubItemRect(m_ptEditting.x, m_ptEditting.y, LVIR_LABEL, comboRect);

  m_pComboBox->MoveWindow(&comboRect);
  m_pComboBox->SetFont(CListCtrl::GetFont());
  m_pComboBox->SetDroppedWidth(comboRect.Width());

  // retrieve combo strings for item to edit from parent window
  CReportListCtrlItemOptions itemOptions(m_ptEditting.x, m_ptEditting.y);
  LRESULT result = GetParent()->SendMessage(UWM_EDIT_NEEDOPTIONS, (WPARAM)&itemOptions, NULL);

  if(result == S_OK && itemOptions.GetCount() > 0)
  {
    int nOptions = (int)itemOptions.GetCount();
    for(int index = 0; index < nOptions; index++)
      m_pComboBox->InsertString(index, itemOptions.GetAt(index));
  }
  else
  {
    m_pComboBox->ShowWindow(SW_HIDE);
    return FALSE;
  }

  // show combo and init selected item
  m_pComboBox->ShowWindow(SW_SHOW);
  m_pComboBox->SetCurSel(m_pComboBox->FindStringExact(-1, GetItemText(m_ptEditting.x, m_ptEditting.y)));

  if(bShowDrop)
  {
    m_pComboBox->ShowDropDown();
    m_pComboBox->SetFocus();
  }
  else
  {
    if(!m_pComboBox->GetDroppedState())
    {
      // set focus back on item (enables tabbing)
      SetItemState(m_ptEditting.x, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
  }

  return TRUE;
}


BOOL CReportListCtrl::EndEdit(BOOL bCommit)
{
  if(!IsComboVisible())
    return FALSE;

  BOOL bChanged = FALSE;
  int nComboCurSel = m_pComboBox->GetCurSel();

  if((nComboCurSel != CB_ERR) && bCommit)
  {
    // get commited string
    CString strComitted;
    m_pComboBox->GetLBText(nComboCurSel, strComitted);

    // process directly edited item (must not have active selection anymore!)
    if(strComitted.Compare(GetItemText(m_ptEditting.x, m_ptEditting.y)) != 0)
    {
      bChanged = TRUE; // at least one item was changed
      CListCtrl::SetItemText(m_ptEditting.x, m_ptEditting.y, strComitted); // update list item
      GetParent()->SendMessage(UWM_EDIT_COMMITTED, (WPARAM)m_ptEditting.x, (LPARAM)m_ptEditting.y); // notify parent
    }

    // process all other selected items (multi-selection)
    POSITION pos = GetFirstSelectedItemPosition();
    while(pos)
    {
      int nItem = GetNextSelectedItem(pos);
      if(!IsItemEnabled(nItem) || nItem == m_ptEditting.x) // no processing needed
        continue;

      if(strComitted.Compare(GetItemText(nItem, m_ptEditting.y)) != 0)
      {
        bChanged = TRUE; // at least one item was changed
        CListCtrl::SetItemText(nItem, m_ptEditting.y, strComitted); // update list item
        GetParent()->SendMessage(UWM_EDIT_COMMITTED, (WPARAM)nItem, (LPARAM)m_ptEditting.y); // notify parent
      }
    }

    // set focus back on item
    SetItemState(m_ptEditting.x, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
  }

  m_pComboBox->ShowWindow(SW_HIDE);

  m_ptEditting.x = -1;
  m_ptEditting.y = -1;

  return bChanged;
}


BOOL CReportListCtrl::HotEdit()
{
  if(!IsComboVisible())
    return FALSE;

  int nComboCurSel = m_pComboBox->GetCurSel();
  if(nComboCurSel != CB_ERR)
  {
    // get selected combo string
    CString strCombo;
    m_pComboBox->GetLBText(nComboCurSel, strCombo);

    // process directly edited item (must not have active selection anymore!)
    if(strCombo.Compare(GetItemText(m_ptEditting.x, m_ptEditting.y)) != 0)
    {
      CListCtrl::SetItemText(m_ptEditting.x, m_ptEditting.y, strCombo); // update list item
      GetParent()->SendMessage(UWM_EDIT_COMMITTED, (WPARAM)m_ptEditting.x, (LPARAM)m_ptEditting.y); // notify parent
    }

    // process all other selected items (multi-selection)
    POSITION pos = GetFirstSelectedItemPosition();
    while(pos)
    {
      int nItem = GetNextSelectedItem(pos);
      if(!IsItemEnabled(nItem) || nItem == m_ptEditting.x) // no processing needed
        continue;

      if(strCombo.Compare(GetItemText(nItem, m_ptEditting.y)) != 0)
      {
        CListCtrl::SetItemText(nItem, m_ptEditting.y, strCombo); // update list item
        GetParent()->SendMessage(UWM_EDIT_COMMITTED, (WPARAM)nItem, (LPARAM)m_ptEditting.y); // notify parent
      }
    }

    // renew focus on combobox if necessary
    if(GetFocus() != m_pComboBox)
    {
      m_pComboBox->SetFocus();
    }
  }

  return TRUE;
}


void CReportListCtrl::UpdateHeader()
{
  CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
  if(pHeader)
    pHeader->Invalidate();
}


void CReportListCtrl::UpdateCombo()
{
  if(!IsComboVisible())
    return;

  CRect rectSubItem;
  GetSubItemRect(m_ptEditting.x, m_ptEditting.y, LVIR_BOUNDS, rectSubItem);

  CRect rectCombo;
  m_pComboBox->GetWindowRect(&rectCombo);
  ScreenToClient(&rectCombo);

  // move or just redraw
  if(rectSubItem.left != rectCombo.left
    || rectSubItem.top != rectCombo.top
    || rectSubItem.Width() != rectCombo.Width())
  {
    // note: don't try to change the height of the combobox! since a combobox has a
    // fixed height it will lead to a drawing loop which results in heavy flickering!
    m_pComboBox->MoveWindow(rectSubItem.left, rectSubItem.top, rectSubItem.Width(), rectCombo.Height());
  }
  else
  {
    m_pComboBox->Invalidate();
  }
}


void CReportListCtrl::OnDestroy()
{
    // free resources
    DeleteAllItems();

    if(m_ImageList.m_hImageList)
        m_ImageList.DeleteImageList();

    CListCtrl::OnDestroy();
}


void CReportListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);

    UpdateEmptyMessage();
}


void CReportListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

  switch(nSBCode)
  {
    case SB_ENDSCROLL:
      {
        if(!m_bCustomGridlines)
        {
          // note: small hack because of crappy standard gridline painting when scrolling
          // appears. only needed when NOT painting custom gridlines.
          if(GetExtendedStyle() & LVS_EX_GRIDLINES)
            RedrawWindow();
        }
      }
      break;

    default:
      break;
  }

  UpdateEmptyMessage();
}


void CReportListCtrl::OnSize(UINT nType, int cx, int cy)
{
    CListCtrl::OnSize(nType, cx, cy);

    UpdateEmptyMessage();
}


void CReportListCtrl::OnHdnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
    UpdateEmptyMessage();
    *pResult = FALSE;
}


void CReportListCtrl::OnHdnTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
    UpdateEmptyMessage();
    *pResult = FALSE;
}


void CReportListCtrl::OnHdnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
    UpdateEmptyMessage();
    *pResult = FALSE;
}


void CReportListCtrl::OnHdnEndTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
    UpdateEmptyMessage();
    *pResult = FALSE;
}


void CReportListCtrl::OnHdnItemClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

    if(m_bSortingEnabled) // sorting is on
    {
        if(phdr->iButton == 0) // left button was clicked
        {
            // notify parent to perform sorting
            CWnd* pParent = GetParent();                                            ASSERT(pParent);
            if(pParent)
            {
                pParent->SendMessage(UWM_SORT_COLUMN, (WPARAM)phdr->iItem, NULL);
            }
        }
    }

    *pResult = 0;
}


BOOL CReportListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  HD_NOTIFY* pHD = (HD_NOTIFY*)lParam;

  if(pHD)
  {
    switch(pHD->hdr.code)
    {
        case HDN_BEGINTRACKA:
        case HDN_BEGINTRACKW:
            OnHdnBeginTrack(&pHD->hdr, pResult);
            break;

        case HDN_TRACKA:
        case HDN_TRACKW:
            OnHdnTrack(&pHD->hdr, pResult);
            break;

        case HDN_ITEMCHANGINGA:
        case HDN_ITEMCHANGINGW:
            OnHdnItemChanging(&pHD->hdr, pResult);
            break;

        case HDN_ENDTRACKA:
        case HDN_ENDTRACKW:
            OnHdnEndTrack(&pHD->hdr, pResult);
            break;

        case HDN_ITEMCLICKA:
        case HDN_ITEMCLICKW:
            OnHdnItemClick(&pHD->hdr, pResult);
            break;

        default:
            break;
    }
  }

  *pResult = 0;
  return CListCtrl::OnNotify(wParam, lParam, pResult);
}


BOOL CReportListCtrl::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
  // need to handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
  TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

  // ignore messages from the built in tooltip, we are processing them internally
  if((pNMHDR->idFrom == (UINT_PTR)m_hWnd)
    && ((pNMHDR->code == TTN_NEEDTEXTA) && (pTTTA->uFlags & TTF_IDISHWND))
      || ((pNMHDR->code == TTN_NEEDTEXTW) && (pTTTW->uFlags & TTF_IDISHWND)) )
  {
    return FALSE;
  }

  *pResult = 0;

  // get cursor position
  CPoint ptCursor(0, 0);
  const MSG* pMessage = GetCurrentMessage();                                        ASSERT(pMessage);
  if(pMessage)
    ptCursor = pMessage->pt;
  else
    GetCursorPos(&ptCursor);
  ScreenToClient(&ptCursor);

  // perform hit test on the cursor position
  LVHITTESTINFO lvhti;
  lvhti.pt = ptCursor;
  int nItem     = SubItemHitTest(&lvhti);
  int nSubItem  = lvhti.iSubItem;

  // was a ListCtrl item hit?
  if(nItem >= 0)
  {
    // get tooltip text from parent
    CReportListCtrlItemToolTip itemToolTip(nItem, nSubItem);
    LRESULT result = GetParent()->SendMessage(UWM_LTTN_NEEDTEXT, (WPARAM)&itemToolTip, NULL);
    if(result != S_OK || itemToolTip.strTooltipText.IsEmpty())
        return FALSE; // not handled or no text provided
    CString strToolTipText = itemToolTip.strTooltipText;

    // copy tooltip text to local buffer
    USES_CONVERSION;
    #ifndef _UNICODE
      if(pNMHDR->code == TTN_NEEDTEXTA)
        m_astrToolTipText = strToolTipText;
      else
        m_wstrToolTipText = A2W(strToolTipText);
    #else
      if(pNMHDR->code == TTN_NEEDTEXTA)
        m_astrToolTipText = W2A(strToolTipText);
      else
        m_wstrToolTipText = strToolTipText;
    #endif

    // set tooltip text
    if(pNMHDR->code == TTN_NEEDTEXTA)
      pTTTA->lpszText = m_astrToolTipText.GetBuffer();
    else
      pTTTW->lpszText = m_wstrToolTipText.GetBuffer();

    return FALSE; // handled

    ////////////////////////////////////////////////////////////////////////////////
    // ****** Special note *****
    //
    // Still don't understand why the function must return FALSE for CListCtrl
    // so as not to cause flickering, as opposed to Nate Maynard's derivation
    // from CTreeCtrl.
    // I have experimented with disabling Tooltips for the control
    // and found out that a "ghost" tooltip appears for a fraction of a second...
    //
    // I am completely at a loss...
    // Seems to work, though...
    //
    ////////////////////////////////////////////////////////////////////////////////
  }

  return FALSE; // not handled
}


void CReportListCtrl::DrawCustomGridlines(HDC hdc)
{
  if(!hdc || !m_bCustomGridlines)
    return;

  ASSERT((CListCtrl::GetExtendedStyle() & LVS_EX_GRIDLINES) == 0);

  // set grid pen
  HPEN hOldPen = (HPEN)SelectObject(hdc, m_GridLinePen);

  // get client rect
  CRect clientRect;
  GetClientRect(&clientRect);

  // subtract header from client rect
  CRect headerRect;
  CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
  if(pHeader)
  {
    pHeader->GetClientRect(&headerRect);
    clientRect.top += headerRect.Height();
  }

  // draw horizontal gridlines
  CRect itemRect;
  if(GetSubItemRect(0, 1, LVIR_BOUNDS, itemRect))
  {
    int yPos = itemRect.bottom;
    while(yPos < clientRect.bottom)
    {
      MoveToEx(hdc, clientRect.left, yPos, NULL);
      LineTo(hdc, clientRect.right, yPos);

      yPos += itemRect.Height();
    }
  }

  // draw vertical gridlines
  int nColumns = GetColumnCount();
  if(nColumns > 0)
  {
    CRect rectColumn;

    for(int column = 1; column < nColumns; column++)
    {
      if(GetSubItemRect(0, column, LVIR_BOUNDS, rectColumn))
      {
        if(column == 1)
        {
          MoveToEx(hdc, rectColumn.left, clientRect.top, NULL);
          LineTo(hdc, rectColumn.left, clientRect.bottom);
        }

        MoveToEx(hdc, rectColumn.right, clientRect.top, NULL);
        LineTo(hdc, rectColumn.right, clientRect.bottom);
      }
    }
  }

  // restore dc
  SelectObject(hdc, hOldPen);
}


void CReportListCtrl::DrawEmptyMessage(HDC hdc)
{
    if(!hdc || !IsEmpty() || m_strEmptyMessage.IsEmpty())
        return;

    int nSavedDC = ::SaveDC(hdc);
    {
        // set up variables
        COLORREF clrText = m_clrEmptyMessage;
        COLORREF clrBack = ::GetSysColor(COLOR_WINDOW);
        CRect clientRect;
        GetClientRect(&clientRect);

        // if there is a header, we need to take its height into account
        CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
        if(pHeader)
        {
            CRect headerRect;
            pHeader->GetClientRect(headerRect);
            clientRect.top += headerRect.bottom;
        }

        // prepare DC for text display
        ::SetTextColor(hdc, clrText);
        ::SetBkColor(hdc, clrBack);
        ::SetBkMode(hdc, OPAQUE);

        // select font into DC
        if(m_bBoldEmptyMessage && m_pBoldFont)
            ::SelectObject(hdc, m_pBoldFont->GetSafeHandle());
        else
            ::SelectObject(hdc, CListCtrl::GetFont()->GetSafeHandle());

        // get item height
        CRect itemRect;
        GetSubItemRect(0, 1, LVIR_BOUNDS, itemRect);
        int nItemHeight = itemRect.Height();

        // adjust empty message rect
        clientRect.top += 5;                        // so text is in the middle
        if(clientRect.Height() > (3 * nItemHeight)) // there is enough space, so put text in second row
            clientRect.top += nItemHeight;

        // draw the text
        ::DrawText(hdc,
                   m_strEmptyMessage,
                   m_strEmptyMessage.GetLength(),
                   &clientRect,
                   DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);
    }
    ::RestoreDC(hdc, nSavedDC);
}


void CReportListCtrl::DrawComboButtons(HDC hdc)
{
  if(!hdc)
    return;

  int nEditableColumn = GetFirstEditableColumn();
  if(nEditableColumn < 0) // no editable column
    return;

  CRect rectSubItem;

  int nItems = GetItemCount();
  for(int i = 0; i < nItems; i++)
  {
    if(!IsItemEnabled(i)) // not enabled so don't paint combo button
      continue;

    if(!GetSubItemRect(i, nEditableColumn, LVIR_BOUNDS, rectSubItem))
      continue; // error: couldn't retrieve subitem rect

    // prepare button rect
    CRect rectButton;
    rectButton = rectSubItem;
    if(rectSubItem.Width() > m_nButtonWidth + 3)
    {
      rectButton.left = rectButton.right - m_nButtonWidth;
      rectSubItem.right -= m_nButtonWidth;
    }
    else
    {
      rectButton.left = rectButton.right; // column is not wide enough, so don't draw the button
    }

    if(rectButton.Width() > 0)
    {
      /*
      // %TODO%: support windows xp styles
      VUXTheme* pUXTheme = VUXTheme::GetUXTheme();
      HTHEME hTheme = pUXTheme ? pUXTheme->OpenThemeData(*this, L"ComboBox") : 0;

      if(hTheme) // theme available
      {
        pUXTheme->DrawThemeBackground(hTheme, hdc, CP_DROPDOWNBUTTON, CBXS_NORMAL, &rectButton, NULL);
        pUXTheme->CloseThemeData(hTheme);
      }
      else // no theme available
      */
      {
        ::DrawFrameControl(hdc, rectButton, DFC_SCROLL, DFCS_SCROLLCOMBOBOX);
      }
    }
  }
}


/** fills the white border in front of nItem with the correct state dependent color (passed color is used as fallback) */
void CReportListCtrl::FillIconGap(HDC hdc, int nItem, COLORREF color)
{
  if(!hdc || !IsValidItemIndex(nItem))
  {
    ASSERT(0);
    return;
  }

  RECT rect;
  GetItemRect(nItem, &rect, LVIR_ICON);
  rect.left = 0; // this takes care of the white border

  CDC* pDC = CDC::FromHandle(hdc);
  bool bHandled = false;

  if(CListCtrl::GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED) // item is selected
  {
    if(GetFocus() == this)                  // selected, focused
    {
      pDC->FillSolidRect(&rect, ::GetSysColor(COLOR_HIGHLIGHT));
      bHandled = true;
    }
    else if(GetStyle() & LVS_SHOWSELALWAYS) // selected, not focused, always show selection
    {
      pDC->FillSolidRect(&rect, ::GetSysColor(COLOR_BTNFACE));
      bHandled = true;
    }
    //else: selected, not focused, no always show selection
  }

  if(!bHandled) // (1) selected, not focused, no always show selection; (2) not selected
  {
    CReportListCtrlItemData* pData = reinterpret_cast<CReportListCtrlItemData*>(CListCtrl::GetItemData(nItem)); ASSERT(pData);
    if(pData)
    {
      if(m_bAlternatingRowColor && (nItem % 2) == 0) // alternate row coloring on and it's the colored row
      {
        color = (pData->aBkColors[0] != COLOR_INVALID)
                  ? pData->aBkColors[0]
                  : m_clrAlternatingRowColor;
      }
      else // alternate row coloring off or not the colored row
      {
        color = (pData->aBkColors[0] != COLOR_INVALID)
                  ? pData->aBkColors[0]
                  : ::GetSysColor(COLOR_WINDOW);
      }
    }

    pDC->FillSolidRect(&rect, color);
  }
}


