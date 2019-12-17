// TreePropSheetOffice2003.cpp: implementation of the CTreePropSheetOffice2003 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreePropSheetOffice2003.h"
#include "PropPageFrameOffice2003.h"

namespace TreePropSheet
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CTreePropSheetOffice2003, CTreePropSheetEx)

CTreePropSheetOffice2003::CTreePropSheetOffice2003()
{
	m_bDrawGradientOnCaption = FALSE;
	m_bDrawLineOnCaption     = TRUE;
}

CTreePropSheetOffice2003::CTreePropSheetOffice2003(UINT nIDCaption,CWnd* pParentWnd /*=NULL*/,UINT iSelectPage /*=0*/)
 : CTreePropSheetEx(nIDCaption, pParentWnd, iSelectPage)
{
	m_bDrawGradientOnCaption = FALSE;
	m_bDrawLineOnCaption     = TRUE;
}

CTreePropSheetOffice2003::CTreePropSheetOffice2003(LPCTSTR pszCaption,CWnd* pParentWnd /*=NULL*/,UINT iSelectPage /*=0*/)
 : CTreePropSheetEx(pszCaption, pParentWnd, iSelectPage)
{
	m_bDrawGradientOnCaption = FALSE;
	m_bDrawLineOnCaption     = TRUE;
}

CTreePropSheetOffice2003::~CTreePropSheetOffice2003()
{
}

//////////////////////////////////////////////////////////////////////
// Overrided implementation helpers
//////////////////////////////////////////////////////////////////////

CPropPageFrame* CTreePropSheetOffice2003::CreatePageFrame()
{
	return new CPropPageFrameOffice2003;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTreePropSheetOffice2003, CTreePropSheetEx)
	//{{AFX_MSG_MAP(CTreePropSheetOffice2003)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////
// Overridings
/////////////////////////////////////////////////////////////////////

BOOL CTreePropSheetOffice2003::OnInitDialog() 
{
	BOOL bRet = CTreePropSheetEx::OnInitDialog();

	// If in tree mode, update the tree style.
	if( IsTreeViewMode() && GetPageTreeControl() )
	{
		GetPageTreeControl()->ModifyStyle( TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_TRACKSELECT, TVS_FULLROWSELECT|TVS_SHOWSELALWAYS, 0 );
	}

	CPropPageFrameOffice2003* pFrame = dynamic_cast<CPropPageFrameOffice2003*>(GetFrameControl());
	if(pFrame)
	{
		pFrame->SetCaptionStyle(m_bDrawGradientOnCaption, m_bDrawLineOnCaption);
	}

	return bRet;
}

void CTreePropSheetOffice2003::SetCaptionStyle(BOOL bGradient, BOOL bLine)
{
	m_bDrawGradientOnCaption = bGradient;
	m_bDrawLineOnCaption     = bLine;
}


}; // namespace TreePropSheet
