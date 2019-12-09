/*----------------------------------------------------------------------------
| File:    ximacfg.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CxImage properties.
|
|-----------------------------------------------------------------------------
| $Author: oddgravity $   $Revision: 28 $
| $Id: ximacfg.h 28 2006-10-15 16:57:50Z oddgravity $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#if !defined(__ximaCFG_h)
#define __ximaCFG_h

/////////////////////////////////////////////////////////////////////////////
// CxImage supported features
#define CXIMAGE_SUPPORT_ALPHA           1   // transparency (4kb)
#define CXIMAGE_SUPPORT_SELECTION       0   // selections (4kb)
#define CXIMAGE_SUPPORT_TRANSFORMATION  1   // geometric transformations (16kb)
#define CXIMAGE_SUPPORT_DSP             1   // image processing (24kb)
#define CXIMAGE_SUPPORT_LAYERS          0   // multiple layers (<4kb)
#define CXIMAGE_SUPPORT_INTERPOLATION   1

#define CXIMAGE_SUPPORT_DECODE          1   // graphic formats ...
#define CXIMAGE_SUPPORT_ENCODE          1   // ... conversion (<4kb)
#define CXIMAGE_SUPPORT_WINDOWS         1   // drawing and windows specific functions (12 kb)
#define CXIMAGE_SUPPORT_WINCE           0   //<T.Peck>

/////////////////////////////////////////////////////////////////////////////
// CxImage supported formats

// built in (24kb)
#define CXIMAGE_SUPPORT_BMP             1
#define CXIMAGE_SUPPORT_GIF             1
#define CXIMAGE_SUPPORT_ICO             1
#define CXIMAGE_SUPPORT_TGA             1
#define CXIMAGE_SUPPORT_PCX             1
#define CXIMAGE_SUPPORT_WBMP            1
#define CXIMAGE_SUPPORT_WMF             1

// jpeg (88kb)
#define CXIMAGE_SUPPORT_JPG             0

// png (104kb)
#define CXIMAGE_SUPPORT_PNG             1

// tiff (124kb)
#define CXIMAGE_SUPPORT_TIF             0

// mng (148kb)
#define CXIMAGE_SUPPORT_MNG             0

// jbig (28kb)
#define CXIMAGE_SUPPORT_JBG             0   // GPL'd see ../jbig/copying.txt & ../jbig/patents.htm

// jasper (176kb)
#define CXIMAGE_SUPPORT_PNM             0
#define CXIMAGE_SUPPORT_RAS             0
#define CXIMAGE_SUPPORT_JP2             0
#define CXIMAGE_SUPPORT_JPC             0
#define CXIMAGE_SUPPORT_PGX             0

// jpeg-2000
#define CXIMAGE_SUPPORT_J2K             0   // Beta, use JP2


/////////////////////////////////////////////////////////////////////////////
#define CXIMAGE_MAX_MEMORY      256000000

#define CXIMAGE_ERR_NOFILE      "null file handler"
#define CXIMAGE_ERR_NOIMAGE     "null image!!!"

/////////////////////////////////////////////////////////////////////////////
//color to grey mapping <H. Muelner> <jurgene>
//#define RGB2GRAY(r,g,b) (((b)*114 + (g)*587 + (r)*299)/1000)
#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)

#endif // __ximaCFG_h
