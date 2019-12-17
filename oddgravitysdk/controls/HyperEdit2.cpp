//********************************************************************************************
//* HYPEREDIT2.CPP - Custom CHyperEdit implementation                                        *
//*                                                                                          *
//* This module contains implementation code specific to this control                        *
//*                                                                                          *
//* Aug.31.04                                                                                *
//*                                                                                          *
//* Copyright PCSpectra 2004 (Free for any purpose, except to sell indivually)               *
//* Website: www.pcspectra.com                                                               *
//*                                                                                          *
//* Notes:                                                                                   *
//* ======                                                                                   *
//* Search module for 'PROGRAMMERS NOTE'                                                     *
//*                                                                                          *
//* History:                                                                                 *
//* ========                                                                                 *
//* Mon.dd.yy - None so far                                                                  *
//********************************************************************************************

#include "stdafx.h"
#include "HyperEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
// DrawHyperlinks() draws a hyperlink colored character(individually) over existing
// CEdit text entered by user. It does this by first building a internal offset list of all
// valid hyperlinks and then iterating over each of them and their individual characters
// painting them the color specified by the client programmer or using default colors.
//

void CHyperEdit::DrawHyperlinks()
{
    CRect rcRect;
    GetRect(rcRect); // Get the formatting rectangle for the edit control

    CDC* pDC = GetDC(); 
    ASSERT(pDC);

    // select the overlay font into the DC
    CFont* pOldFont = pDC->SelectObject(&m_fontOverlay);

    // Prepare the DC for our output on top of existing content
    pDC->SetBkMode(TRANSPARENT);
    pDC->IntersectClipRect(rcRect); // Prevent drawing outside the format rectangle

    // Get the character index of the first and last visible characters
    int iChar1 = LOWORD(CharFromPos(CPoint(rcRect.left, rcRect.top))); // LineIndex(GetFirstVisibleLine()); // Old method!!!
    int iChar2 = LOWORD(CharFromPos(CPoint(rcRect.right, rcRect.bottom)));

    // Build a list of hyperlink character offsets
    BuildOffsetList(iChar1, iChar2);

    CString csBuff; // Holds the text for the entire control
    GetWindowText(csBuff);

    CPoint pt; // Coordinates for a single tokens character which is painted blue

    // Used to determine if user is hovering over a hyperlink or not
    CPoint pt_mouse(GetMessagePos()); // Current mouse location
    ScreenToClient(&pt_mouse);

    CString csTemp;

    // Draw our hyperlink(s)    
    for(int i = 0; i < (int)m_linkOffsets.size(); i++)
    {
        // Determine if mouse pointer is over a hyperlink
        csTemp = IsHyperlink(pt_mouse);
            
        // If return URL is empty then we're not over a hyperlink
        if(csTemp.IsEmpty())
        {
            pDC->SetTextColor(m_clrNormal);
        }
        else
        {
            // Make sure we only highlight the URL we're over. This technique will
            // cause duplicate URLs to highlight in hover color.
            if(csTemp == csBuff.Mid(m_linkOffsets[i].iStart, m_linkOffsets[i].iLength))
            {
                // Get the coordinates of last character in entire buffer
                CPoint pt_lastchar = PosFromChar(GetWindowTextLength() - 1);

                // Paint normally if mouse is below last visible character
                if(pt_mouse.y > (pt_lastchar.y + (long)m_nLineHeight))
                    pDC->SetTextColor(m_clrNormal);
                else
                    pDC->SetTextColor(m_clrHover);
            }
            else
            {
                pDC->SetTextColor(m_clrNormal);
            }
        }

        // Paint each URL, email, etc character individually so we can have URL's that wrap
        // onto different lines
        for(int j = m_linkOffsets[i].iStart; j < (m_linkOffsets[i].iStart + m_linkOffsets[i].iLength); j++)
        {
            // get the current token and its coordinates
            CString strToken(csBuff.GetAt(j));
            pt = PosFromChar(j);

            // get the current selection
            int iSelStart = 0, iSelFinish = 0; 
            GetSel(iSelStart, iSelFinish);

            // determine if current token is selected
            if(IsSelection(iSelStart, iSelFinish))
            {
                if(j >= iSelStart && j < iSelFinish)
                {
                    // current token is selected, so we don't paint it by ourselves
                    continue;
                }
            }

            // draw current token overtop of existing character
            if(m_bFontSmoothingFixEnabled)
            {
                CSize tokenSize = pDC->GetTextExtent(strToken);

                if(j == m_linkOffsets[i].iStart) // first token
                    pDC->FillSolidRect(pt.x - 1, pt.y, tokenSize.cx + 1, tokenSize.cy, m_clrBackground);
                else
                    pDC->FillSolidRect(pt.x, pt.y, tokenSize.cx, tokenSize.cy, m_clrBackground);
            }
            pDC->TextOut(pt.x, pt.y, strToken);
        }
    }

    // free resources
    pDC->SelectObject(pOldFont);
    ReleaseDC(pDC);
}

//
// Builds an offset list of all valid hyperlinks. This function is optimized by only
// searching through characters which are visible, ignoring those that would require 
// scrolling to view. When this function encounters a token and it determines it's
// hyperlink-able (Using virtual IsWordHyper) the starting offset and length of where
// the hyperlink exists within the actual CEdit buffer is recorded and pushed onto a vector
//

void CHyperEdit::BuildOffsetList(int iCharStart, int iCharFinish)
{
    // Entire control buffer and individual token buffer
    CString csBuff, csToken; 
    GetWindowText(csBuff);

    // Clear previous hyperlink offsets from vector and rebuild list
    m_linkOffsets.clear(); 

    // Rebuild list of hyperlink character offsets starting at iChar1 and ending at iChar2
    for(int i = iCharStart, iCurr = iCharStart; i <= iCharFinish; i++)
    {
        if(IsWhiteSpace(csBuff, i)) // Also checks for EOB (End of buffer)
        {
            _TOKEN_OFFSET off = { iCurr /* Start offset */, i - iCurr /* Length */ };

            // Let client programmer define what tokens can be hyperlinked or not
            // if one desires he/she could easily implement an easy check using a
            // regex library on email addresses without using the mailto: suffix
            if(IsWordHyper(csToken))
                m_linkOffsets.push_back(off); // Save the starting offset for current token

            csToken.Empty(); // Flush previous token        
            iCurr = i+1; // Initialize the start offset for next token
        }
        else 
            csToken += csBuff.GetAt(i); // Concatenate another char onto token
    }
}

//
// Returns a hyperlinks URL if mouse cursor is actually over a URL
// and if mouse isn't over any hyperlink it returns a empty CString
//

CString CHyperEdit::IsHyperlink(CPoint& pt) const 
{
    CString csBuff, csTemp;
    GetWindowText(csBuff);

    // Get the index of the character caret is currently over or closest too
    int iChar = LOWORD(CharFromPos(pt));

    // Check 'iChar' against vector offsets and determine if current character
    // user is hovering over is inside any hyperlink range
    for(int i=0; i < (int)m_linkOffsets.size(); i++){
            
        // If character user is over is within range of this token URL, let's exit and send the URL
        if(iChar>=m_linkOffsets[i].iStart && iChar<(m_linkOffsets[i].iStart+m_linkOffsets[i].iLength)){
            csTemp = csBuff.Mid(m_linkOffsets[i].iStart, m_linkOffsets[i].iLength);
            return csTemp;
        }
    }

    csTemp.Empty(); // NULL string on error
    return csTemp; 
}

//
// Returns TRUE if indexed character within buffer is whitespace
//

BOOL CHyperEdit::IsWhiteSpace(const CString& csBuff, int iIndex) const 
{ 
    // Check for End of buffer 
    if(csBuff.GetLength() == iIndex) return TRUE;
        
    // Check for whitespace
    if(csBuff.GetAt(iIndex) == WHITE_SPACE1) return TRUE;
    if(csBuff.GetAt(iIndex) == WHITE_SPACE2) return TRUE;
    if(csBuff.GetAt(iIndex) == WHITE_SPACE3) return TRUE;
    if(csBuff.GetAt(iIndex) == WHITE_SPACE4) return TRUE;

    return FALSE;
}

//
// Virtual function which can be overridden by client programmer to allow advanced
// hyperlinking. For example in a derived class we could use regex to validate
// email addresses without using the 'mailto:' suffix.
//

BOOL CHyperEdit::IsWordHyper(const CString& csToken) const
{
    if(IsWhiteSpace(csToken, 0)) return FALSE; // Whitespace YUCK!!!

    CString csTemp(csToken); // Make a temp copy so we can convert it's case
    csTemp.MakeLower();

    // A trivial approach to hyperlinking web sites or email addresses
    // In a derived class we can use regex if we like to only hyperlink
    // fully qualified URL's etc...
    if(csTemp.Left(7) == "http://") return TRUE;
    if(csTemp.Left(7) == "mailto:") return TRUE;
    //if(csTemp.Left(5) == "file:") return TRUE;

    return FALSE; // Not a valid token by default
}     

