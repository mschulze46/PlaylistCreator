#include "stdafx.h"
#include "FECFileDialog.h"

#ifndef _INC_CDERR
#include <cderr.h>     // for FNERR_BUFFERTOSMALL
#endif // _INC_CDERR

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFECFileDialog

IMPLEMENT_DYNAMIC(CFECFileDialog, CFileDialog)
/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog constructor  (public member function)
//    Initializes the class variables
//
//  Parameters :
//    See CFileDialog::CFileDialog for parameter information
//
//  Returns :
//    Nothing
//
/////////////////////////////////////////////////////////////////////////////


CFECFileDialog::CFECFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
                               DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd, DWORD dwSize, BOOL bVistaStyle) :
CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, dwSize, bVistaStyle)
{
    Files = NULL;
    Folder = NULL;
    bParsed = FALSE;
}
/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog destructor  (public member function)
//    Cleans up the class variables
//
//  Parameters :
//    None
//
//  Returns :
//    Nothing
//
/////////////////////////////////////////////////////////////////////////////


CFECFileDialog::~CFECFileDialog()
{
    if (Files)
    {
        delete[] Files;
        delete[] Folder;
    }
}

BEGIN_MESSAGE_MAP(CFECFileDialog, CFileDialog)
//{{AFX_MSG_MAP(CFECFileDialog)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::DoModal  (public member function)
//    Starts the CFileDialog and properly handles the return code
//
//  Parameters :
//    None
//
//  Returns :
//    IDOK if the user selected the OK button
//    IDCANCEL if the user selected the Cancel button or an error occured
//
/////////////////////////////////////////////////////////////////////////////

int CFECFileDialog::DoModal()
{
    if (Files)
    {
        delete[] Files;
        Files = NULL;
        delete[] Folder;
        Folder = NULL;
    }

    int ret = int(CFileDialog::DoModal());

    if (ret == IDCANCEL)
    {
        DWORD err = CommDlgExtendedError();
        if (err == FNERR_BUFFERTOOSMALL/*0x3003*/ && Files)
            ret = IDOK;
    }
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::GetNextPathName  (public member function)
//    Call this function to retrieve the next file name from the group
//    selected in the dialog box.
//
//  Parameters :
//    pos - A reference to a POSITION value returned from a previous GetNextPathName
//          or GetStartPosition function call. Will be set to NULL when the end of
//          the list has been reached.
//
//  Returns :
//    The full path of the file
//
/////////////////////////////////////////////////////////////////////////////

CString CFECFileDialog::GetNextPathName(POSITION &pos) const
{
    if (!Files)
        return CFileDialog::GetNextPathName(pos);

    ASSERT(pos);    
    TCHAR *ptr = (TCHAR *)pos;

    CString ret = Folder;
    ret += _T("\\");
    ret += ptr;

    ptr += _tcslen(ptr) + 1;
    if (*ptr)
        pos = (POSITION)ptr;
    else
        pos = NULL;

    return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::GetStartPosition  (public member function)
//    Call this member function to retrieve the POSITION of the first file
//    in a list of multiple selected files.
//
//  Parameters :
//    None
//
//  Returns :
//    A POSITION value that is used by the GetNextPathName function
//
/////////////////////////////////////////////////////////////////////////////

POSITION CFECFileDialog::GetStartPosition()
{
    if (!Files)
        return CFileDialog::GetStartPosition();

    if (!bParsed)
    {
        CString temp = Files;
        temp.Replace(_T("\" \""), _T("\""));
        temp.Delete(0, 1);                      // remove leading quote mark
        temp.Delete(temp.GetLength() - 1, 1);   // remove trailing space
    
#if _MSC_VER >= 1400  // VS2005
        size_t nBufferSize = strlen(Files)+1;
        _tcsncpy_s(Files, nBufferSize, temp, _TRUNCATE);
#else
        _tcscpy(Files, temp);
#endif
    
        TCHAR *ptr = Files;
        while (*ptr)
        {
            if ('\"' == *ptr)
                *ptr = '\0';
            ++ptr;
        }
        bParsed = TRUE;
    }
    
    return (POSITION)Files;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::OnFileNameChange  (protected member function)
//    If the lpstrFile and nMaxFile variables in the OPENFILENAME structure
//    are not large enough to handle the users selection, we handle the
//    selection ourselves.
//
//  Parameters :
//    None
//
//  Returns :
//    Nothing
//
//  Note :
//    Works only if the CFileDialog is created with the OFN_EXPLORER flag
//
/////////////////////////////////////////////////////////////////////////////

#pragma message(Reminder "CFECFileDialog::OnFileNameChange(): Doesn't work on Windows 7 up to now, dialog structure changed completely.")
// Multiple File Selection Bug in Windows7
//  http://social.msdn.microsoft.com/Forums/en-US/vclanguage/thread/2ea6c33c-6a9a-44ca-8821-da5da3dc6a16
// Problem with Multiple File Selection Dialog in Windows7
//  http://social.technet.microsoft.com/Forums/en-US/w7itproappcompat/thread/56a0bcfd-39e0-4e1c-85da-26735992576e
// Multiple Selection in a File Dialog (see thread of 'Galo Vinueza S.' from 2009-04-07)
//  http://www.codeproject.com/KB/dialog/pja_multiselect.aspx

void CFECFileDialog::OnFileNameChange()
{
    TCHAR dummy_buffer;

    // try to get parent window (Windows 7: no parent available!)
    CWnd* pParentWnd = GetParent();
    HWND hWnd = (pParentWnd) ? pParentWnd->m_hWnd : m_hWnd;

    // Get the required size for the 'files' buffer
    UINT nfiles = CommDlg_OpenSave_GetSpec(hWnd, &dummy_buffer, 1);

    // Get the required size for the 'folder' buffer
    UINT nfolder = CommDlg_OpenSave_GetFolderPath(hWnd, &dummy_buffer, 1);

    // Check if lpstrFile and nMaxFile are large enough
    if (nfiles + nfolder > m_ofn.nMaxFile)
    {
        bParsed = FALSE;
        if (Files)
            delete[] Files;
        Files = new TCHAR[nfiles + 1];
        CommDlg_OpenSave_GetSpec(hWnd, Files, nfiles);

        if (Folder)
            delete[] Folder;
        Folder = new TCHAR[nfolder + 1];
        CommDlg_OpenSave_GetFolderPath(hWnd, Folder, nfolder);
    }
    else if (Files)
    {
        delete[] Files;
        Files = NULL;
        delete[] Folder;
        Folder = NULL;
    }

    CFileDialog::OnFileNameChange();
}
