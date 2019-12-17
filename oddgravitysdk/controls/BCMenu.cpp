//*************************************************************************
// BCMenu.cpp : implementation file
// Version : 3.036
// Date : June 2005
// Author : Brent Corkum
// Email :  corkum@rocscience.com
// Latest Version : http://www.rocscience.com/~corkum/BCMenu.html
// 
// Bug Fixes and portions of code supplied by:
//
// Ben Ashley,Girish Bharadwaj,Jean-Edouard Lachand-Robert,
// Robert Edward Caldecott,Kenny Goers,Leonardo Zide,
// Stefan Kuhr,Reiner Jung,Martin Vladic,Kim Yoo Chul,
// Oz Solomonovich,Tongzhe Cui,Stephane Clog,Warren Stevens,
// Damir Valiulin,David Kinder,Marc Loiry
//
// You are free to use/modify this code but leave this header intact.
// This class is public domain so you are free to use it any of
// your applications (Freeware,Shareware,Commercial). All I ask is
// that you let me know so that if you have a real winner I can
// brag to my buddies that some of my code is in your app. I also
// wouldn't mind if you sent me a copy of your application since I
// like to play with new stuff.
//*************************************************************************
//
// oddgravity extensions
//
// 2006/10/07: SetXPBitmapDisabled3D() / GetXPBitmapDisabled3D()
//             (sets whether disabled bitmaps are drawn flat or 3D)
//
//*************************************************************************

#include "stdafx.h"        // Standard windows header file
#include "BCMenu.h"        // BCMenu class declaration
#include <afxpriv.h>       //SK: makes A2W and other spiffy AFX macros work

#ifdef BCMENU_USE_MEMDC
#include <shared/MemDC.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BCMENU_GAP 1
#ifndef OBM_CHECK
#define OBM_CHECK 32760 // from winuser.h
#endif

#if _MFC_VER <0x400
#error This code does not work on Versions of MFC prior to 4.0
#endif

static CPINFO CPInfo;

// how the menu is drawn on Win9x/NT/2000
BCMenu::BCMDRAWMODE BCMenu::original_drawmode = BCMDM_ORIGINAL;
BOOL BCMenu::original_select_disabled = TRUE;

// how the menu is drawn on WinXP
BCMenu::BCMDRAWMODE BCMenu::xp_drawmode = BCMDM_XP;
BOOL BCMenu::xp_select_disabled = FALSE;
BOOL BCMenu::xp_draw_3D_bitmaps = TRUE;
BOOL BCMenu::xp_draw_3D_bitmaps_disabled = TRUE;
BOOL BCMenu::hicolor_bitmaps = FALSE;

// Variable to set how accelerators are justified. The default mode (TRUE) right
// justifies them to the right of the longes string in the menu. FALSE
// just right justifies them.
BOOL BCMenu::xp_space_accelerators = TRUE;
BOOL BCMenu::original_space_accelerators = TRUE;

CTypedPtrArray<CPtrArray, HMENU> BCMenu::m_AllSubMenus;  // stores list of all sub-menus
CImageList BCMenu::m_AllImages;
CArray<int,int&> BCMenu::m_AllImagesID;

// icon size
int BCMenu::m_iconWidth  = 16;
int BCMenu::m_iconHeight = 15;

// Windows version helper
enum Win32Type
{
    Win32s,
    WinNT3,
    Win95,
    Win98,
    WinME,
    WinNT4,
    Win2000,
    WinXP,
    WinVista,
    Win7
};

Win32Type IsShellType()
{
    Win32Type ShellType;
    DWORD winVer;
    OSVERSIONINFO* posvi;

    winVer = GetVersion();
    if(winVer < 0x80000000) // NT
    {
        ShellType = WinNT3;
        posvi = (OSVERSIONINFO*)malloc(sizeof(OSVERSIONINFO));
        if(posvi!=NULL)
        {
            memset(posvi, 0, sizeof(OSVERSIONINFO));
            posvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(posvi);
            if(posvi->dwMajorVersion == 4L) ShellType = WinNT4;
            else if(posvi->dwMajorVersion == 5L && posvi->dwMinorVersion == 0L) ShellType = Win2000;
            else if(posvi->dwMajorVersion == 5L && posvi->dwMinorVersion == 1L) ShellType = WinXP;
            else if(posvi->dwMajorVersion == 6L && posvi->dwMinorVersion == 0L) ShellType = WinVista;
            else if(posvi->dwMajorVersion == 6L && posvi->dwMinorVersion == 1L) ShellType = Win7;
            free(posvi);
        }
    }
    else if(LOBYTE(LOWORD(winVer)) < 4)
    {
        ShellType = Win32s;
    }
    else
    {
        ShellType = Win95;
        posvi = (OSVERSIONINFO*)malloc(sizeof(OSVERSIONINFO));
        if(posvi != NULL)
        {
            memset(posvi, 0, sizeof(OSVERSIONINFO));
            posvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(posvi);
            if(posvi->dwMajorVersion == 4L && posvi->dwMinorVersion == 10L) ShellType = Win98;
            else if(posvi->dwMajorVersion == 4L && posvi->dwMinorVersion == 90L) ShellType = WinME;
            free(posvi);
        }
    }
    return ShellType;
}

static Win32Type g_Shell = IsShellType();


void BCMenuData::SetAnsiString(LPCSTR szAnsiString)
{
    USES_CONVERSION;
    SetWideString(A2W(szAnsiString));  //SK:  see MFC Tech Note 059
}

//returns the menu text in ANSI or UNICODE depending on the MFC-Version we are using
CString BCMenuData::GetString()
{
    CString strText;
    if(m_szMenuText)
    {
#ifdef UNICODE
        strText = m_szMenuText;
#else
        USES_CONVERSION;
        strText=W2A(m_szMenuText); //SK: see MFC Tech Note 059
#endif    
    }
    return strText;
}


IMPLEMENT_DYNAMIC(BCMenu, CMenu)

/*
===============================================================================
BCMenu::BCMenu()
BCMenu::~BCMenu()
-----------------

Constructor and Destructor.

===============================================================================
*/

BCMenu::BCMenu()
{
    m_bDynIcons = FALSE;     // O.S. - no dynamic icons by default
    m_bOldStyleDisableMode = FALSE;
    m_selectcheck = -1;
    m_unselectcheck = -1;
    checkmaps = NULL;
    checkmapsshare = FALSE;
    // set the color used for the transparent background in all bitmaps
    m_bitmapBackground = RGB(192,192,192); //gray
    m_bitmapBackgroundFlag = FALSE;
    GetCPInfo(CP_ACP, &CPInfo);
    m_loadmenu = FALSE;
}


BCMenu::~BCMenu()
{
    DestroyMenu();
}

BOOL BCMenu::IsNewShell()
{
    return (g_Shell >= Win95);
}

BOOL BCMenu::IsThemed()
{
    BOOL ret = FALSE;

    typedef BOOL WINAPI ISAPPTHEMED();
    typedef BOOL WINAPI ISTHEMEACTIVE();

    ISAPPTHEMED* pISAPPTHEMED = NULL;
    ISTHEMEACTIVE* pISTHEMEACTIVE = NULL;

    HMODULE hMod = LoadLibrary(_T("uxtheme.dll"));
    if(hMod)
    {
        pISAPPTHEMED = reinterpret_cast<ISAPPTHEMED*>(GetProcAddress(hMod,_T("IsAppThemed")));
        pISTHEMEACTIVE = reinterpret_cast<ISTHEMEACTIVE*>(GetProcAddress(hMod,_T("IsThemeActive")));
        if(pISAPPTHEMED && pISTHEMEACTIVE)
        {
            if(pISAPPTHEMED() && pISTHEMEACTIVE())                
            {                
                typedef HRESULT CALLBACK DLLGETVERSION(DLLVERSIONINFO*);
                DLLGETVERSION* pDLLGETVERSION = NULL;

                HMODULE hModComCtl = LoadLibrary(_T("comctl32.dll"));
                if(hModComCtl)
                {
                    pDLLGETVERSION = reinterpret_cast<DLLGETVERSION*>(GetProcAddress(hModComCtl,_T("DllGetVersion")));
                    if(pDLLGETVERSION)
                    {
                        DLLVERSIONINFO dvi = {0};
                        dvi.cbSize = sizeof dvi;
                        if(pDLLGETVERSION(&dvi) == NOERROR)
                        {
                            ret = dvi.dwMajorVersion >= 6; // >= Windows XP
                        }
                    }
                    FreeLibrary(hModComCtl);                    
                }
            }
        }
        FreeLibrary(hMod);
    }
    return ret;
}

BOOL BCMenu::IsMenuThemeActive()
{
    if(IsThemed())
    {
        if(xp_drawmode == BCMDM_XP)
            return TRUE;
    }
    else // not themed
    {
        if(original_drawmode == BCMDM_XP)
            return TRUE;
    }
    return FALSE;
}

BCMenuData::~BCMenuData()
{
    if(bitmap)
        delete(bitmap);
    
    delete[] m_szMenuText; //Need not check for NULL because ANSI X3J16 allows "delete NULL"
}


void BCMenuData::SetWideString(const wchar_t *szWideString)
{
    delete[] m_szMenuText;//Need not check for NULL because ANSI X3J16 allows "delete NULL"
    
    if (szWideString)
    {
        size_t nChars = wcslen(szWideString)+1;
        m_szMenuText = new wchar_t[nChars];
        if (m_szMenuText)
        {
#if _MSC_VER >= 1400  // VS2005
            wcsncpy_s(m_szMenuText, nChars, szWideString, _TRUNCATE);
#else
            wcscpy(m_szMenuText, szWideString);
#endif
        }
    }
    else
        m_szMenuText=NULL;//set to NULL so we need not bother about dangling non-NULL Ptrs
}

BOOL BCMenu::IsMenu(CMenu *submenu)
{
    int m;
    int numSubMenus = (int)m_AllSubMenus.GetUpperBound();
    for(m=0;m<=numSubMenus;++m){
        if(submenu->m_hMenu==m_AllSubMenus[m])return(TRUE);
    }
    return(FALSE);
}

BOOL BCMenu::IsMenu(HMENU submenu)
{
    int m;
    int numSubMenus = (int)m_AllSubMenus.GetUpperBound();
    for(m=0;m<=numSubMenus;++m){
        if(submenu==m_AllSubMenus[m])return(TRUE);
    }
    return(FALSE);
}

BOOL BCMenu::DestroyMenu()
{
    // Destroy Sub menus:
    int m,n;
    int numAllSubMenus = (int)m_AllSubMenus.GetUpperBound();
    for(n = numAllSubMenus; n>= 0; n--){
        if(m_AllSubMenus[n]==this->m_hMenu)m_AllSubMenus.RemoveAt(n);
    }
    int numSubMenus = (int)m_SubMenus.GetUpperBound();
    for(m = numSubMenus; m >= 0; m--){
        numAllSubMenus = (int)m_AllSubMenus.GetUpperBound();
        for(n = numAllSubMenus; n>= 0; n--){
            if(m_AllSubMenus[n]==m_SubMenus[m])m_AllSubMenus.RemoveAt(n);
        }
        CMenu *ptr=FromHandle(m_SubMenus[m]);
        if(ptr){
            BOOL flag=ptr->IsKindOf(RUNTIME_CLASS( BCMenu ));
            if(flag)delete((BCMenu *)ptr);
        }
    }
    m_SubMenus.RemoveAll();
    // Destroy menu data
    int numItems = (int)m_MenuList.GetUpperBound();
    for(m = 0; m <= numItems; m++)delete(m_MenuList[m]);
    m_MenuList.RemoveAll();
    if(checkmaps&&!checkmapsshare){
        delete checkmaps;
        checkmaps=NULL;
    }
    // Call base-class implementation last:
    return(CMenu::DestroyMenu());
};

/*
==========================================================================
void BCMenu::DrawItem(LPDRAWITEMSTRUCT)
---------------------------------------

  Called by the framework when a particular item needs to be drawn.  We
  overide this to draw the menu item in a custom-fashion, including icons
  and the 3D rectangle bar.
  ==========================================================================
*/

void BCMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    ASSERT(lpDIS != NULL);
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);
    if(pDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
    {
        DrawItem_Win9xNT2000(lpDIS);
    }
    else
    {
        if(IsThemed())
        {
            if(xp_drawmode == BCMDM_XP)
                DrawItem_WinXP(lpDIS);
            else
                DrawItem_Win9xNT2000(lpDIS);
        }
        else // not themed
        {
            if(original_drawmode == BCMDM_XP)
                DrawItem_WinXP(lpDIS);
            else
                DrawItem_Win9xNT2000(lpDIS);
        }	
    }
}

void BCMenu::DrawItem_Win9xNT2000(LPDRAWITEMSTRUCT lpDIS)
{
    ASSERT(lpDIS != NULL);
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);
    CRect rect;
    UINT state = (((BCMenuData*)(lpDIS->itemData))->nFlags);
    CBrush brushBackground;
    COLORREF crBackground;

    if(IsThemed())crBackground=GetSysColor(COLOR_3DFACE);
    else crBackground=GetSysColor(COLOR_MENU);
    
    brushBackground.CreateSolidBrush(crBackground);

    // remove the selected bit if it's grayed out
    if(lpDIS->itemState & ODS_GRAYED&&!original_select_disabled){
        if(lpDIS->itemState & ODS_SELECTED)lpDIS->itemState=lpDIS->itemState & ~ODS_SELECTED;
    }
    
    if(state & MF_SEPARATOR){
        rect.CopyRect(&lpDIS->rcItem);
        pDC->FillRect (rect,&brushBackground);
        rect.top += (rect.Height()>>1);
        pDC->DrawEdge(&rect,EDGE_ETCHED,BF_TOP);
    }
    else{
        CRect rect2;
        BOOL flagStandard=FALSE,flagSelected=FALSE,flagDisabled=FALSE;
        BOOL flagCheck=FALSE;
        COLORREF crText = GetSysColor(COLOR_MENUTEXT);
        CBrush brushSelect;
        CPen penBackground;
        int x0,y0,dy;
        int nIconNormal=-1,nOffsetX=-1,nOffsetGlobal=-1;
        CImageList *bitmap=NULL;
        
        // set some colors
        penBackground.CreatePen (PS_SOLID,0,crBackground);
        brushSelect.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
        
        // draw the colored rectangle portion
        
        rect.CopyRect(&lpDIS->rcItem);
        rect2=rect;
        
        // draw the up/down/focused/disabled state
        
        UINT state = lpDIS->itemState;
        CString strText;
        
        if(lpDIS->itemData != NULL)
        {
            nIconNormal   = (((BCMenuData*)(lpDIS->itemData))->menuIconNormal);
            nOffsetX      = (((BCMenuData*)(lpDIS->itemData))->nOffsetX);
            nOffsetGlobal = (((BCMenuData*)(lpDIS->itemData))->nOffsetGlobal);
            bitmap        = (((BCMenuData*)(lpDIS->itemData))->bitmap);
            strText       = ((BCMenuData*) (lpDIS->itemData))->GetString();

            if(nIconNormal<0&&nOffsetGlobal>=0){
                nOffsetX=nOffsetGlobal;
                nIconNormal=0;
                bitmap = &m_AllImages;
            }
            
            if(state&ODS_CHECKED && nIconNormal<0){
                if(state&ODS_SELECTED && m_selectcheck>0)flagCheck=TRUE;
                else if(m_unselectcheck>0) flagCheck=TRUE;
            }
            else if(nIconNormal != -1){
                flagStandard=TRUE;
                if(state&ODS_SELECTED && !(state&ODS_GRAYED))flagSelected=TRUE;
                else if(state&ODS_GRAYED) flagDisabled=TRUE;
            }
        }
        else{
            strText.Empty();
        }
        
        if(state&ODS_SELECTED){ // draw the down edges
            
            CPen *pOldPen = pDC->SelectObject (&penBackground);
            
            // You need only Text highlight and thats what you get
            
            if(flagCheck||flagStandard||flagSelected||flagDisabled||state&ODS_CHECKED)
                rect2.SetRect(rect.left+m_iconWidth+4+BCMENU_GAP,rect.top,rect.right,rect.bottom);
            pDC->FillRect (rect2,&brushSelect);
            
            pDC->SelectObject (pOldPen);
            crText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        }
        else {
            CPen *pOldPen = pDC->SelectObject (&penBackground);
            pDC->FillRect (rect,&brushBackground);
            pDC->SelectObject (pOldPen);
            
            // draw the up edges	
            pDC->Draw3dRect (rect,crBackground,crBackground);
        }
        
        // draw the text if there is any
        //We have to paint the text only if the image is nonexistant
        
        dy = (rect.Height()-4-m_iconHeight)/2;
        dy = dy<0 ? 0 : dy;
        
        if(flagCheck||flagStandard||flagSelected||flagDisabled){
            rect2.SetRect(rect.left+1,rect.top+1+dy,rect.left+m_iconWidth+3,
                rect.top+m_iconHeight+3+dy);
            pDC->Draw3dRect (rect2,crBackground,crBackground);
            if(flagCheck && checkmaps){
                pDC->FillRect (rect2,&brushBackground);
                rect2.SetRect(rect.left,rect.top+dy,rect.left+m_iconWidth+4,
                    rect.top+m_iconHeight+4+dy);
                
                pDC->Draw3dRect (rect2,crBackground,crBackground);
                CPoint ptImage(rect.left+2,rect.top+2+dy);
                
                if(state&ODS_SELECTED)checkmaps->Draw(pDC,1,ptImage,ILD_TRANSPARENT);
                else checkmaps->Draw(pDC,0,ptImage,ILD_TRANSPARENT);
            }
            else if(flagDisabled){
                if(!flagSelected){
                    CBitmap bitmapstandard;
                    GetBitmapFromImageList(pDC,bitmap,nOffsetX,bitmapstandard);
                    rect2.SetRect(rect.left,rect.top+dy,rect.left+m_iconWidth+4,
                        rect.top+m_iconHeight+4+dy);
                    pDC->Draw3dRect (rect2,crBackground,crBackground);
                    if(m_bOldStyleDisableMode)
                        DitherBlt(lpDIS->hDC,rect.left+2,rect.top+2+dy,m_iconWidth,m_iconHeight,
                        (HBITMAP)(bitmapstandard),0,0,crBackground);
                    else{
                        if(hicolor_bitmaps)
                            DitherBlt3(pDC,rect.left+2,rect.top+2+dy,m_iconWidth,m_iconHeight,
                            bitmapstandard,crBackground);
                        else
                            DitherBlt2(pDC,rect.left+2,rect.top+2+dy,m_iconWidth,m_iconHeight,
                            bitmapstandard,0,0,crBackground);
                    }
                    bitmapstandard.DeleteObject();
                }
            }
            else if(flagSelected){
                pDC->FillRect (rect2,&brushBackground);
                rect2.SetRect(rect.left,rect.top+dy,rect.left+m_iconWidth+4,
                    rect.top+m_iconHeight+4+dy);
                if (IsNewShell()){
                    if(state&ODS_CHECKED)
                        pDC->Draw3dRect(rect2,GetSysColor(COLOR_3DSHADOW),
                        GetSysColor(COLOR_3DHILIGHT));
                    else
                        pDC->Draw3dRect(rect2,GetSysColor(COLOR_3DHILIGHT),
                        GetSysColor(COLOR_3DSHADOW));
                }
                CPoint ptImage(rect.left+2,rect.top+2+dy);
                if(bitmap)bitmap->Draw(pDC,nOffsetX,ptImage,ILD_TRANSPARENT);
            }
            else{
                if(state&ODS_CHECKED){
                    CBrush brush;
                    COLORREF col = crBackground;
                    col = LightenColor(col,0.6);
                    brush.CreateSolidBrush(col);
                    pDC->FillRect(rect2,&brush);
                    brush.DeleteObject();
                    rect2.SetRect(rect.left,rect.top+dy,rect.left+m_iconWidth+4,
                        rect.top+m_iconHeight+4+dy);
                    if (IsNewShell())
                        pDC->Draw3dRect(rect2,GetSysColor(COLOR_3DSHADOW),
                        GetSysColor(COLOR_3DHILIGHT));
                }
                else{
                    pDC->FillRect (rect2,&brushBackground);
                    rect2.SetRect(rect.left,rect.top+dy,rect.left+m_iconWidth+4,
                        rect.top+m_iconHeight+4+dy);
                    pDC->Draw3dRect (rect2,crBackground,crBackground);
                }
                CPoint ptImage(rect.left+2,rect.top+2+dy);
                if(bitmap)bitmap->Draw(pDC,nOffsetX,ptImage,ILD_TRANSPARENT);
            }
        }
        if(nIconNormal<0 && state&ODS_CHECKED && !flagCheck){
            rect2.SetRect(rect.left+1,rect.top+2+dy,rect.left+m_iconWidth+1,
                rect.top+m_iconHeight+2+dy);
            CMenuItemInfo info;
            info.fMask = MIIM_CHECKMARKS;
            ::GetMenuItemInfo((HMENU)lpDIS->hwndItem,lpDIS->itemID,
                MF_BYCOMMAND, &info);
            if(state&ODS_CHECKED || info.hbmpUnchecked) {
                Draw3DCheckmark(pDC, rect2, state&ODS_SELECTED,
                    state&ODS_CHECKED ? info.hbmpChecked :
                info.hbmpUnchecked);
            }
        }
        
        //This is needed always so that we can have the space for check marks
        
        x0=rect.left;y0=rect.top;
        rect.left = rect.left + m_iconWidth + 8 + BCMENU_GAP; 
        
        if(!strText.IsEmpty()){
            
            CRect rectText(rect.left,rect.top-1,rect.right,rect.bottom-1);
            
            //   Find tabs
            
            CString leftStr,rightStr;
            leftStr.Empty();rightStr.Empty();
            int tabPosition=strText.ReverseFind(_T('\t'));
            if(tabPosition!=-1){
                rightStr=strText.Mid(tabPosition+1);
                leftStr=strText.Left(strText.Find(_T('\t')));
                rectText.right-=m_iconWidth;
            }
            else leftStr=strText;
            
            int oldBkMode = pDC->GetBkMode();
            pDC->SetBkMode( TRANSPARENT);
            
            // Draw the text in the correct colour:
            
            UINT nFormat  = DT_LEFT|DT_SINGLELINE|DT_VCENTER;
            UINT nFormatr = DT_RIGHT|DT_SINGLELINE|DT_VCENTER;
            if(!(lpDIS->itemState & ODS_GRAYED)){
                pDC->SetTextColor(crText);
                pDC->DrawText (leftStr,rectText,nFormat);
                if(tabPosition!=-1) pDC->DrawText (rightStr,rectText,nFormatr);
            }
            else{
                
                // Draw the disabled text
                if(!(state & ODS_SELECTED)){
                    RECT offset = *rectText;
                    offset.left+=1;
                    offset.right+=1;
                    offset.top+=1;
                    offset.bottom+=1;
                    pDC->SetTextColor(GetSysColor(COLOR_BTNHILIGHT));
                    pDC->DrawText(leftStr,&offset, nFormat);
                    if(tabPosition!=-1) pDC->DrawText (rightStr,&offset,nFormatr);
                    pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
                    pDC->DrawText(leftStr,rectText, nFormat);
                    if(tabPosition!=-1) pDC->DrawText (rightStr,rectText,nFormatr);
                }
                else{
                    // And the standard Grey text:
                    pDC->SetTextColor(crBackground);
                    pDC->DrawText(leftStr,rectText, nFormat);
                    if(tabPosition!=-1) pDC->DrawText (rightStr,rectText,nFormatr);
                }
            }
            pDC->SetBkMode( oldBkMode );
        }
        
        penBackground.DeleteObject();
        brushSelect.DeleteObject();
    }
    brushBackground.DeleteObject();
}

COLORREF BCMenu::LightenColor(COLORREF color, double factor)
{
    if(factor > 0.0 && factor <= 1.0)
    {
        BYTE red    = GetRValue(color);
        BYTE green  = GetGValue(color);
        BYTE blue   = GetBValue(color);

        BYTE lightred   = (BYTE)((factor * (255 - red))   + red);
        BYTE lightgreen = (BYTE)((factor * (255 - green)) + green);
        BYTE lightblue  = (BYTE)((factor * (255 - blue))  + blue);

        color = RGB(lightred, lightgreen, lightblue);
    }
    return color;
}

COLORREF BCMenu::DarkenColor(COLORREF color, double factor)
{
    if(factor > 0.0 && factor <= 1.0)
    {
        BYTE red    = GetRValue(color);
        BYTE green  = GetGValue(color);
        BYTE blue   = GetBValue(color);

        BYTE darkred   = (BYTE)(red   - (factor * red));
        BYTE darkgreen = (BYTE)(green - (factor * green));
        BYTE darkblue  = (BYTE)(blue  - (factor * blue));
        
        color = RGB(darkred, darkgreen, darkblue);
    }
    return color;
}

void BCMenu::DrawItem_WinXP(LPDRAWITEMSTRUCT lpDIS)
{
    if(!lpDIS)
    {
        ASSERT(0); // error: no draw item struct
        return;
    }

    // get DC
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);                                                     ASSERT(pDC);
#ifdef BCMENU_USE_MEMDC
    CMemoryDC* pMemDC = new CMemoryDC(pDC, &CRect(lpDIS->rcItem));
    pDC = pMemDC;
#endif

    // get menu data
    BCMenuData* pBCMenuData = (BCMenuData*)lpDIS->itemData;                                     ASSERT(pBCMenuData);

    // menu colors
    COLORREF crMenuBackground    = DarkenColor(GetSysColor(COLOR_WINDOW), 0.02);
    COLORREF crIconBarBackground = DarkenColor(GetSysColor(COLOR_WINDOW), 0.06);
    COLORREF crMenuBorder        = DarkenColor(GetSysColor(COLOR_3DFACE), 0.17);
    COLORREF crText              = GetSysColor(COLOR_MENUTEXT);
    COLORREF crTextSelected      = GetSysColor(COLOR_CAPTIONTEXT);

    COLORREF crSelectionBorder;
    COLORREF crSelectionBackgroundStart;
    COLORREF crSelectionBackgroundStop;
    if(IsThemed())
    {
        crSelectionBorder           = LightenColor(GetSysColor(COLOR_HIGHLIGHT), 0.3);
        crSelectionBackgroundStart  = DarkenColor(GetSysColor(COLOR_WINDOW), 0.03);
        crSelectionBackgroundStop   = LightenColor(crSelectionBorder, 0.4);
    }
    else // not themed
    {
        crSelectionBorder           = GetSysColor(COLOR_HIGHLIGHT);
        crSelectionBackgroundStart  = crSelectionBorder;
        crSelectionBackgroundStop   = crSelectionBorder;
    }

    // get menu metrics
    int nTextOffset = GetMenuMetrics(BCMMET_TEXTOFFSET);
    int nIconBarPaddingHorz = GetMenuMetrics(BCMMET_ICONBARPADDING_HORZ);
    int nIconBarWidth = m_iconWidth + nIconBarPaddingHorz;

    // menu font
    NONCLIENTMETRICS nm;
    nm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0));
    CFont menuFont;
    menuFont.CreateFontIndirect(&nm.lfMenuFont);
    CFont* pOldFont = pDC->SelectObject(&menuFont);

    // remove the selected bit if item is grayed out
    if(lpDIS->itemState & ODS_GRAYED && !xp_select_disabled)
    {
        if(lpDIS->itemState & ODS_SELECTED)
            lpDIS->itemState = lpDIS->itemState & ~ODS_SELECTED;
    }

    // buffers
    CRect rect, rect2;

    // is item a separator?
    if(pBCMenuData->nFlags & MF_SEPARATOR)
    {
        //-----------------------------------------------
        // draw separator
        //-----------------------------------------------

        // draw menu background
        rect.CopyRect(&lpDIS->rcItem);
        pDC->FillSolidRect(rect, crMenuBackground);

        // draw icon bar background
        rect2.SetRect(rect.left, rect.top, rect.left + nIconBarWidth, rect.bottom);
        pDC->FillSolidRect(rect2, crIconBarBackground);

        // draw icon bar border
        rect2.left = rect2.right - 1;
        pDC->FillSolidRect(rect2, crMenuBorder);

        // draw edge
        rect.top += rect.Height() >> 1;
        rect.bottom = rect.top + 1;
        rect.left = rect2.right + nTextOffset;
        pDC->FillSolidRect(rect, crMenuBorder);
    }
    else // not a separator
    {
        //-----------------------------------------------
        // prepare for item drawing
        //-----------------------------------------------

        // determine the item state and calculate offsets
        UINT itemState = lpDIS->itemState;
        BOOL flagStandard = FALSE, flagSelected = FALSE, flagDisabled = FALSE, flagCheck = FALSE;

        int nIconNormal = -1, nOffsetX = -1, nOffsetGlobal = -1;
        int nOffsetFaded = 1, nOffsetShadow = 2, nOffsetDisabled = 3;

        CImageList* pMenuImages = NULL;
        BOOL canDraw3D = FALSE;
        CString strItemText;

        if(pBCMenuData != NULL)
        {
            nIconNormal   = pBCMenuData->menuIconNormal;
            nOffsetX      = pBCMenuData->nOffsetX;
            pMenuImages   = pBCMenuData->bitmap;
            strItemText   = pBCMenuData->GetString();
            nOffsetGlobal = pBCMenuData->nOffsetGlobal;

            if(nOffsetX == 0 && xp_draw_3D_bitmaps && pMenuImages && pMenuImages->GetImageCount() > 2)
            {
                canDraw3D = TRUE;
            }

            if(nIconNormal < 0 && nOffsetX < 0 && nOffsetGlobal >= 0)
            {
                nOffsetX = nOffsetGlobal;
                nIconNormal = 0;
                pMenuImages = &m_AllImages;
                if(xp_draw_3D_bitmaps && CanDraw3DImageList(nOffsetGlobal))
                {
                    canDraw3D = TRUE;
                    nOffsetFaded    = nOffsetGlobal + 1;
                    nOffsetShadow   = nOffsetGlobal + 2;
                    nOffsetDisabled = nOffsetGlobal + 3;
                }
            }

            if(itemState & ODS_CHECKED && nIconNormal < 0)
            {
                if(itemState & ODS_SELECTED && m_selectcheck > 0)
                    flagCheck = TRUE;
                else if(m_unselectcheck > 0)
                    flagCheck = TRUE;
            }
            else if(nIconNormal != -1)
            {
                flagStandard = TRUE;
                if(itemState & ODS_SELECTED && !(itemState & ODS_GRAYED))
                    flagSelected = TRUE;
                else if(itemState & ODS_GRAYED)
                    flagDisabled = TRUE;
            }
        }
        else
        {
            strItemText.Empty();
        }

        //-----------------------------------------------
        // draw menu background
        //-----------------------------------------------

        rect.CopyRect(&lpDIS->rcItem);
        rect2 = rect;

        // draw menu background
        pDC->FillSolidRect(rect, crMenuBackground);

        // draw icon bar background
        rect2.SetRect(rect.left, rect.top, rect.left + nIconBarWidth, rect.bottom);
        pDC->FillSolidRect(rect2, crIconBarBackground);

        // draw icon bar border
        rect2.left = rect2.right - 1;
        pDC->FillSolidRect(rect2, crMenuBorder);

        //-----------------------------------------------
        // draw selection rect
        //-----------------------------------------------

        rect.CopyRect(&lpDIS->rcItem);
        rect2 = rect;

        // is item selected?
        if(itemState & ODS_SELECTED)
        {
            // make selection rect slightly smaller
            rect.DeflateRect(1, 0);

            // round rect radius
            int radius = 3;

            // draw selection background
            CRect baseRect;
            baseRect.CopyRect(&rect);
            baseRect.MoveToXY(0, 0);

            CRgn clipRgn;
            clipRgn.CreateRoundRectRgn(baseRect.left, baseRect.top, baseRect.right, baseRect.bottom, radius, radius);
            pDC->SelectClipRgn(&clipRgn, RGN_COPY);
            pDC->OffsetClipRgn(rect.left, 0);

            TRIVERTEX tv[3];
            tv[0].x = rect.left;
            tv[0].y = rect.top;
            SetTrivertexColorFromColorref(&tv[0], crSelectionBackgroundStart);

            tv[1].x = rect.right;
            tv[1].y = rect.CenterPoint().y;
            SetTrivertexColorFromColorref(&tv[1], crSelectionBackgroundStop);

            tv[2].x = rect.left;
            tv[2].y = rect.bottom;
            SetTrivertexColorFromColorref(&tv[2], crSelectionBackgroundStop);

            GRADIENT_RECT gRect[2];
            gRect[0].UpperLeft  = 0;
            gRect[0].LowerRight = 1;

            gRect[1].UpperLeft  = 1;
            gRect[1].LowerRight = 2;

            pDC->GradientFill(tv, 3, &gRect, 2, GRADIENT_FILL_RECT_V);
            pDC->SelectClipRgn(NULL);
            if(clipRgn.m_hObject) clipRgn.DeleteObject();

            // draw selection border
            CRgn clipRgn2;
            clipRgn2.CreateRoundRectRgn(rect.left, rect.top, rect.right, rect.bottom, radius, radius);

            pDC->FrameRgn(&clipRgn2, &CBrush(crSelectionBorder), 1, 1);
            if(clipRgn2.m_hObject) clipRgn2.DeleteObject();

            // restore original item rect
            rect.InflateRect(1, 0);
        }

        //-----------------------------------------------
        // draw menu bitmap / checkbox
        //-----------------------------------------------

        int dx = (int)(0.5 + (nIconBarWidth - m_iconWidth) / 2.0); if(dx < 0) dx = 0;
        int dy = (int)(0.5 + (rect.Height() - m_iconHeight) / 2.0); if(dy < 0) dy = 0;
        rect2.SetRect(rect.left + 1, rect.top + 1, rect.left + nIconBarWidth - 2, rect.bottom - 1);

        if(flagCheck || flagStandard || flagSelected || flagDisabled)
        {
            if(flagCheck && checkmaps) // checked, checkmaps available
            {
                pDC->FillSolidRect(rect2, crIconBarBackground);
                CPoint ptImage(rect.left + dx, rect.top + dy);
                if(itemState & ODS_SELECTED)
                    checkmaps->Draw(pDC, 1, ptImage, ILD_TRANSPARENT);
                else
                    checkmaps->Draw(pDC, 0, ptImage, ILD_TRANSPARENT);
            }
            else if(flagDisabled) // disabled
            {
                if(!flagSelected) // not selected
                {
                    if(xp_draw_3D_bitmaps_disabled) // grayscale image was provided
                    {
                        // draw image
                        CPoint ptImage(rect.left + dx, rect.top + dy);
                        pMenuImages->Draw(pDC, nOffsetDisabled, ptImage, ILD_TRANSPARENT);
                    }
                    else // create own grayscale image
                    {
                        // get standard image
                        CBitmap bmpStandard;
                        GetBitmapFromImageList(pDC, pMenuImages, nOffsetX, bmpStandard);

                        // get transparent color (i.e. current background color)
                        COLORREF crTransparent = crIconBarBackground;
                        if(itemState & ODS_SELECTED)
                            crTransparent = crSelectionBackgroundStop;

                        // draw disabled image
                        if(m_bOldStyleDisableMode)
                            DitherBlt(lpDIS->hDC, rect.left + dx, rect.top + dy, m_iconWidth, m_iconHeight,
                                      (HBITMAP)bmpStandard, 0, 0, crTransparent);
                        else
                            DitherBlt2(pDC, rect.left + dx, rect.top + dy, m_iconWidth, m_iconHeight,
                                       bmpStandard, 0, 0, crTransparent);

                        // draw rect
                        if(itemState & ODS_SELECTED)
                            pDC->Draw3dRect(rect, crSelectionBorder, crSelectionBorder);

                        // free resources
                        bmpStandard.DeleteObject();
                    }
                }
            }
            else if(flagSelected) // selected
            {
                CPoint ptImage(rect.left + dx, rect.top + dy);

                // draw background
                if(itemState & ODS_CHECKED) // checked
                {
                    // draw background
                    pDC->FillSolidRect(rect2, LightenColor(crSelectionBorder, 0.55));

                    // draw rectangle
                    pDC->Draw3dRect(rect2, crSelectionBorder, crSelectionBorder);

                    // add offset to point
                    ptImage.x -= 1; ptImage.y -= 1;
                }

                // draw menu image
                if(pMenuImages)
                {
                    if(canDraw3D && !(itemState & ODS_CHECKED))
                    {
                        CPoint ptImage1(ptImage.x + 1, ptImage.y + 1);
                        CPoint ptImage2(ptImage.x - 1, ptImage.y - 1);
                        pMenuImages->Draw(pDC, nOffsetShadow, ptImage1, ILD_TRANSPARENT);
                        pMenuImages->Draw(pDC, nOffsetX, ptImage2, ILD_TRANSPARENT);
                    }
                    else
                    {
                        pMenuImages->Draw(pDC, nOffsetX, ptImage, ILD_TRANSPARENT);
                    }
                }
            }
            else // standard
            {
                if(itemState & ODS_CHECKED) // checked
                {
                    // draw background
                    pDC->FillSolidRect(rect2, LightenColor(crSelectionBorder, 0.85));

                    // draw rectangle
                    pDC->Draw3dRect(rect2, crSelectionBorder, crSelectionBorder);

                    // draw image
                    CPoint ptImage(rect.left + dx - 1, rect.top + dy - 1);
                    if(pMenuImages)
                        pMenuImages->Draw(pDC, nOffsetX, ptImage, ILD_TRANSPARENT);
                }
                else // not checked
                {
                    // draw background
                    pDC->FillSolidRect(rect2, crIconBarBackground);

                    // draw image
                    CPoint ptImage(rect.left + dx, rect.top + dy);
                    if(pMenuImages)
                    {
                        if(canDraw3D)
                            pMenuImages->Draw(pDC, nOffsetFaded, ptImage, ILD_TRANSPARENT);
                        else
                            pMenuImages->Draw(pDC, nOffsetX, ptImage, ILD_TRANSPARENT);
                    }
                }
            }
        }

        // draw checkbox (if necessary)
        if(nIconNormal < 0 && itemState & ODS_CHECKED && !flagCheck)
        {
            CMenuItemInfo info;
            info.fMask = MIIM_CHECKMARKS;
            ::GetMenuItemInfo((HMENU)lpDIS->hwndItem, lpDIS->itemID, MF_BYCOMMAND, &info);
            if(itemState & ODS_CHECKED || info.hbmpUnchecked)
            {
                DrawXPCheckmark(pDC, rect2, (itemState & ODS_CHECKED) ? info.hbmpChecked : info.hbmpUnchecked,
                    crSelectionBorder, itemState & ODS_SELECTED);
            }
        }

        //-----------------------------------------------
        // draw text
        //-----------------------------------------------

        // remember old position
        int x0 = rect.left;
        int y0 = rect.top;

        // set menu text offset
        rect.left = rect.left + nIconBarWidth + nTextOffset;

        if(!strItemText.IsEmpty()) // item text available
        {
            // set text rect
            CRect rectText(rect.left, rect.top - 1, rect.right, rect.bottom - 1);

            // find tabs in item text
            CString strItemTextDesc, strItemTextAccel;
            int tabPosition = strItemText.ReverseFind(_T('\t'));
            if(tabPosition != -1) // tab found
            {
                strItemTextAccel = strItemText.Mid(tabPosition + 1);
                strItemTextDesc = strItemText.Left(strItemText.Find(_T('\t')));
                rectText.right -= m_iconWidth; // accelerators have rather big right margin
            }
            else // no tabs
            {
                strItemTextDesc = strItemText;
            }

            // prepare DC
            int oldBkMode = pDC->SetBkMode(TRANSPARENT);

            // draw text in correct color
            UINT nFormatDesc  = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
            UINT nFormatAccel = DT_RIGHT | DT_SINGLELINE | DT_VCENTER;
            if(!(lpDIS->itemState & ODS_GRAYED)) // not grayed
            {
                // set text color
                if(IsThemed()) pDC->SetTextColor(crText);
                else pDC->SetTextColor(flagSelected ? crTextSelected : crText);

                // draw text
                pDC->DrawText(strItemTextDesc, rectText, nFormatDesc);
                if(tabPosition != -1) pDC->DrawText(strItemTextAccel, rectText, nFormatAccel);
            }
            else // grayed
            {
                RECT offset = *rectText;
                offset.left   += 1;
                offset.right  += 1;
                offset.top    += 1;
                offset.bottom += 1;

                if(IsThemed())
                {
                    pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
                }
                else // not themed
                {
                    COLORREF graycol = GetSysColor(COLOR_GRAYTEXT);
                    if(!(itemState & ODS_SELECTED)) graycol = LightenColor(graycol, 0.4);
                    pDC->SetTextColor(graycol);
                }

                pDC->DrawText(strItemTextDesc, rectText, nFormatDesc);
                if(tabPosition != -1) pDC->DrawText(strItemTextAccel, rectText, nFormatAccel);
            }

            pDC->SetBkMode(oldBkMode);
        }
    }

#ifdef BCMENU_USE_MEMDC
    if(pOldFont) pDC->SelectObject(pOldFont);
    if(menuFont.m_hObject) menuFont.DeleteObject();
    if(pMemDC) delete pMemDC;
#endif
}

BOOL BCMenu::GetBitmapFromImageList(CDC* pDC, CImageList* pImageList, int nIndex, CBitmap& bmp)
{
    HICON hIcon = pImageList->ExtractIcon(nIndex);

    CDC dc;
    dc.CreateCompatibleDC(pDC);
    bmp.CreateCompatibleBitmap(pDC, m_iconWidth, m_iconHeight);
    CBitmap* pOldBmp = dc.SelectObject(&bmp);

    COLORREF bgColor = GetSysColor(COLOR_3DFACE);
    CBrush brush;
    brush.CreateSolidBrush(bgColor);

    ::DrawIconEx(
        dc.GetSafeHdc(),
        0,
        0,
        hIcon,
        m_iconWidth,
        m_iconHeight,
        0,
        (HBRUSH)brush,
        DI_NORMAL
        );

    // free resources
    dc.SelectObject(pOldBmp);
    dc.DeleteDC();
    ::DestroyIcon(hIcon);

    return TRUE;
}

/*
==========================================================================
void BCMenu::MeasureItem(LPMEASUREITEMSTRUCT)
---------------------------------------------

  Called by the framework when it wants to know what the width and height
  of our item will be.  To accomplish this we provide the width of the
  icon plus the width of the menu text, and then the height of the icon.

==========================================================================
*/

void BCMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
    UINT nFlags = ((BCMenuData*)lpMIS->itemData)->nFlags;

    // get menu metrics
    int nIconBarPaddingVert = GetMenuMetrics(BCMMET_ICONBARPADDING_VERT);
    int nIconBarPaddingHorz = GetMenuMetrics(BCMMET_ICONBARPADDING_HORZ);
    int nTextOffset = GetMenuMetrics(BCMMET_TEXTOFFSET);

    // is item a separator?
    if(nFlags & MF_SEPARATOR)
    {
        // set width
        lpMIS->itemWidth = 0;

        // set height
        int nDefaultHeight = GetSystemMetrics(SM_CYMENU) >> 1;
        if(IsMenuThemeActive())
            lpMIS->itemHeight = 3;
        else
            lpMIS->itemHeight = (nDefaultHeight > (m_iconHeight + nIconBarPaddingVert) / 2) ? nDefaultHeight : (m_iconHeight + nIconBarPaddingVert) / 2;
    }
    else // not a separator
    {
        // get font
        CFont menuFont;
        LOGFONT lf;
        ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
        NONCLIENTMETRICS nm;
        nm.cbSize = sizeof(NONCLIENTMETRICS);
        VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0));
        lf = nm.lfMenuFont;

        // is it the default item?
        if(GetDefaultItem(GMDI_USEDISABLED, FALSE) == lpMIS->itemID) // it's the default item
        {
            // use bold font for width calculation
            lf.lfWeight = FW_BOLD;
        }

        // create font
        menuFont.CreateFontIndirect(&lf);

        // select font into DC
        CWnd* pWnd = AfxGetMainWnd();
        if(pWnd == NULL) pWnd = CWnd::GetDesktopWindow();
        CDC* pDC = pWnd->GetDC();
        CFont* pOldFont = NULL;

        if(IsNewShell()) // >= Win95
            pOldFont = pDC->SelectObject(&menuFont);

        // get item text
        const wchar_t* lpItemText = ((BCMenuData*)lpMIS->itemData)->GetWideString();

        // calculate the size of the text
        SIZE sizeText;
        sizeText.cx = 0;
        sizeText.cy = 0;

        if(g_Shell != Win32s)
        {
            VERIFY(::GetTextExtentPoint32W(pDC->m_hDC, lpItemText, (int)wcslen(lpItemText), &sizeText));
        }
#ifndef UNICODE // can't be UNICODE for Win32s
        else // Win32s
        {
            RECT rect;
            rect.left = 0;
            rect.top = 0;

            sizeText.cy = DrawText(pDC->m_hDC, (LPCTSTR)lpItemText, (int)wcslen(lpItemText), &rect,
                DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);

            // get text size
            sizeText.cx  = rect.right - rect.left + 3; // add some extra space for the menu border
            sizeText.cx += 3 * (sizeText.cx / (int)wcslen(lpItemText));
        }
#endif

        // free resources
        if(IsNewShell()) pDC->SelectObject(pOldFont);
        if(menuFont.m_hObject) menuFont.DeleteObject();
        pWnd->ReleaseDC(pDC);

        // set item width and height
        if(IsMenuThemeActive())
            lpMIS->itemWidth = m_iconWidth + nIconBarPaddingHorz + nTextOffset + sizeText.cx;
        else // classic style
            lpMIS->itemWidth = m_iconWidth + sizeText.cx + m_iconWidth + BCMENU_GAP;

        int nDefaultHeight = GetSystemMetrics(SM_CYMENU);
        lpMIS->itemHeight = (nDefaultHeight > m_iconHeight + nIconBarPaddingVert) ? nDefaultHeight : m_iconHeight + nIconBarPaddingVert;
    }
}

void BCMenu::SetIconSize(int width, int height)
{
    m_iconWidth = width;
    m_iconHeight = height;
}

BOOL BCMenu::AppendODMenuA(LPCSTR lpstrText, UINT nFlags, UINT nID, int nIconNormal)
{
    USES_CONVERSION;
    return AppendODMenuW(A2W(lpstrText), nFlags, nID, nIconNormal); //SK: See MFC Tech Note 059
}


BOOL BCMenu::AppendODMenuW(wchar_t* lpstrText, UINT nFlags, UINT nID, int nIconNormal)
{
    // Add the MF_OWNERDRAW flag if not specified
    if(!nID)
    {
        if(nFlags & MF_BYPOSITION)
            nFlags = MF_SEPARATOR | MF_OWNERDRAW | MF_BYPOSITION;
        else
            nFlags = MF_SEPARATOR | MF_OWNERDRAW;
    }
    else if(!(nFlags & MF_OWNERDRAW))
    {
        nFlags |= MF_OWNERDRAW;
    }

    if(nFlags & MF_POPUP)
    {
        m_AllSubMenus.Add((HMENU)(UINT_PTR)nID);
        m_SubMenus.Add((HMENU)(UINT_PTR)nID);
    }

    BCMenuData* mdata = new BCMenuData();
    m_MenuList.Add(mdata);
    mdata->SetWideString(lpstrText); // SK: modified for dynamic allocation

    mdata->menuIconNormal = -1;
    mdata->nOffsetX = -1;

    if(nIconNormal >= 0)
    {
        CImageList bitmap;
        int nOffsetX = 0;
        LoadFromToolBar(nID, nIconNormal, nOffsetX);
        if(mdata->bitmap)
        {
            mdata->bitmap->DeleteImageList();
            mdata->bitmap = NULL;
        }
        bitmap.Create(m_iconWidth, m_iconHeight, ILC_COLORDDB | ILC_MASK, 1, 1);
        if(AddBitmapToImageList(&bitmap, nIconNormal))
        {
            mdata->nOffsetGlobal = AddToGlobalImageList(&bitmap, nOffsetX, nID);
        }
    }
    else
    {
        mdata->nOffsetGlobal = GlobalImageListOffset(nID);
    }

    mdata->nFlags = nFlags;
    mdata->nID = nID;
    BOOL returnflag = CMenu::AppendMenu(nFlags, nID, (LPCTSTR)mdata);

    if(m_loadmenu)
        RemoveTopLevelOwnerDraw();

    return returnflag;
}

BOOL BCMenu::AppendODMenuA(LPCSTR lpstrText, UINT nFlags, UINT nID, CImageList* il, int nOffsetX)
{
    USES_CONVERSION;
    return AppendODMenuW(A2W(lpstrText), nFlags, nID, il, nOffsetX);
}

BOOL BCMenu::AppendODMenuW(wchar_t* lpstrText, UINT nFlags, UINT nID, CImageList* il, int nOffsetX)
{
    // Add the MF_OWNERDRAW flag if not specified
    if(!nID)
    {
        if(nFlags & MF_BYPOSITION)
            nFlags = MF_SEPARATOR | MF_OWNERDRAW | MF_BYPOSITION;
        else
            nFlags = MF_SEPARATOR | MF_OWNERDRAW;
    }
    else if(!(nFlags & MF_OWNERDRAW))
    {
        nFlags |= MF_OWNERDRAW;
    }

    if(nFlags & MF_POPUP)
    {
        m_AllSubMenus.Add((HMENU)(UINT_PTR)nID);
        m_SubMenus.Add((HMENU)(UINT_PTR)nID);
    }

    BCMenuData* mdata = new BCMenuData();
    m_MenuList.Add(mdata);
    mdata->SetWideString(lpstrText); // SK: modified for dynamic allocation

    if(il)
    {
        mdata->menuIconNormal = 0;
        mdata->nOffsetX = 0;
        if(mdata->bitmap)
            mdata->bitmap->DeleteImageList();
        else
            mdata->bitmap = new(CImageList);

        ImageListDuplicate(il, nOffsetX, mdata->bitmap);
    }
    else
    {
        mdata->menuIconNormal = -1;
        mdata->nOffsetX = -1;
    }

    mdata->nFlags = nFlags;
    mdata->nID = nID;
    return(CMenu::AppendMenu(nFlags, nID, (LPCTSTR)mdata));
}

BOOL BCMenu::InsertODMenuA(UINT nPosition,LPCSTR lpstrText,UINT nFlags,UINT nID,
                           int nIconNormal)
{
    USES_CONVERSION;
    return InsertODMenuW(nPosition,A2W(lpstrText),nFlags,nID,nIconNormal);
}


BOOL BCMenu::InsertODMenuW(UINT nPosition,wchar_t *lpstrText,UINT nFlags,UINT nID,
                           int nIconNormal)
{
    if(!(nFlags & MF_BYPOSITION)){
        int iPosition =0;
        BCMenu* pMenu = FindMenuOption(nPosition,iPosition);
        if(pMenu){
            return(pMenu->InsertODMenuW(iPosition,lpstrText,nFlags|MF_BYPOSITION,nID,nIconNormal));
        }
        else return(FALSE);
    }
    
    if(!nID)nFlags=MF_SEPARATOR|MF_OWNERDRAW|MF_BYPOSITION;
    else if(!(nFlags & MF_OWNERDRAW))nFlags |= MF_OWNERDRAW;

    int menustart=0;

    if(nFlags & MF_POPUP){
        if(m_loadmenu){
            menustart=GetMenuStart();
            if(nPosition<(UINT)menustart)menustart=0;
        }
        m_AllSubMenus.Add((HMENU)(UINT_PTR)nID);
        m_SubMenus.Add((HMENU)(UINT_PTR)nID);
    }

    //Stephane Clog suggested adding this, believe it or not it's in the help 
    if(nPosition==(UINT)-1)nPosition=GetMenuItemCount();
    
    BCMenuData *mdata = new BCMenuData;
    m_MenuList.InsertAt(nPosition-menustart,mdata);
    mdata->SetWideString(lpstrText);    //SK: modified for dynamic allocation
    
    mdata->menuIconNormal = nIconNormal;
    mdata->nOffsetX=-1;
    if(nIconNormal>=0){
        CImageList bitmap;
        int nOffsetX=0;
        LoadFromToolBar(nID,nIconNormal,nOffsetX);
        if(mdata->bitmap){
            mdata->bitmap->DeleteImageList();
            mdata->bitmap=NULL;
        }
        bitmap.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
        if(AddBitmapToImageList(&bitmap,nIconNormal)){
            mdata->nOffsetGlobal = AddToGlobalImageList(&bitmap,nOffsetX,nID);
        }
    }
    else mdata->nOffsetGlobal = GlobalImageListOffset(nID);
    mdata->nFlags = nFlags;
    mdata->nID = nID;
    BOOL returnflag=CMenu::InsertMenu(nPosition,nFlags,nID,(LPCTSTR)mdata);
    if(m_loadmenu)RemoveTopLevelOwnerDraw();
    return(returnflag);
}

BOOL BCMenu::InsertODMenuA(UINT nPosition,LPCSTR lpstrText,UINT nFlags,UINT nID,
                           CImageList *il,int nOffsetX)
{
    USES_CONVERSION;
    return InsertODMenuW(nPosition,A2W(lpstrText),nFlags,nID,il,nOffsetX);
}

BOOL BCMenu::InsertODMenuW(UINT nPosition,wchar_t *lpstrText,UINT nFlags,UINT nID,
                           CImageList *il,int nOffsetX)
{
    if(!(nFlags & MF_BYPOSITION)){
        int iPosition =0;
        BCMenu* pMenu = FindMenuOption(nPosition,iPosition);
        if(pMenu){
            return(pMenu->InsertODMenuW(iPosition,lpstrText,nFlags|MF_BYPOSITION,nID,il,nOffsetX));
        }
        else return(FALSE);
    }
    
    if(!nID)nFlags=MF_SEPARATOR|MF_OWNERDRAW|MF_BYPOSITION;
    else if(!(nFlags & MF_OWNERDRAW))nFlags |= MF_OWNERDRAW;
    
    if(nFlags & MF_POPUP){
        m_AllSubMenus.Add((HMENU)(UINT_PTR)nID);
        m_SubMenus.Add((HMENU)(UINT_PTR)nID);
    }
    
    //Stephane Clog suggested adding this, believe it or not it's in the help 
    if(nPosition==(UINT)-1)nPosition=GetMenuItemCount();
    
    BCMenuData *mdata = new BCMenuData;
    m_MenuList.InsertAt(nPosition,mdata);
    mdata->SetWideString(lpstrText);    //SK: modified for dynamic allocation
    
    mdata->menuIconNormal = -1;
    mdata->nOffsetX = -1;

    if(il){
        if(mdata->bitmap){
            mdata->bitmap->DeleteImageList();
            mdata->bitmap=NULL;
        }
        mdata->nOffsetGlobal = AddToGlobalImageList(il,nOffsetX,nID);
    }
    mdata->nFlags = nFlags;
    mdata->nID = nID;
    return(CMenu::InsertMenu(nPosition,nFlags,nID,(LPCTSTR)mdata));
}

BOOL BCMenu::ModifyODMenuA(const char * lpstrText,UINT nID,int nIconNormal)
{
    USES_CONVERSION;
    return ModifyODMenuW(A2W(lpstrText),nID,nIconNormal);//SK: see MFC Tech Note 059
}

BOOL BCMenu::ModifyODMenuW(wchar_t *lpstrText,UINT nID,int nIconNormal)
{
    int nLoc;
    BCMenuData *mdata;
    CArray<BCMenu*,BCMenu*>bcsubs;
    CArray<int,int&>bclocs;
    
    // Find the old BCMenuData structure:
    BCMenu *psubmenu = FindMenuOption(nID,nLoc);
    do{
        if(psubmenu && nLoc>=0)mdata = psubmenu->m_MenuList[nLoc];
        else{
            // Create a new BCMenuData structure:
            mdata = new BCMenuData;
            m_MenuList.Add(mdata);
        }
        
        ASSERT(mdata);
        if(lpstrText)
            mdata->SetWideString(lpstrText);  //SK: modified for dynamic allocation
        mdata->menuIconNormal = -1;
        mdata->nOffsetX = -1;
        if(nIconNormal>=0){
            CImageList bitmap;
            int nOffsetX=0;
            LoadFromToolBar(nID,nIconNormal,nOffsetX);
            if(mdata->bitmap){
                mdata->bitmap->DeleteImageList();
                mdata->bitmap=NULL;
            }
            bitmap.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
            if(AddBitmapToImageList(&bitmap,nIconNormal)){
                mdata->nOffsetGlobal = AddToGlobalImageList(&bitmap,nOffsetX,nID);
            }
        }
        else mdata->nOffsetGlobal = GlobalImageListOffset(nID);
        mdata->nFlags &= ~(MF_BYPOSITION);
        mdata->nFlags |= MF_OWNERDRAW;
        mdata->nID = nID;
        bcsubs.Add(psubmenu);
        bclocs.Add(nLoc);
        if(psubmenu && nLoc>=0)psubmenu = FindAnotherMenuOption(nID,nLoc,bcsubs,bclocs);
        else psubmenu=NULL;
    }while(psubmenu);
    return (CMenu::ModifyMenu(nID,mdata->nFlags,nID,(LPCTSTR)mdata));
}

BOOL BCMenu::ModifyODMenuA(const char * lpstrText,UINT nID,CImageList *il,int nOffsetX)
{
    USES_CONVERSION;
    return ModifyODMenuW(A2W(lpstrText),nID,il,nOffsetX);
}

BOOL BCMenu::ModifyODMenuW(wchar_t *lpstrText,UINT nID,CImageList *il,int nOffsetX)
{
    int nLoc;
    BCMenuData *mdata;
    CArray<BCMenu*,BCMenu*>bcsubs;
    CArray<int,int&>bclocs;
    
    // Find the old BCMenuData structure:
    BCMenu *psubmenu = FindMenuOption(nID,nLoc);
    do{
        if(psubmenu && nLoc>=0)mdata = psubmenu->m_MenuList[nLoc];
        else{
            // Create a new BCMenuData structure:
            mdata = new BCMenuData;
            m_MenuList.Add(mdata);
        }
        
        ASSERT(mdata);
        if(lpstrText)
            mdata->SetWideString(lpstrText);  //SK: modified for dynamic allocation
        mdata->menuIconNormal = -1;
        mdata->nOffsetX = -1;
        if(il){
            if(mdata->bitmap){
                mdata->bitmap->DeleteImageList();
                mdata->bitmap=NULL;
            }
            mdata->nOffsetGlobal = AddToGlobalImageList(il,nOffsetX,nID);
        }
        mdata->nFlags &= ~(MF_BYPOSITION);
        mdata->nFlags |= MF_OWNERDRAW;
        mdata->nID = nID;
        bcsubs.Add(psubmenu);
        bclocs.Add(nLoc);
        if(psubmenu && nLoc>=0)psubmenu = FindAnotherMenuOption(nID,nLoc,bcsubs,bclocs);
        else psubmenu=NULL;
    }while(psubmenu);
    return (CMenu::ModifyMenu(nID,mdata->nFlags,nID,(LPCTSTR)mdata));
}

BOOL BCMenu::ModifyODMenuA(const char * lpstrText,UINT nID,CBitmap *bmp)
{
    USES_CONVERSION;
    return ModifyODMenuW(A2W(lpstrText),nID,bmp);
}

BOOL BCMenu::ModifyODMenuW(wchar_t *lpstrText,UINT nID,CBitmap *bmp)
{
    if(bmp){
        CImageList temp;
        temp.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
        if(m_bitmapBackgroundFlag)temp.Add(bmp,m_bitmapBackground);
        else temp.Add(bmp, GetSysColor(COLOR_3DFACE));
        return ModifyODMenuW(lpstrText,nID,&temp,0);
    }
    return ModifyODMenuW(lpstrText,nID,NULL,0);
}

// courtesy of Warren Stevens
BOOL BCMenu::ModifyODMenuA(const char * lpstrText,UINT nID,COLORREF fill,COLORREF border,int hatchstyle,CSize *pSize)
{
    USES_CONVERSION;
    return ModifyODMenuW(A2W(lpstrText),nID,fill,border,hatchstyle,pSize);
}

BOOL BCMenu::ModifyODMenuW(wchar_t *lpstrText,UINT nID,COLORREF fill,COLORREF border,int hatchstyle,CSize *pSize)
{
    CWnd *pWnd = AfxGetMainWnd();
    CDC *pDC = pWnd->GetDC();
    SIZE sz;
    if(!pSize){
        sz.cx = m_iconWidth;
        sz.cy = m_iconHeight;
    }
    else{
        sz.cx = pSize->cx;
        sz.cy = pSize->cy;
    }
    CSize bitmap_size(sz);
    CSize icon_size(m_iconWidth,m_iconHeight);
    CBitmap bmp;
    ColorBitmap(pDC,bmp,bitmap_size,icon_size,fill,border,hatchstyle);		
    pWnd->ReleaseDC(pDC);
    return ModifyODMenuW(lpstrText,nID,&bmp);
}


BOOL BCMenu::ModifyODMenuA(const char *lpstrText,const char *OptionText,
                           int nIconNormal)
{
    USES_CONVERSION;
    return ModifyODMenuW(A2W(lpstrText),A2W(OptionText),nIconNormal);//SK: see MFC  Tech Note 059
}

BOOL BCMenu::ModifyODMenuW(wchar_t *lpstrText,wchar_t *OptionText,
                           int nIconNormal)
{
    BCMenuData *mdata;
    
    // Find the old BCMenuData structure:
    mdata=FindMenuOption(OptionText);
    if(mdata){
        if(lpstrText)
            mdata->SetWideString(lpstrText);//SK: modified for dynamic allocation
        mdata->menuIconNormal = nIconNormal;
        mdata->nOffsetX=-1;
        if(nIconNormal>=0){
            mdata->nOffsetX=0;
            if(mdata->bitmap)mdata->bitmap->DeleteImageList();
            else mdata->bitmap=new(CImageList);
            mdata->bitmap->Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
            if(!AddBitmapToImageList(mdata->bitmap,nIconNormal)){
                mdata->bitmap->DeleteImageList();
                delete mdata->bitmap;
                mdata->bitmap=NULL;
                mdata->menuIconNormal = nIconNormal = -1;
                mdata->nOffsetX = -1;
            }
        }
        return(TRUE);
    }
    return(FALSE);
}

BOOL BCMenu::SetImageForPopupFromToolbarA (const char *strPopUpText, UINT toolbarID, UINT command_id_to_extract_icon_from)
{
    USES_CONVERSION;
    return SetImageForPopupFromToolbarW(A2W(strPopUpText),toolbarID,command_id_to_extract_icon_from);
}
BOOL BCMenu::SetImageForPopupFromToolbarW (wchar_t *strPopUpText, UINT toolbarID, UINT command_id_to_extract_icon_from)
{
    CWnd* pWnd = AfxGetMainWnd();
    if (pWnd == NULL)pWnd = CWnd::GetDesktopWindow();

    CToolBar bar;
    bar.Create(pWnd);

    if(bar.LoadToolBar(toolbarID)){
        BCMenuData *mdata = FindMenuOption(strPopUpText);
        if (mdata != NULL)
        {
            if (mdata->bitmap != NULL){
                mdata->bitmap->DeleteImageList();
                delete mdata->bitmap;
                mdata->bitmap=NULL;
            }
            CImageList imglist;
            imglist.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);

            if(AddBitmapToImageList (&imglist, toolbarID)){
                int ind = bar.CommandToIndex (command_id_to_extract_icon_from);
                if (ind < 0) { return FALSE; }
                
                UINT dummyID, dummyStyle;
                int image_index;
                bar.GetButtonInfo (ind, dummyID, dummyStyle, image_index);
                ASSERT (dummyID == command_id_to_extract_icon_from);
                
                mdata->bitmap = new CImageList;
                mdata->bitmap->Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,0,1);
                mdata->bitmap->Add (imglist.ExtractIcon (image_index));

                mdata->menuIconNormal = toolbarID;
                mdata->nOffsetX = 0;
                
                return TRUE;
            }
            else{
                mdata->menuIconNormal = -1;
                mdata->nOffsetX = -1;
            }
        }
    }

    return FALSE;
}


BCMenuData *BCMenu::NewODMenu(UINT pos,UINT nFlags,UINT nID,CString string)
{
    BCMenuData *mdata;
    
    mdata = new BCMenuData;
    mdata->menuIconNormal = -1;
    mdata->nOffsetX=-1;
#ifdef UNICODE
    mdata->SetWideString((LPCTSTR)string);//SK: modified for dynamic allocation
#else
    mdata->SetAnsiString(string);
#endif
    mdata->nFlags = nFlags;
    mdata->nID = nID;
    
//	if(nFlags & MF_POPUP)m_AllSubMenus.Add((HMENU)(UINT_PTR)nID);
        
    if (nFlags&MF_OWNERDRAW){
        ASSERT(!(nFlags&MF_STRING));
        ModifyMenu(pos,nFlags,nID,(LPCTSTR)mdata);
    }
    else if (nFlags&MF_STRING){
        ASSERT(!(nFlags&MF_OWNERDRAW));
        ModifyMenu(pos,nFlags,nID,mdata->GetString());
    }
    else{
        ASSERT(nFlags&MF_SEPARATOR);
        ModifyMenu(pos,nFlags,nID);
    }
    
    return(mdata);
};

BOOL BCMenu::LoadToolbars(const UINT *arID,int n)
{
    ASSERT(arID);
    BOOL returnflag=TRUE;
    for(int i=0;i<n;++i){
        if(!LoadToolbar(arID[i]))returnflag=FALSE;
    }
    return(returnflag);
}

BOOL BCMenu::LoadToolbar(UINT nToolBar)
{
    UINT nID,nStyle;
    BOOL returnflag=FALSE;
    CToolBar bar;
    int nOffsetX=-1,xset;
    
    CWnd* pWnd = AfxGetMainWnd();
    if (pWnd == NULL)pWnd = CWnd::GetDesktopWindow();
    bar.Create(pWnd);
    if(bar.LoadToolBar(nToolBar)){
        CImageList imglist;
        imglist.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
        if(AddBitmapToImageList(&imglist,nToolBar)){
            returnflag=TRUE;
            for(int i=0;i<bar.GetCount();++i){
                nID = bar.GetItemID(i); 
                if(nID && GetMenuState(nID, MF_BYCOMMAND)
                    !=0xFFFFFFFF){
                    nOffsetX=bar.CommandToIndex(nID);
                    if(nOffsetX>=0){
                        bar.GetButtonInfo(nOffsetX,nID,nStyle,xset);
                        if(xset>0)nOffsetX=xset;
                    }
                    ModifyODMenu(NULL,nID,&imglist,nOffsetX);
                }
            }
        }
    }
    return(returnflag);
}

BOOL BCMenu::LoadFromToolBar(UINT nID,UINT nToolBar,int& nOffsetX)
{
    int xset,offset;
    UINT nStyle;
    BOOL returnflag=FALSE;
    CToolBar bar;
    
    CWnd* pWnd = AfxGetMainWnd();
    if (pWnd == NULL)pWnd = CWnd::GetDesktopWindow();
    bar.Create(pWnd);
    if(bar.LoadToolBar(nToolBar)){
        offset=bar.CommandToIndex(nID);
        if(offset>=0){
            bar.GetButtonInfo(offset,nID,nStyle,xset);
            if(xset>0)nOffsetX=xset;
            returnflag=TRUE;
        }
    }
    return(returnflag);
}

// O.S.
BCMenuData *BCMenu::FindMenuItem(UINT nID)
{
    BCMenuData *pData = NULL;
    int i;
    
    for(i = 0; i <= m_MenuList.GetUpperBound(); i++){
        if (m_MenuList[i]->nID == nID){
            pData = m_MenuList[i];
            break;
        }
    }
    if (!pData){
        int loc;
        BCMenu *pMenu = FindMenuOption(nID, loc);
        ASSERT(pMenu != this);
        if (loc >= 0){
            return pMenu->FindMenuItem(nID);
        }
    }
    return pData;
}


BCMenu *BCMenu::FindAnotherMenuOption(int nId,int& nLoc,CArray<BCMenu*,BCMenu*>&bcsubs,
                                      CArray<int,int&>&bclocs)
{
    int i,numsubs,j;
    BCMenu *psubmenu,*pgoodmenu;
    BOOL foundflag;
    
    for(i=0;i<(int)(GetMenuItemCount());++i){
#ifdef _CPPRTTI 
        psubmenu=dynamic_cast<BCMenu *>(GetSubMenu(i));
#else
        psubmenu=(BCMenu *)GetSubMenu(i);
#endif
        if(psubmenu){
            pgoodmenu=psubmenu->FindAnotherMenuOption(nId,nLoc,bcsubs,bclocs);
            if(pgoodmenu)return(pgoodmenu);
        }
        else if(nId == (int)GetMenuItemID(i)){
            numsubs = (int)bcsubs.GetSize();
            foundflag = TRUE;
            for(j=0;j<numsubs;++j){
                if(bcsubs[j]==this&&bclocs[j]==i){
                    foundflag=FALSE;
                    break;
                }
            }
            if(foundflag){
                nLoc=i;
                return(this);
            }
        }
    }
    nLoc = -1;
    return(NULL);
}

BCMenu *BCMenu::FindMenuOption(int nId,int& nLoc)
{
    int i;
    BCMenu *psubmenu,*pgoodmenu;
    
    for(i=0;i<(int)(GetMenuItemCount());++i){
#ifdef _CPPRTTI 
        psubmenu=dynamic_cast<BCMenu *>(GetSubMenu(i));
#else
        psubmenu=(BCMenu *)GetSubMenu(i);
#endif
        if(psubmenu){
            pgoodmenu=psubmenu->FindMenuOption(nId,nLoc);
            if(pgoodmenu)return(pgoodmenu);
        }
        else if(nId==(int)GetMenuItemID(i)){
            nLoc=i;
            return(this);
        }
    }
    nLoc = -1;
    return(NULL);
}

BCMenuData *BCMenu::FindMenuOption(wchar_t *lpstrText)
{
    int i,j;
    BCMenu *psubmenu;
    BCMenuData *pmenulist;
    
    for(i=0;i<(int)(GetMenuItemCount());++i){
#ifdef _CPPRTTI 
        psubmenu=dynamic_cast<BCMenu *>(GetSubMenu(i));
#else
        psubmenu=(BCMenu *)GetSubMenu(i);
#endif
        if(psubmenu){
            pmenulist=psubmenu->FindMenuOption(lpstrText);
            if(pmenulist)return(pmenulist);
        }
        else{
            const wchar_t *szWide;//SK: we use const to prevent misuse of this Ptr
            for(j=0;j<=m_MenuList.GetUpperBound();++j){     
                szWide = m_MenuList[j]->GetWideString ();
                if(szWide && !wcscmp(lpstrText,szWide))//SK: modified for dynamic allocation
                    return(m_MenuList[j]);
            }
        }
    }
    return(NULL);
}


BOOL BCMenu::LoadMenu(int nResource)
{
    return(BCMenu::LoadMenu(MAKEINTRESOURCE(nResource)));
};

BOOL BCMenu::LoadMenu(LPCTSTR lpszResourceName)
{
    ASSERT_VALID(this);
    ASSERT(lpszResourceName != NULL);
    
    // Find the Menu Resource:
    HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName,RT_MENU);
    HRSRC hRsrc = ::FindResource(hInst,lpszResourceName,RT_MENU);
    if (hRsrc == NULL){
        hInst = NULL;
        hRsrc = ::FindResource(hInst,lpszResourceName,RT_MENU);
    }
    if(hRsrc == NULL)return FALSE;

    // Load the Menu Resource:

    HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
    if(hGlobal == NULL)return FALSE;

    // first destroy the menu if we're trying to loadmenu again
    DestroyMenu();

    // Attempt to create us as a menu...
    if(!CMenu::CreateMenu())return FALSE;

    // Get Item template Header, and calculate offset of MENUITEMTEMPLATES

    MENUITEMTEMPLATEHEADER *pTpHdr=
        (MENUITEMTEMPLATEHEADER*)LockResource(hGlobal);
    BYTE* pTp=(BYTE*)pTpHdr + 
        (sizeof(MENUITEMTEMPLATEHEADER) + pTpHdr->offset);


    // Variables needed during processing of Menu Item Templates:

    int j=0;
    WORD    dwFlags = 0;              // Flags of the Menu Item
    WORD    dwID  = 0;              // ID of the Menu Item
    UINT    uFlags;                  // Actual Flags.
    wchar_t *szCaption=NULL;
    int      nLen   = 0;                // Length of caption
    CTypedPtrArray<CPtrArray, BCMenu*>  m_Stack;    // Popup menu stack
    CArray<BOOL,BOOL>  m_StackEnd;    // Popup menu stack
    m_Stack.Add(this);                  // Add it to this...
    m_StackEnd.Add(FALSE);

    do{
        // Obtain Flags and (if necessary), the ID...
        memcpy(&dwFlags, pTp, sizeof(WORD));pTp+=sizeof(WORD);// Obtain Flags
        if(!(dwFlags & MF_POPUP)){
            memcpy(&dwID, pTp, sizeof(WORD)); // Obtain ID
            pTp+=sizeof(WORD);
        }
        else dwID = 0;

        uFlags = (UINT)dwFlags; // Remove MF_END from the flags that will
        if(uFlags & MF_END) // be passed to the Append(OD)Menu functions.
            uFlags -= MF_END;

        // Obtain Caption (and length)

        nLen = 0;
        size_t nChars = wcslen((wchar_t *)pTp)+1;
        szCaption = new wchar_t[nChars];

#if _MSC_VER >= 1400  // VS2005
        wcsncpy_s(szCaption, nChars, (wchar_t *)pTp, _TRUNCATE);
#else
        wcscpy(szCaption,(wchar_t *)pTp);
#endif

        pTp=&pTp[(wcslen((wchar_t *)pTp)+1)*sizeof(wchar_t)];//modified SK

        // Handle popup menus first....

        //WideCharToMultiByte
        if(dwFlags & MF_POPUP){
            if(dwFlags & MF_END)m_StackEnd.SetAt(m_Stack.GetUpperBound(),TRUE);
            BCMenu* pSubMenu = new BCMenu;
            pSubMenu->m_unselectcheck=m_unselectcheck;
            pSubMenu->m_selectcheck=m_selectcheck;
            pSubMenu->checkmaps=checkmaps;
            pSubMenu->checkmapsshare=TRUE;
            pSubMenu->CreatePopupMenu();

            // Append it to the top of the stack:

            m_Stack[m_Stack.GetUpperBound()]->AppendODMenuW(szCaption,uFlags,
                (UINT)(UINT_PTR)pSubMenu->m_hMenu, -1);
            m_Stack.Add(pSubMenu);
            m_StackEnd.Add(FALSE);
        }
        else {
            m_Stack[m_Stack.GetUpperBound()]->AppendODMenuW(szCaption, uFlags,
                dwID, -1);
            if(dwFlags & MF_END)m_StackEnd.SetAt(m_Stack.GetUpperBound(),TRUE);
            j = (int)m_Stack.GetUpperBound();
            while(j>=0 && m_StackEnd.GetAt(j)){
                m_Stack[m_Stack.GetUpperBound()]->InsertSpaces();
                m_Stack.RemoveAt(j);
                m_StackEnd.RemoveAt(j);
                --j;
            }
        }
        
        delete[] szCaption;
    }while(m_Stack.GetUpperBound() != -1);
    
    for(int i=0;i<(int)GetMenuItemCount();++i){
        CString str=m_MenuList[i]->GetString();
        if(GetSubMenu(i)){
            m_MenuList[i]->nFlags=MF_POPUP|MF_BYPOSITION;
            ModifyMenu(i,MF_POPUP|MF_BYPOSITION,
                (UINT_PTR)GetSubMenu(i)->m_hMenu,str);
        }
        else{
            m_MenuList[i]->nFlags=MF_STRING|MF_BYPOSITION;
            ModifyMenu(i,MF_STRING|MF_BYPOSITION,m_MenuList[i]->nID,str);
        }
    }

    m_loadmenu=TRUE;
    
    return(TRUE);
}

int BCMenu::GetMenuStart(void)
{
    if(!m_loadmenu)return(0);

    CString name, str;
    int menuloc = -1, listloc = -1, menustart = 0, i = 0, j = 0;
    int nummenulist = (int)m_MenuList.GetSize();
    int nummenu = (int)GetMenuItemCount();

    while(i<nummenu&&menuloc==-1){
        GetMenuString (i, name, MF_BYPOSITION);
        if(name.GetLength()>0){
            for(j=0;j<nummenulist;++j){
                str=m_MenuList[j]->GetString();
                if(name==str){
                    menuloc=i;
                    listloc=j;
                    break;
                }
            }
        }
        ++i;
    }
    if(menuloc>=0&&listloc>=0&&menuloc>=listloc)menustart=menuloc-listloc;
    return(menustart);
}

void BCMenu::RemoveTopLevelOwnerDraw(void)
{
    CString str;
    int i = 0, j = 0;
    int nummenulist = (int)m_MenuList.GetSize(), menustart;

    menustart=GetMenuStart();
    for(i=menustart,j=0;i<(int)GetMenuItemCount();++i,++j){
        if(j<nummenulist){
            str=m_MenuList[j]->GetString();
            if(GetSubMenu(i)){
                m_MenuList[j]->nFlags=MF_POPUP|MF_BYPOSITION;
                ModifyMenu(i,MF_POPUP|MF_BYPOSITION,
                    (UINT_PTR)GetSubMenu(i)->m_hMenu,str);
            }
        }
    }

}

void BCMenu::InsertSpaces(void)
{
    if(IsMenuThemeActive())
        if(!xp_space_accelerators) return;
    else
        if(!original_space_accelerators) return;
    
    int i,j,numitems,maxlength;
    CString string,newstring;
    CSize t;
    CFont menuFont;
    LOGFONT lf;
    
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    NONCLIENTMETRICS nm;
    nm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0)); 
    lf = nm.lfMenuFont;
    menuFont.CreateFontIndirect(&lf);
    
    CWnd* pWnd = AfxGetMainWnd();  
    if(pWnd == NULL) pWnd = CWnd::GetDesktopWindow();
    CDC* pDC = pWnd->GetDC();
    CFont* pOldFont = pDC->SelectObject(&menuFont);
    
    numitems=GetMenuItemCount();
    maxlength = -1;
    for(i=0;i<numitems;++i){
        string=m_MenuList[i]->GetString();
        j=string.Find((char)9);
        newstring.Empty();
        if(j!=-1)newstring=string.Left(j);
        else newstring=string;
        newstring+=_T(" ");//SK: modified for Unicode correctness. 
        LPCTSTR lpstrText = (LPCTSTR)newstring;
        t=pDC->GetTextExtent(lpstrText,(int)_tcslen(lpstrText));
        if(t.cx>maxlength)maxlength = t.cx;
    }
    for(i=0;i<numitems;++i){
        string=m_MenuList[i]->GetString();
        j=string.Find((char)9);
        if(j!=-1){
            newstring.Empty();
            newstring=string.Left(j);
            LPCTSTR lpstrText = (LPCTSTR)(newstring);
            t=pDC->GetTextExtent(lpstrText,(int)_tcslen(lpstrText));
            while(t.cx<maxlength){
                newstring+=_T(' ');//SK: modified for Unicode correctness
                LPCTSTR lpstrText = (LPCTSTR)(newstring);
                t=pDC->GetTextExtent(lpstrText,(int)_tcslen(lpstrText));
            }
            newstring+=string.Mid(j);
#ifdef UNICODE      
            m_MenuList[i]->SetWideString(newstring);//SK: modified for dynamic allocation
#else
            m_MenuList[i]->SetAnsiString(newstring);
#endif
        }
    }

    pDC->SelectObject(pOldFont);
    pWnd->ReleaseDC(pDC);
    menuFont.DeleteObject();
}

void BCMenu::LoadCheckmarkBitmap(int unselect, int select)
{
    if(unselect>0 && select>0){
        m_selectcheck=select;
        m_unselectcheck=unselect;
        if(checkmaps)checkmaps->DeleteImageList();
        else checkmaps=new(CImageList);
        checkmaps->Create(m_iconWidth,m_iconHeight,ILC_MASK,2,1);
        BOOL flag1=AddBitmapToImageList(checkmaps,unselect);
        BOOL flag2=AddBitmapToImageList(checkmaps,select);
        if(!flag1||!flag2){
            checkmaps->DeleteImageList();
            delete checkmaps;
            checkmaps=NULL;
        }
    }
}

//--------------------------------------------------------------------------
//[18.06.99 rj]
BOOL BCMenu::GetMenuText(UINT id, CString& string, UINT nFlags/*= MF_BYPOSITION*/)
{
    BOOL returnflag=FALSE;
    
    if(MF_BYPOSITION&nFlags){
        UINT numMenuItems = (UINT)m_MenuList.GetUpperBound();
        if(id<=numMenuItems){
            string=m_MenuList[id]->GetString();
            returnflag=TRUE;
        }
    }
    else{
        int uiLoc;
        BCMenu* pMenu = FindMenuOption(id,uiLoc);
        if(NULL!=pMenu) returnflag = pMenu->GetMenuText(uiLoc,string);
    }
    return(returnflag);
}


void BCMenu::DrawRadioDot(CDC *pDC,int x,int y,COLORREF color)
{
    CRect rcDot(x,y,x+6,y+6);
    CBrush brush;
    CPen pen;
    brush.CreateSolidBrush(color);
    pen.CreatePen(PS_SOLID,0,color);
    CBrush *pOldBrush=pDC->SelectObject(&brush);
    CPen *pOldPen=pDC->SelectObject(&pen);
    pDC->Ellipse(&rcDot);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
    pen.DeleteObject();
    brush.DeleteObject();
}

void BCMenu::DrawCheckMark(CDC* pDC,int x,int y,COLORREF color,BOOL narrowflag)
{
    int dp=0;
    CPen penBackground;
    penBackground.CreatePen (PS_SOLID,0,color);
    CPen *pOldPen = pDC->SelectObject (&penBackground);
    if(narrowflag)dp=1;

    pDC->MoveTo(x,y+2);
    pDC->LineTo(x,y+5-dp);
    
    pDC->MoveTo(x+1,y+3);
    pDC->LineTo(x+1,y+6-dp);
    
    pDC->MoveTo(x+2,y+4);
    pDC->LineTo(x+2,y+7-dp);
    
    pDC->MoveTo(x+3,y+3);
    pDC->LineTo(x+3,y+6-dp);
    
    pDC->MoveTo(x+4,y+2);
    pDC->LineTo(x+4,y+5-dp);
    
    pDC->MoveTo(x+5,y+1);
    pDC->LineTo(x+5,y+4-dp);
    
    pDC->MoveTo(x+6,y);
    pDC->LineTo(x+6,y+3-dp);
    
    pDC->SelectObject (pOldPen);
    penBackground.DeleteObject();
}

BCMenuData *BCMenu::FindMenuList(UINT nID)
{
    for(int i=0;i<=m_MenuList.GetUpperBound();++i){
        if(m_MenuList[i]->nID==nID && !m_MenuList[i]->syncflag){
            m_MenuList[i]->syncflag=1;
            return(m_MenuList[i]);
        }
    }
    return(NULL);
}

void BCMenu::InitializeMenuList(int value)
{
    for(int i=0;i<=m_MenuList.GetUpperBound();++i)
        m_MenuList[i]->syncflag=value;
}

void BCMenu::DeleteMenuList(void)
{
    for(int i=0;i<=m_MenuList.GetUpperBound();++i){
        if(!m_MenuList[i]->syncflag){
            delete m_MenuList[i];
        }
    }
}

void BCMenu::SynchronizeMenu(void)
{
    CTypedPtrArray<CPtrArray, BCMenuData*> temp;
    BCMenuData *mdata;
    CString string;
    UINT submenu,nID=0,state,j;
    
    InitializeMenuList(0);
    for(j=0;j<GetMenuItemCount();++j){
        mdata=NULL;
        state=GetMenuState(j,MF_BYPOSITION);
        if(state&MF_POPUP){
            submenu=(UINT)(UINT_PTR)GetSubMenu(j)->m_hMenu;
            mdata=FindMenuList(submenu);
            GetMenuString(j,string,MF_BYPOSITION);
            if(!mdata)mdata=NewODMenu(j,
                (state&0xFF)|MF_BYPOSITION|MF_POPUP|MF_OWNERDRAW,submenu,string);
            else if(string.GetLength()>0)
#ifdef UNICODE
                mdata->SetWideString(string);  //SK: modified for dynamic allocation
#else
            mdata->SetAnsiString(string);
#endif
        }
        else if(state&MF_SEPARATOR){
            mdata=FindMenuList(0);
            if(!mdata)mdata=NewODMenu(j,
                state|MF_BYPOSITION|MF_SEPARATOR|MF_OWNERDRAW,0,_T(""));//SK: modified for Unicode correctness
            else ModifyMenu(j,mdata->nFlags,nID,(LPCTSTR)mdata);
        }
        else{
            nID=GetMenuItemID(j);
            mdata=FindMenuList(nID);
            GetMenuString(j,string,MF_BYPOSITION);
            if(!mdata)mdata=NewODMenu(j,state|MF_BYPOSITION|MF_OWNERDRAW,
                nID,string);
            else{
                mdata->nFlags=state|MF_BYPOSITION|MF_OWNERDRAW;
                if(string.GetLength()>0)
#ifdef UNICODE
                    mdata->SetWideString(string);//SK: modified for dynamic allocation
#else
                mdata->SetAnsiString(string);
#endif
                
                ModifyMenu(j,mdata->nFlags,nID,(LPCTSTR)mdata);
            }
        }
        if(mdata)temp.Add(mdata);
    }
    DeleteMenuList();
    m_MenuList.RemoveAll();
    m_MenuList.Append(temp);
    temp.RemoveAll(); 
}

void BCMenu::UpdateMenu(CMenu *pmenu)
{
#ifdef _CPPRTTI 
    BCMenu *psubmenu = dynamic_cast<BCMenu *>(pmenu);
#else
    BCMenu *psubmenu = (BCMenu *)pmenu;
#endif
    if(psubmenu)psubmenu->SynchronizeMenu();
}

LRESULT BCMenu::FindKeyboardShortcut(UINT nChar, UINT nFlags,
                                     CMenu *pMenu)
{
#ifdef _CPPRTTI 
    BCMenu *pBCMenu = dynamic_cast<BCMenu *>(pMenu);
#else
    BCMenu *pBCMenu = (BCMenu *)pMenu;
#endif
    if(pBCMenu && nFlags&MF_POPUP){
        CString key(_T('&'),2);//SK: modified for Unicode correctness
        key.SetAt(1,(TCHAR)nChar);
        key.MakeLower();
        CString menutext;
        int menusize = (int)pBCMenu->GetMenuItemCount();
        if(menusize!=(pBCMenu->m_MenuList.GetUpperBound()+1))
            pBCMenu->SynchronizeMenu();
        for(int i=0;i<menusize;++i){
            if(pBCMenu->GetMenuText(i,menutext)){
                menutext.MakeLower();
                if(menutext.Find(key)>=0)return(MAKELRESULT(i,2));
            }
        }
    }
    return(0);
}

void BCMenu::DitherBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, 
                       int nHeight, HBITMAP hbm, int nXSrc, int nYSrc,
                       COLORREF bgcolor)
{
    ASSERT(hdcDest && hbm);
    ASSERT(nWidth > 0 && nHeight > 0);
    
    // Create a generic DC for all BitBlts
    HDC hDC = CreateCompatibleDC(hdcDest);
    ASSERT(hDC);
    
    if (hDC)
    {
        // Create a DC for the monochrome DIB section
        HDC bwDC = CreateCompatibleDC(hDC);
        ASSERT(bwDC);
        
        if (bwDC)
        {
            // Create the monochrome DIB section with a black and white palette
            struct {
                BITMAPINFOHEADER bmiHeader; 
                RGBQUAD      bmiColors[2]; 
            } RGBBWBITMAPINFO = {
                
                {    // a BITMAPINFOHEADER
                    sizeof(BITMAPINFOHEADER),  // biSize 
                        nWidth,         // biWidth; 
                        nHeight,        // biHeight; 
                        1,            // biPlanes; 
                        1,            // biBitCount 
                        BI_RGB,         // biCompression; 
                        0,            // biSizeImage; 
                        0,            // biXPelsPerMeter; 
                        0,            // biYPelsPerMeter; 
                        0,            // biClrUsed; 
                        0            // biClrImportant; 
                },    
                {
                    { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 }
                    } 
            };
            VOID *pbitsBW;
            HBITMAP hbmBW = CreateDIBSection(bwDC,
                (LPBITMAPINFO)&RGBBWBITMAPINFO, DIB_RGB_COLORS, &pbitsBW, NULL, 0);
            ASSERT(hbmBW);
            
            if (hbmBW)
            {
                // Attach the monochrome DIB section and the bitmap to the DCs
                HBITMAP olddib = (HBITMAP)SelectObject(bwDC, hbmBW);
                HBITMAP hdcolddib = (HBITMAP)SelectObject(hDC, hbm);
                
                // BitBlt the bitmap into the monochrome DIB section
                BitBlt(bwDC, 0, 0, nWidth, nHeight, hDC, nXSrc, nYSrc, SRCCOPY);
                
                // Paint the destination rectangle in gray
                FillRect(hdcDest, CRect(nXDest, nYDest, nXDest + nWidth, nYDest + nHeight), GetSysColorBrush(bgcolor));
                //SK: looks better on the old shell
                // BitBlt the black bits in the monochrome bitmap into COLOR_3DHILIGHT
                // bits in the destination DC
                // The magic ROP comes from the Charles Petzold's book
                HBRUSH hb = CreateSolidBrush(GetSysColor(COLOR_3DHILIGHT));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdcDest, hb);
                BitBlt(hdcDest,nXDest+1,nYDest+1,nWidth,nHeight,bwDC,0,0,0xB8074A);
                
                // BitBlt the black bits in the monochrome bitmap into COLOR_3DSHADOW
                // bits in the destination DC
                hb = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
                DeleteObject(SelectObject(hdcDest, hb));
                BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight,bwDC,0,0,0xB8074A);
                DeleteObject(SelectObject(hdcDest, oldBrush));
                VERIFY(DeleteObject(SelectObject(bwDC, olddib)));
                SelectObject(hDC, hdcolddib);
            }
            
            VERIFY(DeleteDC(bwDC));
        }
        
        VERIFY(DeleteDC(hDC));
    }
}

void BCMenu::GetFadedBitmap(CBitmap& bmp)
{
    CDC ddc;
    COLORREF bgcol, col;
    BITMAP BitMap;

    bmp.GetBitmap(&BitMap);
    ddc.CreateCompatibleDC(NULL);
    CBitmap* pddcOldBmp = ddc.SelectObject(&bmp);

    // use this to get the background color, takes into account color shifting
    CDC ddc2;
    CBitmap bmp2;
    ddc2.CreateCompatibleDC(NULL);
    bmp2.CreateCompatibleBitmap(&ddc, BitMap.bmWidth, BitMap.bmHeight);
    col = GetSysColor(COLOR_3DFACE);
    CBitmap* pddcOldBmp2 = ddc2.SelectObject(&bmp2);
    CRect rect(0, 0, BitMap.bmWidth, BitMap.bmHeight);
    ddc2.FillSolidRect(&rect, col);
    bgcol = ddc2.GetPixel(1, 1);
    ddc2.SelectObject(pddcOldBmp2);

    for(int i = 0; i < BitMap.bmWidth; ++i)
    {
        for(int j = 0; j < BitMap.bmHeight; ++j)
        {
            col = ddc.GetPixel(i, j);
            if(col != bgcol) // not the background
                ddc.SetPixel(i, j, LightenColor(col, 0.3));
            else // background
                ddc.SetPixel(i, j, col);
        }
    }
    ddc.SelectObject(pddcOldBmp);
}

void BCMenu::GetTransparentBitmap(CBitmap& bmp)
{
    CDC ddc;
    COLORREF bgcol, col, newcol;
    BITMAP BitMap;

    bmp.GetBitmap(&BitMap);
    ddc.CreateCompatibleDC(NULL);
    CBitmap* pddcOldBmp = ddc.SelectObject(&bmp);

    // use this to get the background color, takes into account color shifting
    CDC ddc2;
    CBitmap bmp2;
    ddc2.CreateCompatibleDC(NULL);
    bmp2.CreateCompatibleBitmap(&ddc, BitMap.bmWidth, BitMap.bmHeight);
    col = RGB(192,192,192);
    CBitmap* pddcOldBmp2 = ddc2.SelectObject(&bmp2);
    CRect rect(0, 0, BitMap.bmWidth, BitMap.bmHeight);
    ddc2.FillSolidRect(&rect, col);
    bgcol = ddc2.GetPixel(1, 1);
    ddc2.SelectObject(pddcOldBmp2);
    newcol = GetSysColor(COLOR_3DFACE);

    for(int i = 0; i < BitMap.bmWidth; ++i)
    {
        for(int j = 0; j < BitMap.bmHeight; ++j)
        {
            col = ddc.GetPixel(i, j);
            if(col == bgcol)
                ddc.SetPixel(i, j, newcol);
        }
    }
    ddc.SelectObject(pddcOldBmp);
}

void BCMenu::GetDisabledBitmap(CBitmap& bmp, COLORREF newBgColor)
{
    // get info about bitmap
    BITMAP bmpInfo;
    bmp.GetBitmap(&bmpInfo);

    // create DC for bitmap
    CDC targetDC;
    targetDC.CreateCompatibleDC(NULL);
    CBitmap* pOldBmp = targetDC.SelectObject(&bmp);

    // colors
    COLORREF bgColor = GetSysColor(COLOR_3DFACE);
    COLORREF disabledBaseColor = GetSysColor(COLOR_BTNSHADOW);

    // create disabled bitmap
    COLORREF targetColor;
    for(int yPos = 0; yPos < bmpInfo.bmHeight; yPos++)
    {
        for(int xPos = 0; xPos < bmpInfo.bmWidth; xPos++)
        {
            // get source pixel
            targetColor = targetDC.GetPixel(xPos, yPos);

            // calculate target pixel
            if(targetColor != bgColor) // not the background
            {
                int r = (int)GetRValue(targetColor);
                int g = (int)GetGValue(targetColor);
                int b = (int)GetBValue(targetColor);
                int gray = ((b * 114) + (g * 587) + (r * 299)) / 1000;
                double factor = gray / 255.0;
                targetDC.SetPixel(xPos, yPos, LightenColor(disabledBaseColor, factor));
            }
            else // background
            {
                targetDC.SetPixel(xPos, yPos, (newBgColor) ? newBgColor : targetColor);
            }
        }
    }
    targetDC.SelectObject(pOldBmp);
}

void BCMenu::GetShadowBitmap(CBitmap& bmp)
{
    CDC ddc;
    COLORREF bgcol, col, shadowcol = GetSysColor(COLOR_BTNSHADOW);
    BITMAP BitMap;

    if(!IsThemed()) shadowcol = LightenColor(shadowcol, 0.49);
    bmp.GetBitmap(&BitMap);
    ddc.CreateCompatibleDC(NULL);
    CBitmap* pddcOldBmp = ddc.SelectObject(&bmp);

    // use this to get the background color, takes into account color shifting
    CDC ddc2;
    CBitmap bmp2;
    ddc2.CreateCompatibleDC(NULL);
    bmp2.CreateCompatibleBitmap(&ddc, BitMap.bmWidth, BitMap.bmHeight);
    col = GetSysColor(COLOR_3DFACE);
    CBitmap* pddcOldBmp2 = ddc2.SelectObject(&bmp2);
    CRect rect(0, 0, BitMap.bmWidth, BitMap.bmHeight);
    ddc2.FillSolidRect(&rect, col);
    bgcol = ddc2.GetPixel(1, 1);
    ddc2.SelectObject(pddcOldBmp2);

    for(int i = 0; i < BitMap.bmWidth; ++i)
    {
        for(int j = 0; j < BitMap.bmHeight; ++j)
        {
            col = ddc.GetPixel(i, j);
            if(col != bgcol) // not the background
                ddc.SetPixel(i, j, shadowcol);
            else // background
                ddc.SetPixel(i, j, col);
        }
    }
    ddc.SelectObject(pddcOldBmp);
}


BOOL BCMenu::AddBitmapToImageList(CImageList *bmplist,UINT nResourceID)
{
    BOOL bReturn=FALSE;

    // O.S.
    if (m_bDynIcons){
        bmplist->Add((HICON)(UINT_PTR)nResourceID);
        bReturn=TRUE;
    }
    else{
        HBITMAP hbmp=LoadSysColorBitmap(nResourceID);
        if(hbmp){
            CBitmap bmp;
            bmp.Attach(hbmp);
            if(m_bitmapBackgroundFlag){
                if(bmplist->Add(&bmp,m_bitmapBackground)>=0)bReturn=TRUE;
            }
            else{
                if(bmplist->Add(&bmp,GetSysColor(COLOR_3DFACE))>=0)bReturn=TRUE;
            }
            bmp.Detach();
            DeleteObject(hbmp);
        }
        else{ // a hicolor bitmap
            CBitmap mybmp;
            if(mybmp.LoadBitmap(nResourceID)){
                hicolor_bitmaps=TRUE;
                GetTransparentBitmap(mybmp);
                if(m_bitmapBackgroundFlag){
                    if(bmplist->Add(&mybmp,m_bitmapBackground)>=0)bReturn=TRUE;
                }
                else{
                    if(bmplist->Add(&mybmp,GetSysColor(COLOR_3DFACE))>=0)bReturn=TRUE;
                }
            }
        }
    }
    if(bReturn && IsMenuThemeActive() && (xp_draw_3D_bitmaps || xp_draw_3D_bitmaps_disabled))
    {
        CWnd *pWnd = AfxGetMainWnd();
        if (pWnd == NULL) pWnd = CWnd::GetDesktopWindow();
        CDC *pDC = pWnd->GetDC();
        CBitmap bmp,bmp2,bmp3;
        GetBitmapFromImageList(pDC,bmplist,0,bmp);
        GetFadedBitmap(bmp);
        bmplist->Add(&bmp,GetSysColor(COLOR_3DFACE));
        GetBitmapFromImageList(pDC,bmplist,0,bmp2);
        GetShadowBitmap(bmp2);
        bmplist->Add(&bmp2,GetSysColor(COLOR_3DFACE));
        GetBitmapFromImageList(pDC,bmplist,0,bmp3);
        GetDisabledBitmap(bmp3);
        bmplist->Add(&bmp3,GetSysColor(COLOR_3DFACE));
        pWnd->ReleaseDC(pDC);
    }
    return(bReturn);
}

void BCMenu::SetBitmapBackgroundColor(COLORREF color)
{
    m_bitmapBackground = color;
    m_bitmapBackgroundFlag = TRUE;
}

void BCMenu::ResetBitmapBackgroundColor()
{
    m_bitmapBackgroundFlag = FALSE;
}

// Given a toolbar, append all the options from it to this menu
// Passed a ptr to the toolbar object and the toolbar ID
// Author : Robert Edward Caldecott
void BCMenu::AddFromToolBar(CToolBar* pToolBar, int nResourceID)
{
    for (int i = 0; i < pToolBar->GetCount(); i++) {
        UINT nID = pToolBar->GetItemID(i);
        // See if this toolbar option
        // appears as a command on this
        // menu or is a separator
        if (nID == 0 || GetMenuState(nID, MF_BYCOMMAND) == 0xFFFFFFFF)
            continue; // Item doesn't exist
        UINT nStyle;
        int nImage;
        // Get the toolbar button info
        pToolBar->GetButtonInfo(i, nID, nStyle, nImage);
        // OK, we have the command ID of the toolbar
        // option, and the tollbar bitmap offset
        int nLoc;
        BCMenuData* pData;
        BCMenu *pSubMenu = FindMenuOption(nID, nLoc);
        if (pSubMenu && nLoc >= 0)pData = pSubMenu->m_MenuList[nLoc];
        else {
            // Create a new BCMenuData structure
            pData = new BCMenuData;
            m_MenuList.Add(pData);
        }
        // Set some default structure members
        pData->menuIconNormal = nResourceID;
        pData->nID = nID;
        pData->nFlags =  MF_BYCOMMAND | MF_OWNERDRAW;
        pData->nOffsetX = nImage;
        if (pData->bitmap)pData->bitmap->DeleteImageList();
        else pData->bitmap = new CImageList;
        pData->bitmap->Create(m_iconWidth, m_iconHeight,ILC_COLORDDB|ILC_MASK, 1, 1);
        
        if(!AddBitmapToImageList(pData->bitmap, nResourceID)){
            pData->bitmap->DeleteImageList();
            delete pData->bitmap;
            pData->bitmap=NULL;
            pData->menuIconNormal = -1;
            pData->nOffsetX = -1;
        }
        
        // Modify our menu
        ModifyMenu(nID,pData->nFlags,nID,(LPCTSTR)pData);
    }
}

BOOL BCMenu::Draw3DCheckmark(CDC *dc, const CRect& rc,
                             BOOL bSelected, HBITMAP hbmCheck)
{
    CRect rcDest = rc;
    CBrush brush;
    COLORREF col;
    if(IsThemed())col=GetSysColor(COLOR_3DFACE);
    else col=GetSysColor(COLOR_MENU);
    if(!bSelected)col = LightenColor(col,0.6);
    brush.CreateSolidBrush(col);
    dc->FillRect(rcDest,&brush);
    brush.DeleteObject();
    if (IsNewShell()) //SK: looks better on the old shell
        dc->DrawEdge(&rcDest, BDR_SUNKENOUTER, BF_RECT);
    if (!hbmCheck)DrawCheckMark(dc,rc.left+4,rc.top+4,GetSysColor(COLOR_MENUTEXT));
    else DrawRadioDot(dc,rc.left+5,rc.top+4,GetSysColor(COLOR_MENUTEXT));
    return TRUE;
}

BOOL BCMenu::DrawXPCheckmark(CDC *dc, const CRect& rc, HBITMAP hbmCheck,COLORREF &colorout,BOOL selected)
{
    CBrush brushin;
    COLORREF col;
    int dx,dy;
    dy = (rc.Height()>>1)-3;
    dy = dy<0 ? 0 : dy;
    dx = (rc.Width()>>1)-3;
    dx = dx<0 ? 0 : dx;

    if(selected) col = LightenColor(colorout,0.55);
    else col = LightenColor(colorout,0.85);
    brushin.CreateSolidBrush(col);
    dc->FillRect(rc,&brushin);
    brushin.DeleteObject();
    dc->Draw3dRect (rc,colorout,colorout);

    if (!hbmCheck)DrawCheckMark(dc,rc.left+dx,rc.top+dy,GetSysColor(COLOR_MENUTEXT));
    else DrawRadioDot(dc,rc.left+dx,rc.top+dy,GetSysColor(COLOR_MENUTEXT));
    return TRUE;
}

void BCMenu::DitherBlt2(CDC *drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, CBitmap &bmp, int nXSrc, int nYSrc,
                        COLORREF bgcolor)
{
    // create a monochrome memory DC
    CDC ddc;
    ddc.CreateCompatibleDC(0);
    CBitmap bwbmp;
    bwbmp.CreateCompatibleBitmap(&ddc, nWidth, nHeight);
    CBitmap * pddcOldBmp = ddc.SelectObject(&bwbmp);
    
    CDC dc;
    dc.CreateCompatibleDC(0);
    CBitmap * pdcOldBmp = dc.SelectObject(&bmp);
    
    // build a mask
    ddc.PatBlt(0, 0, nWidth, nHeight, WHITENESS);
    dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
    ddc.BitBlt(0, 0, nWidth, nHeight, &dc, nXSrc,nYSrc, SRCCOPY);
    dc.SetBkColor(GetSysColor(COLOR_BTNHILIGHT));
    ddc.BitBlt(0, 0, nWidth, nHeight, &dc, nXSrc,nYSrc, SRCPAINT);
    
    // Copy the image from the toolbar into the memory DC
    // and draw it (grayed) back into the toolbar.
    dc.FillSolidRect(0,0, nWidth, nHeight, bgcolor);
    //SK: Looks better on the old shell
    dc.SetBkColor(RGB(0, 0, 0));
    dc.SetTextColor(RGB(255, 255, 255));
    CBrush brShadow, brHilight;
    brHilight.CreateSolidBrush(GetSysColor(COLOR_BTNHILIGHT));
    brShadow.CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
    CBrush * pOldBrush = dc.SelectObject(&brHilight);
    dc.BitBlt(0,0, nWidth, nHeight, &ddc, 0, 0, 0x00E20746L);
    drawdc->BitBlt(nXDest+1,nYDest+1,nWidth, nHeight, &dc,0,0,SRCCOPY);
    dc.BitBlt(1,1, nWidth, nHeight, &ddc, 0, 0, 0x00E20746L);
    dc.SelectObject(&brShadow);
    dc.BitBlt(0,0, nWidth, nHeight, &ddc, 0, 0, 0x00E20746L);
    drawdc->BitBlt(nXDest,nYDest,nWidth, nHeight, &dc,0,0,SRCCOPY);
    // reset DCs
    ddc.SelectObject(pddcOldBmp);
    ddc.DeleteDC();
    dc.SelectObject(pOldBrush);
    dc.SelectObject(pdcOldBmp);
    dc.DeleteDC();
    
    brShadow.DeleteObject();
    brHilight.DeleteObject();
    bwbmp.DeleteObject();
}

void BCMenu::DitherBlt3(CDC *drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, CBitmap &bmp,COLORREF bgcolor)
{
    GetDisabledBitmap(bmp,bgcolor);
    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CBitmap * pdcOldBmp = dc.SelectObject(&bmp);
    drawdc->BitBlt(nXDest,nYDest,nWidth, nHeight, &dc,0,0,SRCCOPY);
    // reset DCs
    dc.SelectObject(pdcOldBmp);
    dc.DeleteDC();
}

BOOL BCMenu::GetOldStyleDisableMode()
{
    return m_bOldStyleDisableMode;
}

void BCMenu::SetOldStyleDisableMode()
{
    m_bOldStyleDisableMode = TRUE;
}

void BCMenu::ResetOldStyleDisableMode()
{
    m_bOldStyleDisableMode = FALSE;
}

WORD BCMenu::NumBitmapColors(LPBITMAPINFOHEADER lpBitmap)
{
    WORD returnval = 0;

    if ( lpBitmap->biClrUsed != 0){
        returnval=(WORD)lpBitmap->biClrUsed;
    }
    else{
        switch (lpBitmap->biBitCount){
            case 1:
                returnval=2;
                break;
            case 4:
                returnval=16;
                break;
            case 8:
                returnval=256;
                break;
            default:
                returnval=0;
                break;
        }
    }
    return returnval;
}

HBITMAP BCMenu::LoadSysColorBitmap(int nResourceId)
{
    HINSTANCE hInst = 
        AfxFindResourceHandle(MAKEINTRESOURCE(nResourceId),RT_BITMAP);
    HRSRC hRsrc = 
        ::FindResource(hInst,MAKEINTRESOURCE(nResourceId),RT_BITMAP);
    if (hRsrc == NULL){
        hInst = NULL;
        hRsrc = ::FindResource(hInst,MAKEINTRESOURCE(nResourceId),RT_BITMAP);
    }
    if (hRsrc == NULL)return NULL;

    // determine how many colors in the bitmap
    HGLOBAL hglb;
    if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
        return NULL;
    LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);
    if (lpBitmap == NULL)return NULL;
    WORD numcol = NumBitmapColors(lpBitmap);
    ::FreeResource(hglb);

    if(numcol!=16)return(NULL);

    return AfxLoadSysColorBitmap(hInst, hRsrc, FALSE);
}

BOOL BCMenu::RemoveMenu(UINT uiId,UINT nFlags)
{
    if(MF_BYPOSITION&nFlags){
        UINT uint = GetMenuState(uiId,MF_BYPOSITION);
        if(uint&MF_SEPARATOR && !(uint&MF_POPUP)){
            delete m_MenuList.GetAt(uiId);
            m_MenuList.RemoveAt(uiId);
        }
        else{
            BCMenu* pSubMenu = (BCMenu*) GetSubMenu(uiId);
            if(NULL==pSubMenu){
                UINT uiCommandId = GetMenuItemID(uiId);
                for(int i=0;i<m_MenuList.GetSize(); i++){
                    if(m_MenuList[i]->nID==uiCommandId){
                        delete m_MenuList.GetAt(i);
                        m_MenuList.RemoveAt(i);
                        break;
                    }
                }
            }
            else{
                int numSubMenus = (int)m_SubMenus.GetUpperBound();
                for(int m = numSubMenus; m >= 0; m--){
                    if(m_SubMenus[m]==pSubMenu->m_hMenu){
                        int numAllSubMenus = (int)m_AllSubMenus.GetUpperBound();
                        for(int n = numAllSubMenus; n>= 0; n--){
                            if(m_AllSubMenus[n]==m_SubMenus[m])m_AllSubMenus.RemoveAt(n);
                        }
                        m_SubMenus.RemoveAt(m);
                    }
                }
                int num = pSubMenu->GetMenuItemCount();
                for(int i=num-1;i>=0;--i)pSubMenu->RemoveMenu(i,MF_BYPOSITION);
                for(int i=(int)m_MenuList.GetUpperBound();i>=0;i--){
                    if(m_MenuList[i]->nID==(UINT_PTR)pSubMenu->m_hMenu){
                        delete m_MenuList.GetAt(i);
                        m_MenuList.RemoveAt(i);
                        break;
                    }
                }
                delete pSubMenu; 
            }
        }
    }
    else{
        int iPosition =0;
        BCMenu* pMenu = FindMenuOption(uiId,iPosition);
        // bug fix RIA 14th September 2000 
        // failed to return correct value on call to remove menu as the item was 
        // removed twice. The second time its not found 
        // so a value of 0 was being returned 
        if(pMenu) return pMenu->RemoveMenu(iPosition,MF_BYPOSITION); // added return 
    }
    return CMenu::RemoveMenu(uiId,nFlags);
}

BOOL BCMenu::DeleteMenu(UINT uiId,UINT nFlags)
{
    if(MF_BYPOSITION&nFlags){
        UINT uint = GetMenuState(uiId,MF_BYPOSITION);
        if(uint&MF_SEPARATOR && !(uint&MF_POPUP)){
            // make sure it's a separator
            int menulistsize=(int)m_MenuList.GetSize();	
            if(uiId<(UINT)menulistsize){
                CString str=m_MenuList[uiId]->GetString();
                if(str==""){
                    delete m_MenuList.GetAt(uiId);
                    m_MenuList.RemoveAt(uiId);
                }
            }
        }
        else{
            BCMenu* pSubMenu = (BCMenu*) GetSubMenu(uiId);
            if(NULL==pSubMenu){
                UINT uiCommandId = GetMenuItemID(uiId);
                for(int i=0;i<m_MenuList.GetSize(); i++){
                    if(m_MenuList[i]->nID==uiCommandId){
                        delete m_MenuList.GetAt(i);
                        m_MenuList.RemoveAt(i);
                        break;
                    }
                }
            }
            else{
                int numSubMenus = (int)m_SubMenus.GetUpperBound();
                for(int m = numSubMenus; m >= 0; m--){
                    if(m_SubMenus[m]==pSubMenu->m_hMenu){
                        int numAllSubMenus = (int)m_AllSubMenus.GetUpperBound();
                        for(int n = numAllSubMenus; n>= 0; n--){
                            if(m_AllSubMenus[n]==m_SubMenus[m])m_AllSubMenus.RemoveAt(n);
                        }
                        m_SubMenus.RemoveAt(m);
                    }
                }
                int num = pSubMenu->GetMenuItemCount();
                for(int i=num-1;i>=0;--i)pSubMenu->DeleteMenu(i,MF_BYPOSITION);
                for(int i=(int)m_MenuList.GetUpperBound();i>=0;i--){
                    if(m_MenuList[i]->nID==(UINT_PTR)pSubMenu->m_hMenu){
                        delete m_MenuList.GetAt(i);
                        m_MenuList.RemoveAt(i);
                        break;
                    }
                }
                delete pSubMenu;
            }
        }
    }
    else{
        int iPosition =0;
        BCMenu* pMenu = FindMenuOption(uiId,iPosition);
        if(pMenu)return pMenu->DeleteMenu(iPosition,MF_BYPOSITION);
    }

    return(CMenu::DeleteMenu(uiId,nFlags));
}


BOOL BCMenu::AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,int nIconNormal)
{
    USES_CONVERSION;
    return AppendMenuW(nFlags,nIDNewItem,A2W(lpszNewItem),nIconNormal);
}

BOOL BCMenu::AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,int nIconNormal)
{
    return AppendODMenuW(lpszNewItem,nFlags,nIDNewItem,nIconNormal);
}

BOOL BCMenu::AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CImageList *il,int nOffsetX)
{
    USES_CONVERSION;
    return AppendMenuW(nFlags,nIDNewItem,A2W(lpszNewItem),il,nOffsetX);
}

BOOL BCMenu::AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CImageList *il,int nOffsetX)
{
    return AppendODMenuW(lpszNewItem,nFlags,nIDNewItem,il,nOffsetX);
}

BOOL BCMenu::AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CBitmap *bmp)
{
    USES_CONVERSION;
    return AppendMenuW(nFlags,nIDNewItem,A2W(lpszNewItem),bmp);
}

BOOL BCMenu::AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CBitmap *bmp)
{
    if(bmp){
        CImageList temp;
        temp.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
        if(m_bitmapBackgroundFlag)temp.Add(bmp,m_bitmapBackground);
        else temp.Add(bmp,GetSysColor(COLOR_3DFACE));
        return AppendODMenuW(lpszNewItem,nFlags,nIDNewItem,&temp,0);
    }
    return AppendODMenuW(lpszNewItem,nFlags,nIDNewItem,NULL,0);
}

BOOL BCMenu::InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,int nIconNormal)
{
    USES_CONVERSION;
    return InsertMenuW(nPosition,nFlags,nIDNewItem,A2W(lpszNewItem),nIconNormal);
}

BOOL BCMenu::InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,int nIconNormal)
{
    return InsertODMenuW(nPosition,lpszNewItem,nFlags,nIDNewItem,nIconNormal);
}

BOOL BCMenu::InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CImageList *il,int nOffsetX)
{
    USES_CONVERSION;
    return InsertMenuW(nPosition,nFlags,nIDNewItem,A2W(lpszNewItem),il,nOffsetX);
}

BOOL BCMenu::InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CImageList *il,int nOffsetX)
{
    return InsertODMenuW(nPosition,lpszNewItem,nFlags,nIDNewItem,il,nOffsetX);
}

BOOL BCMenu::InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CBitmap *bmp)
{
    USES_CONVERSION;
    return InsertMenuW(nPosition,nFlags,nIDNewItem,A2W(lpszNewItem),bmp);
}

BOOL BCMenu::InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CBitmap *bmp)
{
    if(bmp){
        CImageList temp;
        temp.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
        if(m_bitmapBackgroundFlag)temp.Add(bmp,m_bitmapBackground);
        else temp.Add(bmp,GetSysColor(COLOR_3DFACE));
        return InsertODMenuW(nPosition,lpszNewItem,nFlags,nIDNewItem,&temp,0);
    }
    return InsertODMenuW(nPosition,lpszNewItem,nFlags,nIDNewItem,NULL,0);
}

//--------------------------------------------------------------------------
//[21.06.99 rj]
BCMenu* BCMenu::AppendODPopupMenuW(wchar_t *lpstrText)
{
    BCMenu* pSubMenu = new BCMenu;
    pSubMenu->m_unselectcheck=m_unselectcheck;
    pSubMenu->m_selectcheck=m_selectcheck;
    pSubMenu->checkmaps=checkmaps;
    pSubMenu->checkmapsshare=TRUE;
    pSubMenu->CreatePopupMenu();
    AppendODMenuW(lpstrText,MF_POPUP,(UINT)(UINT_PTR)pSubMenu->m_hMenu, -1);
    return pSubMenu;
}

//--------------------------------------------------------------------------
//[21.06.99 rj]
BCMenu* BCMenu::AppendODPopupMenuA(LPCSTR lpstrText)
{
    USES_CONVERSION;
    return AppendODPopupMenuW(A2W(lpstrText));
}

BOOL BCMenu::ImageListDuplicate(CImageList* il, int nOffsetX, CImageList* newlist)
{
    if(il == NULL || newlist == NULL || nOffsetX < 0)
        return FALSE;

    // get icon size
    int cx, cy;
    ImageList_GetIconSize(il->m_hImageList, &cx, &cy);

    // get normal image and insert it into new image list
    HICON hIcon = il->ExtractIcon(nOffsetX);
    newlist->Create(cx, cy, ILC_COLOR32 | ILC_MASK, 1, 1);
    newlist->Add(hIcon);
    ::DestroyIcon(hIcon);

    // add state images
    if(IsMenuThemeActive() && (xp_draw_3D_bitmaps || xp_draw_3D_bitmaps_disabled))
    {
        CWnd* pWnd = AfxGetMainWnd();
        if(pWnd == NULL) pWnd = CWnd::GetDesktopWindow();
        CDC* pDC = pWnd->GetDC();
        {
            // create and add faded bitmap
            CBitmap bmpFaded;
            GetBitmapFromImageList(pDC, newlist, 0, bmpFaded);
            GetFadedBitmap(bmpFaded);
            newlist->Add(&bmpFaded, GetSysColor(COLOR_3DFACE));

            // create and add shadow bitmap
            CBitmap bmpShadow;
            GetBitmapFromImageList(pDC, newlist, 0, bmpShadow);
            GetShadowBitmap(bmpShadow);
            newlist->Add(&bmpShadow, GetSysColor(COLOR_3DFACE));

            // create and add disabled bitmap
            CBitmap bmpDisabled;
            GetBitmapFromImageList(pDC, newlist, 0, bmpDisabled);
            GetDisabledBitmap(bmpDisabled);
            newlist->Add(&bmpDisabled, GetSysColor(COLOR_3DFACE));
        }
        pWnd->ReleaseDC(pDC);
    }

    return TRUE;
}

// 2001-07-12, Damir Valiulin:
//          Added GetSubMenu (LPCTSTR lpszSubMenuName) function
//

CMenu* BCMenu::GetSubMenu(int nPos)
{
    return CMenu::GetSubMenu (nPos);
}

CMenu* BCMenu::GetSubMenu(LPCTSTR lpszSubMenuName)
{
    int num = GetMenuItemCount ();
    CString name;
    
    for (int i=0; i<num; i++)
    {
        GetMenuString (i, name, MF_BYPOSITION);
        if (name.Compare (lpszSubMenuName) == 0)
        {
            return CMenu::GetSubMenu (i);
        }
    }
    
    return NULL;
}

BCMenu* BCMenu::GetSubBCMenu(char* pText)
{
    USES_CONVERSION;
    return GetSubBCMenu(A2W(pText));
}

BCMenu* BCMenu::GetSubBCMenu(wchar_t* lpszSubMenuName)
{
    BCMenuData *mdata;
    mdata=FindMenuOption(lpszSubMenuName);
    if(mdata){
        HMENU bchmenu=(HMENU)(UINT_PTR)mdata->nID;
        CMenu *ptr=FromHandle(bchmenu);
        if(ptr){
            BOOL flag=ptr->IsKindOf(RUNTIME_CLASS( BCMenu ));
            if(flag)return((BCMenu *)ptr);
        }
    }
    return NULL;
}


// Tongzhe Cui, Functions to remove a popup menu based on its name. Seperators
// before and after the popup menu can also be removed if they exist.
int BCMenu::GetMenuPosition(char* pText)
{
    USES_CONVERSION;
    return GetMenuPosition(A2W(pText));
}

int BCMenu::GetMenuPosition(wchar_t* pText)
{
    int i,j;
    BCMenu* psubmenu;
    for(i=0;i<(int)(GetMenuItemCount());++i)
    {
        psubmenu=(BCMenu *)GetSubMenu(i);
        if(!psubmenu)
        {
            const wchar_t *szWide;//SK: we use const to prevent misuse of this Ptr
            for(j=0;j<=m_MenuList.GetUpperBound();++j)
            {     
                szWide = m_MenuList[j]->GetWideString ();
                if(szWide && !wcscmp(pText,szWide))//SK: modified for dynamic allocation
                    return j;
            }
        }
    }
    // means no found;
    return -1;
}

int BCMenu::RemoveMenu(char* pText, BCMSEPERATOR sPos)
{
    USES_CONVERSION;
    return RemoveMenu(A2W(pText), sPos);
}

int BCMenu::RemoveMenu(wchar_t* pText, BCMSEPERATOR sPos)
{
    int nPos = GetMenuPosition(pText);
    if(nPos != -1)
    {
        RemoveMenu(nPos, MF_BYPOSITION);
    }
    if(sPos == BCMSEP_HEAD)
    {
        ASSERT(nPos - 1 >= 0);
        RemoveMenu(nPos-1, MF_BYPOSITION);
    }
    else if(sPos == BCMSEP_TAIL)
    {
        RemoveMenu(nPos-1, MF_BYPOSITION);
    }
    else if(sPos == BCMSEP_BOTH)
    {
        // remove the end first;
        RemoveMenu(nPos-1, MF_BYPOSITION);
        // remove the head;
        ASSERT(nPos - 1 >= 0);
        RemoveMenu(nPos-1, MF_BYPOSITION);
    }
    return nPos;
}

int BCMenu::DeleteMenu(char* pText, BCMSEPERATOR sPos)
{
    USES_CONVERSION;
    return DeleteMenu(A2W(pText), sPos);
}

int BCMenu::DeleteMenu(wchar_t* pText, BCMSEPERATOR sPos)
{
    int nPos = GetMenuPosition(pText);
    if(nPos != -1)
    {
        DeleteMenu(nPos, MF_BYPOSITION);
    }
    if(sPos == BCMSEP_HEAD)
    {
        ASSERT(nPos - 1 >= 0);
        DeleteMenu(nPos-1, MF_BYPOSITION);
    }
    else if(sPos == BCMSEP_TAIL)
    {
        DeleteMenu(nPos-1, MF_BYPOSITION);
    }
    else if(sPos == BCMSEP_BOTH)
    {
        // remove the end first;
        DeleteMenu(nPos-1, MF_BYPOSITION);
        // remove the head;
        ASSERT(nPos - 1 >= 0);
        DeleteMenu(nPos-1, MF_BYPOSITION);
    }
    return nPos;
}

// Larry Antram
BOOL BCMenu::SetMenuText(UINT id, CString string, UINT nFlags/*= MF_BYPOSITION*/ )
{
    BOOL returnflag=FALSE;
    
    if(MF_BYPOSITION&nFlags)
    {
        UINT numMenuItems = (int)m_MenuList.GetUpperBound();
        if(id<=numMenuItems){
#ifdef UNICODE
            m_MenuList[id]->SetWideString((LPCTSTR)string);
#else
            m_MenuList[id]->SetAnsiString(string);
#endif
            returnflag=TRUE;
        }
    }
    else{
        int uiLoc;
        BCMenu* pMenu = FindMenuOption(id,uiLoc);
        if(NULL!=pMenu) returnflag = pMenu->SetMenuText(uiLoc,string);
    }
    return(returnflag);
}

// courtesy of Warren Stevens
void BCMenu::ColorBitmap(CDC* pDC,CBitmap& bmp,CSize bitmap_size,CSize icon_size,COLORREF fill,COLORREF border,int hatchstyle)
{
    CDC bmpdc;
    COLORREF crIconBarBackground;
    int x1=0,y1=0,x2=bitmap_size.cx,y2=bitmap_size.cy;

    if(IsThemed())crIconBarBackground=GetSysColor(COLOR_3DFACE);
    else crIconBarBackground=GetSysColor(COLOR_MENU);

    bmpdc.CreateCompatibleDC(pDC);

    bmp.CreateCompatibleBitmap(pDC, icon_size.cx, icon_size.cy);	
    CBitmap* pOldBitmap = bmpdc.SelectObject(&bmp);

    if(bitmap_size!=icon_size){
        CBrush background_brush;
        background_brush.CreateSolidBrush(crIconBarBackground);
        CRect rect(0,0, icon_size.cx, icon_size.cy);
        bmpdc.FillRect(rect,&background_brush);
        x1 = (icon_size.cx-bitmap_size.cx)/2;
        y1 = (icon_size.cy-bitmap_size.cy)/2;
        x2 = x1+bitmap_size.cx;
        y2 = y1+bitmap_size.cy;
        background_brush.DeleteObject();
    }
    
    CPen border_pen(PS_SOLID, 1, border);
    CBrush fill_brush;
    if(hatchstyle!=-1) { fill_brush.CreateHatchBrush(hatchstyle, fill); }
    else      { fill_brush.CreateSolidBrush(fill);             }
    
    CPen*    pOldPen    = bmpdc.SelectObject(&border_pen);
    CBrush*  pOldBrush  = bmpdc.SelectObject(&fill_brush);

    bmpdc.Rectangle(x1,y1,x2,y2);

    if(NULL!=pOldBrush)  { bmpdc.SelectObject(pOldBrush);  }
    if(NULL!=pOldPen)    { bmpdc.SelectObject(pOldPen);    }
    if(NULL!=pOldBitmap) { bmpdc.SelectObject(pOldBitmap); }
}

int BCMenu::GlobalImageListOffset(int nID)
{
    int numcurrent = (int)m_AllImagesID.GetSize();
    int existsloc = -1;
    for(int i=0;i<numcurrent;++i){
        if(m_AllImagesID[i]==nID){
            existsloc=i;
            break;
        }
    }
    return existsloc;
}

BOOL BCMenu::CanDraw3DImageList(int offset)
{
    BOOL retflag=FALSE;
    int numcurrent = (int)m_AllImagesID.GetSize();
    if(offset+1<numcurrent&&offset+2<numcurrent){
        int nID=m_AllImagesID[offset];
        if(m_AllImagesID[offset+1]==nID&&m_AllImagesID[offset+2]==nID)retflag=TRUE;
    }
    return(retflag);
}

int BCMenu::AddToGlobalImageList(CImageList* il, int nOffsetX, int nID)
{
    int loc = -1;
    HIMAGELIST hImageList = m_AllImages.m_hImageList;
    if(!hImageList)
    {
        m_AllImages.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
    }
    HICON hIcon = il->ExtractIcon(nOffsetX);
    if(hIcon)
    {
        CBitmap bmp,bmp2,bmp3;
        if(IsMenuThemeActive() && (xp_draw_3D_bitmaps || xp_draw_3D_bitmaps_disabled))
        {
            CWnd *pWnd = AfxGetMainWnd();
            if (pWnd == NULL) pWnd = CWnd::GetDesktopWindow();
            CDC *pDC = pWnd->GetDC();
            GetBitmapFromImageList(pDC,il,nOffsetX,bmp);
            GetFadedBitmap(bmp);
            GetBitmapFromImageList(pDC,il,nOffsetX,bmp2);
            GetShadowBitmap(bmp2);
            GetBitmapFromImageList(pDC,il,nOffsetX,bmp3);
            GetDisabledBitmap(bmp3);
            pWnd->ReleaseDC(pDC);
        }
        int numcurrent = (int)m_AllImagesID.GetSize();
        int existsloc = -1;
        for(int i=0;i<numcurrent;++i)
        {
            if(m_AllImagesID[i]==nID)
            {
                existsloc=i;
                break;
            }
        }
        if(existsloc>=0)
        {
            m_AllImages.Replace(existsloc,hIcon);
            loc = existsloc;
            if(IsMenuThemeActive() && (xp_draw_3D_bitmaps || xp_draw_3D_bitmaps_disabled))
            {
                if(existsloc+1<numcurrent&&m_AllImagesID[existsloc+1]==nID)
                {
                    if(existsloc+2<numcurrent&&m_AllImagesID[existsloc+2]==nID)
                    {
                        CImageList il2;
                        il2.Create(m_iconWidth,m_iconHeight,ILC_COLORDDB|ILC_MASK,1,1);
                        il2.Add(&bmp,GetSysColor(COLOR_3DFACE));
                        HICON hIcon2 = il2.ExtractIcon(0);
                        m_AllImages.Replace(existsloc+1,hIcon2);
                        il2.Add(&bmp2,GetSysColor(COLOR_3DFACE));
                        HICON hIcon3 = il2.ExtractIcon(1);
                        m_AllImages.Replace(existsloc+2,hIcon3);
                        il2.Add(&bmp3,GetSysColor(COLOR_3DFACE));
                        HICON hIcon4 = il2.ExtractIcon(2);
                        m_AllImages.Replace(existsloc+3,hIcon4);
                        ::DestroyIcon(hIcon2);
                        ::DestroyIcon(hIcon3);
                        ::DestroyIcon(hIcon4);
                    }
                }
            }
        }
        else
        {
            m_AllImages.Add(hIcon);
            m_AllImagesID.Add(nID);
            loc=numcurrent;
            if(IsMenuThemeActive() && (xp_draw_3D_bitmaps || xp_draw_3D_bitmaps_disabled))
            {
                m_AllImages.Add(&bmp,GetSysColor(COLOR_3DFACE));
                m_AllImages.Add(&bmp2,GetSysColor(COLOR_3DFACE));
                m_AllImages.Add(&bmp3,GetSysColor(COLOR_3DFACE));
                m_AllImagesID.Add(nID);
                m_AllImagesID.Add(nID);
                m_AllImagesID.Add(nID);
            }
        }
        ::DestroyIcon(hIcon);
    }
    return(loc);
}

bool BCMenu::AppendMenu (BCMenu* pMenuToAdd, bool add_separator /*= true*/, int num_items_to_remove_at_end /*= 0*/)
{
    // Appends a new menu to the end of existing menu.
    //
    // If 'add_separator' is true, adds separator before appending another menu.
    // separator will not be added if the original menu is empty or if the last
    // item already is a separator.
    //
    // If 'num_items_to_remove_at_end' is greater than 0, removes a specified
    // number of menu option from the end of original menu before appending
    // new menu. Removal of items is done before adding a separator (add_separator flag)

    // Original code from http://www.codeproject.com/menu/mergemenu.asp
    // Posted by Oskar Wieland on Nov 16, 2001
    //
    // Modified and added to BCMenu by Damir Valiulin on 2004-12-10:
    //   * Modified to account for BCMenu's owner-drawn stuff.
    //   * Made adding a separator optional
    //   * Added an option to remove a few items from original menu before appending
    //   * Removed bTopLevel stuff because I didn't test it.

    // Sanity checks
    if (pMenuToAdd == NULL || !pMenuToAdd->IsKindOf (RUNTIME_CLASS(BCMenu))){
        ASSERT(FALSE);
        return false;
    }

    // Anything to add?
    int iMenuAddItemCount = pMenuToAdd->GetMenuItemCount();
    if( iMenuAddItemCount == 0 )
        return true;

    // Delete last few items from the menu if specified
    if (num_items_to_remove_at_end > 0){
        int original_num_menu_items = GetMenuItemCount();
        if (original_num_menu_items >= num_items_to_remove_at_end)
        {
            int first_delete_item = original_num_menu_items - 1;
            int last_delete_item = original_num_menu_items - num_items_to_remove_at_end;
            for (int i=first_delete_item; i>=last_delete_item; i--){
                if (!DeleteMenu (i, MF_BYPOSITION)){
                    ASSERT(FALSE); // Something went wrong, but not so critical to abort everything. Just stop deleting items.
                    break;
                }
            }
        }else{
            ASSERT(FALSE); // Number of items to delete is greater than existing number of menu items!
        }
    }

    // Append a separator if existing menu has any items already and last item is not separator
    if (add_separator && GetMenuItemCount() > 0){
        if (MF_SEPARATOR != (MF_SEPARATOR & GetMenuState (GetMenuItemCount()-1, MF_BYPOSITION))){
            AppendMenu(MF_SEPARATOR);
        }
    }

    // Iterate through the top level of menu to add
    for (int iLoop = 0; iLoop < iMenuAddItemCount; iLoop++ )
    {
        // Get the menu string from the add menu
        CString sMenuAddString;
        pMenuToAdd->GetMenuText( iLoop, sMenuAddString, MF_BYPOSITION );

        // Try to get the sub-menu of the current menu item
        BCMenu* pSubMenu = (BCMenu*)pMenuToAdd->GetSubMenu(iLoop);

        // Check if we have a sub menu
        if (!pSubMenu)
        {
            // Normal menu item

            // Read the source and append at the destination
            UINT nState = pMenuToAdd->GetMenuState( iLoop, MF_BYPOSITION );
            UINT nItemID = pMenuToAdd->GetMenuItemID( iLoop );

            if (!AppendMenu( nState, nItemID, sMenuAddString )){
                ASSERT(FALSE); //TRACE( "MergeMenu: AppendMenu failed!\n" );
                return false;
            }
        }
        else
        {
            // Create a new pop-up menu and insert sub-menu's items into it
            BCMenu* pNewPopupMenu = AppendODPopupMenu (sMenuAddString);
            if (pNewPopupMenu == NULL){
                ASSERT(FALSE); //TRACE( "MergeMenu: CreatePopupMenu failed!\n" );
                return false;
            }

            // Add items to new pop-up recursively
            if (! pNewPopupMenu->AppendMenu (pSubMenu, false, 0)){
                return false;
            }
        }
    }

    return true;
}

void BCMenu::SetTrivertexColorFromColorref(TRIVERTEX* pVertex, COLORREF color)
{
    if(!pVertex)
    {
        ASSERT(0); // error: no vertex passed
        return;
    }

    pVertex->Red   = (COLOR16)GetRValue(color) << 8;
    pVertex->Green = (COLOR16)GetGValue(color) << 8;
    pVertex->Blue  = (COLOR16)GetBValue(color) << 8;
    pVertex->Alpha = 0x0000;
}

int BCMenu::GetMenuMetrics(BCMMETRIC metric)
{
    int value = -1;

    switch(metric)
    {
        // space between icon bar and menu text
        case BCMMET_TEXTOFFSET:
            value = 8;
            break;

        // space to the left and right of the menu image
        case BCMMET_ICONBARPADDING_HORZ:
            value = 11;
            break;

        // space above and below of the menu image
        case BCMMET_ICONBARPADDING_VERT:
            if(IsMenuThemeActive())
                value = 7;
            else
                value = 4;
            break;

        default:
            ASSERT(0); // error: unknown metric type
    }

    return value;
}