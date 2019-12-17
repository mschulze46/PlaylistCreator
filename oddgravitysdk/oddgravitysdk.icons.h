/*----------------------------------------------------------------------------
| File:    oddgravitysdk.icons.h
| Project: oddgravitysdk
|
| Description:
|   Icon functions
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 210 $
| $Id: oddgravitysdk.icons.h 210 2007-06-12 16:51:02Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_ICONS_H__
#define __ODDGRAVITYSDK_ICONS_H__

#include "oddgravitysdk.gdi.h"


namespace oddgravity
{

// returns the icon handle for the given icon id
static HICON GetIconHandleFromID(int nIcon, int nWidth, int nHeight)
{
    if(nIcon <= 0)
    {
        ASSERT(0);
        return NULL;
    }

    // if at least one dimension is zero, both have to be zero
    if(nWidth == 0 || nHeight == 0)
    {
        nWidth = 0;
        nHeight = 0;
    }

    HINSTANCE hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nIcon), RT_GROUP_ICON);
    if(!hInstResource)
    {
        hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nIcon), RT_ICON);
        if(!hInstResource)
        {
            ASSERT(0);
            return NULL;
        }
    }

    // If LR_DEFAULTSIZE is not specified and cxDesired and cyDesired are set to zero,
    // the function uses the actual resource size
    HICON hIcon = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(nIcon), IMAGE_ICON, nWidth, nHeight, LR_DEFAULTCOLOR);
    ASSERT(hIcon);

    return hIcon;
}

// returns a grayscaled version of the transfered icon
static HICON GrayscaleIcon(HWND hWnd, HICON hIcon)
{
    if(!IsWindow(hWnd) || !hIcon)
    {
        ASSERT(0);
        return NULL;
    }

    HICON hGrayIcon = NULL;

    ICONINFO iconinfo;
    BOOL bOK = ::GetIconInfo(hIcon, &iconinfo);

    if(bOK && iconinfo.hbmColor)
    {
        HBITMAP hbmGrayscale = GrayscaleBitmap(hWnd, iconinfo.hbmColor);
        if(!hbmGrayscale)
        {
            ASSERT(0);
            return NULL;
        }

        ::DeleteObject(iconinfo.hbmColor);
        iconinfo.hbmColor = hbmGrayscale;
        hGrayIcon = ::CreateIconIndirect(&iconinfo);
        ::DeleteObject(iconinfo.hbmColor);
        ::DeleteObject(iconinfo.hbmMask);
    }

    return hGrayIcon;
}

// returns a grayscaled version of the transfered icon in the desired size
static HICON GrayscaleIcon(HWND hWnd, int nIcon, int nWidth, int nHeight)
{
    if(!IsWindow(hWnd) || nIcon <= 0)
    {
        ASSERT(0);
        return NULL;
    }

    HICON hIcon = GetIconHandleFromID(nIcon, nWidth, nHeight);
    if(!hIcon)
    {
        ASSERT(0);
        return NULL;
    }
    return GrayscaleIcon(hWnd, hIcon);
}

// returns a darkened version of the transfered icon
static HICON DarkenIcon(HWND hWnd, HICON hIcon, double dFactor = 0.25)
{
    if(!IsWindow(hWnd) || !hIcon)
    {
        ASSERT(0);
        return NULL;
    }

    HICON hDarkIcon = NULL;

    ICONINFO iconinfo;
    BOOL bOK = ::GetIconInfo(hIcon, &iconinfo);

    if(bOK && iconinfo.hbmColor)
    {
        HBITMAP hbmDarkened = DarkenBitmap(hWnd, iconinfo.hbmColor, dFactor);
        if(!hbmDarkened)
        {
            ASSERT(0);
            return NULL;
        }

        ::DeleteObject(iconinfo.hbmColor);
        iconinfo.hbmColor = hbmDarkened;
        hDarkIcon = ::CreateIconIndirect(&iconinfo);
        ::DeleteObject(iconinfo.hbmColor);
        ::DeleteObject(iconinfo.hbmMask);
    }

    return hDarkIcon;
}

// returns a darkened version of the transfered icon in the desired size
static HICON DarkenIcon(HWND hWnd, int nIcon, int nWidth, int nHeight, double dFactor = 0.25)
{
    if(!IsWindow(hWnd) || nIcon <= 0)
    {
        ASSERT(0);
        return NULL;
    }

    HICON hIcon = GetIconHandleFromID(nIcon, nWidth, nHeight);
    if(!hIcon)
    {
        ASSERT(0);
        return NULL;
    }
    return DarkenIcon(hWnd, hIcon, dFactor);
}

// returns a gamma changed version of the transfered icon
static HICON ChangeIconGamma(HWND hWnd, HICON hIcon, double fGamma = 0.75, bool bChangeAlphaGamma = false)
{
    if(!IsWindow(hWnd) || !hIcon)
    {
        ASSERT(0);
        return NULL;
    }

    HICON hGammaChangedIcon = NULL;

    ICONINFO iconinfo;
    BOOL bOK = ::GetIconInfo(hIcon, &iconinfo);

    if(bOK && iconinfo.hbmColor)
    {
        HBITMAP hbmGammaChanged = ChangeBitmapGamma(hWnd, iconinfo.hbmColor, fGamma, bChangeAlphaGamma);
        if(!hbmGammaChanged)
        {
            ASSERT(0);
            return NULL;
        }

        ::DeleteObject(iconinfo.hbmColor);
        iconinfo.hbmColor = hbmGammaChanged;
        hGammaChangedIcon = ::CreateIconIndirect(&iconinfo);
        ::DeleteObject(iconinfo.hbmColor);
        ::DeleteObject(iconinfo.hbmMask);
    }

    return hGammaChangedIcon;
}

// returns a gamma changed version of the transfered icon in the desired size
static HICON ChangeIconGamma(HWND hWnd, int nIcon, int nWidth, int nHeight, double fGamma = 0.75, bool bChangeAlphaGamma = false)
{
    if(!IsWindow(hWnd) || nIcon <= 0)
    {
        ASSERT(0);
        return NULL;
    }

    HICON hIcon = GetIconHandleFromID(nIcon, nWidth, nHeight);
    if(!hIcon)
    {
        ASSERT(0);
        return NULL;
    }
    return ChangeIconGamma(hWnd, hIcon, fGamma, bChangeAlphaGamma);
}


class CButtonIcons
{
protected:
    HWND	m_hWnd;

    HICON	m_hIconOut;
    HICON	m_hIconIn;
    HICON	m_hIconDisabled;

    BOOL	m_bAutoCleanUp;

    double	m_fGamma;
    bool    m_bChangeAlphaGamma;


public:
    CButtonIcons(HWND hWnd, HICON hIconOut, BOOL bAutoCleanUp = FALSE)
    {
        __init__();

        m_hWnd = (IsWindow(hWnd)) ? hWnd : NULL;                                ASSERT(m_hWnd);

        m_hIconOut = hIconOut;                                                  ASSERT(m_hIconOut);

        m_bAutoCleanUp = bAutoCleanUp;
    }

    CButtonIcons(HWND hWnd, int nIconOut, int nWidth, int nHeight, BOOL bAutoCleanUp = FALSE)
    {
        __init__();

        m_hWnd = (IsWindow(hWnd)) ? hWnd : NULL;                                ASSERT(m_hWnd);

        m_hIconOut = GetIconHandleFromID(nIconOut, nWidth, nHeight);            ASSERT(m_hIconOut);

        m_bAutoCleanUp = bAutoCleanUp;
    }

    ~CButtonIcons()
    {
        if(m_bAutoCleanUp)
        {
            if(m_hIconOut)
                DestroyIcon(m_hIconOut);

            if(m_hIconIn)
                DestroyIcon(m_hIconIn);

            if(m_hIconDisabled)
                DestroyIcon(m_hIconDisabled);
        }
    }

    void __init__()
    {
        m_hWnd			    = NULL;

        m_hIconOut		    = NULL;
        m_hIconIn		    = NULL;
        m_hIconDisabled	    = NULL;

        m_bAutoCleanUp	    = FALSE;

        m_fGamma		    = 0.75;
        m_bChangeAlphaGamma = false;
    }

    double GetGamma()
    {
        return m_fGamma;
    }

    void SetGamma(double fGamma)
    {
        if(fGamma < 0.0)
        {
            ASSERT(0);
            m_fGamma = 0.0;
            return;
        }
        m_fGamma = fGamma;
    }

    HICON GetIconOut()
    {
        return m_hIconOut;
    }

    HICON GetIconIn()
    {
        if(!m_hIconIn && m_hWnd && m_hIconOut)
        {
            m_hIconIn = ChangeIconGamma(m_hWnd, m_hIconOut, m_fGamma, m_bChangeAlphaGamma);
        }

        if(!m_hIconIn) // generating icon failed
        {
            ASSERT(0);
            return m_hIconOut; // same as 'out' icon
        }

        return m_hIconIn;
    }

    HICON GetIconPressed()
    {
        return GetIconIn();
    }

    HICON GetIconDisabled()
    {
        if(!m_hIconDisabled && m_hWnd && m_hIconOut)
        {
            m_hIconDisabled = GrayscaleIcon(m_hWnd, m_hIconOut);
        }

        if(!m_hIconDisabled) // generating icon failed
        {
            ASSERT(0);
            return m_hIconOut; // same as 'out' icon
        }

        return m_hIconDisabled;
    }
};





}

#endif // __ODDGRAVITYSDK_ICONS_H__
