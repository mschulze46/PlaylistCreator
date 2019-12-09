/*----------------------------------------------------------------------------
| File:    AppSettings.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CAppSettings class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: AppSettings.h 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once


namespace PlaylistCreator
{
    ///// defines /////
    #define PRODUCT_ID                          _T("PC3")

    #define DEFAULT_ALTERNATE_ROW_COLOR         RGB(245,245,245)
    #define DEFAULT_MISSING_ITEMS_COLOR         RGB(235,135,135)

    
    ///// types /////

    enum PRODUCT_VERSION
    {
        PRODUCT_VERSION_MAJOR = 1,
        PRODUCT_VERSION_MINOR,
        PRODUCT_VERSION_FIX,
        PRODUCT_VERSION_BUILD,
    };

    enum NAMESTYLE
    {
        NAMESTYLE_FULL = 1,
        NAMESTYLE_SHORT,
        NAMESTYLE_TINY,
    };

    enum LANGUAGE
    {
        LANGUAGE_DE = 0,
        LANGUAGE_EN,
    };

    enum NEW_ENTRIES
    {
        NEW_ENTRIES_TOP = 0,
        NEW_ENTRIES_BEFORESELECTION,
        NEW_ENTRIES_AFTERSELECTION,
        NEW_ENTRIES_BOTTOM,
        NEW_ENTRIES_CUSTOM,
    };

    enum SAVE_MODE
    {
        SAVE_MODE_ABSOLUTE = 0,
        SAVE_MODE_ABSOLUTE_NODRIVE,
        SAVE_MODE_RELATIVE,
    };

    enum PROPERTY
    {
        // int properties
        PROP_FIRSTTIMEINITCOMPLETE = 1,
        PROP_SINGLEINSTANCE,
        PROP_PLAYSOUND_SUCCESS,
        PROP_TRANSPARENCY,
        PROP_WRITEEXTENDEDINFO,
        PROP_READTAGS,
        PROP_ADDNEWENTRIES,
        PROP_SORTNEWITEMS,
        PROP_SAVEMODE,
        PROP_SAVEMODE_USEFSLASHES,
        PROP_AUTOPLAYLISTNAME,
        PROP_AUTOPLAYLISTFOLDER,
        PROP_AUTOCLEAR,
        PROP_TOTALPLAYLISTS,
        PROP_TOTALSONGS,
        PROP_AUTOCLOSECREATEDLG,

        PROP_MAINDLG_WNDPOSX,
        PROP_MAINDLG_WNDPOSY,
        PROP_MAINDLG_WNDSIZEX,
        PROP_MAINDLG_WNDSIZEY,
        PROP_MAINDLG_WNDSHOWSTATE,

        PROP_SHOWGRIDLINES,
        PROP_SHOWALTERNATEROWCOLOR,
        PROP_ALTERNATEROWCOLOR,
        PROP_SNAPTOSCREENBORDER,
        PROP_FOLDERDLG_ADDRECURSIVE,
        PROP_SETTINGS_LASTACTIVEPAGE,
        PROP_HIGHLIGHTMISSINGITEMS,
        PROP_MISSINGITEMSCOLOR,
        PROP_APPLYFILTERONOPENPLAYLISTS,
        PROP_AUTOUPDATECHECK,

        // string properties
        PROP_LANGUAGE,
        PROP_LASTPLAYLISTFOLDER,
        PROP_LASTSONGFOLDER,
        PROP_ADDFILESFOLDER,
        PROP_ADDFILESFILTER,
        PROP_ADDPLAYLISTFOLDER,
        PROP_ADDPLAYLISTFILTER,
        PROP_TITLEMASK,
        PROP_PLAYLISTTYPE,
        PROP_OPENPLAYLISTFOLDER,
        PROP_OPENPLAYLISTFILTER,
    };

} // namespace PlaylistCreator


// CAppSettings

using namespace PlaylistCreator;

class CAppSettings
{
    ///// types /////


    ///// members /////
private:
    static const int m_VersionMajor     = 3;
    static const int m_VersionMinor     = 6;
    static const int m_VersionFix       = 2;
    static const int m_VersionBuild     = 0;

    static CMapStringToString m_mapStringTable;   // string table with the set language
    static LANGUAGE m_currentLanguage;            // current language of the string table


    ///// methods /////
public:
    CAppSettings();
    virtual ~CAppSettings();

    // returns the company name
    static CString GetCompanyName();

    // returns the URL of the oddgravity website
    static CString GetWebsiteUrl();

    // returns the name of the application
    static CString GetAppName(NAMESTYLE style = NAMESTYLE_FULL);

    // returns the registry key of the application
    static CString GetRegistryKey(NAMESTYLE style = NAMESTYLE_SHORT);

    // returns the version number as a string beginning from the major version
    // number to the defined parameter (e.g. PRODUCT_VERSION_FIX = 3.1.0)
    static CString GetVersionString(PRODUCT_VERSION version = PRODUCT_VERSION_FIX);

    // returns the smart version number as a string (e.g. 3.1)
    static CString GetSmartVersionString();

    // returns the specified version number as integer
    static int GetVersionInt(PRODUCT_VERSION version = PRODUCT_VERSION_MAJOR);

    // returns the URL for checking whether a program update is available
    static CString GetUpdateCheckUrl();

    // getter/setter for int properties
    static int GetIntProperty(PROPERTY eProperty, int iDefaultValue = 0);
    static BOOL SetIntProperty(PROPERTY eProperty, int iValue);

    // getter/setter for string properties
    static CString GetStringProperty(PROPERTY eProperty, const CString& strDefaultValue = _T(""));
    static BOOL SetStringProperty(PROPERTY eProperty, const CString& strValue);

    // gets all available languages
    static void GetSupportedLanguages(CStringArray& arrayLanguages);

    // gets/sets the language registry setting as string
    static CString  GetLanguageString();
    static BOOL     SetLanguageString(LPCTSTR strLanguageID = _T("EN"));

    // gets/sets the language registry setting as int (use enum!)
    static int      GetLanguageInt();
    static BOOL     SetLanguageInt(int iLanguage = LANGUAGE_EN);

    // gets/sets the current language as string
    static CString  GetCurrentLanguageString();
    static BOOL     SetCurrentLanguageString(LPCTSTR strLanguageID = _T("EN"));

    // gets/sets the current language as int (use enum!)
    static int      GetCurrentLanguageInt();
    static BOOL     SetCurrentLanguageInt(int iLanguage = LANGUAGE_EN);

    // gets/updates the timestamp of the last update check
    static time_t   GetLastUpdateCheckTimestamp();
    static BOOL     UpdateLastUpdateCheckTimestamp();

    // returns a string from the string table resource in the current language
    static CString LoadString(LPCTSTR lpszID, BOOL bReload = FALSE);

    // reloads the string table (e.g. when language was changed)
    static void ReloadStringTable();

    // gets the default parameters for the XMessageBox 3rd party component
    static void GetDefaultXMessageBoxParams(XMSGBOXPARAMS& params);

    // returns the credits in the current language (default: English)
    static CString GetCredits();

    // returns the font for standard dialogs
    static LOGFONT GetDialogFont();

    // returns the bold font for standard dialogs
    static LOGFONT GetDialogBoldFont();

    // returns the large font for special dialog elements
    static LOGFONT GetDialogLargeFont();

    // returns the large bold font for special dialog elements
    static LOGFONT GetDialogLargeBoldFont();

    // returns the size of a toolbar icon
    static int GetToolBarIconSize();

    // returns the size of a context menu icon
    static int GetContextMenuIconSize();

    // inits all registry entries
    static void InitRegistryEntries();

    // deletes all registry entries
    static void DeleteRegistryEntries();

    // resets all registry entries
    static void ResetRegistryEntries();

    // resets active and inactive file types
    static void ResetFileTypes();

    // shows all hidden messages again
    static void ShowHiddenMessages();

    // returns the standard window title
    static CString GetWindowTitle();

    // checks if the user is a nice guy who gave us money
    static BOOL GetIsSupporter();

    // sets the supporter flag
    static void SetIsSupporter(BOOL bSupporter);

    // gets the styles for adding new playlist entries
    static void GetAddNewEntries(CStringArray& arrayNewEntries);

    // gets the styles for saving playlist entries
    static void GetSaveModes(CStringArray& arraySaveModes);

    // gets the default title mask
    static CString GetDefaultTitleMask();

    // checks if the given 'dont show dialog' id is set
    static BOOL GetDontShowDialog(const CString& strID);

    // sets an id for a 'dont show dialog'
    static void SetDontShowDialog(const CString& strID);

    // opens the help
    static void OnHelp();

    // returns the URL to the company's donations site
    static CString GetCompanyDonationsUrl();

    // returns the URL to the PayPal donations site
    static CString GetPayPalDonationsUrl();

protected:
    // inits the default language
    static void InitRegistryEntries_Language();

    // inits the supported file types
    static void InitRegistryEntries_FileTypes();

    // gets all default active file types
    static void GetDefaultActiveFileTypes(CStringArray& arrayActiveFileTypes);

    // gets all default inactive file types
    static void GetDefaultInactiveFileTypes(CStringArray& arrayInactiveFileTypes);

    // gets the user defined button captions for the XMessageBox 3rd party component
    static void GetXMessageBoxUserDefinedButtonCaptions(XMSGBOXPARAMS::CUserDefinedButtonCaptions& captions);
};
