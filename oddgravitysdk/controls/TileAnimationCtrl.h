/*----------------------------------------------------------------------------
| File:    TileAnimationCtrl.h
| Project: oddgravitysdk
|
| Description:
|   Implementation of the CTileAnimationCtrl class.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 186 $
| $Id: TileAnimationCtrl.h 186 2007-06-02 23:16:50Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include <CxImage/ximage.h>             // CxImage library
#include <vector>


// CTileAnimationCtrl

class CTileAnimationCtrl : public CStatic
{
    DECLARE_DYNAMIC(CTileAnimationCtrl)


    ///// types /////
    typedef struct Tile
    {
        CxImage image;
        bool    bShow;
    };

    typedef std::vector<Tile*>           TileList;
    typedef std::vector<Tile*>::iterator TileListIterator;

    typedef enum BKGND_COLOR_MODE
    {
        BCM_CUSTOM,     // custom background color
        BCM_SYSTEM,     // default system background color
        BCM_PARENT      // background color of the parent
    };


    ///// attributes /////
protected:
    // animation container
    TileList    m_TileList;

    // animation state
    bool        m_bIsRunning;
    UINT        m_nSpeed;
    int         m_nCurrentTile;

    // drawing stuff
    BKGND_COLOR_MODE m_bkgndColorMode;
    COLORREF    m_colBkgnd;
    CRect       m_clientRect;
    CDC         m_memDC;
    CBitmap     m_memBmp;
    CBitmap*    m_pOldBmp;


  ///// construction / destruction /////
public:
    CTileAnimationCtrl();
    virtual ~CTileAnimationCtrl();


  ///// methods /////
public:
    bool SetAnimationImage(CxImage* pImage, UINT tileWidth, UINT tileHeight, bool bAutoSize = true);

    void SetExcludedTile(int nIndex, bool bExclude = true);
    bool IsExcludedTile(int nIndex);
    void ResetExcludedTiles();

    bool IsRunning()                        { return m_bIsRunning; }
    UINT GetSpeed()                         { return m_nSpeed; }
    void SetSpeed(UINT mSecs)               { m_nSpeed = max(mSecs, 1); }

    int GetTileCount()                      { return (int)m_TileList.size(); }
    bool HasTiles()                         { return ((int)m_TileList.size() > 0); }

    COLORREF GetBkgndColor()                { return m_colBkgnd; }
    void     SetBkgndColor(COLORREF col)    { m_colBkgnd = col; m_bkgndColorMode = BCM_CUSTOM; }

    void UseSystemBkgndColor();
    void UseParentBkgndColor();

    void DrawBorder(bool bDrawBorder);

    bool Start();
    bool Stop();
    void Restart();

protected:
    void __init__();

    void Step();

    void ClearTileList();

	void DrawBackground(CDC& dc, RECT& rect);
	void DrawCurrentTile(CDC& dc, RECT& rect);

  ///// message handler / overrides /////
protected:
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnPaint();
    afx_msg void OnDestroy();
    afx_msg void OnSysColorChange();
    afx_msg void OnSize(UINT nType, int cx, int cy);

    virtual void PreSubclassWindow();

    DECLARE_MESSAGE_MAP()
};


