// TreePropSheetBordered.cpp: implementation of the CTreePropSheetBordered class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreePropSheetBordered.h"
#include "PropPageFrameBordered.h"

namespace TreePropSheet
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreePropSheetBordered::CTreePropSheetBordered()
{
}

CTreePropSheetBordered::CTreePropSheetBordered(UINT nIDCaption,CWnd* pParentWnd /*=NULL*/,UINT iSelectPage /*=0*/)
 : CTreePropSheetEx(nIDCaption, pParentWnd, iSelectPage)
{
}

CTreePropSheetBordered::CTreePropSheetBordered(LPCTSTR pszCaption,CWnd* pParentWnd /*=NULL*/,UINT iSelectPage /*=0*/)
 : CTreePropSheetEx(pszCaption, pParentWnd, iSelectPage)
{
}

CTreePropSheetBordered::~CTreePropSheetBordered()
{
}

//////////////////////////////////////////////////////////////////////
// Overrided implementation helpers
//////////////////////////////////////////////////////////////////////

CPropPageFrame* CTreePropSheetBordered::CreatePageFrame()
{
  return new CPropPageFrameBordered;
}

}; // namespace TreePropSheet