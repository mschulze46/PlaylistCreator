/*----------------------------------------------------------------------------
| File:    TileAnimationCtrl.cpp
| Project: oddgravitysdk
|
| Description:
|   Declaration of the CTileAnimationCtrl class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 363 $
| $Id: TileAnimationCtrl.cpp 363 2008-07-07 19:28:22Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "TileAnimationCtrl.h"


///// defines /////
#define TIMER_STEP_ANIMATION          2342


// CTileAnimationCtrl

IMPLEMENT_DYNAMIC(CTileAnimationCtrl, CStatic)
CTileAnimationCtrl::CTileAnimationCtrl()
{
    __init__();
}


CTileAnimationCtrl::~CTileAnimationCtrl()
{
    // delete memory dc
    if(m_pOldBmp)
    {
        m_memDC.SelectObject(m_pOldBmp);
        m_memBmp.DeleteObject();
        m_memDC.DeleteDC();
    }

    // delete animation tiles
    ClearTileList();
}


void CTileAnimationCtrl::__init__()
{
    // animation container
    m_TileList.clear();

    // animation state
    m_bIsRunning        = false;
    m_nSpeed            = 100;
    m_nCurrentTile     = -1;

    // drawing stuff
    m_bkgndColorMode    = BCM_SYSTEM;
    m_colBkgnd          = GetSysColor(COLOR_3DFACE);
    m_clientRect.SetRectEmpty();
    m_pOldBmp           = NULL;
}


BEGIN_MESSAGE_MAP(CTileAnimationCtrl, CStatic)
    ON_WM_TIMER()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CTileAnimationCtrl message handlers

bool CTileAnimationCtrl::SetAnimationImage(CxImage* pImage, UINT tileWidth, UINT tileHeight, bool bAutoSize /*true*/)
{
    if(!pImage || !pImage->IsValid()            // no or invalid image
        || tileWidth  > pImage->GetWidth()      // image width too small resp. tile width not correct
        || tileHeight > pImage->GetHeight())    // image height too small resp. tile height not correct
    {
        ASSERT(0); // error: invalid image and/or tile size passed!
        return false;
    }

    // animation must be stopped while setting the new graphic
    bool bWasRunning = false;
    if(m_bIsRunning)
    {
        Stop();
        bWasRunning = true;
    }

    // release current animation tiles
    if(GetTileCount() > 0)
    {
        ClearTileList();
    }

    // auto-size client rect to tile rect?
    if(bAutoSize)
    {
        SetWindowPos(&wndTop, 0, 0, tileWidth, tileHeight, SWP_NOMOVE | SWP_FRAMECHANGED);
    }

    // calculate number of tiles
    int nTileRows    = pImage->GetHeight() / tileHeight;
    int nTilesPerRow = pImage->GetWidth() / tileWidth;
    int nTileCount   = nTileRows * nTilesPerRow;

    // split bitmap into tiles and add them to the tile list
    // note: splitting is done from top left to bottom right (index 0...n)
    for(int nLine = 0; nLine < nTileRows; nLine++)
    {
        for(int nTile = 0; nTile < nTilesPerRow; nTile++)
        {
            // calculate tile rect
            RECT tileRect;
            tileRect.left   = nTile * tileWidth;
            tileRect.top    = nLine * tileHeight;
            tileRect.right  = tileRect.left + tileWidth;
            tileRect.bottom = tileRect.top + tileHeight;

            // create new tile
            Tile* pTile = new Tile();
            pTile->bShow = true;
            pImage->Crop(tileRect, &pTile->image);

            // add tile to list
            m_TileList.push_back(pTile);
        }
    }

    // rewind current tile marker
    m_nCurrentTile = -1;

    // restart animation if we stopped it
    if(bWasRunning)
    {
        Start();
    }

    return true;
}


void CTileAnimationCtrl::SetExcludedTile(int nIndex, bool bExclude /*true*/)
{
    // check bounds
    if(nIndex >= 0 && nIndex < GetTileCount())
    {
        m_TileList.at(nIndex)->bShow = (bExclude) ? false : true;
    }
}


bool CTileAnimationCtrl::IsExcludedTile(int nIndex)
{
    // check bounds
    if(nIndex >= 0 && nIndex < GetTileCount())
    {
        return (m_TileList.at(nIndex)->bShow) ? false : true;
    }

    return false;
}


void CTileAnimationCtrl::ResetExcludedTiles()
{
    for(TileListIterator iter = m_TileList.begin(); iter != m_TileList.end(); iter++)
    {
        Tile* pTile = *iter;
        if(pTile)
        {
            pTile->bShow = true;
        }
    }
}


void CTileAnimationCtrl::UseSystemBkgndColor()
{
    m_bkgndColorMode = BCM_SYSTEM;
    m_colBkgnd = GetSysColor(COLOR_3DFACE);

    Invalidate();
}


void CTileAnimationCtrl::UseParentBkgndColor()
{
    m_bkgndColorMode = BCM_PARENT;
    // note: color must be queried from parent on every redraw

    Invalidate();
}


void CTileAnimationCtrl::DrawBorder(bool bDrawBorder)
{
    if(bDrawBorder)
    {
        ModifyStyle(0, WS_BORDER);
        ModifyStyleEx(0, WS_EX_CLIENTEDGE);
    }
    else
    {
        ModifyStyle(WS_BORDER, 0);
        ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
    }

    SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
}


bool CTileAnimationCtrl::Start()
{
    // do we have tiles?
    if(!HasTiles())
    {
        ASSERT(0);
        return false;
    }

    // already running?
    if(m_bIsRunning)
    {
        return true;
    }

    // first start?
    bool bCanStart = true;
    if(m_nCurrentTile < 0)
    {
        // init current tile
        m_nCurrentTile = 0;

        // skip excluded tiles
        int currentTileTemp = m_nCurrentTile;
        while(IsExcludedTile(m_nCurrentTile))
        {
            m_nCurrentTile++;

            // restart animation?
            if(m_nCurrentTile >= GetTileCount())
                m_nCurrentTile = 0;

            if(m_nCurrentTile == currentTileTemp)
            {
                ASSERT(0); // error: all frames are excluded
                bCanStart = false;
                break;
            }
        }

        if(!bCanStart)
            return false;
    }

    // start animation timer
    SetTimer(TIMER_STEP_ANIMATION, m_nSpeed, NULL);

    // update running flag
    m_bIsRunning = true;

    // trigger first redraw
    Invalidate();

    return true;
}


bool CTileAnimationCtrl::Stop()
{
    // already stopped?
    if(!m_bIsRunning)
    {
        return true;
    }

    // stop animation timer
    KillTimer(TIMER_STEP_ANIMATION);

    // update running flag
    m_bIsRunning = false;

    return true;
}


void CTileAnimationCtrl::Restart()
{
    Stop();
    Start();
}


void CTileAnimationCtrl::Step()
{
    // increment current tile
    m_nCurrentTile++;

    // restart animation?
    if(m_nCurrentTile >= GetTileCount())
        m_nCurrentTile = 0;

    // skip excluded tiles
    int currentTileTemp = m_nCurrentTile;
    while(IsExcludedTile(m_nCurrentTile))
    {
        m_nCurrentTile++;

        // restart animation?
        if(m_nCurrentTile >= GetTileCount())
            m_nCurrentTile = 0;

        if(m_nCurrentTile == currentTileTemp)
        {
            ASSERT(0); // error: all frames are excluded
            Stop(); // stop the animation

            break;
        }
    }

    // trigger redraw
    Invalidate();
}


void CTileAnimationCtrl::ClearTileList()
{
    if(!HasTiles())
        return;

    // delete the tiles
    for(TileListIterator iter = m_TileList.begin(); iter != m_TileList.end(); iter++)
    {
        Tile* pTile = *iter;
        if(pTile)
        {
            delete pTile;
            pTile = NULL;
        }
    }

    // clear container
    m_TileList.clear();
}


void CTileAnimationCtrl::DrawBackground(CDC& dc, RECT& rect)
{
    if(m_bkgndColorMode == BCM_PARENT) // use parent bkgnd color
    {
        // query bkgnd color from parent
        CWnd* pParent = GetParent();
        HBRUSH hBrush = NULL;
        if(pParent)
        {
            hBrush = (HBRUSH)(pParent->SendMessage(WM_CTLCOLORSTATIC,
                                                   (WPARAM)dc.GetSafeHdc(),
                                                   (LPARAM)GetSafeHwnd()));
        }

        if(hBrush)
        {
            dc.FillRect(&rect, CBrush::FromHandle(hBrush));
        }
        else
        {
            ASSERT(0); // error: couldn't retrieve bkgnd color from parent!
            dc.FillSolidRect(&rect, m_colBkgnd);
        }
    }
    else // use custom or system bkgnd color
    {
        dc.FillSolidRect(&rect, m_colBkgnd);
    }
}


void CTileAnimationCtrl::DrawCurrentTile(CDC& dc, RECT& rect)
{
    if(GetTileCount() > 0 && m_nCurrentTile >= 0)
    {
        // get current tile
        Tile* pTile = m_TileList.at(m_nCurrentTile);
        if(pTile && pTile->bShow && pTile->image.IsValid())
        {
            // draw tile
            pTile->image.Draw(dc.m_hDC, rect);
        }
    }
}


void CTileAnimationCtrl::OnTimer(UINT nIDEvent)
{
    switch(nIDEvent)
    {
        case TIMER_STEP_ANIMATION:
            Step();
            break;

        default:
            ASSERT(0);
            break;
    }

    CStatic::OnTimer(nIDEvent);
}


void CTileAnimationCtrl::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // Do not call CStatic::OnPaint() for painting messages

    // create a memory dc if necessary
    if(!m_memDC.m_hDC)
    {
        m_memDC.CreateCompatibleDC(&dc);
        m_memBmp.CreateCompatibleBitmap(&dc, m_clientRect.Width(), m_clientRect.Height());
        m_pOldBmp = m_memDC.SelectObject(&m_memBmp);
    }

    // draw animation parts to memory dc
    DrawBackground(m_memDC, m_clientRect);
    DrawCurrentTile(m_memDC, m_clientRect);

    // render the final image
    dc.BitBlt(0, 0, m_clientRect.Width(), m_clientRect.Height(), &m_memDC, 0, 0, SRCCOPY);
}


void CTileAnimationCtrl::OnDestroy()
{
    CStatic::OnDestroy();

    // stop animation timer
    KillTimer(TIMER_STEP_ANIMATION);
}


void CTileAnimationCtrl::OnSysColorChange() 
{
    CStatic::OnSysColorChange();

    switch(m_bkgndColorMode)
    {
        case BCM_SYSTEM:
            m_colBkgnd = GetSysColor(COLOR_3DFACE);
            Invalidate();
            break;

        case BCM_PARENT:
            Invalidate();
            break;

        default:
            break;
    }
}


void CTileAnimationCtrl::OnSize(UINT nType, int cx, int cy)
{
    CStatic::OnSize(nType, cx, cy);

    // update client rect
    GetClientRect(&m_clientRect);

    // delete memory dc to allow OnPaint to recreate the dc for the new size
    if(m_memDC.m_hDC)
    {
        m_memDC.SelectObject(m_pOldBmp);
        m_memBmp.DeleteObject();
        m_memDC.DeleteDC();
    }
}


void CTileAnimationCtrl::PreSubclassWindow()
{
    CStatic::PreSubclassWindow();

    if(m_clientRect.IsRectEmpty())
    {
        GetClientRect(&m_clientRect);
    }
}

