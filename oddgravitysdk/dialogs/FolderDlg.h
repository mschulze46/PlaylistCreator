////////////////////////////////////////////////////////////////
// MSDN Magazine -- June 2005
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio .NET 2003 (V7.1) on Windows XP. Tab size=3.
//
#pragma once
#include "debug.h" // debugging tools

//////////////////
// BRTRACEFN is like TRACEFN but only does anything if
// CFolderDialog::bTRACE is on. See Debug.h.
//
#ifdef _DEBUG
#define BFTRACE															\
	if (CFolderDialog::bTRACE)										\
		TRACE
#else
#define BFTRACE
#endif

//////////////////
// Class to encapsulate SHBrowseForFolder. To use, instantiate in your app
// and call BrowseForFolder, which returns a PIDL. You can call GetPathName
// to get the path name from the PIDL. For example:
//
//		CFolderDialog dlg(this);
//		LPCITEMIDLIST pidl = dlg.BrowseForFolder(...);
//		CString path = dlg.GetPathName(pidl);
//
//	You can also derive your own class from CFolderDialog to override virtual
//	message handler functions like OnInitialized and OnSelChanged to do stuff
//	when various things happen. This replaces the callback mechanism for
//	SHBrowseForFolder. You call various wrapper functions from your hanlers to
//	send messages to the browser window. For example:
//
//		int CMyFolderDialog::OnInitialized()
//		{
//			CFolderDialog::OnInitialized();
//			SetStatusText(_T("Nice day, isn't it?"));
//			SetOKText(L"Choose Me!");
//			return 0;
//		}
//
// You can set CFolderDialog::bTRACE=TRUE to turn on debugging TRACE
// diagnostics to help you understand what's going on.
//
class CFolderDialog : public CWnd {
public:
	static BOOL bTRACE;		// controls tracing

	CFolderDialog(CWnd* pWnd);
	~CFolderDialog();

	LPCITEMIDLIST BrowseForFolder(LPCTSTR title, UINT flags,
		LPCITEMIDLIST pidRoot = NULL, BOOL bFilter = FALSE);

	CString GetDisplayName() { return m_sDisplayName; }

	// helpers
	static CString GetPathName(LPCITEMIDLIST pidl);
	static CString GetDisplayNameOf(IShellFolder* psf, LPCITEMIDLIST pidl, DWORD uFlags);
	static void FreePIDL(LPCITEMIDLIST pidl);

protected:
	BROWSEINFO m_brinfo;						 // internal structure for SHBrowseForFolder
	CString m_sDisplayName;					 // display name of folder chosen
	BOOL m_bFilter;							 // do custom filtering?
	CComQIPtr<IShellFolder> m_shfRoot;	 // handy to have root folder

	static int CALLBACK CallbackProc(HWND hwnd, UINT msg, LPARAM lp, LPARAM lpData);

	virtual int OnMessage(UINT msg, LPARAM lp);	// internal catch-all

	// Virtual message handlers: override these instead of using callback
	virtual void OnInitialized();
	virtual void OnIUnknown(IUnknown* punk);
	virtual void OnSelChanged(LPCITEMIDLIST pidl);
	virtual BOOL OnValidateFailed(LPCTSTR lpsz);

	// Wrapper functions for folder dialog messages--call these only from
	// virtual handler functions above!

	// Enable or disable the OK button
	void EnableOK(BOOL bEnable) {
		SendMessage(BFFM_ENABLEOK,0,bEnable);
	}

	// The Microsoft documentation is wrong for this: text in LPARAM, not WPARAM!
	void SetOKText(LPCWSTR lpText) {
		SendMessage(BFFM_SETOKTEXT,0,(LPARAM)lpText);
	}

	// Set selected item from string or PIDL.
	// The documentation says lpText must be Unicode, but it can be LPCTSTR.
	void SetSelection(LPCTSTR lpText) {
		SendMessage(BFFM_SETSELECTION,TRUE,(LPARAM)lpText);
	}
	void SetSelection(LPCITEMIDLIST pidl) {
		SendMessage(BFFM_SETSELECTION,FALSE,(LPARAM)pidl);
	}

	// Expand item from string or PIDL
	void SetExpanded(LPCWSTR lpText) {
		SendMessage(BFFM_SETEXPANDED,TRUE,(LPARAM)lpText);
	}
	void SetExpanded(LPCITEMIDLIST pidl) {
		SendMessage(BFFM_SETEXPANDED,FALSE,(LPARAM)pidl);
	}

	// Set status window text
	void SetStatusText(LPCTSTR pText) {
		SendMessage(BFFM_SETSTATUSTEXT,0,(LPARAM)pText);
	}

	// Override for custom filtering. You must call BrowseForFolder with bFilter=TRUE.
	virtual HRESULT OnGetEnumFlags(IShellFolder* psf,
		LPCITEMIDLIST pidlFolder,
		DWORD *pgrfFlags);
	virtual HRESULT OnShouldShow(IShellFolder* psf,
		LPCITEMIDLIST pidlFolder,
		LPCITEMIDLIST pidlItem);

   // COM interfaces. The only one currently is IFolderFilter
	DECLARE_INTERFACE_MAP()

	// IUnknown--all nested interfaces call these
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj);

	// COM interface IFolderFilter, used to do custom filtering
	BEGIN_INTERFACE_PART(FolderFilter, IFolderFilter)
   STDMETHOD(GetEnumFlags) (IShellFolder* psf,
		LPCITEMIDLIST pidlFolder,
		HWND *pHwnd,
		DWORD *pgrfFlags);
   STDMETHOD(ShouldShow) (IShellFolder* psf,
		LPCITEMIDLIST pidlFolder,
		LPCITEMIDLIST pidlItem);
	END_INTERFACE_PART(FolderFilter)

	DECLARE_DYNAMIC(CFolderDialog)
};


//////////////////
// Class derived from CFolderDialog with extended features.
//
class CFolderDialogEx : public CFolderDialog
{
    typedef CFolderDialog inherited;

public:
	CFolderDialogEx(CWnd* pWnd);
	~CFolderDialogEx();

	LPCITEMIDLIST BrowseForFolder(LPCTSTR caption, LPCTSTR title, LPCTSTR initialFolder,
        UINT flags, LPCITEMIDLIST pidRoot = NULL, BOOL bFilter = FALSE);

    void SetCaption(LPCTSTR caption);
    void SetOkBtnText(LPCTSTR okBtn);

    void ShowCheckbox(LPCTSTR checkbox, BOOL bChecked);
    BOOL IsCheckboxChecked();

    void RemoveContextHelpButton();

protected:
    DECLARE_MESSAGE_MAP()

    CString m_sCaption;                 // caption text
    CStringW m_sOkBtn;                  // OK button text
    CString m_sInitialFolder;           // folder to be initially selected

    BOOL m_bRemoveContextHelpButton;    // defines whether context help button should be removed

    // checkbox
    CButton     m_btnCheckbox;
    CString     m_sCheckboxText;
    int         m_nCheckboxWidth;
    int         m_nCheckboxHeight;
    BOOL        m_bCheckboxChecked;

    // message and event handlers
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnCustomCheckboxClicked();

	// virtual overrides
	virtual void OnInitialized();
	virtual void OnSelChanged(LPCITEMIDLIST pidl);
	virtual BOOL OnValidateFailed(LPCTSTR lpsz);
	virtual HRESULT OnGetEnumFlags(IShellFolder* psf, LPCITEMIDLIST pidlFolder, DWORD* pgrfFlags);
	virtual HRESULT OnShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem);

	DECLARE_DYNAMIC(CFolderDialogEx);
};
