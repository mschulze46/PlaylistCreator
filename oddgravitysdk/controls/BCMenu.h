//*************************************************************************
// BCMenu.h : header file
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
// 2006-10-07: SetXPBitmapDisabled3D() / GetXPBitmapDisabled3D()
//             (sets whether disabled bitmaps are drawn flat or 3D)
//
// 2009-10-21: * Use separate memory DC source file
//
//*************************************************************************

#ifndef BCMenu_H
#define BCMenu_H

#include <afxtempl.h>

#define BCMENU_USE_MEMDC    // enable to use memory DC for drawing


//-----------------------------------------------
// BCMenuData class
//  Fill this class structure to define a single menu item
//-----------------------------------------------
class BCMenuData
{
protected:
    wchar_t* m_szMenuText;

public:
    BCMenuData()
    {
        menuIconNormal = -1; nOffsetX = -1; bitmap = NULL; pContext = NULL;
        nFlags = 0; nID = 0; syncflag = 0; m_szMenuText = NULL; nOffsetGlobal = -1;
    };
    ~BCMenuData();

    void SetAnsiString(LPCSTR szAnsiString);
    void SetWideString(const wchar_t* szWideString);

    const wchar_t* GetWideString() { return m_szMenuText; };
    CString GetString(); //returns the menu text in ANSI or UNICODE

    int nOffsetX, nOffsetGlobal;
    int menuIconNormal;
    UINT nFlags, nID, syncflag;
    CImageList* bitmap;
    void* pContext; // used to attach user data
};

//struct CMenuItemInfo : public MENUITEMINFO {
struct CMenuItemInfo : public 
//MENUITEMINFO 
#ifndef UNICODE   //SK: this fixes warning C4097: typedef-name 'MENUITEMINFO' used as synonym for class-name 'tagMENUITEMINFOA'
tagMENUITEMINFOA
#else
tagMENUITEMINFOW
#endif
{
    CMenuItemInfo()
    {
        memset(this, 0, sizeof(MENUITEMINFO));
        cbSize = sizeof(MENUITEMINFO);
    }
};

// defines for unicode support
#ifndef UNICODE
#define AppendMenu AppendMenuA
#define InsertMenu InsertMenuA
#define InsertODMenu InsertODMenuA
#define AppendODMenu AppendODMenuA
#define AppendODPopupMenu AppendODPopupMenuA
#define ModifyODMenu ModifyODMenuA
#define SetImageForPopupFromToolbar SetImageForPopupFromToolbarA
#else
#define AppendMenu AppendMenuW
#define InsertMenu InsertMenuW
#define InsertODMenu InsertODMenuW
#define AppendODMenu AppendODMenuW
#define ModifyODMenu ModifyODMenuW
#define AppendODPopupMenu AppendODPopupMenuW
#define SetImageForPopupFromToolbar SetImageForPopupFromToolbarW
#endif


//-----------------------------------------------
// BCMenu class
//-----------------------------------------------
class BCMenu : public CMenu
{
    DECLARE_DYNAMIC( BCMenu )
public:
    BCMenu(); 
    virtual ~BCMenu();

    // menu draw mode
    enum BCMDRAWMODE
    {
        BCMDM_ORIGINAL,
        BCMDM_XP,
    };

    // defines seperator handling when removing a menu (Tongzhe Cui)
    enum BCMSEPERATOR
    {
        BCMSEP_NONE,
        BCMSEP_HEAD,
        BCMSEP_TAIL,
        BCMSEP_BOTH,
    };

    // menu metrics
    enum BCMMETRIC
    {
        BCMMET_TEXTOFFSET,
        BCMMET_ICONBARPADDING_HORZ,
        BCMMET_ICONBARPADDING_VERT,
    };

    // Functions for loading and applying bitmaps to menus (see example application)
    virtual BOOL LoadMenu(LPCTSTR lpszResourceName);
    virtual BOOL LoadMenu(int nResource);
    BOOL LoadToolbar(UINT nToolBar);
    BOOL LoadToolbars(const UINT *arID,int n);
    void AddFromToolBar(CToolBar* pToolBar, int nResourceID);
    BOOL LoadFromToolBar(UINT nID,UINT nToolBar,int& nOffsetX);
    BOOL AddBitmapToImageList(CImageList *list,UINT nResourceID);
    static HBITMAP LoadSysColorBitmap(int nResourceId);
    void LoadCheckmarkBitmap(int unselect,int select); // custom check mark bitmaps
    
    // functions for appending a menu option, use the AppendMenu call (see above define)
    BOOL AppendMenuA(UINT nFlags,UINT nIDNewItem=0,const char *lpszNewItem=NULL,int nIconNormal=-1);
    BOOL AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CImageList *il,int nOffsetX);
    BOOL AppendMenuA(UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CBitmap *bmp);
    BOOL AppendMenuW(UINT nFlags,UINT nIDNewItem=0,wchar_t *lpszNewItem=NULL,int nIconNormal=-1);
    BOOL AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CImageList *il,int nOffsetX);
    BOOL AppendMenuW(UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CBitmap *bmp);
    BOOL AppendODMenuA(LPCSTR lpstrText, UINT nFlags = MF_OWNERDRAW, UINT nID = 0, int nIconNormal = -1);  
    BOOL AppendODMenuW(wchar_t* lpstrText, UINT nFlags = MF_OWNERDRAW, UINT nID = 0, int nIconNormal = -1);  
    BOOL AppendODMenuA(LPCSTR lpstrText, UINT nFlags, UINT nID, CImageList* il, int nOffsetX);
    BOOL AppendODMenuW(wchar_t* lpstrText, UINT nFlags, UINT nID, CImageList* il, int nOffsetX);
    bool AppendMenu (BCMenu* pMenuToAdd, bool add_separator = true, int num_items_to_remove_at_end = 0);
    
    // for appending a popup menu (see example application)
    BCMenu* AppendODPopupMenuA(LPCSTR lpstrText);
    BCMenu* AppendODPopupMenuW(wchar_t *lpstrText);

    // functions for inserting a menu option, use the InsertMenu call (see above define)
    BOOL InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem=0,const char *lpszNewItem=NULL,int nIconNormal=-1);
    BOOL InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CImageList *il,int nOffsetX);
    BOOL InsertMenuA(UINT nPosition,UINT nFlags,UINT nIDNewItem,const char *lpszNewItem,CBitmap *bmp);
    BOOL InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem=0,wchar_t *lpszNewItem=NULL,int nIconNormal=-1);
    BOOL InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CImageList *il,int nOffsetX);
    BOOL InsertMenuW(UINT nPosition,UINT nFlags,UINT nIDNewItem,wchar_t *lpszNewItem,CBitmap *bmp);
    BOOL InsertODMenuA(UINT nPosition,LPCSTR lpstrText,UINT nFlags = MF_OWNERDRAW,UINT nID = 0,int nIconNormal = -1); 
    BOOL InsertODMenuW(UINT nPosition,wchar_t *lpstrText,UINT nFlags = MF_OWNERDRAW,UINT nID = 0,int nIconNormal = -1);  
    BOOL InsertODMenuA(UINT nPosition,LPCSTR lpstrText,UINT nFlags,UINT nID,CImageList *il,int nOffsetX);
    BOOL InsertODMenuW(UINT nPosition,wchar_t *lpstrText,UINT nFlags,UINT nID,CImageList *il,int nOffsetX);
    
    // functions for modifying a menu option, use the ModifyODMenu call (see above define)
    BOOL ModifyODMenuA(const char *lpstrText,UINT nID=0,int nIconNormal=-1);
    BOOL ModifyODMenuA(const char *lpstrText,UINT nID,CImageList *il,int nOffsetX);
    BOOL ModifyODMenuA(const char *lpstrText,UINT nID,CBitmap *bmp);
    BOOL ModifyODMenuA(const char *lpstrText,const char *OptionText,int nIconNormal);
    BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID=0,int nIconNormal=-1);
    BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID,CImageList *il,int nOffsetX);
    BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID,CBitmap *bmp);
    BOOL ModifyODMenuW(wchar_t *lpstrText,wchar_t *OptionText,int nIconNormal);

    BOOL SetImageForPopupFromToolbarA (const char *strPopUpText, UINT toolbarID, UINT command_id_to_extract_icon_from);
    BOOL SetImageForPopupFromToolbarW (wchar_t *strPopUpText, UINT toolbarID, UINT command_id_to_extract_icon_from);

    // use this method for adding a solid/hatched colored square beside a menu option
    // courtesy of Warren Stevens
    BOOL ModifyODMenuA(const char *lpstrText,UINT nID,COLORREF fill,COLORREF border,int hatchstyle=-1,CSize *pSize=NULL);
    BOOL ModifyODMenuW(wchar_t *lpstrText,UINT nID,COLORREF fill,COLORREF border,int hatchstyle=-1,CSize *pSize=NULL);
    
    // for deleting and removing menu options
    BOOL	RemoveMenu(UINT uiId,UINT nFlags);
    BOOL	DeleteMenu(UINT uiId,UINT nFlags);
    // sPos means Seperator's position, since we have no way to find the seperator's position in the menu
    // we have to specify them when we call the RemoveMenu to make sure the unused seperators are removed;
    // sPos  = None no seperator removal;
    //       = Head  seperator in front of this menu item;
    //       = Tail  seperator right after this menu item;
    //       = Both  seperators at both ends;
    // remove the menu item based on their text, return -1 if not found, otherwise return the menu position;
    int RemoveMenu(char* pText, BCMSEPERATOR sPos = BCMSEP_NONE);
    int RemoveMenu(wchar_t* pText, BCMSEPERATOR sPos = BCMSEP_NONE);
    int DeleteMenu(char* pText, BCMSEPERATOR sPos = BCMSEP_NONE);
    int DeleteMenu(wchar_t* pText, BCMSEPERATOR sPos = BCMSEP_NONE);
    
    // Destoying
    virtual BOOL DestroyMenu();

    // function for retrieving and setting a menu options text (use this function
    // because it is ownerdrawn)
    BOOL GetMenuText(UINT id,CString &string,UINT nFlags = MF_BYPOSITION);
    BOOL SetMenuText(UINT id,CString string, UINT nFlags = MF_BYPOSITION);

    // Getting a submenu from it's name or position
    BCMenu* GetSubBCMenu(char* lpszSubMenuName);
    BCMenu* GetSubBCMenu(wchar_t* lpszSubMenuName);
    CMenu* GetSubMenu (LPCTSTR lpszSubMenuName);
    CMenu* GetSubMenu (int nPos);
    int GetMenuPosition(char* pText);
    int GetMenuPosition(wchar_t* pText);

    // drawing
    virtual void DrawItem( LPDRAWITEMSTRUCT);  // Draw an item
    virtual void MeasureItem( LPMEASUREITEMSTRUCT );  // Measure an item

    // static functions used for handling menus in the mainframe
    static void UpdateMenu(CMenu *pmenu);
    static BOOL IsMenu(CMenu *submenu);
    static BOOL IsMenu(HMENU submenu);
    static LRESULT FindKeyboardShortcut(UINT nChar,UINT nFlags,CMenu *pMenu);

    // gets/sets how the menu is drawn (e.g. classic style, XP style)
    static BCMDRAWMODE BCMenu::GetMenuDrawMode() { return (IsThemed()) ? xp_drawmode : original_drawmode; }
    static void SetMenuDrawMode(BCMDRAWMODE mode) { BCMenu::original_drawmode = mode; BCMenu::xp_drawmode = mode; }

    // gets/sets how disabled items are drawn ("mode = FALSE" means they are not drawn selected)
    static BOOL BCMenu::GetSelectDisableMode() { return (IsMenuThemeActive()) ? xp_select_disabled : original_select_disabled; }
    static void SetSelectDisableMode(BOOL mode) { BCMenu::original_select_disabled = mode; BCMenu::xp_select_disabled = mode; }

    // gets/sets how the bitmaps are drawn in XP Luna mode (raised when hovered or not)
    static BOOL GetXPBitmap3D() { return BCMenu::xp_draw_3D_bitmaps; }
    static void SetXPBitmap3D(BOOL val) { BCMenu::xp_draw_3D_bitmaps = val; }

    // gets/sets how the bitmaps are drawn when disabled (flat or 3D)
    static BOOL GetXPBitmapDisabled3D(void) { return BCMenu::xp_draw_3D_bitmaps_disabled; }
    static void SetXPBitmapDisabled3D(BOOL val) { BCMenu::xp_draw_3D_bitmaps_disabled = val; }

    // Customizing:

    // set icon size
    static void SetIconSize(int width, int height);

    // set bitmap background color (i.e. transparent color)
    void SetBitmapBackgroundColor(COLORREF color);
    void ResetBitmapBackgroundColor();

    // obsolete functions for setting how menu images are dithered for disabled menu options
    BOOL GetOldStyleDisableMode();
    void SetOldStyleDisableMode();
    void ResetOldStyleDisableMode();

    static COLORREF LightenColor(COLORREF color, double factor);
    static COLORREF DarkenColor(COLORREF color, double factor);

public:
    static BOOL IsNewShell(void);
    // Miscellaneous Protected Member functions
protected:
    static BOOL IsThemed();
    static BOOL IsMenuThemeActive();
    BCMenuData *BCMenu::FindMenuItem(UINT nID);
    BCMenu *FindMenuOption(int nId,int& nLoc);
    BCMenu *FindAnotherMenuOption(int nId,int& nLoc,CArray<BCMenu*,BCMenu*>&bcsubs,
                                  CArray<int,int&>&bclocs);
    BCMenuData *FindMenuOption(wchar_t *lpstrText);
    void InsertSpaces(void);
    void DrawCheckMark(CDC* pDC,int x,int y,COLORREF color,BOOL narrowflag=FALSE);
    void DrawRadioDot(CDC *pDC,int x,int y,COLORREF color);
    BCMenuData *NewODMenu(UINT pos,UINT nFlags,UINT nID,CString string);
    void SynchronizeMenu(void);
    void BCMenu::InitializeMenuList(int value);
    void BCMenu::DeleteMenuList(void);
    BCMenuData *BCMenu::FindMenuList(UINT nID);
    void DrawItem_Win9xNT2000(LPDRAWITEMSTRUCT lpDIS);
    void DrawItem_WinXP(LPDRAWITEMSTRUCT lpDIS);
    BOOL Draw3DCheckmark(CDC *dc, const CRect& rc,BOOL bSelected,HBITMAP hbmCheck);
    BOOL DrawXPCheckmark(CDC *dc, const CRect& rc, HBITMAP hbmCheck,COLORREF &colorout,BOOL selected);
    void DitherBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, 
        int nHeight, HBITMAP hbm, int nXSrc, int nYSrc,COLORREF bgcolor);
    void DitherBlt2(CDC *drawdc, int nXDest, int nYDest, int nWidth, 
        int nHeight, CBitmap &bmp, int nXSrc, int nYSrc,COLORREF bgcolor);
    void DitherBlt3(CDC *drawdc, int nXDest, int nYDest, int nWidth, 
        int nHeight, CBitmap &bmp,COLORREF bgcolor);
    BOOL GetBitmapFromImageList(CDC* pDC, CImageList* pImageList, int nIndex, CBitmap& bmp);
    BOOL ImageListDuplicate(CImageList* il, int nOffsetX, CImageList* newlist);
    static WORD NumBitmapColors(LPBITMAPINFOHEADER lpBitmap);
    void ColorBitmap(CDC* pDC, CBitmap& bmp,CSize bitmap_size,CSize icon_size,COLORREF fill,COLORREF border,int hatchstyle=-1);
    void RemoveTopLevelOwnerDraw(void);
    int GetMenuStart(void);
    void GetFadedBitmap(CBitmap& bmp);
    void GetTransparentBitmap(CBitmap& bmp);
    void GetDisabledBitmap(CBitmap& bmp, COLORREF newBgColor = 0);
    void GetShadowBitmap(CBitmap& bmp);
    int AddToGlobalImageList(CImageList* il, int nOffsetX, int nID);
    int GlobalImageListOffset(int nID);
    BOOL CanDraw3DImageList(int offset);
    void SetTrivertexColorFromColorref(TRIVERTEX* pVertex, COLORREF color);
    int GetMenuMetrics(BCMMETRIC metric);

// Member Variables
protected:
    CTypedPtrArray<CPtrArray, BCMenuData*> m_MenuList;  // Stores list of menu items 
    // When loading an owner-drawn menu using a Resource, BCMenu must keep track of
    // the popup menu's that it creates. Warning, this list *MUST* be destroyed
    // last item first :)
    CTypedPtrArray<CPtrArray, HMENU>  m_SubMenus;  // Stores list of sub-menus 
    // Stores a list of all BCMenu's ever created 
    static CTypedPtrArray<CPtrArray, HMENU>  m_AllSubMenus;
    // Global ImageList
    static CImageList m_AllImages;
    static CArray<int,int&> m_AllImagesID;
    // icon size
    static int m_iconWidth;
    static int m_iconHeight;
    COLORREF m_bitmapBackground;
    BOOL m_bitmapBackgroundFlag;
    BOOL m_bOldStyleDisableMode;
    static BCMDRAWMODE original_drawmode;
    static BOOL original_select_disabled;
    static BCMDRAWMODE xp_drawmode;
    static BOOL xp_select_disabled;
    static BOOL xp_draw_3D_bitmaps;
    static BOOL xp_draw_3D_bitmaps_disabled;
    static BOOL hicolor_bitmaps;
    static BOOL xp_space_accelerators;
    static BOOL original_space_accelerators;
    CImageList *checkmaps;
    BOOL checkmapsshare;
    int m_selectcheck;
    int m_unselectcheck;
    BOOL m_bDynIcons;
    BOOL m_loadmenu;
};

#endif
