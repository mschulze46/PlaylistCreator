/*----------------------------------------------------------------------------
| File:    oddgravitysdk.gdi.h
| Project: oddgravitysdk
|
| Description:
|   GDI functions
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 462 $
| $Id: oddgravitysdk.gdi.h 462 2009-10-22 20:03:07Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_GDI_H__
#define __ODDGRAVITYSDK_GDI_H__

#include <cmath>
#include "oddgravitysdk.macros.h"


namespace oddgravity
{
#define RGBA(r,g,b,a)   ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16))|(((DWORD)(BYTE)(a))<<24))


// gets information about the bitmap
static bool GetBitmapInfo(CBitmap& bmp, BITMAP& bmInfo)
{
    if(bmp.GetBitmap(&bmInfo) == 0)
    {
        ASSERT(0);
        return false;
    }
    return true;
}


// replaces the background color of the bitmap with the system button color.
// the first pixel in top left corner is the color key. (only 24 bit!)
static bool ReplaceBackgroundColor(CBitmap& bmp)
{
    // get bitmap infos
    BITMAP bmInfo;
    if(!GetBitmapInfo(bmp, bmInfo))
        return false;

    if(bmInfo.bmBitsPixel != 24) // add support for additional bit depths if you choose
    {
        ASSERT(0);
        return false;
    }

    // get pixel count
    const UINT numPixels(bmInfo.bmWidth * bmInfo.bmHeight);

    // get a pointer to the pixels
    DIBSECTION ds;
    VERIFY(bmp.GetObject(sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION));
    RGBTRIPLE* pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
    if(!pixels)
    {
        ASSERT(0);
        return false;
    }

    // get system background color
    const COLORREF crButtonColor(::GetSysColor(COLOR_BTNFACE));
    const RGBTRIPLE rgbNewBkgndColor = {GetBValue(crButtonColor), GetGValue(crButtonColor), GetRValue(crButtonColor)};

    // get color key (first pixel at top left corner)
    const RGBTRIPLE rgbColorKey = pixels[0];

    // iterate over pixels substituting the pixels having the color key
    for(UINT i = 0; i < numPixels; ++i)
    {
        if(pixels[i].rgbtBlue == rgbColorKey.rgbtBlue
            && pixels[i].rgbtGreen == rgbColorKey.rgbtGreen
            && pixels[i].rgbtRed == rgbColorKey.rgbtRed)
        {
            pixels[i] = rgbNewBkgndColor;
        }
    }
    return true;
}


// gets the grayscaled value of a RGB pixel separated into its components
static int Grayscale(int red, int green, int blue)
{
    // check bounds
    if(red > 255)   red = 255;
    if(red < 0)     red = 0;

    if(green > 255) green = 255;
    if(green < 0)   green = 0;

    if(blue > 255)  blue = 255;
    if(blue < 0)    blue = 0;

    // calc gray value
    return ((blue * 114) + (green * 587) + (red * 299)) / 1000;
}


// gets the grayscaled value of a RGB pixel
static int Grayscale(COLORREF rgb)
{
    int red   = (int)GetRValue(rgb);
    int green = (int)GetGValue(rgb);
    int blue  = (int)GetBValue(rgb);
    return Grayscale(red, green, blue);
}


// returns the lightened color for the passed color components
static COLORREF LightenColor(BYTE red, BYTE green, BYTE blue, double dFactor = 0.25)
{
    // check bounds
    if(dFactor < 0.0) dFactor = 0.0;
    if(dFactor > 1.0) dFactor = 1.0;

    // calc lighter color
    BYTE lightRed   = (BYTE)((dFactor * (255 - red))   + red);
    BYTE lightGreen = (BYTE)((dFactor * (255 - green)) + green);
    BYTE lightBlue  = (BYTE)((dFactor * (255 - blue))  + blue);

    return RGB(lightRed, lightGreen, lightBlue);
}

// returns the lightened color for the passed color
static COLORREF LightenColor(COLORREF rgb, double dFactor = 0.25)
{
    return LightenColor(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), dFactor);
}


// returns the darkened color for the passed color components
static COLORREF DarkenColor(BYTE red, BYTE green, BYTE blue, double dFactor = 0.25)
{
    // check bounds
    if(dFactor < 0.0) dFactor = 0.0;
    if(dFactor > 1.0) dFactor = 1.0;

    // calc darker color
    BYTE darkRed   = (BYTE)(red   - (dFactor * red));
    BYTE darkGreen = (BYTE)(green - (dFactor * green));
    BYTE darkBlue  = (BYTE)(blue  - (dFactor * blue));

    return RGB(darkRed, darkGreen, darkBlue);
}


// returns the darkened color for the passed color
static COLORREF DarkenColor(COLORREF rgb, double dFactor = 0.25)
{
    return DarkenColor(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb), dFactor);
}


// returns a grayscaled version of the transfered bitmap
static HBITMAP GrayscaleBitmap(HWND hWnd, HBITMAP hBitmap)
{
    if(!IsWindow(hWnd) || !hBitmap)
    {
        ASSERT(0);
        return NULL;
    }

    CDC* pDC = new CDC();                                           ASSERT(pDC);
    pDC->m_hDC = ::GetDC(hWnd);

    // get the bitmap's size and color information
    BITMAP bm;
    if(::GetObject(hBitmap, sizeof(BITMAP), &bm) == 0)
    {
        ASSERT(0);
        return NULL;
    }

    // create a DIBSection copy of the original bitmap
    HBITMAP hDib = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG|LR_CREATEDIBSECTION);
    if(!hDib)
    {
        ASSERT(0);
        return NULL;
    }

    // process bitmap according to its color depth
    if(bm.bmBitsPixel < 16) // bitmap has a color table, so we modify the table
    {
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        int iSaveDC = memDC.SaveDC();
        memDC.SelectObject(hDib);
        int nColors = 1 << bm.bmBitsPixel;

        RGBQUAD pal[256];

        // get the color table
        if(::GetDIBColorTable(memDC.m_hDC, 0, nColors, pal) != 0)
        {
            // modify the color table
            for(int x = 0; x < nColors; x++)
            {
                BYTE nGray = (BYTE)Grayscale(pal[x].rgbRed, pal[x].rgbGreen, pal[x].rgbBlue);
                pal[x].rgbRed = nGray;
                pal[x].rgbGreen = nGray;
                pal[x].rgbBlue = nGray;
            }

            // set the modified color tab to the DIBSection bitmap
            if(::SetDIBColorTable(memDC.m_hDC, 0, nColors, pal) == 0)
            {
                ASSERT(0);
                hDib = NULL;
            }
        }
        else
        {
            ASSERT(0);
            hDib = NULL;
        }

        memDC.RestoreDC(iSaveDC);
        memDC.DeleteDC();
        ::ReleaseDC(hWnd, pDC->m_hDC);
        if(pDC)
        {
            delete pDC;
            pDC = NULL;
        }
        return hDib;
    }

    else // bitmap does not have a color table, so we modify the bitmap bits directly
    {
        int Size = bm.bmHeight * bm.bmWidth;

        BITMAPINFO bmi;
        bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biHeight        = bm.bmHeight;
        bmi.bmiHeader.biWidth         = bm.bmWidth;
        bmi.bmiHeader.biPlanes        = 1;
        bmi.bmiHeader.biBitCount      = bm.bmBitsPixel;
        bmi.bmiHeader.biCompression   = BI_RGB;
        bmi.bmiHeader.biSizeImage     = ((bm.bmWidth * bm.bmBitsPixel + 31) & (~31)) / 8 * bm.bmHeight;
        bmi.bmiHeader.biXPelsPerMeter = 0;
        bmi.bmiHeader.biYPelsPerMeter = 0;
        bmi.bmiHeader.biClrUsed       = 0;
        bmi.bmiHeader.biClrImportant  = 0;

        // get the bitmaps data bits
        BYTE* pBits = NULL;
        int iSize = (int)bmi.bmiHeader.biSizeImage;
        try
        {
            pBits = new BYTE[iSize];
        }
        catch(CMemoryException* pEx)
        {
#ifdef _DEBUG
            pEx->ReportError(MB_OK|MB_ICONERROR);
#endif
            ASSERT(0);
            pEx->Delete();
            return NULL;
        }
        catch(...)
        {
            ASSERT(0);
            return NULL;
        }

        if(::GetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS) == 0)
        {
            ASSERT(0);
            return NULL;
        }

        if(bm.bmBitsPixel == 32) // 32 bpp color depth
        {
            DWORD* dst = (DWORD*)pBits;

            while(Size--)
            {
                int nGray = Grayscale(GetBValue(*dst), GetGValue(*dst), GetRValue(*dst)); // bytes are flipped!
                BYTE* pAlpha = ((BYTE*)dst)+3; // get alpha byte

                if(pAlpha)
                {
                    // make bitmap slightly translucent otherwise
                    // grayscaled image looks quite dark
                    if(*pAlpha > 20)
                        *pAlpha -= 20;
                    else
                        *pAlpha = 0;

                    *dst = (DWORD)RGBA(nGray, nGray, nGray, *pAlpha);
                }
                else
                {
                    *dst = (DWORD)RGB(nGray, nGray, nGray);
                }

                dst++;
            }
        }

        else if(bm.bmBitsPixel == 24) // 24 bpp color depth
        {
            BYTE* dst = (BYTE*)pBits;

            for(int dh = 0; dh < bm.bmHeight; dh++)
            {
                for (int dw = 0; dw < bm.bmWidth; dw++)
                {
                    int nGray = Grayscale(dst[2], dst[1], dst[0]); // bytes are flipped!

                    dst[0]=(BYTE)nGray;
                    dst[1]=(BYTE)nGray;
                    dst[2]=(BYTE)nGray;

                    dst += 3;
                }

                // each row is DWORD aligned, so when we reach the end of a row, we
                // have to realign the pointer to point to the start of the next row
                #pragma message(Reminder "Must be tested with 24 bpp desktop color depth! (not available)")
                //int pos = (int)dst - (int)pBits; // DEPRECATED
                int pos = (int)(dst - pBits);
                int rem = pos % 4;
                if(rem)
                    dst += 4 - rem;
            }
        }

        else if(bm.bmBitsPixel == 16) // 16 bpp color depth
        {
            WORD* dst = (WORD*)pBits;

            while (Size--)
            {
                BYTE b = (BYTE)((*dst)&(0x1F)); // bytes are flipped!
                BYTE g = (BYTE)(((*dst)>>5)&(0x1F));
                BYTE r = (BYTE)(((*dst)>>10)&(0x1F));

                int nGray = Grayscale(r, g, b);
                *dst = ((WORD)(((BYTE)(nGray)|((WORD)((BYTE)(nGray))<<5))|(((DWORD)(BYTE)(nGray))<<10)));

                dst++;
            }
        }

        // set the modified bitmap data bits to the DIBSection
        ::SetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS);
        if(pBits)
        {
            delete[] pBits;
            pBits = NULL;
        }

        ::ReleaseDC(hWnd, pDC->m_hDC);
        if(pDC)
        {
            delete pDC;
            pDC = NULL;
        }

        return hDib;
    }
}


// returns a darkened version of the transfered bitmap
static HBITMAP DarkenBitmap(HWND hWnd, HBITMAP hBitmap, double dFactor = 0.25)
{
    if(!IsWindow(hWnd) || !hBitmap)
    {
        ASSERT(0);
        return NULL;
    }

    CDC* pDC = new CDC();                                           ASSERT(pDC);
    pDC->m_hDC = ::GetDC(hWnd);

    // get the bitmap's size and color information
    BITMAP bm;
    if(::GetObject(hBitmap, sizeof(BITMAP), &bm) == 0)
    {
        ASSERT(0);
        return NULL;
    }

    // create a DIBSection copy of the original bitmap
    HBITMAP hDib = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG|LR_CREATEDIBSECTION);
    if(!hDib)
    {
        ASSERT(0);
        return NULL;
    }

    // process bitmap according to its color depth
    if(bm.bmBitsPixel < 16) // bitmap has a color table, so we modify the color table
    {
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        int iSaveDC = memDC.SaveDC();
        memDC.SelectObject(hDib);
        int nColors = 1 << bm.bmBitsPixel;

        RGBQUAD pal[256];

        // Get the color table
        if(::GetDIBColorTable(memDC.m_hDC, 0, nColors, pal) != 0)
        {
            // modify the color table
            for(int x = 0; x < nColors; x++)
            {
                COLORREF crDarkColor = DarkenColor(pal[x].rgbRed, pal[x].rgbGreen, pal[x].rgbBlue, dFactor);
                pal[x].rgbRed   = GetRValue(crDarkColor);
                pal[x].rgbGreen = GetGValue(crDarkColor);
                pal[x].rgbBlue  = GetBValue(crDarkColor);
            }

            // set the modified color tab to the DIBSection bitmap
            if(::SetDIBColorTable(memDC.m_hDC, 0, nColors, pal) == 0)
            {
                ASSERT(0);
                hDib = NULL;
            }
        }
        else
        {
            ASSERT(0);
            hDib = NULL;
        }

        memDC.RestoreDC(iSaveDC);
        memDC.DeleteDC();
        ::ReleaseDC(hWnd, pDC->m_hDC);
        if(pDC)
        {
            delete pDC;
            pDC = NULL;
        }
        return hDib;
    }

    else // bitmap does not have a color table, so we modify the bitmap bits directly
    {
        int Size = bm.bmHeight * bm.bmWidth;

        BITMAPINFO bmi;
        bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biHeight        = bm.bmHeight;
        bmi.bmiHeader.biWidth         = bm.bmWidth;
        bmi.bmiHeader.biPlanes        = 1;
        bmi.bmiHeader.biBitCount      = bm.bmBitsPixel;
        bmi.bmiHeader.biCompression   = BI_RGB;
        bmi.bmiHeader.biSizeImage     = ((bm.bmWidth * bm.bmBitsPixel + 31) & (~31)) / 8 * bm.bmHeight;
        bmi.bmiHeader.biXPelsPerMeter = 0;
        bmi.bmiHeader.biYPelsPerMeter = 0;
        bmi.bmiHeader.biClrUsed       = 0;
        bmi.bmiHeader.biClrImportant  = 0;

        // get the bitmaps data bits
        BYTE* pBits = NULL;
        int iSize = (int)bmi.bmiHeader.biSizeImage;
        try
        {
            pBits = new BYTE[iSize];
        }
        catch(CMemoryException* pEx)
        {
#ifdef _DEBUG
            pEx->ReportError(MB_OK|MB_ICONERROR);
#endif
            ASSERT(0);
            pEx->Delete();
            return NULL;
        }
        catch(...)
        {
            ASSERT(0);
            return NULL;
        }

        if(::GetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS) == 0)
        {
            ASSERT(0);
            return NULL;
        }

        if(bm.bmBitsPixel == 32) // 32 bpp color depth
        {
            DWORD* dst = (DWORD*)pBits;

            while(Size--)
            {
                COLORREF crDarkColor = DarkenColor(GetBValue(*dst), GetGValue(*dst), GetRValue(*dst), dFactor); // bytes are flipped!
                BYTE* pAlpha = ((BYTE*)dst)+3; // get alpha byte

                if(pAlpha)
                    *dst = (DWORD)RGBA(GetBValue(crDarkColor), GetGValue(crDarkColor), GetRValue(crDarkColor), *pAlpha);
                else
                    *dst = (DWORD)RGB(GetBValue(crDarkColor), GetGValue(crDarkColor), GetRValue(crDarkColor));

                //*dst = (DWORD)RGB(nGray, nGray, nGray); // *** kills the alpha channel *** //

                dst++;
            }
        }

        else if(bm.bmBitsPixel == 24) // 24 bpp color depth
        {
            BYTE* dst = (BYTE*)pBits;

            for(int dh = 0; dh < bm.bmHeight; dh++)
            {
                for(int dw = 0; dw < bm.bmWidth; dw++)
                {
                    COLORREF crDarkColor = DarkenColor(dst[2], dst[1], dst[0], dFactor); // bytes are flipped!

                    dst[0] = GetBValue(crDarkColor);
                    dst[1] = GetGValue(crDarkColor);
                    dst[2] = GetRValue(crDarkColor);

                    dst += 3;
                }

                // each row is DWORD aligned, so when we reach the end of a row, we
                // have to realign the pointer to point to the start of the next row
                #pragma message(Reminder "Must be tested with 24 bpp desktop color depth! (not available)")
                //int pos = (int)dst - (int)pBits; // DEPRECATED
                int pos = (int)(dst - pBits);
                int rem = pos % 4;
                if (rem)
                    dst += 4 - rem;
            }
        }

        else if(bm.bmBitsPixel == 16) // 16 bpp color depth
        {
            WORD* dst = (WORD*)pBits;

            while(Size--)
            {
                BYTE b = (BYTE)((*dst)&(0x1F)); // bytes are flipped!
                BYTE g = (BYTE)(((*dst)>>5)&(0x1F));
                BYTE r = (BYTE)(((*dst)>>10)&(0x1F));

                COLORREF crDarkColor = DarkenColor(r, g, b, dFactor);
                *dst = ((WORD)((GetBValue(crDarkColor)|((WORD)(GetGValue(crDarkColor))<<5))|(((DWORD)GetRValue(crDarkColor))<<10)));

                dst++;
            }
        }

        // set the modified bitmap data bits to the DIBSection
        ::SetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS);
        if(pBits)
        {
            delete[] pBits;
            pBits = NULL;
        }
        ::ReleaseDC(hWnd, pDC->m_hDC);
        if(pDC)
        {
            delete pDC;
            pDC = NULL;
        }
        return hDib;
    }
}


// returns the gamma changed version of a bitmap
static HBITMAP ChangeBitmapGamma(HWND hWnd, HBITMAP hBitmap, double fGamma = 0.75, bool bChangeAlphaGamma = false)
{
    if(!IsWindow(hWnd) || !hBitmap)
    {
        ASSERT(0);
        return NULL;
    }

    if(fGamma == 0.0)
    {
        ASSERT(0);
        fGamma = 0.01;
    }

    CDC* pDC = new CDC();                                           ASSERT(pDC);
    pDC->m_hDC = ::GetDC(hWnd);

    // get the bitmap's size and color information
    BITMAP bm;
    if(::GetObject(hBitmap, sizeof(BITMAP), &bm) == 0)
    {
        ASSERT(0);
        return NULL;
    }

    // create a DIBSection copy of the original bitmap
    HBITMAP hDib = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG | LR_CREATEDIBSECTION);
    if(!hDib)
    {
        ASSERT(0);
        return NULL;
    }

    // init gamma values
    BYTE redGamma[256];
    BYTE greenGamma[256];
    BYTE blueGamma[256];
    BYTE alphaGamma[256];

    double fGammaInv = 1.0 / fGamma;

    for(int i = 0; i < 256; ++i)
    {
        redGamma[i]   = (BYTE)min(255, (int)((255.0 * pow(i/255.0, fGammaInv)) + 0.5));
        greenGamma[i] = (BYTE)min(255, (int)((255.0 * pow(i/255.0, fGammaInv)) + 0.5));
        blueGamma[i]  = (BYTE)min(255, (int)((255.0 * pow(i/255.0, fGammaInv)) + 0.5));
        if(bChangeAlphaGamma) alphaGamma[i] = (BYTE)min(255, (int)((255.0 * pow(i/255.0, fGamma)) + 0.5));
    }

    // process bitmap according to its color depth
    if(bm.bmBitsPixel < 16) // bitmap has a color table, so we modify the color table
    {
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        int iSaveDC = memDC.SaveDC();
        memDC.SelectObject(hDib);
        int nColors = 1 << bm.bmBitsPixel;

        RGBQUAD pal[256];

        // Get the color table
        if(::GetDIBColorTable(memDC.m_hDC, 0, nColors, pal) != 0)
        {
            // modify the color table
            for(int x = 0; x < nColors; x++)
            {
                pal[x].rgbRed   = redGamma[pal[x].rgbRed];
                pal[x].rgbGreen = greenGamma[pal[x].rgbGreen];
                pal[x].rgbBlue  = blueGamma[pal[x].rgbBlue];
            }

            // set the modified color tab to the DIBSection bitmap
            if(::SetDIBColorTable(memDC.m_hDC, 0, nColors, pal) == 0)
            {
                ASSERT(0);
                hDib = NULL;
            }
        }
        else
        {
            ASSERT(0);
            hDib = NULL;
        }

        memDC.RestoreDC(iSaveDC);
        memDC.DeleteDC();
        ::ReleaseDC(hWnd, pDC->m_hDC);
        if(pDC)
        {
            delete pDC;
            pDC = NULL;
        }
        return hDib;
    }

    else // bitmap does not have a color table, so we modify the bitmap bits directly
    {
        int Size = bm.bmHeight * bm.bmWidth;

        BITMAPINFO bmi;
        bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biHeight        = bm.bmHeight;
        bmi.bmiHeader.biWidth         = bm.bmWidth;
        bmi.bmiHeader.biPlanes        = 1;
        bmi.bmiHeader.biBitCount      = bm.bmBitsPixel;
        bmi.bmiHeader.biCompression   = BI_RGB;
        bmi.bmiHeader.biSizeImage     = ((bm.bmWidth * bm.bmBitsPixel + 31) & (~31)) / 8 * bm.bmHeight;
        bmi.bmiHeader.biXPelsPerMeter = 0;
        bmi.bmiHeader.biYPelsPerMeter = 0;
        bmi.bmiHeader.biClrUsed       = 0;
        bmi.bmiHeader.biClrImportant  = 0;

        // Get the bitmaps data bits
        BYTE* pBits = NULL;
        int iSize = (int)bmi.bmiHeader.biSizeImage;
        try
        {
            pBits = new BYTE[iSize];
            //pBits = new BYTE[sizeof(BYTE) * 0x7FFFFFFF]; // will throw an exception
        }
        catch(CMemoryException* pEx)
        {
#ifdef _DEBUG
            pEx->ReportError(MB_OK|MB_ICONERROR);
#endif
            ASSERT(0);
            pEx->Delete();
            return NULL;
        }
        catch(...)
        {
            ASSERT(0);
            return NULL;
        }

        if(::GetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS) == 0)
        {
            ASSERT(0);
            return NULL;
        }

        if(bm.bmBitsPixel == 32) // 32 bpp color depth
        {
            BYTE* dst = (BYTE*)pBits;

            while(Size--)
            {
                // true color with alpha (rgba: 8bit/8bit/8bit/8bit)
                dst[0] = blueGamma[dst[0]]; // bytes are flipped!
                dst[1] = greenGamma[dst[1]];
                dst[2] = redGamma[dst[2]];
                if(bChangeAlphaGamma) dst[3] = alphaGamma[dst[3]];

                dst += sizeof(DWORD);
            }
        }

        else if(bm.bmBitsPixel == 24) // 24 bpp color depth
        {
            BYTE* dst = (BYTE*)pBits;

            for(int dh = 0; dh < bm.bmHeight; dh++)
            {
                for(int dw = 0; dw < bm.bmWidth; dw++)
                {
                    // true color (rgb: 8bit/8bit/8bit)
                    dst[0] = blueGamma[dst[0]]; // bytes are flipped!
                    dst[1] = greenGamma[dst[1]];
                    dst[2] = redGamma[dst[2]];

                    dst += 3;
                }

                // each row is DWORD aligned, so when we reach the end of a row, we
                // have to realign the pointer to point to the start of the next row
                #pragma message(Reminder "Must be tested with 24 bpp desktop color depth! (not available)")
                //int pos = (int)dst - (int)pBits; // DEPRECATED
                int pos = (int)(dst - pBits);
                int rem = pos % 4;
                if (rem)
                    dst += 4 - rem;
            }
        }

        else if(bm.bmBitsPixel == 16) // 16 bpp color depth
        {
            WORD* dst = (WORD*)pBits;

            while(Size--)
            {
                // high color (rgb: 5bit/6bit/5bit)
                // fake gamma - just darken the pixels!
                BYTE b = (BYTE)((*dst)&(0x1F)); // bytes are flipped!
                BYTE g = (BYTE)(((*dst)>>5)&(0x1F));
                BYTE r = (BYTE)(((*dst)>>10)&(0x1F));

                COLORREF crDarkColor = DarkenColor(r, g, b, 0.07);
                *dst = ((WORD)((GetBValue(crDarkColor)|((WORD)(GetGValue(crDarkColor))<<5))|(((DWORD)GetRValue(crDarkColor))<<10)));

                dst++;
            }
        }

        // set the modified bitmap data bits to the DIBSection
        ::SetDIBits(pDC->m_hDC, hDib, 0, bm.bmHeight, pBits, &bmi, DIB_RGB_COLORS);
        if(pBits)
        {
            delete[] pBits;
            pBits = NULL;
        }
        ::ReleaseDC(hWnd, pDC->m_hDC);
        if(pDC)
        {
            delete pDC;
            pDC = NULL;
        }
        return hDib;
    }
}


// sets the color components of a TRIVERTEX structure according to the passed COLORREF
static void SetTrivertexColorFromColorref(TRIVERTEX* pVertex, COLORREF color)
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



}

#endif // __ODDGRAVITYSDK_GDI_H__
