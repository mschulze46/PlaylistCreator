////////////////////////////////////////////////////////////////
// MSDN Magazine -- June 2005
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio .NET 2003 (V7.1) on Windows XP. Tab size=3.
//
#include "stdafx.h"
#include "FolderDlg.h"
#include <shlwapi.h>

// You must link shlwapi.lib for StrRetToBuf
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////
// FolderDlg control IDs
//
#define IDC_FOLDERDLG_TREE_FOLDERS          0x00064
#define IDC_FOLDERDLG_STATIC_TITLE          0x03742
#define IDC_FOLDERDLG_EDIT_FOLDER           0x03744
#define IDC_FOLDERDLG_BTN_NEW_FOLDER        0x03746
#define IDC_FOLDERDLG_STATIC_FOLDER         0x03748
#define IDC_FOLDERDLG_CUSTOM_CHECKBOX       0x02342

//////////////////
// Timers
//
#define TIMER_SET_SELECTION_WORKAROUND      2342

BOOL CFolderDialog::bTRACE=0; // controls tracing

//////////////////
// For deugging: names of interfaces easier to read than GUIDs!
//
DEBUG_BEGIN_INTERFACE_NAMES()
	DEBUG_INTERFACE_NAME(IFolderFilterSite)
	DEBUG_INTERFACE_NAME(IFolderFilter)
DEBUG_END_INTERFACE_NAMES();

IMPLEMENT_DYNAMIC(CFolderDialog, CCmdTarget);

//////////////////
// ctor: initialize most stuff to NULL
//
CFolderDialog::CFolderDialog(CWnd* pWnd)
{
	ASSERT(pWnd);
	memset(&m_brinfo,0,sizeof(m_brinfo));
	m_brinfo.hwndOwner=pWnd->m_hWnd;		 // use parent window
	m_bFilter = FALSE;						 // default: no filtering
	SHGetDesktopFolder(&m_shfRoot);		 // get root IShellFolder
}

//////////////////
// dtor: detach browser window before it's destroyed!
//
CFolderDialog::~CFolderDialog()
{
}

//////////////////
// Browse for folder. Args are same as for SHBrowseForFolder, but with extra
// bFilter that tells whether to do custom filtering. Note this requires
// BIF_NEWDIALOGSTYLE, which is inconsistent with some other flags--be
// careful!
//
LPCITEMIDLIST CFolderDialog::BrowseForFolder(LPCTSTR title, UINT flags,
	LPCITEMIDLIST root, BOOL bFilter)
{
	BFTRACE(_T("CFolderDialog::BrowseForFolder\n"));
	TCHAR* buf = m_sDisplayName.GetBuffer(MAX_PATH);
	m_brinfo.pidlRoot = root;
	m_brinfo.pszDisplayName = buf;
	m_brinfo.lpszTitle = title;
	m_brinfo.ulFlags = flags;
	m_brinfo.lpfn = CallbackProc;
	m_brinfo.lParam = (LPARAM)this;

	// filtering only supported for new-style dialogs
	m_bFilter = bFilter;
	ASSERT(!bFilter||(m_brinfo.ulFlags & BIF_NEWDIALOGSTYLE));

	LPCITEMIDLIST pidl = SHBrowseForFolder(&m_brinfo); // do it
	m_sDisplayName.ReleaseBuffer();

	return pidl;
}

//////////////////
// Handy function to get the string pathname from pidl.
//
CString CFolderDialog::GetPathName(LPCITEMIDLIST pidl)
{
	CString path;
	TCHAR* buf = path.GetBuffer(MAX_PATH);
	SHGetPathFromIDList(pidl, buf);
	path.ReleaseBuffer();
	return path;
}

//////////////////
// Handy function to get the display name from pidl.
//
CString CFolderDialog::GetDisplayNameOf(IShellFolder* psf, LPCITEMIDLIST pidl,
	DWORD uFlags)
{
	CString dn;
	STRRET strret;								 // special struct for GetDisplayNameOf
	strret.uType = STRRET_CSTR;			 // get as CSTR
	if (SUCCEEDED(psf->GetDisplayNameOf(pidl, uFlags, &strret))) {
		StrRetToBuf(&strret, pidl, dn.GetBuffer(MAX_PATH), MAX_PATH);
		dn.ReleaseBuffer();
	}
	return dn;
}

//////////////////
// Free PIDL using shell's IMalloc
//
void CFolderDialog::FreePIDL(LPCITEMIDLIST pidl)
{
	CComQIPtr<IMalloc> iMalloc;	// shell's IMalloc
	HRESULT hr = SHGetMalloc(&iMalloc);
	ASSERT(SUCCEEDED(hr));
	iMalloc->Free((void*)pidl);
}

//////////////////
// Internal callback proc used for SHBrowseForFolder passes control to
// appropriate virtual function after attaching browser window.
//
int CALLBACK CFolderDialog::CallbackProc(HWND hwnd,
	UINT msg, LPARAM lp, LPARAM lpData)
{
	CFolderDialog* pDlg = (CFolderDialog*)lpData;
	ASSERT(pDlg);
	if (pDlg->m_hWnd!=hwnd) {
		if (pDlg->m_hWnd)
			pDlg->UnsubclassWindow();
		pDlg->SubclassWindow(hwnd);
	}
	return pDlg->OnMessage(msg, lp);
}

//////////////////
// Handle notification from browser window: parse args and pass to specific
// virtual handler function.
//
int CFolderDialog::OnMessage(UINT msg, LPARAM lp)
{
	switch (msg) {
	case BFFM_INITIALIZED:
		OnInitialized();
		return 0;
	case BFFM_IUNKNOWN:
		OnIUnknown((IUnknown*)lp);
		return 0;
	case BFFM_SELCHANGED:
		OnSelChanged((LPCITEMIDLIST)lp);
		return 0;
	case BFFM_VALIDATEFAILED:
		return OnValidateFailed((LPCTSTR)lp);
	default:
		TRACE(_T("***Warning: unknown message %d in CFolderDialog::OnMessage\n"));
	}
	return 0;
}

/////////////////
// Browser window initialized.
//
void CFolderDialog::OnInitialized()
{
	BFTRACE(_T("CFolderDialog::OnInitialized\n"));
}

/////////////////
// Browser is notifying me with its IUnknown: use it to set filter if
// requested. Note this can be called with punk=NULL when shutting down!
//
void CFolderDialog::OnIUnknown(IUnknown* punk)
{
	BFTRACE(_T("CFolderDialog::OnIUnknown: %p\n"), punk);
	if (punk && m_bFilter) {
		CComQIPtr<IFolderFilterSite> iffs;
		VERIFY(SUCCEEDED(punk->QueryInterface(IID_IFolderFilterSite, (void**)&iffs)));
		iffs->SetFilter((IFolderFilter*)&m_xFolderFilter);
		// smart pointer automatically Releases iffs,
		// no longer needed once you call SetFilter
	}
}

//////////////////
// User selected a different folder.
//
void CFolderDialog::OnSelChanged(LPCITEMIDLIST pidl)
{
	BFTRACE(_T("CFolderDialog::OnSelChanged: %s\n"),
		GetDisplayNameOf(m_shfRoot, pidl, SHGDN_FORPARSING));
}

//////////////////
// User attempted to enter a name in the edit box that isn't a folder.
//
BOOL CFolderDialog::OnValidateFailed(LPCTSTR lpsz)
{
	BFTRACE(_T("CFolderDialog::OnValidateFailed: %s\n"), lpsz);
	return TRUE; // don't close dialog.
}

//////////////////
// Used for custom filtering. You must override to specify filter flags.
//
HRESULT CFolderDialog::OnGetEnumFlags(
	IShellFolder* psf,			// this folder's IShellFolder
	LPCITEMIDLIST pidlFolder,	// folder's PIDL
	DWORD *pgrfFlags)				// [out] return flags you want to allow
{
	BFTRACE(_T("CFolderDialog::OnGetEnumFlags(%p): %s\n"),
		psf, GetPathName(pidlFolder));
	return S_OK;
}

//////////////////
// Used for custom filtering. You must override to filter items.
//
HRESULT CFolderDialog::OnShouldShow(
	IShellFolder* psf,			// This folder's IShellFolder
	LPCITEMIDLIST pidlFolder,	// PIDL for folder containing item
	LPCITEMIDLIST pidlItem)		// PIDL for item
{
	BFTRACE(_T("CFolderDialog::OnShouldShow(%p): %s: %s\n"), psf,
		GetDisplayNameOf(psf,pidlFolder,SHGDN_NORMAL),
		GetDisplayNameOf(psf,pidlItem,SHGDN_NORMAL));
	return S_OK;
}

//////////////// Standard MFC IUnknown -- nested classes call these ////////////////

STDMETHODIMP_(ULONG) CFolderDialog::AddRef()
{
	BFTRACE(_T("CFolderDialog(%p)::AddRef\n"),this);
	return ExternalAddRef();
}

STDMETHODIMP_(ULONG) CFolderDialog::Release()
{
	BFTRACE(_T("CFolderDialog(%p)::Release\n"), this);
	return ExternalRelease();
}

STDMETHODIMP CFolderDialog::QueryInterface(REFIID iid, LPVOID* ppvRet)
{
	if (ppvRet==NULL)
		return E_INVALIDARG;
	BFTRACE(_T("CFolderDialog(%p)::QueryInterface(%s)\n"),this,_TR(iid));
	HRESULT hr = ExternalQueryInterface(&iid, ppvRet);
	BFTRACE(_T(">CFolderDialog::QueryInterface returns %s, *ppv=%p\n"),_TR(hr),*ppvRet);
   return hr;
}

//////////////////////////////// IFolderFilter ////////////////////////////////
//
// Implementation passes control to parent class CFolderDialog (pThis)
//
BEGIN_INTERFACE_MAP(CFolderDialog, CCmdTarget)
	INTERFACE_PART(CFolderDialog, IID_IFolderFilter, FolderFilter)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CFolderDialog::XFolderFilter::AddRef()
{
	METHOD_PROLOGUE(CFolderDialog, FolderFilter);
	return pThis->AddRef();
}

STDMETHODIMP_(ULONG) CFolderDialog::XFolderFilter::Release()
{
	METHOD_PROLOGUE(CFolderDialog, FolderFilter);
	return pThis->Release();
}

STDMETHODIMP CFolderDialog::XFolderFilter::QueryInterface(REFIID iid, LPVOID* ppv)
{
	METHOD_PROLOGUE(CFolderDialog, FolderFilter);
	return pThis->QueryInterface(iid, ppv);
}

// Note: pHwnd is always NULL here as far as I can tell.
STDMETHODIMP CFolderDialog::XFolderFilter::GetEnumFlags(IShellFolder* psf,
	LPCITEMIDLIST pidlFolder, HWND *pHwnd, DWORD *pgrfFlags)
{
	METHOD_PROLOGUE(CFolderDialog, FolderFilter);
	return pThis->OnGetEnumFlags(psf, pidlFolder, pgrfFlags);
}

STDMETHODIMP CFolderDialog::XFolderFilter::ShouldShow(IShellFolder* psf,
	LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
	METHOD_PROLOGUE(CFolderDialog, FolderFilter);
	return pThis->OnShouldShow(psf, pidlFolder, pidlItem);
}


//////////////////
// Class derived from CFolderDialog with extended features.
//

IMPLEMENT_DYNAMIC(CFolderDialogEx, CFolderDialog)

BEGIN_MESSAGE_MAP(CFolderDialogEx, inherited)
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_BN_CLICKED(IDC_FOLDERDLG_CUSTOM_CHECKBOX, OnCustomCheckboxClicked)
END_MESSAGE_MAP()

//////////////////
// ctor: initialize most stuff to NULL
//
CFolderDialogEx::CFolderDialogEx(CWnd* pWnd)
    : inherited(pWnd)
{
    m_bRemoveContextHelpButton = FALSE;

    // checkbox
    int m_nCheckboxWidth    = 0;
    int m_nCheckboxHeight   = 0;
    BOOL m_bCheckboxChecked = FALSE;
}

//////////////////
// dtor
//
CFolderDialogEx::~CFolderDialogEx()
{
}

//////////////////
// Browse for folder. Args are same as for SHBrowseForFolder, but with extra
// bFilter that tells whether to do custom filtering. Note this requires
// BIF_NEWDIALOGSTYLE, which is inconsistent with some other flags--be
// careful!
//
LPCITEMIDLIST CFolderDialogEx::BrowseForFolder(LPCTSTR caption, LPCTSTR title, LPCTSTR initialFolder,
    UINT flags, LPCITEMIDLIST root, BOOL bFilter)
{
    m_sCaption = caption;
    m_sInitialFolder = initialFolder;

    return inherited::BrowseForFolder(title, flags, root, bFilter);
}

//////////////////
// Sets the caption of the folder dialog.
//
void CFolderDialogEx::SetCaption(LPCTSTR caption)
{
    m_sCaption = caption;                                                                       ASSERT(!m_sCaption.IsEmpty());
}

//////////////////
// Sets the text of the OK button.
//
void CFolderDialogEx::SetOkBtnText(LPCTSTR okBtn)
{
    m_sOkBtn = okBtn;                                                                           ASSERT(!m_sOkBtn.IsEmpty());
}

//////////////////
// Shows the custom checkbox.
//
void CFolderDialogEx::ShowCheckbox(LPCTSTR checkbox, BOOL bChecked)
{
    m_sCheckboxText = checkbox;                                                                 ASSERT(!m_sCheckboxText.IsEmpty());
    m_bCheckboxChecked = bChecked;
}

//////////////////
// Returns whether the custom checkbox is currently checked.
//
BOOL CFolderDialogEx::IsCheckboxChecked()
{
    return m_bCheckboxChecked;
}

//////////////////
// Removes the context help button.
//
void CFolderDialogEx::RemoveContextHelpButton()
{
    m_bRemoveContextHelpButton = TRUE;
}

//////////////////
// event: resize
//
void CFolderDialogEx::OnSize(UINT nType, int cx, int cy)
{
    inherited::OnSize(nType, cx, cy);

    // custom checkbox
    if(m_btnCheckbox.m_hWnd)
    {
        // get client rect
        CRect rectDialog;
        GetClientRect(&rectDialog);

        // get folder editbox rect
        CRect rectFolderEditbox;
        CWnd* pFolderEditbox = GetDlgItem(IDC_FOLDERDLG_EDIT_FOLDER);                           ASSERT(pFolderEditbox);
        if(pFolderEditbox)
        {
            pFolderEditbox->GetWindowRect(&rectFolderEditbox);
            ScreenToClient(&rectFolderEditbox);
        }

        // get checkbox rect
        CRect rectCheckbox;
        m_btnCheckbox.GetClientRect(&rectCheckbox);

        // position checkbox
        rectCheckbox.left   = rectFolderEditbox.left;
        rectCheckbox.right  = rectCheckbox.left + m_nCheckboxWidth;
        rectCheckbox.top    = rectFolderEditbox.bottom + 5;
        rectCheckbox.bottom = rectCheckbox.top + m_nCheckboxHeight;
        m_btnCheckbox.MoveWindow(&rectCheckbox, TRUE);
        m_btnCheckbox.Invalidate();
    }
}

//////////////////
// event: timer
//
void CFolderDialogEx::OnTimer(UINT_PTR nIDEvent)
{
    switch(nIDEvent)
    {
        case TIMER_SET_SELECTION_WORKAROUND:
            KillTimer(TIMER_SET_SELECTION_WORKAROUND);
            SetSelection(m_sInitialFolder);
            break;

        default:
            break;
    }
}

//////////////////
// event: custom checkbox clicked
//
void CFolderDialogEx::OnCustomCheckboxClicked()
{
    if(m_btnCheckbox.GetCheck() == BST_UNCHECKED)
    {
        m_btnCheckbox.SetCheck(BST_CHECKED);
        m_bCheckboxChecked = TRUE;
    }
    else // checked
    {
        m_btnCheckbox.SetCheck(BST_UNCHECKED);
        m_bCheckboxChecked = FALSE;
    }
}

//////////////////
// Folder browser window initialized: set OK button text and status panel
// text if not a new-style dialog. (Status text not supported for new-style
// folder dialog.)
//
void CFolderDialogEx::OnInitialized()
{
	BFTRACE(_T("CFolderDialogEx::OnInitialized\n"));

    // set caption (if specified)
    if(!m_sCaption.IsEmpty())
        SetWindowText(m_sCaption);

    // set OK button text (if specified)
    if(!m_sOkBtn.IsEmpty())
        SetOKText(m_sOkBtn);

    // set initial folder (if specified)
    if(!m_sInitialFolder.IsEmpty())
    {
        SetSelection(m_sInitialFolder);

        // TODO: Replace workaround for known Microsoft bug #518103 with final solution
        //  -> BFFM_SETSELECTION does not work with SHBrowseForFolder on Windows 7
        //     (https://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=518103)
        if(WinVersion.IsSevenOrLater())
        {
            SetTimer(TIMER_SET_SELECTION_WORKAROUND, 250, NULL);
        }
    }

    // remove context help button (if specified)
    if(m_bRemoveContextHelpButton)
    {
        DWORD dwExStyleNow = GetWindowExStyle(m_hWnd);
        dwExStyleNow &= ~WS_EX_CONTEXTHELP;
        SetWindowExStyle(m_hWnd, dwExStyleNow);
    }

    // display checkbox?
    if(!m_sCheckboxText.IsEmpty())
    {
        // calculate checkbox size
        CSize sizeCheckbox;
        CDC* pDC = GetDC();                                                                     ASSERT(pDC);
        if(pDC)
        {
            ::GetTextExtentPoint32(pDC->m_hDC, m_sCheckboxText, m_sCheckboxText.GetLength(), &sizeCheckbox);
            ReleaseDC(pDC);
        }

        m_nCheckboxWidth  = max(sizeCheckbox.cx + sizeCheckbox.cy, 50); // 'cy' is for the checkbox graphic
        m_nCheckboxHeight = max(sizeCheckbox.cy, 12);

        // move/resize common dialog controls to gain space for the checkbox.
        // the checkbox will be located below the folder editbox.
        CWnd* pWnd = NULL;
        CRect rectWnd;

        // folders tree container (if available)
        CWnd* pWndContainer = FindWindowEx(m_hWnd, NULL, _T("SHBrowseForFolder ShellNameSpace Control"), NULL);
        if(pWndContainer) // container found
        {
            pWndContainer->GetWindowRect(&rectWnd);
            ScreenToClient(&rectWnd);
            rectWnd.DeflateRect(0, 0, 0, m_nCheckboxHeight);
            pWndContainer->MoveWindow(&rectWnd, TRUE);
        }

        // folders tree
        pWnd = FindWindowEx((pWndContainer) ? pWndContainer->m_hWnd : m_hWnd, NULL, _T("SysTreeView32"), NULL); ASSERT(pWnd);
        if(pWnd)
        {
            pWnd->GetWindowRect(&rectWnd);
            pWnd->GetParent()->ScreenToClient(&rectWnd); // works with and without container

            if(pWndContainer)
            {
                // get client rect of container
                CRect rectContainer;
                pWndContainer->GetClientRect(&rectContainer);

                // calculate container border width and new bottom of folders tree
                int borderWidth = rectWnd.top; // offset from top border of client coords is the border width
                int newBottom = rectContainer.bottom - borderWidth;

                // reposition folders tree
                rectWnd.bottom = newBottom;
                pWnd->MoveWindow(&rectWnd, TRUE);
            }
            else // no container
            {
                rectWnd.DeflateRect(0, 0, 0, m_nCheckboxHeight);
                pWnd->MoveWindow(&rectWnd, TRUE);
            }
        }

        // folder static
        pWnd = GetDlgItem(IDC_FOLDERDLG_STATIC_FOLDER);                                         ASSERT(pWnd);
        if(pWnd)
        {
            pWnd->GetWindowRect(&rectWnd);
            ScreenToClient(&rectWnd);
            rectWnd.OffsetRect(0, -m_nCheckboxHeight);
            pWnd->MoveWindow(&rectWnd, TRUE);
        }

        // folder editbox
        pWnd = GetDlgItem(IDC_FOLDERDLG_EDIT_FOLDER);                                           ASSERT(pWnd);
        if(pWnd)
        {
            pWnd->GetWindowRect(&rectWnd);
            ScreenToClient(&rectWnd);
            rectWnd.OffsetRect(0, -m_nCheckboxHeight);
            pWnd->MoveWindow(&rectWnd, TRUE);
        }

        // get client rect
        CRect rectDialog;
        GetClientRect(&rectDialog);

        // get folder editbox rect
        CRect rectFolderEditbox;
        CWnd* pFolderEditbox = GetDlgItem(IDC_FOLDERDLG_EDIT_FOLDER);                           ASSERT(pFolderEditbox);
        if(pFolderEditbox)
        {
            pFolderEditbox->GetWindowRect(&rectFolderEditbox);
            ScreenToClient(&rectFolderEditbox);
        }

        // position checkbox
        CRect rectCheckbox;
        rectCheckbox.left   = rectFolderEditbox.left;
        rectCheckbox.right  = rectCheckbox.left + m_nCheckboxWidth;
        rectCheckbox.top    = rectFolderEditbox.bottom + 5;
        rectCheckbox.bottom = rectCheckbox.top + m_nCheckboxHeight;

        // checkbox styles
        static const UINT checkboxStyles = BS_CHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP;

        // create the checkbox
        if(!m_btnCheckbox.Create(m_sCheckboxText,
                                checkboxStyles,
                                rectCheckbox,
                                this,
                                IDC_FOLDERDLG_CUSTOM_CHECKBOX))
        {
            ASSERT(0);
            AfxThrowOleException(HRESULT_FROM_WIN32(::GetLastError()));
        }

        // set dialog font to checkbox
        CFont* pFont = GetFont();
        if(pFont)
        {
            m_btnCheckbox.SetFont(pFont, TRUE);
        }

        // init checkbox state
        m_btnCheckbox.SetCheck((m_bCheckboxChecked) ? BST_CHECKED : BST_UNCHECKED);
    }
}

//////////////////
// Selection changed: For new-style dialogs, disable OK button if object is
// not a real folder, ie., if its pathname is empty. Note that testing for the
// shell item attribute SFGAO_FILESYSTEM doesn't work here!
//
void CFolderDialogEx::OnSelChanged(LPCITEMIDLIST pidl)
{
	BFTRACE(_T("CFolderDialogEx::OnSelChanged: %s\n"),
		GetDisplayNameOf(m_shfRoot, pidl, SHGDN_FORPARSING));

    if (m_brinfo.ulFlags & BIF_NEWDIALOGSTYLE)
    {
		CString path = GetPathName(pidl);
		EnableOK(!path.IsEmpty());
	}
}

//////////////////
// User attempted to enter a name in the edit box that isn't a folder.
//
BOOL CFolderDialogEx::OnValidateFailed(LPCTSTR lpsz)
{
	BFTRACE(_T("CFolderDialogEx::OnValidateFailed: %s\n"), lpsz);

	return TRUE; // don't close dialog.
}

//////////////////
// Get flags for custom filtering: Only enumerate folders. This is only
// called when custom filtering is used.
//
HRESULT CFolderDialogEx::OnGetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder,
	DWORD *pgrfFlags)
{
	BFTRACE(_T("CFolderDialogEx::OnGetEnumFlags\n"));

    *pgrfFlags = SHCONTF_FOLDERS; // show only folders
	return S_OK;
}

//////////////////
// Determine whether to display item: doesn't actually do anything, just
// illustrates the function in action through TRACE diagnostics.
//
HRESULT CFolderDialogEx::OnShouldShow(IShellFolder* psf,
	LPCITEMIDLIST pidlFolder,
	LPCITEMIDLIST pidlItem)
{
	BFTRACE(_T("CFolderDialogEx::OnShouldShow(%p): %s: %s"),
		psf, GetPathName(pidlFolder), GetDisplayNameOf(psf, pidlItem, SHGDN_NORMAL));

    //SFGAOF attrs;
	//psf->GetAttributesOf(1, &pidlItem, &attrs);
	//BOOL bShow = attrs & (SFGAO_FILESYSTEM|SFGAO_HASSUBFOLDER);
	//BFTRACE(_T(" [attrs: %08x]: %s\n"), attrs, bShow ? _T("Yes") : _T("No"));
	//return bShow ? S_OK : S_FALSE;

	return S_OK;
}
